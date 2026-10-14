#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-LOGIC.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 8 (13.10.): line break of "Frames per second", "BMS SILENT" line. Bau 9: minute factor of the vehicle age and
// the daily sleep (time of day, duration, kilometres).

namespace {

using Bat = RenaultTwingoGen1Battery;

class Bau89Twingo : public RenaultTwingoGen1Battery {
 public:
  bool clock_set = false;
  uint32_t clock_secs = 0;
  time_t unix_now = 1791590400;
  Bau89Twingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.age_minute_factor = 1;
    t.daily_sleep_time_min = 0;
    t.daily_sleep_duration_min = 5;  // the default of the automatic sleep
    t.daily_sleep_km = 0;
    t.daily_sleep_enabled = false;
    t.bus_state_manual = true;
    t.bus_state = twingo_bus::BUS_WACH;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.bus_format_zoe_old = false;
    t.simulator_enabled_mask = 1ULL << Bat::SIM_ROW_436;  // the age goes out in 0x436 (bytes 1-3)
    t.simulator_enabled_mask_hi = 0;
  }
  ~Bau89Twingo() {
    datalayer_extended.twingoGen1.age_minute_factor = 1;
    datalayer_extended.twingoGen1.daily_sleep_enabled = false;
    datalayer_extended.twingoGen1.daily_sleep_duration_min = 5;
    time_436_clear();
    Bat::age_manual_clear();
  }
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t& now_utc) override {
    now_utc = unix_now;
    return true;
  }
  bool get_wall_clock_seconds_of_day(uint32_t& secs) override {
    if (!clock_set) {
      return false;
    }
    secs = clock_secs;
    return true;
  }
};

struct Tx {
  uint64_t t;
  CAN_frame f;
};

void tick(RenaultTwingoGen1Battery& b, uint64_t t, std::vector<Tx>* log = nullptr) {
  set_millis64(t);
  clear_transmitted_frames();
  b.transmit_can((unsigned long)t);
  if (log) {
    for (const CAN_frame& f : get_transmitted_frames()) {
      log->push_back({t, f});
    }
  }
}

size_t count_of(const std::string& s, const std::string& what, size_t from, size_t to) {
  size_t n = 0;
  for (size_t p = s.find(what, from); p != std::string::npos && p < to; p = s.find(what, p + 1)) {
    n++;
  }
  return n;
}

// The age that goes out in the 0x436 frame (bytes 1-3) of one transmit_can() call at time t (0xFFFFFFFF = no frame).
uint32_t age_at(RenaultTwingoGen1Battery& b, uint64_t t) {
  set_millis64(t);
  clear_transmitted_frames();
  b.transmit_can((unsigned long)t);
  for (const CAN_frame& f : get_transmitted_frames()) {
    if (!f.ext_ID && f.ID == 0x436) {
      return ((uint32_t)f.data.u8[1] << 16) | (f.data.u8[2] << 8) | f.data.u8[3];
    }
  }
  return 0xFFFFFFFFu;
}

CAN_frame frame_424() {
  CAN_frame f = {};
  f.ID = 0x424;
  f.DLC = 8;
  f.ext_ID = false;
  f.data.u8[0] = 0x11;
  f.data.u8[1] = 0x40;
  f.data.u8[2] = 0x56;
  f.data.u8[3] = 0x90;
  f.data.u8[4] = 0x39;
  f.data.u8[5] = 0x64;
  f.data.u8[6] = 0x55;
  f.data.u8[7] = 0x39;
  return f;
}

}  // namespace

// ---------------------------------------------------------------------------
// Bau 8
// ---------------------------------------------------------------------------

TEST(TwingoBau8, SilencePageBreaksTheSecondsLineAfterTenValuesAndNamesTheRealLastFrame) {
  Bau89Twingo b;
  b.setup();
  b.request_sleep();
  uint64_t t = 1000;
  // the BMS sends during the first 100 s of the shutdown sequence, then it is silent (as in the bench run)
  for (; t < 1000 + 100000; t += 100) {
    set_millis64(t);
    b.handle_incoming_can_frame(frame_424());
    tick(b, t);
  }
  for (; t < 1000 + 215000; t += 100) {
    tick(b, t);  // the shutdown sequence (about 188 s) is over, the quiet phase has run for a while
  }
  const std::string page = b.get_uds_info_html().c_str();
  const size_t from = page.find("Frames per second (first 60 s): ");
  ASSERT_NE(from, std::string::npos);
  const size_t to = page.find("Frames per 10 s", from);
  ASSERT_NE(to, std::string::npos);
  // 60 values -> a <br> after every group of ten: five inside, one at the end of the line
  EXPECT_EQ(count_of(page, "<br>", from, to), 6u);
  // the BMS stopped during the shutdown sequence, i.e. BEFORE the quiet phase: the line says so (no overflow number)
  EXPECT_NE(page.find("BMS SILENT: last frame "), std::string::npos);
  EXPECT_NE(page.find(" s before the quiet phase began"), std::string::npos);
  EXPECT_EQ(page.find("BMS SILENT since"), std::string::npos);
  // the age of the last frame is the real one (about 115 s), not the run time of the ESP
  const size_t p = page.find(", last one ");
  ASSERT_NE(p, std::string::npos);
  const double age_s = std::atof(page.c_str() + p + strlen(", last one "));
  EXPECT_GT(age_s, 100.0);
  EXPECT_LT(age_s, 140.0);
}

// ---------------------------------------------------------------------------
// Bau 9: minute factor
// ---------------------------------------------------------------------------

TEST(TwingoBau9, AgeStepsOncePerFactorMinutes) {
  const int64_t t0 = 1791590400;
  for (uint32_t factor : {1u, 3u, 10u}) {
    for (int minutes : {0, 1, 2, 3, 9, 10, 29, 30, 61}) {
      const uint32_t want = 1000 + (uint32_t)minutes / factor;
      EXPECT_EQ(twingo::age_plain(1000, t0, t0 + minutes * 60 + 59, factor), want) << factor << "/" << minutes;
    }
  }
  EXPECT_EQ(twingo::age_plain(1000, t0, t0 + 25 * 60, 0), 1025u);  // 0 counts as 1
  EXPECT_EQ(twingo::age_auto(1000, t0, t0 + 25 * 60, 10), 1002u);
  EXPECT_EQ(twingo::age_plain(1000, t0, t0 - 60, 10), 1000u);  // a clock step back never lowers the age
}

TEST(TwingoBau9, TheFramesCarryTheSteppedAge) {
  Bau89Twingo b;
  b.setup();
  datalayer_extended.twingoGen1.age_minute_factor = 10;
  b.unix_now = 1791590400 + 25 * 60;  // 25 minutes after the reference: 2 steps of 10 minutes
  std::vector<Tx> log;
  for (uint64_t t = 1000; t < 1500; t += 100) {
    tick(b, t, &log);
  }
  bool seen = false;
  for (const Tx& x : log) {
    if (x.f.ID == 0x436 && !x.f.ext_ID) {
      const uint32_t age = ((uint32_t)x.f.data.u8[1] << 16) | (x.f.data.u8[2] << 8) | x.f.data.u8[3];
      EXPECT_EQ(age, 1025301u + 2u);
      seen = true;
    }
  }
  EXPECT_TRUE(seen);
}

TEST(TwingoBau9, ANewFactorKeepsTheAgeThatWasSent) {
  Bau89Twingo b;
  b.setup();
  set_millis64(1000);
  Bat::age_manual_set(5000, 0);  // counted by the run time
  EXPECT_EQ(age_at(b, 1000 + 30UL * 60000UL), 5030u);
  set_millis64(1000 + 30UL * 60000UL);
  Bat::age_set_factor(10);
  EXPECT_EQ(datalayer_extended.twingoGen1.age_minute_factor, 10u);
  const uint64_t t1 = 1000 + 30UL * 60000UL;
  EXPECT_EQ(age_at(b, t1 + 1000), 5030u) << "the age is not lowered by the new factor";
  EXPECT_EQ(age_at(b, t1 + 9UL * 60000UL), 5030u);
  EXPECT_EQ(age_at(b, t1 + 10UL * 60000UL), 5031u);
  EXPECT_EQ(age_at(b, t1 + 20UL * 60000UL), 5032u);
  set_millis64(t1 + 20UL * 60000UL);
  Bat::age_set_factor(0);  // 0 is taken as 1
  EXPECT_EQ(datalayer_extended.twingoGen1.age_minute_factor, 1u);
  EXPECT_EQ(age_at(b, t1 + 20UL * 60000UL + 1000), 5032u);
  EXPECT_EQ(age_at(b, t1 + 21UL * 60000UL), 5033u);
}

// ---------------------------------------------------------------------------
// Bau 9: daily sleep
// ---------------------------------------------------------------------------

TEST(TwingoBau9, DailySleepIsDueInTheConfiguredMinuteOnly) {
  const uint32_t at3 = 3 * 3600;
  EXPECT_TRUE(twingo::daily_sleep_due(true, 180, 5, at3));
  EXPECT_TRUE(twingo::daily_sleep_due(true, 180, 5, at3 + 59));
  EXPECT_FALSE(twingo::daily_sleep_due(true, 180, 5, at3 - 1));
  EXPECT_FALSE(twingo::daily_sleep_due(true, 180, 5, at3 + 60));
  EXPECT_FALSE(twingo::daily_sleep_due(false, 180, 5, at3)) << "checkbox off";
  EXPECT_FALSE(twingo::daily_sleep_due(true, 180, 0, at3)) << "a duration of 0 would wake up at once";
  EXPECT_TRUE(twingo::daily_sleep_due(true, 0, 1, 0)) << "00:00";
  EXPECT_FALSE(twingo::daily_sleep_due(true, 1440, 1, 0));
}

TEST(TwingoBau9, DailySleepRaisesTheKilometresStartsTheSleepAndLastsTheConfiguredDuration) {
  Bau89Twingo b;
  b.setup();
  auto& t = datalayer_extended.twingoGen1;
  t.daily_sleep_time_min = 3 * 60;
  t.daily_sleep_duration_min = 2;
  t.daily_sleep_km = 5;
  t.daily_sleep_enabled = true;
  t.bus_sleep_path = twingo_bus::SLEEP_LADELOG2;  // the user's choice, the daily sleep takes the long way for its run
  t.bus_state_manual = false;                     // the state of the bus follows the run: the wake-up shows in 0x423
  t.simulator_enabled_mask = (1ULL << Bat::SIM_ROW_423) | (1ULL << Bat::SIM_ROW_436);
  const uint32_t save_km = Bat::odo_426_km;
  Bat::odo_426_km = 6844;
  b.clock_set = true;
  b.clock_secs = 2 * 3600 + 59 * 60 + 50;  // 02:59:50: not yet
  std::vector<Tx> log;
  uint64_t now = 1000;
  for (; now < 1000 + 5000; now += 100) {
    tick(b, now, &log);
  }
  EXPECT_EQ(Bat::odo_426_km, 6844u);
  EXPECT_EQ(t.bus_sleep_path, twingo_bus::SLEEP_LADELOG2);

  b.clock_secs = 3 * 3600 + 10;  // 03:00:10, then the clock stays in this minute: it must fire only once
  const uint64_t t_fire = now;
  log.clear();
  for (; now < t_fire + 120000; now += 100) {
    tick(b, now, &log);
  }
  EXPECT_EQ(Bat::odo_426_km, 6849u) << "raised once by 5";
  EXPECT_EQ(t.bus_sleep_path, twingo_bus::SLEEP_LANG) << "the long way during the run";
  bool wrote_9281 = false;
  for (const Tx& x : log) {
    if (x.f.ext_ID && x.f.data.u8[1] == 0x2E && x.f.data.u8[2] == 0x92 && x.f.data.u8[3] == 0x81) {
      wrote_9281 = true;
    }
  }
  EXPECT_TRUE(wrote_9281) << "Sleep 0x9281=1: the temporisation is written";

  // through the shutdown sequence (about 188 s) and the silence of 2 minutes: the wake burst comes after about 5 min,
  // not after the failsafe window of 30 minutes
  uint64_t t_wake = 0;
  log.clear();
  for (; now < t_fire + 420000; now += 100) {
    tick(b, now, &log);
  }
  for (const Tx& x : log) {
    if (!x.f.ext_ID && x.f.ID == 0x423 && x.f.data.u8[0] == 0x30 && x.t > t_fire + 250000) {
      t_wake = x.t;
      break;
    }
  }
  ASSERT_GT(t_wake, 0u) << "no wake-up (0x423 in the wake form) after the daily sleep";
  EXPECT_GT(t_wake, t_fire + 295000);  // 188 s + 120 s of silence, minus the start-up of the run
  EXPECT_LT(t_wake, t_fire + 330000);
  EXPECT_EQ(t.bus_sleep_path, twingo_bus::SLEEP_LADELOG2) << "the chosen way is back after the run";
  EXPECT_EQ(Bat::odo_426_km, 6849u) << "no second run in the same minute";
  Bat::odo_426_km = save_km;
}

TEST(TwingoBau9, DailySleepDoesNothingWithTheCheckboxOffOrWithoutAClock) {
  for (int variant = 0; variant < 2; variant++) {
    Bau89Twingo b;
    b.setup();
    auto& t = datalayer_extended.twingoGen1;
    t.daily_sleep_time_min = 3 * 60;
    t.daily_sleep_duration_min = 2;
    t.daily_sleep_km = 5;
    t.daily_sleep_enabled = (variant == 1);  // variant 0: checkbox off
    const uint32_t save_km = Bat::odo_426_km;
    Bat::odo_426_km = 6844;
    b.clock_set = (variant == 0);  // variant 1: no clock set (checkbox on)
    b.clock_secs = 3 * 3600 + 10;
    for (uint64_t now = 1000; now < 1000 + 5000; now += 100) {
      tick(b, now);
    }
    EXPECT_EQ(Bat::odo_426_km, 6844u) << variant;
    Bat::odo_426_km = save_km;
  }
}

TEST(TwingoBau9, AdvancedPageHasTheFieldsUnderTheSleepMinutes) {
  Bau89Twingo b;
  b.setup();
  auto& t = datalayer_extended.twingoGen1;
  t.daily_sleep_time_min = 3 * 60 + 5;
  t.daily_sleep_duration_min = 7;
  t.daily_sleep_km = 12;
  t.daily_sleep_enabled = true;
  t.age_minute_factor = 3;
  const std::string page = b.get_uds_info_html().c_str();
  const size_t failsafe = page.find("Sleep failsafe manual (auto wake after)");
  ASSERT_NE(failsafe, std::string::npos);
  const size_t head = page.find("<h4>Daily sleep</h4>", failsafe);
  ASSERT_NE(head, std::string::npos) << "the heading comes after the sleep minutes";
  const size_t en_box = page.find("<input type='checkbox' id='twingoDsEn' checked>", head);
  ASSERT_NE(en_box, std::string::npos) << "the checkbox enable stands in the row of the daily sleep";
  const size_t row = page.find("id='twingoDsTime' value='03:05'", en_box);
  ASSERT_NE(row, std::string::npos);
  // time, duration and kilometres stand in ONE line (one <h4>), with one Set button
  const size_t end_of_row = page.find("</h4>", row);
  const std::string line = page.substr(row, end_of_row - row);
  EXPECT_NE(line.find("id='twingoDsDur' min='1' max='1440' value='7'"), std::string::npos);
  EXPECT_NE(line.find("id='twingoDsKm' min='0' max='65535' value='12'"), std::string::npos);
  EXPECT_NE(line.find("twingoSetDailySleep()"), std::string::npos);
  EXPECT_NE(page.find("id='twingoAgeFactor' min='1' max='1440' value='3'", end_of_row), std::string::npos);
  EXPECT_NE(page.find("/editTwingoDailySleep?time="), std::string::npos);
  EXPECT_NE(page.find("/editTwingoAgeFactor?value="), std::string::npos);
}

TEST(TwingoBau10, CheckboxIsNotCheckedAndDurationIsFiveMinutesByDefault) {
  Bau89Twingo b;  // the fixture sets the defaults of the datalayer
  b.setup();
  EXPECT_FALSE(datalayer_extended.twingoGen1.daily_sleep_enabled);
  EXPECT_EQ(datalayer_extended.twingoGen1.daily_sleep_duration_min, 5u);
  const std::string page = b.get_uds_info_html().c_str();
  EXPECT_NE(page.find("<input type='checkbox' id='twingoDsEn' >"), std::string::npos);
  EXPECT_NE(page.find("id='twingoDsDur' min='1' max='1440' value='5'"), std::string::npos);
  EXPECT_NE(page.find("&en='+e"), std::string::npos) << "the Set button sends the checkbox";
}

TEST(TwingoBau10, SleepRunDetailsAreOpenWhileTheRunGoesAndClosedAfterwards) {
  Bau89Twingo b;
  b.setup();
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 1;
  {
    const std::string before = b.get_uds_info_html().c_str();
    EXPECT_EQ(before.find("twingoQuiet"), std::string::npos) << "nothing to fold before a run";
    EXPECT_NE(before.find("not run yet"), std::string::npos);
  }
  b.request_sleep();
  uint64_t t = 1000;
  for (; t < 1000 + 20000; t += 100) {
    tick(b, t);
  }
  std::string page = b.get_uds_info_html().c_str();
  EXPECT_NE(page.find("<details id='twingoQuiet' open><summary><b>Sleep run details</b></summary>"), std::string::npos);
  // the shutdown sequence (about 188 s), 1 minute of silence, the wake-up burst
  for (; t < 1000 + 330000; t += 100) {
    tick(b, t);
  }
  page = b.get_uds_info_html().c_str();
  EXPECT_NE(page.find("<details id='twingoQuiet'><summary><b>Sleep run details</b></summary>"), std::string::npos)
      << "closed after the run";
  EXPECT_NE(page.find("Frames per second (first 60 s)"), std::string::npos) << "the result is still in the page";
  EXPECT_EQ(page.find("twingoQuiet' open"), std::string::npos);
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 30;
}

TEST(TwingoBau11, TimeIn0x436CountsByTheMinuteFactor) {
  for (uint16_t factor : {1, 3}) {
    Bau89Twingo b;
    b.setup();
    datalayer_extended.twingoGen1.age_minute_factor = factor;
    b.time_436_set(1000);
    for (uint64_t t = 1000; t < 1000 + 3 * 60000 + 5000; t += 1000) {
      tick(b, t);
    }
    EXPECT_EQ(Bat::time_436_value, factor == 1 ? 1003u : 1001u) << factor;
    for (uint64_t t = 1000 + 3 * 60000 + 5000; t < 1000 + 6 * 60000 + 5000; t += 1000) {
      tick(b, t);
    }
    EXPECT_EQ(Bat::time_436_value, factor == 1 ? 1006u : 1002u) << factor;
  }
}
