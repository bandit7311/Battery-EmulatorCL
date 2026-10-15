#include <gtest/gtest.h>
#include <cstring>
#include <map>
#include <set>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 24 (15.10.): a click on the Betriebsart (driving / charging) starts the wake-up sequence of that target again and
// it stays; in the target "charging" the wake-up forms of the car (charge log "Ladelog 2") and all EVC frames are sent; the
// awake time and byte 0 of 0x423 at the end of the charge follow the log.

namespace {
using Bat = RenaultTwingoGen1Battery;

class LadenTwingo : public RenaultTwingoGen1Battery {
 public:
  LadenTwingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.cell_source_broadcast = false;
    t.age_minute_factor = 1;
    t.bus_format_zoe_old = false;
    t.bus_state_manual = false;
    t.bus_target_restart = false;
    t.bus_wake_target = twingo_bus::TARGET_FAHREN;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.sleep_failsafe_minutes = 30;
    t.daily_sleep_enabled = false;
    t.sleep_button_enabled = false;
    t.simulator_enabled_mask = SIM_DEFAULT_MASK;  // the twelve frames of the default
    t.simulator_enabled_mask_hi = 0;
    Bat::shutdown_like_car = false;
  }
  ~LadenTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.bus_wake_target = twingo_bus::TARGET_FAHREN;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.bus_state_manual = false;
    t.bus_target_restart = false;
    t.simulator_enabled_mask = 0x10;
    Bat::shutdown_like_car = false;
  }
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t& now_utc) override {
    now_utc = 1791590400;
    return true;
  }
  bool get_wall_clock_seconds_of_day(uint32_t& secs) override {
    secs = 12 * 3600;
    return true;
  }
};

struct Rec {
  uint64_t t;
  CAN_frame f;
};

// Runs the driver in steps of 10 ms and records every frame sent.
struct Runner {
  LadenTwingo& b;
  uint64_t now = 1000;
  std::vector<Rec> rec;
  explicit Runner(LadenTwingo& bat) : b(bat) {}
  void run(uint64_t ms) {
    for (uint64_t end = now + ms; now < end; now += 10) {
      set_millis64(now);
      clear_transmitted_frames();
      b.transmit_can((unsigned long)now);
      for (const CAN_frame& f : get_transmitted_frames()) {
        rec.push_back({now, f});
      }
    }
  }
  std::vector<Rec> of(uint32_t id, uint64_t from = 0, uint64_t to = ~0ULL) const {
    std::vector<Rec> out;
    for (const Rec& r : rec) {
      if (r.f.ID == id && r.t >= from && r.t < to) {
        out.push_back(r);
      }
    }
    return out;
  }
  size_t count(uint32_t id) const { return of(id).size(); }
};

bool bytes_eq(const CAN_frame& f, std::initializer_list<uint8_t> v) {
  size_t i = 0;
  for (uint8_t b : v) {
    if (f.data.u8[i++] != b) {
      return false;
    }
  }
  return true;
}

// the state of the bus the way the page shows it
uint8_t shown() {
  return Bat::bus_state_shown;
}
}  // namespace

// ---------------------------------------------------------------------------
// Click on the Betriebsart: the sequence starts again and stays
// ---------------------------------------------------------------------------

TEST(TwingoBau24Restart, FromDrivingToChargingRunsTheWholeWakeUpAndStaysCharging) {
  LadenTwingo b;
  b.setup();
  Runner r(b);
  r.run(30000);  // the sequence of driving has run, the bus is in the state "driving"
  ASSERT_EQ(shown(), twingo_bus::BUS_FAHRT);
  auto& t = datalayer_extended.twingoGen1;
  // what the web server does at the click on "Laden"
  t.bus_wake_target = twingo_bus::TARGET_LADEN;
  t.bus_target_restart = true;
  const uint64_t t_click = r.now;
  r.run(500);
  EXPECT_EQ(shown(), twingo_bus::BUS_ZU) << "the bus starts from 'closed' again, no jump to charging";
  r.run(7500);  // 8.0 s after the click: the step "A0/41" (from 7.9 s)
  EXPECT_EQ(shown(), twingo_bus::BUS_LADEN);
  // 0x426 goes through the steps of the wake-up through the plug: 00/02, 08/02, 00/02, 00/03, 60/63, 60/61, A0/41
  std::vector<std::pair<uint8_t, uint8_t>> seen;
  for (const Rec& rec : r.of(0x426, t_click)) {
    const std::pair<uint8_t, uint8_t> p(rec.f.data.u8[1], rec.f.data.u8[2]);
    if (seen.empty() || seen.back() != p) {
      seen.push_back(p);
    }
  }
  ASSERT_GE(seen.size(), 7u);
  EXPECT_EQ(seen[0], std::make_pair((uint8_t)0x00, (uint8_t)0x02));
  EXPECT_EQ(seen[1], std::make_pair((uint8_t)0x08, (uint8_t)0x02));
  EXPECT_EQ(seen[2], std::make_pair((uint8_t)0x00, (uint8_t)0x02));
  EXPECT_EQ(seen[3], std::make_pair((uint8_t)0x00, (uint8_t)0x03));
  EXPECT_EQ(seen[4], std::make_pair((uint8_t)0x60, (uint8_t)0x63));
  EXPECT_EQ(seen[5], std::make_pair((uint8_t)0x60, (uint8_t)0x61));
  EXPECT_EQ(seen[6], std::make_pair((uint8_t)0xA0, (uint8_t)0x41));
  r.run(20000);
  EXPECT_EQ(shown(), twingo_bus::BUS_LADEN) << "and it stays";
  const std::vector<Rec> last = r.of(0x426, r.now - 2000);
  ASSERT_FALSE(last.empty());
  EXPECT_TRUE(bytes_eq(last.back().f, {0x00, 0xA0, 0x41}));
  EXPECT_EQ(last.back().f.data.u8[7], 0x80) << "byte 7 = 80 all the time while it charges";
}

TEST(TwingoBau24Restart, BackToDrivingRunsTheDrivingWakeUpAndStaysDriving) {
  LadenTwingo b;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  b.setup();
  Runner r(b);
  r.run(30000);
  ASSERT_EQ(shown(), twingo_bus::BUS_LADEN);
  auto& t = datalayer_extended.twingoGen1;
  t.bus_wake_target = twingo_bus::TARGET_FAHREN;
  t.bus_target_restart = true;
  r.run(500);
  EXPECT_EQ(shown(), twingo_bus::BUS_ZU);
  r.run(40000);  // the wake-up of driving takes about 20 s
  EXPECT_EQ(shown(), twingo_bus::BUS_FAHRT);
}

TEST(TwingoBau24Restart, SleepingDoesNotRestartAndWithoutAClickNothingChanges) {
  LadenTwingo b;
  b.setup();
  Runner r(b);
  r.run(30000);
  ASSERT_EQ(shown(), twingo_bus::BUS_FAHRT);
  auto& t = datalayer_extended.twingoGen1;
  t.bus_wake_target = twingo_bus::TARGET_SCHLAFEN;  // the web server sets no restart for "sleeping"
  r.run(2000);
  EXPECT_EQ(shown(), twingo_bus::BUS_FAHRT) << "the bus stays as it is";
}

TEST(TwingoBau24Restart, NotInManualAndNotDuringASleepRun) {
  {
    LadenTwingo b;
    b.setup();
    Runner r(b);
    r.run(30000);
    auto& t = datalayer_extended.twingoGen1;
    t.bus_state_manual = true;
    t.bus_state = twingo_bus::BUS_WACH;
    r.run(200);
    t.bus_wake_target = twingo_bus::TARGET_LADEN;
    t.bus_target_restart = true;
    r.run(1000);
    EXPECT_EQ(shown(), twingo_bus::BUS_WACH) << "in manual the field sets the state";
    EXPECT_FALSE(t.bus_target_restart) << "the request is used up";
  }
  {
    LadenTwingo b;
    b.setup();
    Runner r(b);
    r.run(30000);
    b.request_sleep();
    r.run(5000);
    auto& t = datalayer_extended.twingoGen1;
    t.bus_wake_target = twingo_bus::TARGET_LADEN;
    t.bus_target_restart = true;
    const uint64_t t0 = r.now;
    r.run(2000);
    EXPECT_FALSE(t.bus_target_restart);
    // the walk of the sleep run goes on (state awake / ignition off, never "closed" restarting with 00/02 frames of the wake-up)
    EXPECT_NE(shown(), twingo_bus::BUS_LADEN);
    for (const Rec& rec : r.of(0x423, t0, t0 + 2000)) {
      EXPECT_NE(rec.f.data.u8[0], 0x30) << "no wake frame of a new session during the sleep run";
    }
  }
}

// ---------------------------------------------------------------------------
// Wake-up forms of the car while the target is "charging" (charge log Ladelog 2)
// ---------------------------------------------------------------------------

TEST(TwingoBau24Wake, Frame423HasByte0_87ForTheFramesOfTheLogAndTheRampOfByte1) {
  LadenTwingo b;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  b.setup();
  Runner r(b);
  r.run(20000);
  std::vector<Rec> f423 = r.of(0x423);
  ASSERT_GT(f423.size(), 100u);
  // the first two frames of every wake-up (Bau 7), then the regular frames k = 0, 1, ...
  EXPECT_EQ(f423[0].f.data.u8[0], 0x30);
  EXPECT_EQ(f423[1].f.data.u8[0], 0x33);
  for (uint32_t k = 0; k < 100; k++) {
    const CAN_frame& f = f423[2 + k].f;
    const bool in87 = k >= twingo_bus::LADEN_423_FIRST && k <= twingo_bus::LADEN_423_LAST;
    EXPECT_EQ(f.data.u8[0], in87 ? 0x87 : 0x07) << "k=" << k;
    if (k >= twingo_bus::LADEN_423_B1_FIRST && k <= twingo_bus::LADEN_423_LAST) {
      EXPECT_EQ(f.data.u8[1], twingo_bus::LADEN_423_B1[k - twingo_bus::LADEN_423_B1_FIRST]) << "k=" << k;
    }
  }
  // log: 87 1E at frame 29, the peak 2A at frame 41, 20 for a long time, 1F at the end
  EXPECT_EQ(f423[2 + 29].f.data.u8[1], 0x1E);
  EXPECT_EQ(f423[2 + 41].f.data.u8[1], 0x2A);
  EXPECT_EQ(f423[2 + 70].f.data.u8[1], 0x20);
  EXPECT_EQ(f423[2 + 84].f.data.u8[1], 0x1F);
}

TEST(TwingoBau24Wake, NoOverlayInTheWakeUpOfDriving) {
  LadenTwingo b;
  b.setup();  // target driving
  Runner r(b);
  r.run(20000);
  for (const Rec& rec : r.of(0x423)) {
    EXPECT_NE(rec.f.data.u8[0], 0x87);
  }
  // 0x1C7: no wake form (B3/B4 02/36) and no frame at all (row off, no charging)
  EXPECT_EQ(r.count(0x1C7), 0u);
  EXPECT_EQ(r.count(0x1A1), 0u);
  EXPECT_EQ(r.count(0x4F7), 0u);
}

TEST(TwingoBau24Wake, FramesOf100msStartWithTheFramesOfTheLog) {
  LadenTwingo b;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  b.setup();
  Runner r(b);
  r.run(20000);
  const struct {
    uint32_t id;
    uint8_t dlc;
  } IDS[] = {{0x419, 6}, {0x428, 8}, {0x42F, 6}, {0x435, 8}, {0x4F7, 8}};
  for (const auto& d : IDS) {
    const std::vector<Rec> fr = r.of(d.id);
    ASSERT_GT(fr.size(), (size_t)twingo_bus::LADEN_WAKE_FRAMES + 20) << std::hex << d.id;
    const uint8_t* table = twingo_bus::laden_wake_table((uint16_t)d.id);
    ASSERT_NE(table, nullptr);
    for (uint32_t n = 0; n < twingo_bus::LADEN_WAKE_FRAMES; n++) {
      for (uint8_t i = 0; i < d.dlc; i++) {
        ASSERT_EQ(fr[n].f.data.u8[i], table[n * 8 + i]) << std::hex << "0x" << d.id << " frame " << std::dec << n << " byte " << (int)i;
      }
    }
  }
  // spot checks against the log: 0x428 starts 00 00 00 00 00 07 C0 00, then 06 80 for a long time, ends with 04 8F
  const std::vector<Rec> f428 = r.of(0x428);
  EXPECT_TRUE(bytes_eq(f428[0].f, {0, 0, 0, 0, 0, 0x07, 0xC0, 0}));
  EXPECT_TRUE(bytes_eq(f428[40].f, {0, 0, 0, 0, 0, 0x06, 0x80, 0}));
  EXPECT_TRUE(bytes_eq(f428[129].f, {0, 0, 0, 0, 0, 0x04, 0x8F, 0}));
  const std::vector<Rec> f435 = r.of(0x435);
  EXPECT_TRUE(bytes_eq(f435[0].f, {0xFF, 0xFF, 0x1E, 0xEB, 0x35, 0xFC, 0, 0}));
}

TEST(TwingoBau24Wake, FastFramesHaveTheirWakeForm) {
  LadenTwingo b;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  b.setup();
  Runner r(b);
  r.run(20000);
  const std::vector<Rec> f1c7 = r.of(0x1C7);
  ASSERT_GT(f1c7.size(), 1000u);
  EXPECT_EQ(f1c7[0].f.data.u8[3], 0x02);
  EXPECT_EQ(f1c7[0].f.data.u8[4], 0x36);
  EXPECT_EQ(f1c7[803].f.data.u8[4], 0x36);
  EXPECT_EQ(f1c7[804].f.data.u8[4], 0x34);
  EXPECT_EQ(f1c7[804].f.data.u8[3], 0x02);
  EXPECT_EQ(f1c7[812].f.data.u8[3], 0x04) << "then the state 'charging': 04/34";
  EXPECT_EQ(f1c7[812].f.data.u8[4], 0x34);
  const std::vector<Rec> f1a1 = r.of(0x1A1);
  ASSERT_GT(f1a1.size(), 300u);
  EXPECT_TRUE(bytes_eq(f1a1[0].f, {0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0x00, 0x20}));
  EXPECT_TRUE(bytes_eq(f1a1[6].f, {0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFC, 0x00, 0x20}));
  EXPECT_EQ(f1a1[7].f.data.u8[0], 0x00);
  EXPECT_EQ(f1a1[7].f.data.u8[1], 0x47);
  EXPECT_EQ(f1a1[186].f.data.u8[0], 0x08);
  EXPECT_EQ(f1a1[236].f.data.u8[0], 0xB0);
  const std::vector<Rec> f157 = r.of(0x157);
  ASSERT_GT(f157.size(), 1500u);
  EXPECT_EQ(f157[0].f.data.u8[5], 0xE0);
  EXPECT_EQ(f157[0].f.data.u8[6], 0x00);
  EXPECT_EQ(f157[1099].f.data.u8[6], 0x00);
  EXPECT_EQ(f157[1215].f.data.u8[5], 0xE0);
  EXPECT_EQ(f157[1216].f.data.u8[5], 0xE1);
  EXPECT_EQ(f157[1339].f.data.u8[5], 0xE2) << "the state 'charging'";
}

TEST(TwingoBau24Wake, FormsEndAfterTheWakeUp) {
  uint8_t out[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  EXPECT_FALSE(twingo_bus::laden_wake_form(0x419, twingo_bus::LADEN_WAKE_FRAMES, 6, out));
  EXPECT_FALSE(twingo_bus::laden_wake_form(0x1C7, 812, 6, out));
  EXPECT_FALSE(twingo_bus::laden_wake_form(0x157, 1339, 8, out));
  EXPECT_FALSE(twingo_bus::laden_wake_form(0x1A1, 237, 8, out));
  EXPECT_FALSE(twingo_bus::laden_wake_form(0x155, 0, 8, out)) << "frames of the pack are not touched";
  EXPECT_FALSE(twingo_bus::laden_wake_form(0x426, 0, 8, out));
  EXPECT_EQ(out[0], 1);
  uint8_t b0 = 0;
  uint8_t b1 = 0;
  EXPECT_FALSE(twingo_bus::laden_wake_423(28, b0, b1));
  EXPECT_TRUE(twingo_bus::laden_wake_423(29, b0, b1));
  EXPECT_EQ(b0, 0x87);
  EXPECT_EQ(b1, 0x1E);
  EXPECT_TRUE(twingo_bus::laden_wake_423(84, b0, b1));
  EXPECT_FALSE(twingo_bus::laden_wake_423(85, b0, b1));
}

// ---------------------------------------------------------------------------
// All frames of the EVC while charging
// ---------------------------------------------------------------------------

TEST(TwingoBau24AllFrames, ChargingSendsAllFramesOfTheEvcExceptThoseOfThePackAndExcept0x500) {
  static_assert(LADEN_SENDS_ALL_EVC_FRAMES, "Bau 24: default true");
  LadenTwingo b;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  b.setup();
  Runner r(b);
  r.run(40000);
  ASSERT_EQ(shown(), twingo_bus::BUS_LADEN);
  std::set<uint32_t> ids;
  for (const Rec& rec : r.of(0x100, 0, 0)) {
    (void)rec;
  }
  for (const Rec& rec : r.rec) {
    if (rec.t >= 30000) {
      ids.insert(rec.f.ID);
    }
  }
  // the twelve of the default plus the four whose row is off
  for (uint32_t id : {0x157u, 0x0ECu, 0x0EDu, 0x19Fu, 0x426u, 0x436u, 0x423u, 0x419u, 0x428u, 0x42Fu, 0x435u, 0x69Fu, 0x1A1u,
                      0x1C7u, 0x4F7u, 0x511u}) {
    EXPECT_TRUE(ids.count(id) == 1) << std::hex << "0x" << id;
  }
  for (uint32_t id : {0x500u, 0x155u, 0x0C5u, 0x1C9u, 0x424u, 0x425u, 0x43Au, 0x445u, 0x464u, 0x588u, 0x6BEu, 0x4AEu, 0x4AFu,
                      0x659u}) {
    EXPECT_TRUE(ids.count(id) == 0) << std::hex << "0x" << id << " must not be sent";
  }
  // the rates of the car: 0x1A1 and 0x1C7 every 10 ms, 0x4F7 and 0x511 every 100 ms
  EXPECT_NEAR((double)r.of(0x1A1, 30000, 40000).size(), 1000.0, 5.0);
  EXPECT_NEAR((double)r.of(0x1C7, 30000, 40000).size(), 1000.0, 5.0);
  EXPECT_NEAR((double)r.of(0x4F7, 30000, 40000).size(), 100.0, 3.0);
  EXPECT_NEAR((double)r.of(0x511, 30000, 40000).size(), 100.0, 3.0);
}

TEST(TwingoBau24AllFrames, DrivingStaysWithTheRowsOfTheMask) {
  LadenTwingo b;
  b.setup();  // target driving
  Runner r(b);
  r.run(40000);
  ASSERT_EQ(shown(), twingo_bus::BUS_FAHRT);
  std::set<uint32_t> ids;
  for (const Rec& rec : r.rec) {
    ids.insert(rec.f.ID);
  }
  EXPECT_EQ(ids.count(0x1A1), 0u);
  EXPECT_EQ(ids.count(0x1C7), 0u);
  EXPECT_EQ(ids.count(0x4F7), 0u);
  EXPECT_EQ(ids.count(0x511), 0u);
}

TEST(TwingoBau24AllFrames, ManualStateChargingSendsThemToo) {
  LadenTwingo b;
  auto& t = datalayer_extended.twingoGen1;
  t.bus_state_manual = true;
  t.bus_state = twingo_bus::BUS_LADEN;
  b.setup();
  Runner r(b);
  r.run(5000);
  EXPECT_GT(r.count(0x1A1), 100u);
  EXPECT_GT(r.count(0x511), 10u);
  EXPECT_EQ(r.count(0x500), 0u);
  t.bus_state = twingo_bus::BUS_FAHRT;
  const size_t before = r.count(0x1A1);
  r.run(1000);
  EXPECT_EQ(r.count(0x1A1), before) << "left the state charging: the rows of the mask only";
}

// ---------------------------------------------------------------------------
// End of the charge: the way "Ladelog 2" of the Sleep run
// ---------------------------------------------------------------------------

namespace {
// time after the press of Sleep at which 0x426 shows the closing stage 00/02 for the first time
uint64_t closing_start_ms(uint8_t path, bool like_car) {
  LadenTwingo b;
  auto& t = datalayer_extended.twingoGen1;
  t.bus_sleep_path = path;
  Bat::shutdown_like_car = like_car;
  b.setup();
  Runner r(b);
  r.run(30000);
  const uint64_t press = r.now;
  b.request_sleep();
  r.run(200000);
  for (const Rec& rec : r.of(0x426, press)) {
    if (rec.f.data.u8[1] == 0x00 && rec.f.data.u8[2] == 0x02) {
      return rec.t - press;
    }
  }
  return 0;
}
}  // namespace

TEST(TwingoBau24End, AwakeTimeOfTheWayLadelog2IsThatOfTheCar) {
  // log: the end of A0/41 at 553.27 s, the closing stage 00/02 at 677.87 s = 124.6 s
  for (bool like_car : {false, true}) {
    const uint64_t t = closing_start_ms(twingo_bus::SLEEP_LADELOG2, like_car);
    EXPECT_NEAR((double)t, 124600.0, 150.0) << "like_car=" << like_car;
  }
}

TEST(TwingoBau24End, OtherWaysKeepTheirTimes) {
  EXPECT_NEAR((double)closing_start_ms(twingo_bus::SLEEP_LANG, false), 136000.0, 150.0);
  EXPECT_NEAR((double)closing_start_ms(twingo_bus::SLEEP_LANG, true), 133200.0, 150.0);
  EXPECT_NEAR((double)closing_start_ms(twingo_bus::SLEEP_KURZ, false), 136000.0, 150.0);
}

TEST(TwingoBau24End, Frame423HasByte0_87From330msFor5500msOnTheWayLadelog2Only) {
  for (uint8_t path : {twingo_bus::SLEEP_LADELOG2, twingo_bus::SLEEP_LANG}) {
    LadenTwingo b;
    datalayer_extended.twingoGen1.bus_sleep_path = path;
    datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
    b.setup();
    Runner r(b);
    r.run(40000);
    const uint64_t press = r.now;
    b.request_sleep();
    r.run(20000);
    uint64_t first87 = 0;
    uint64_t last87 = 0;
    for (const Rec& rec : r.of(0x423, press, press + 20000)) {
      if (rec.f.data.u8[0] == 0x87) {
        if (first87 == 0) {
          first87 = rec.t - press;
        }
        last87 = rec.t - press;
      }
    }
    if (path == twingo_bus::SLEEP_LADELOG2) {
      EXPECT_GE(first87, 330u);
      EXPECT_LE(first87, 450u);
      EXPECT_LT(last87, 5830u);
      EXPECT_GE(last87, 5700u);
    } else {
      EXPECT_EQ(first87, 0u) << "no 87 on the other ways";
    }
  }
}

TEST(TwingoBau24End, WakeUpAfterTheSleepRunOfTheTargetChargingRunsTheChargingWakeUpAgain) {
  LadenTwingo b;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 1;
  b.setup();
  Runner r(b);
  r.run(30000);
  const uint64_t press = r.now;
  b.request_sleep();
  r.run(400000);  // shutdown (125 s), closing stage, silence of the failsafe time (1 min), wake-up burst, then the bus
  // the first frame of 0x423 after the silence is the first wake frame 30 7F ...
  uint64_t wake = 0;
  const std::vector<Rec> f423 = r.of(0x423, press + 100000);
  for (size_t i = 1; i < f423.size(); i++) {
    if (f423[i].t - f423[i - 1].t > 5000) {
      wake = f423[i].t;
      break;
    }
  }
  ASSERT_NE(wake, 0u) << "a silence and a wake-up";
  EXPECT_EQ(shown(), twingo_bus::BUS_LADEN);
  // the wake-up through the plug runs again: 0x423 has the frames with byte 0 = 87 and 0x1C7 starts with its wake form 02/36
  size_t n87 = 0;
  for (const Rec& rec : r.of(0x423, wake)) {
    n87 += rec.f.data.u8[0] == 0x87 ? 1 : 0;
  }
  EXPECT_GE(n87, 50u);
  const std::vector<Rec> f1c7 = r.of(0x1C7, wake);
  ASSERT_FALSE(f1c7.empty());
  EXPECT_EQ(f1c7[0].f.data.u8[4], 0x36);
}
