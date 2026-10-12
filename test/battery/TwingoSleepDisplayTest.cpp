#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "../../Software/src/devboard/display/sleep_display.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// 13.10.: the OLED display shows "Sleep requested" / "Sleep pls wait" and the time while a battery goes to sleep.

namespace {

class DispTwingo : public RenaultTwingoGen1Battery {
 public:
  DispTwingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.bus_state_manual = true;
    t.bus_state = twingo_bus::BUS_WACH;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.simulator_enabled_mask = 0x3FF;
    t.simulator_enabled_mask_hi = 0;
    datalayer.system.status.sleep_display_phase = 0;
  }
  using Bat = RenaultTwingoGen1Battery;
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

void step(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms, uint64_t step_ms = 100) {
  for (uint64_t end = t + ms; t < end; t += step_ms) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
  }
}

}  // namespace

// ---------------------------------------------------------------------------
// Formatting
// ---------------------------------------------------------------------------

TEST(TwingoSleepDisplay, TimesAreHhMmSs) {
  char s[9];
  sleep_display::format_hms(0, s);
  EXPECT_STREQ(s, "00:00:00");
  sleep_display::format_hms(274, s);
  EXPECT_STREQ(s, "00:04:34");
  sleep_display::format_hms(600, s);
  EXPECT_STREQ(s, "00:10:00");
  sleep_display::format_hms(24 * 3600, s);
  EXPECT_STREQ(s, "24:00:00");
  sleep_display::format_hms(3661, s);
  EXPECT_STREQ(s, "01:01:01");
}

TEST(TwingoSleepDisplay, WhileTheShutdownSequenceRunsItSaysSleepRequestedAndZero) {
  char r4[22], r5[22];
  ASSERT_TRUE(sleep_display::build_rows(sleep_display::PHASE_REQUESTED, 99, 600, r4, r5));
  EXPECT_STREQ(r4, "   Sleep requested   ");
  EXPECT_STREQ(r5, " 00:00:00 / 00:10:00 ");
}

TEST(TwingoSleepDisplay, InTheSilenceItSaysSleepPlsWaitAndCountsUp) {
  char r4[22], r5[22];
  ASSERT_TRUE(sleep_display::build_rows(sleep_display::PHASE_WAIT, 274, 600, r4, r5));
  EXPECT_STREQ(r4, "   Sleep pls wait    ");
  EXPECT_STREQ(r5, " 00:04:34 / 00:10:00 ");
}

TEST(TwingoSleepDisplay, RowsAreAlways21CharactersLongAndCentred) {
  char r4[22], r5[22];
  for (uint32_t e : {0u, 1u, 59u, 3600u, 86400u}) {
    ASSERT_TRUE(sleep_display::build_rows(sleep_display::PHASE_WAIT, e, 86400, r4, r5));
    EXPECT_EQ(strlen(r4), 21u);
    EXPECT_EQ(strlen(r5), 21u);
  }
  // the time row has 19 characters, 1 blank on each side
  ASSERT_TRUE(sleep_display::build_rows(sleep_display::PHASE_WAIT, 5, 600, r4, r5));
  EXPECT_EQ(r5[0], ' ');
  EXPECT_EQ(r5[20], ' ');
  EXPECT_EQ(r5[1], '0');
  EXPECT_EQ(r5[19], '0');
}

TEST(TwingoSleepDisplay, TheElapsedTimeNeverPassesTheEndTime) {
  char r4[22], r5[22];
  ASSERT_TRUE(sleep_display::build_rows(sleep_display::PHASE_WAIT, 9999, 600, r4, r5));
  EXPECT_STREQ(r5, " 00:10:00 / 00:10:00 ");
}

TEST(TwingoSleepDisplay, WithoutASleepRunNothingIsShownAndBothRowsAreBlank) {
  char r4[22], r5[22];
  EXPECT_FALSE(sleep_display::build_rows(sleep_display::PHASE_NONE, 5, 600, r4, r5));
  EXPECT_STREQ(r4, "                     ");
  EXPECT_STREQ(r5, "                     ");
  EXPECT_FALSE(sleep_display::build_rows(7, 5, 600, r4, r5));
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------

// Steps until the phase is reached (at most max_ms); returns true when it was reached.
static bool step_until_phase(RenaultTwingoGen1Battery& b, uint64_t& t, uint8_t phase, uint64_t max_ms) {
  for (uint64_t end = t + max_ms; t < end; t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
    if (datalayer.system.status.sleep_display_phase == phase) {
      return true;
    }
  }
  return false;
}

TEST(TwingoSleepDisplayDriver, ShutdownSequenceShowsPhase1ThenTheSilenceCountsFromItsStart) {
  DispTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 10;
  datalayer_extended.twingoGen1.bus_state_manual = false;
  uint64_t t = 1000;
  step(b, t, 25000);
  EXPECT_EQ(datalayer.system.status.sleep_display_phase, 0) << "no sleep run yet";
  b.request_sleep();
  ASSERT_TRUE(step_until_phase(b, t, 1, 5000)) << "shutdown sequence: Sleep requested";
  EXPECT_EQ(datalayer.system.status.sleep_display_elapsed_s, 0u);
  EXPECT_EQ(datalayer.system.status.sleep_display_total_s, 600u) << "the end time is the failsafe time only";
  step(b, t, 20000);
  EXPECT_EQ(datalayer.system.status.sleep_display_phase, 1);
  EXPECT_EQ(datalayer.system.status.sleep_display_elapsed_s, 0u) << "the counter stays at zero in the sequence";
  // the silence starts
  ASSERT_TRUE(step_until_phase(b, t, 2, 300000));
  EXPECT_LE(datalayer.system.status.sleep_display_elapsed_s, 1u);
  const uint32_t e1 = datalayer.system.status.sleep_display_elapsed_s;
  step(b, t, 60000, 1000);
  const uint32_t e2 = datalayer.system.status.sleep_display_elapsed_s;
  EXPECT_NEAR((double)(e2 - e1), 60.0, 2.0);
  EXPECT_EQ(datalayer.system.status.sleep_display_total_s, 600u);
  char r4[22], r5[22];
  ASSERT_TRUE(sleep_display::build_rows(datalayer.system.status.sleep_display_phase,
                                        datalayer.system.status.sleep_display_elapsed_s,
                                        datalayer.system.status.sleep_display_total_s, r4, r5));
  EXPECT_STREQ(r4, "   Sleep pls wait    ");
  EXPECT_NE(std::string(r5).find(" / 00:10:00"), std::string::npos);
}

TEST(TwingoSleepDisplayDriver, TheDisplayIsClearedWhenTheSilenceIsOver) {
  DispTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 1;
  datalayer_extended.twingoGen1.bus_state_manual = false;
  uint64_t t = 1000;
  step(b, t, 25000);
  b.request_sleep();
  ASSERT_TRUE(step_until_phase(b, t, 2, 300000));
  EXPECT_EQ(datalayer.system.status.sleep_display_total_s, 60u);
  // the end time (1 min) is reached: the wake-up burst starts, the display may write the events again
  step(b, t, 65000, 100);
  EXPECT_EQ(datalayer.system.status.sleep_display_phase, 0);
  EXPECT_EQ(datalayer.system.status.sleep_display_elapsed_s, 0u);
  EXPECT_EQ(datalayer.system.status.sleep_display_total_s, 0u);
}

TEST(TwingoSleepDisplayDriver, AWakeUpRequestClearsTheDisplayToo) {
  DispTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 30;
  datalayer_extended.twingoGen1.bus_state_manual = false;
  uint64_t t = 1000;
  step(b, t, 25000);
  b.request_sleep();
  ASSERT_TRUE(step_until_phase(b, t, 2, 300000));
  b.request_wake_up();
  step(b, t, 2000, 100);
  EXPECT_EQ(datalayer.system.status.sleep_display_phase, 0);
}
