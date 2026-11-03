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

// Daily balance (03.11.2026): the pure logic (LOGIC.h) and the run of the driver, step by step.

// ---------------------------------------------------------------------------------------------------------
// Pure logic
// ---------------------------------------------------------------------------------------------------------

TEST(TwingoDailyBalanceLogic, CyclesAndCheckAreClamped) {
  EXPECT_EQ(twingo::db_cycles_sane(0), 1);
  EXPECT_EQ(twingo::db_cycles_sane(1), 1);
  EXPECT_EQ(twingo::db_cycles_sane(5), 5);
  EXPECT_EQ(twingo::db_cycles_sane(6), 5);
  EXPECT_EQ(twingo::db_cycles_sane(60000), 5);
  EXPECT_EQ(twingo::db_check_sane(0), 10);
  EXPECT_EQ(twingo::db_check_sane(120), 120);
  EXPECT_EQ(twingo::db_check_sane(99999), 3600);
}

TEST(TwingoDailyBalanceLogic, RunModeIsSleepUnlessSleep9281IsChosen) {
  EXPECT_EQ(twingo::db_mode_sane(0), 1);
  EXPECT_EQ(twingo::db_mode_sane(1), 1) << "Sleep is the default";
  EXPECT_EQ(twingo::db_mode_sane(2), 2);
  EXPECT_EQ(twingo::db_mode_sane(7), 1);
}

TEST(TwingoDailyBalanceLogic, TestCycleIsCycleZeroAndOnlyWithATestTime) {
  EXPECT_EQ(twingo::db_first_cycle(10), 0);
  EXPECT_EQ(twingo::db_first_cycle(0), 1) << "test = 0 switches the test cycle off";
  EXPECT_EQ(twingo::db_cycle_min(0, 65, 10), 10);
  EXPECT_EQ(twingo::db_cycle_min(1, 65, 10), 65);
  EXPECT_EQ(twingo::db_cycle_min(3, 65, 10), 65);
}

TEST(TwingoDailyBalanceLogic, ThePlanIsCheckedAfterEveryWakeUpButTheLast) {
  EXPECT_TRUE(twingo::db_check_plan_after(0, 3)) << "after the test";
  EXPECT_TRUE(twingo::db_check_plan_after(1, 3));
  EXPECT_TRUE(twingo::db_check_plan_after(2, 3));
  EXPECT_FALSE(twingo::db_check_plan_after(3, 3)) << "the last cycle: no check, back to normal";
  EXPECT_FALSE(twingo::db_check_plan_after(1, 1));
  EXPECT_TRUE(twingo::db_check_plan_after(0, 1)) << "the test before a single cycle";
}

TEST(TwingoDailyBalanceLogic, StartConditionsInTheOrderOfTheMessages) {
  using twingo::db_start_check;
  EXPECT_EQ(db_start_check(false, true, true, false, true, true), twingo::DBS_NOT_DUE) << "checkbox off";
  EXPECT_EQ(db_start_check(true, false, true, false, true, true), twingo::DBS_NOT_DUE) << "no clock";
  EXPECT_EQ(db_start_check(true, true, false, false, true, true), twingo::DBS_NOT_DUE) << "not the minute";
  EXPECT_EQ(db_start_check(true, true, true, true, true, true), twingo::DBS_BUSY);
  EXPECT_EQ(db_start_check(true, true, true, false, false, true), twingo::DBS_NOT_DRIVE);
  EXPECT_EQ(db_start_check(true, true, true, false, true, false), twingo::DBS_NO_TYPED_TIME);
  EXPECT_EQ(db_start_check(true, true, true, false, true, true), twingo::DBS_OK);
}

TEST(TwingoDailyBalanceLogic, PlanBitsAreCountedOverAllTwelveBytes) {
  uint8_t none[12] = {0};
  EXPECT_EQ(twingo::plan_bits_count(none, 12), 0);
  uint8_t some[12] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01};
  EXPECT_EQ(twingo::plan_bits_count(some, 12), 2);
  uint8_t all[12];
  memset(all, 0xFF, sizeof(all));
  EXPECT_EQ(twingo::plan_bits_count(all, 12), 96);
}

TEST(TwingoDailyBalanceLogic, PlanVerdictIsYesAtOnceAndNoOnlyAfterTheWaitAndThreeAnswers) {
  using twingo::plan_verdict;
  EXPECT_EQ(plan_verdict(0, 0, 0, 120000), twingo::PLAN_WAIT);
  EXPECT_EQ(plan_verdict(0, 5, 1000, 120000), twingo::PLAN_WAIT) << "a bit of the old answer does not count";
  EXPECT_EQ(plan_verdict(1, 1, 1000, 120000), twingo::PLAN_YES) << "one answer with one bit: at once";
  EXPECT_EQ(plan_verdict(2, 0, 30000, 120000), twingo::PLAN_WAIT);
  EXPECT_EQ(plan_verdict(3, 0, 119999, 120000), twingo::PLAN_WAIT) << "three answers but not yet the wait";
  EXPECT_EQ(plan_verdict(2, 0, 200000, 120000), twingo::PLAN_WAIT) << "the wait is over but only two answers";
  EXPECT_EQ(plan_verdict(3, 0, 120000, 120000), twingo::PLAN_NO);
  EXPECT_EQ(plan_verdict(4, 0, 300000, 120000), twingo::PLAN_NO);
  EXPECT_EQ(plan_verdict(5, 2, 300000, 120000), twingo::PLAN_YES) << "a plan wins even after the wait";
}

TEST(TwingoDailyBalanceLogic, BookedTimeAddsTheMinutesAndStaysBelowTheMaximum) {
  EXPECT_EQ(twingo::db_booked_time(71240, 70, 0xFFFFFF), 71310u);
  EXPECT_EQ(twingo::db_booked_time(0xFFFFF0, 70, 0xFFFFFF), 0xFFFFFFu);
  EXPECT_EQ(twingo::db_booked_time(100, 0, 0xFFFFFF), 100u);
}

TEST(TwingoDailyBalanceLogic, ReleaseWaitsForTheStateAndThePackButNotForever) {
  using twingo::db_release_ready;
  EXPECT_FALSE(db_release_ready(false, true, 0, 1000)) << "state not reached";
  EXPECT_FALSE(db_release_ready(true, false, 5000, 20000)) << "reached, pack not yet heard, 5 s";
  EXPECT_TRUE(db_release_ready(true, true, 0, 20000)) << "reached and the pack answered";
  EXPECT_TRUE(db_release_ready(true, false, 10000, 40000)) << "10 s after reaching driving: released anyway";
  EXPECT_FALSE(db_release_ready(false, false, 0, 179999));
  EXPECT_TRUE(db_release_ready(false, false, 0, 180000)) << "in any case after the hard cap";
}

// ---------------------------------------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------------------------------------

namespace {
using Bat = RenaultTwingoGen1Battery;

// the phases of the driver (enum DbPhase of the class)
constexpr uint8_t DB_IDLE = 0;
constexpr uint8_t DB_ZERO_WAIT = 1;
constexpr uint8_t DB_SLEEPING = 2;
constexpr uint8_t DB_AWAKE = 3;
constexpr uint8_t DB_RELEASE = 4;

class BalanceTwingo : public RenaultTwingoGen1Battery {
 public:
  uint32_t clock_secs = 3 * 3600 + 10;  // 03:00:10
  bool clock_set = true;
  BalanceTwingo() {
    Bat::age_manual_clear();
    time_436_clear();
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
    t.bus_sleep_path = twingo_bus::SLEEP_LADELOG2;  // the user's choice, must be back after the run
    t.sleep_failsafe_minutes = 30;
    t.sleep_button_enabled = false;
    t.daily_sleep_enabled = true;
    t.daily_sleep_time_min = 3 * 60;
    t.daily_sleep_duration_min = 1;
    t.daily_balance_cycles = 2;
    t.daily_balance_mode = 1;
    t.daily_balance_test_min = 1;
    t.daily_balance_check_s = 10;
    t.simulator_enabled_mask = SIM_DEFAULT_MASK;
    t.simulator_enabled_mask_hi = 0;
    Bat::shutdown_like_car = false;
  }
  ~BalanceTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.daily_sleep_enabled = false;
    t.daily_sleep_duration_min = 5;
    t.daily_balance_cycles = 1;
    t.daily_balance_mode = 1;
    t.daily_balance_abort = false;
    t.daily_balance_test_min = 10;
    t.daily_balance_check_s = 120;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.bus_state_manual = false;
    t.simulator_enabled_mask = 0x10;
    time_436_clear();
    Bat::age_manual_clear();
  }
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t& now_utc) override {
    now_utc = 1791590400;
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

struct Sim {
  BalanceTwingo& b;
  uint64_t now = 1000;
  std::vector<Tx> log;
  explicit Sim(BalanceTwingo& bat) : b(bat) {}
  void run(uint64_t ms) {
    for (uint64_t end = now + ms; now < end; now += 50) {
      set_millis64(now);
      clear_transmitted_frames();
      b.transmit_can((unsigned long)now);
      for (const CAN_frame& f : get_transmitted_frames()) {
        log.push_back({now, f});
      }
    }
  }
  // runs until the condition holds (checked every step), at most `max_ms`; returns whether it held
  template <typename F>
  bool until(F cond, uint64_t max_ms) {
    for (uint64_t end = now + max_ms; now < end; now += 50) {
      set_millis64(now);
      clear_transmitted_frames();
      b.transmit_can((unsigned long)now);
      for (const CAN_frame& f : get_transmitted_frames()) {
        log.push_back({now, f});
      }
      if (cond()) {
        return true;
      }
    }
    return false;
  }
};

const uint8_t PLAN_96[12] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const uint8_t PLAN_NONE[12] = {0};

// drives the bus to "driving" and activates the typed time of 0x436, with the clock not yet in the minute
void prepare(BalanceTwingo& b, Sim& s, uint32_t typed = 71240) {
  b.setup();
  b.clock_secs = 2 * 3600 + 59 * 60;  // 02:59:00
  b.test_time_436_set(typed);
  s.run(30000);
  ASSERT_EQ(Bat::bus_state_shown, twingo_bus::BUS_FAHRT);
}

bool wrote_9281(const std::vector<Tx>& log, uint64_t from) {
  for (const Tx& x : log) {
    if (x.t >= from && x.f.ext_ID && x.f.data.u8[1] == 0x2E && x.f.data.u8[2] == 0x92 && x.f.data.u8[3] == 0x81) {
      return true;
    }
  }
  return false;
}
}  // namespace

TEST(TwingoDailyBalanceRun, DoesNothingWithTheCheckboxOffOrWithoutAClockOrOutsideTheMinute) {
  for (int variant = 0; variant < 3; variant++) {
    BalanceTwingo b;
    Sim s(b);
    prepare(b, s);
    datalayer_extended.twingoGen1.daily_sleep_enabled = (variant != 0);
    b.clock_set = (variant != 1);
    b.clock_secs = variant == 2 ? 3 * 3600 + 120 : 3 * 3600 + 10;  // variant 2: 03:02, not the minute
    s.run(20000);
    EXPECT_EQ(b.test_db_phase(), DB_IDLE) << variant;
    EXPECT_FALSE(b.test_db_hold_zero()) << variant;
    EXPECT_FALSE(wrote_9281(s.log, 31000)) << variant;
  }
}

TEST(TwingoDailyBalanceRun, DoesNotStartAndDoesNotTouchAnythingOutsideDriveOrWithoutTheTypedTime) {
  {
    BalanceTwingo b;  // the typed time is not active
    Sim s(b);
    b.setup();
    b.clock_secs = 2 * 3600 + 59 * 60;  // the bus walk to driving first, not in the minute yet
    s.run(30000);
    ASSERT_EQ(Bat::bus_state_shown, twingo_bus::BUS_FAHRT);
    b.clock_secs = 3 * 3600 + 10;
    const uint32_t before = b.test_time_436();
    s.run(20000);
    EXPECT_EQ(b.test_db_phase(), DB_IDLE);
    EXPECT_STREQ(b.test_db_status(), "not started: time in 0x436 not active");
    EXPECT_EQ(b.test_time_436(), before) << "nothing is incremented";
    EXPECT_FALSE(b.test_db_hold_zero());
  }
  {
    BalanceTwingo b;  // not in Drive (the bus state is held manually in awake)
    Sim s(b);
    b.setup();
    b.test_time_436_set(71240);
    datalayer_extended.twingoGen1.bus_state_manual = true;
    datalayer_extended.twingoGen1.bus_state = twingo_bus::BUS_WACH;
    s.run(5000);
    b.clock_secs = 3 * 3600 + 10;
    s.run(10000);
    EXPECT_EQ(b.test_db_phase(), DB_IDLE);
    EXPECT_STREQ(b.test_db_status(), "not started: not in Drive");
  }
}

TEST(TwingoDailyBalanceRun, ZeroesTheLimitsAtOnceStartsThePlainSleepFiveSecondsLaterOnTheWayShort) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  auto& t = datalayer_extended.twingoGen1;
  b.clock_secs = 3 * 3600 + 10;
  const uint64_t t_fire = s.now;
  s.run(1100);  // the service runs once per second
  EXPECT_EQ(b.test_db_phase(), DB_ZERO_WAIT);
  EXPECT_TRUE(b.test_db_hold_zero());
  datalayer.battery.status.max_charge_power_W = 5000;
  datalayer.battery.status.max_discharge_power_W = 5000;
  datalayer.battery.status.current_dA = 77;
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.max_charge_power_W, 0u) << "the limits towards the inverter are zero";
  EXPECT_EQ(datalayer.battery.status.max_discharge_power_W, 0u);
  EXPECT_EQ(datalayer.battery.status.current_dA, 0);
  EXPECT_FALSE(b.test_nvrol_request()) << "no sleep run during the five seconds";
  EXPECT_FALSE(wrote_9281(s.log, t_fire));
  EXPECT_EQ(t.bus_sleep_path, twingo_bus::SLEEP_LADELOG2) << "the path of the user until the first sleep";
  s.run(4600);  // 5.7 s after the start
  EXPECT_EQ(b.test_db_phase(), DB_SLEEPING);
  EXPECT_TRUE(b.test_nvrol_request());
  EXPECT_EQ(t.bus_sleep_path, twingo_bus::SLEEP_KURZ) << "way short";
  EXPECT_STREQ(b.test_db_status(), "test cycle 1 min: sleeping");
  s.run(1000);
  EXPECT_FALSE(wrote_9281(s.log, t_fire)) << "the plain Sleep: nothing is written to the pack";
  EXPECT_EQ(b.test_nvrol_mode(), 1) << "run mode Sleep";
}

TEST(TwingoDailyBalanceRun, FullRunWithPlanBooksTheTimeAfterTheQuietTimeAndReleasesAtTheEnd) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  auto& t = datalayer_extended.twingoGen1;
  b.clock_secs = 3 * 3600 + 10;
  // test cycle: until the silence
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 5; }, 400000)) << "silence of the test cycle";
  EXPECT_EQ(b.test_db_cycle(), 0);
  // a frame of the BMS arrives right now: the booking comes 30 s after it, not before
  b.test_silence_frame();
  const uint32_t v0 = b.test_time_436();
  const uint64_t t_frame = s.now;
  s.run(29000);
  EXPECT_EQ(b.test_time_436(), v0) << "29 s after the last frame: not yet";
  s.run(2000);
  EXPECT_EQ(b.test_time_436(), v0 + 1) << "30 s after the last frame: +1 minute of the test cycle booked";
  EXPECT_GE(s.now, t_frame + 30000);
  // wake-up, then awake and check
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_AWAKE; }, 120000));
  EXPECT_TRUE(b.test_db_hold_awake());
  s.run(3000);
  EXPECT_EQ(Bat::bus_state_shown, twingo_bus::BUS_WACH) << "held awake while the plan is checked";
  EXPECT_FALSE(b.test_nvrol_request());
  EXPECT_TRUE(b.test_db_hold_zero()) << "the limits stay at zero between the cycles";
  s.run(20000);
  EXPECT_EQ(b.test_db_phase(), DB_AWAKE) << "no plan yet: waiting, not sleeping";
  b.test_bal_answer(PLAN_96);  // the plan arrives
  const uint64_t t_plan = s.now;
  s.run(1100);
  // 10 s after the wake-up the next cycle sleeps (the wait since the wake-up is already over here)
  EXPECT_EQ(b.test_db_phase(), DB_SLEEPING);
  EXPECT_EQ(b.test_db_cycle(), 1);
  EXPECT_FALSE(b.test_db_hold_awake());
  EXPECT_NE(std::string(b.test_db_bit_log()).find("test"), std::string::npos) << "the time to the first bit is written down";
  EXPECT_GE(s.now, t_plan);
  // cycle 1 .. 2: each booked by its 1 minute, the plan check after cycle 1, none after the last
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 5; }, 400000));
  const uint32_t v1 = b.test_time_436();
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_AWAKE; }, 400000));
  EXPECT_EQ(b.test_db_cycle(), 1);
  EXPECT_GE(b.test_time_436(), v1 + 1) << "booked in cycle 1 (the clock of 0x436 may add one more while it counts)";
  EXPECT_LE(b.test_time_436(), v1 + 2);
  b.test_bal_answer(PLAN_96);
  s.run(11000);
  ASSERT_EQ(b.test_db_phase(), DB_SLEEPING);
  EXPECT_EQ(b.test_db_cycle(), 2) << "the last cycle";
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 5; }, 400000));
  const uint32_t v2 = b.test_time_436();
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_RELEASE; }, 600000)) << "after the last wake-up: release phase";
  EXPECT_GE(b.test_time_436(), v2 + 1);
  EXPECT_LE(b.test_time_436(), v2 + 2);
  EXPECT_EQ(t.bus_sleep_path, twingo_bus::SLEEP_LADELOG2) << "the way of the user is back";
  EXPECT_FALSE(b.test_db_hold_awake());
  EXPECT_TRUE(b.test_db_hold_zero()) << "still zero: the state is not as before yet";
  EXPECT_STREQ(b.test_db_status(), "done: all cycles");
  // the walk goes to driving; the limits come back when the state is as before and the pack answered
  b.test_pack_answered();
  ASSERT_TRUE(s.until([&] { return Bat::bus_state_shown == twingo_bus::BUS_FAHRT; }, 60000));
  s.run(1500);
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
  EXPECT_FALSE(b.test_db_hold_zero());
  // not again in the same minute
  s.run(20000);
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
}

TEST(TwingoDailyBalanceRun, NoPlanAfterTheTestAbortsTheWholeRun) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  b.clock_secs = 3 * 3600 + 10;
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 5; }, 600000));
  const uint32_t v0 = b.test_time_436();
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_AWAKE; }, 600000));
  // answers without a bit: not yet "no plan" after two, only after three and the wait of 10 s
  b.test_bal_answer(PLAN_NONE);
  s.run(11000);
  EXPECT_EQ(b.test_db_phase(), DB_AWAKE) << "one answer is not enough";
  b.test_bal_answer(PLAN_NONE);
  b.test_bal_answer(PLAN_NONE);
  s.run(1100);
  EXPECT_EQ(b.test_db_phase(), DB_RELEASE);
  EXPECT_STREQ(b.test_db_status(), "aborted: no plan");
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_LADELOG2);
  EXPECT_FALSE(b.test_db_hold_awake());
  EXPECT_GE(b.test_time_436(), v0 + 1) << "the raised time stays (never backwards)";
  EXPECT_FALSE(b.test_nvrol_request()) << "no second cycle";
  // the limits are released once the state is as before
  b.test_pack_answered();
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_IDLE; }, 120000));
  EXPECT_FALSE(b.test_db_hold_zero());
}

TEST(TwingoDailyBalanceRun, ASilentPackCannotHoldTheLimitsAtZeroForever) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  b.clock_secs = 3 * 3600 + 10;
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_AWAKE; }, 600000));
  // no answer of 0x912B at all: after the wait + 60 s the run is aborted
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_RELEASE; }, 100000));
  EXPECT_STREQ(b.test_db_status(), "aborted: no answer of 0x912B");
  // the pack never answers: the limits are released after the walk has reached driving
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_IDLE; }, 200000));
  EXPECT_FALSE(b.test_db_hold_zero());
}

TEST(TwingoDailyBalanceRun, WithoutATestTheFirstCycleIsCycleOneAndTheSilenceIsTheDuration) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  auto& t = datalayer_extended.twingoGen1;
  t.daily_balance_test_min = 0;
  t.daily_balance_cycles = 1;
  t.daily_sleep_duration_min = 2;
  b.clock_secs = 3 * 3600 + 10;
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 5; }, 400000));
  EXPECT_EQ(b.test_db_cycle(), 1);
  const uint64_t t0 = s.now;
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 8; }, 200000)) << "wake-up burst";
  EXPECT_GE(s.now - t0, 119000u) << "the silence lasts the duration of 2 minutes";
  EXPECT_LE(s.now - t0, 122000u);
  // one cycle and no test: no check after the wake-up, straight to the release
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_RELEASE; }, 60000));
  EXPECT_STREQ(b.test_db_status(), "done: all cycles");
}

TEST(TwingoDailyBalanceRun, AnotherSleepRunIsNotDisturbedAndTheDayIsNotLost) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  b.request_sleep();  // a run of the page is going
  s.run(2000);
  b.clock_secs = 3 * 3600 + 10;
  s.run(3000);
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
  EXPECT_STREQ(b.test_db_status(), "not started: another sleep run is going");
  EXPECT_FALSE(b.test_db_hold_zero());
}

TEST(TwingoDailyBalanceRun, SleepPlus9281WritesTheTemporisationInEveryCycleWhenChosen) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  datalayer_extended.twingoGen1.daily_balance_mode = 2;
  b.clock_secs = 3 * 3600 + 10;
  const uint64_t t_fire = s.now;
  s.run(7000);
  EXPECT_EQ(b.test_db_phase(), DB_SLEEPING);
  EXPECT_EQ(b.test_nvrol_mode(), 2) << "run mode Sleep 0x9281=1";
  EXPECT_TRUE(wrote_9281(s.log, t_fire));
  // the setting is a snapshot of the start: a change during the run does not change the next cycle
  datalayer_extended.twingoGen1.daily_balance_mode = 1;
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_AWAKE; }, 600000));
  b.test_bal_answer(PLAN_96);
  s.log.clear();
  s.run(12000);
  EXPECT_EQ(b.test_db_phase(), DB_SLEEPING);
  EXPECT_EQ(b.test_nvrol_mode(), 2);
  EXPECT_TRUE(wrote_9281(s.log, 0));
}

// ---------------------------------------------------------------------------------------------------------
// Abort button
// ---------------------------------------------------------------------------------------------------------

TEST(TwingoDailyBalanceAbort, WithoutARunNothingHappensAndTheRequestIsTaken) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  datalayer_extended.twingoGen1.daily_balance_abort = true;
  s.run(2000);
  EXPECT_FALSE(datalayer_extended.twingoGen1.daily_balance_abort) << "the request is taken";
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
  EXPECT_FALSE(b.test_db_hold_zero());
  EXPECT_STREQ(b.test_db_status(), "");
  // and it does not stay for a run that starts later
  b.clock_secs = 3 * 3600 + 10;
  s.run(3000);
  EXPECT_EQ(b.test_db_phase(), DB_ZERO_WAIT) << "the run starts normally afterwards";
}

TEST(TwingoDailyBalanceAbort, BeforeTheFirstSleepEndsTheRunAtOnceAndKeepsTheCheckbox) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  b.clock_secs = 3 * 3600 + 10;
  s.run(1500);
  ASSERT_EQ(b.test_db_phase(), DB_ZERO_WAIT);
  datalayer_extended.twingoGen1.daily_balance_abort = true;
  s.run(1100);
  EXPECT_EQ(b.test_db_phase(), DB_RELEASE);
  EXPECT_STREQ(b.test_db_status(), "aborted by user");
  EXPECT_FALSE(b.test_nvrol_request()) << "no sleep was started";
  EXPECT_FALSE(wrote_9281(s.log, 31000));
  EXPECT_TRUE(datalayer_extended.twingoGen1.daily_sleep_enabled) << "the checkbox enable stays";
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_LADELOG2) << "the way of the user";
  EXPECT_TRUE(b.test_db_hold_zero()) << "released only when the state is as before";
  b.test_pack_answered();
  s.run(3000);
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
  EXPECT_FALSE(b.test_db_hold_zero());
}

TEST(TwingoDailyBalanceAbort, InTheSilenceWakesUpAtOnceWithoutCheckAndWithoutAFurtherCycle) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  datalayer_extended.twingoGen1.daily_sleep_duration_min = 30;  // a long silence: the abort must not wait for it
  datalayer_extended.twingoGen1.daily_balance_test_min = 30;
  b.clock_secs = 3 * 3600 + 10;
  ASSERT_TRUE(s.until([&] { return b.test_nvrol_state() == 5; }, 400000));
  s.run(5000);
  const uint32_t v = b.test_time_436();
  const uint64_t t_abort = s.now;
  datalayer_extended.twingoGen1.daily_balance_abort = true;
  s.run(1100);
  EXPECT_TRUE(b.test_db_user_abort());
  EXPECT_STREQ(b.test_db_status(), "abort requested: waking up");
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_RELEASE; }, 20000)) << "the wake-up burst, then the end";
  EXPECT_LT(s.now - t_abort, 20000u) << "not after the 30 minutes of the silence";
  EXPECT_STREQ(b.test_db_status(), "aborted by user");
  EXPECT_FALSE(b.test_nvrol_request()) << "no further cycle";
  EXPECT_FALSE(b.test_db_hold_awake()) << "no awake phase and no plan check";
  EXPECT_GE(b.test_time_436(), v) << "the raised time stays (never backwards)";
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_LADELOG2);
  EXPECT_TRUE(datalayer_extended.twingoGen1.daily_sleep_enabled);
  EXPECT_TRUE(b.test_db_hold_zero());
  // the walk goes to driving, then the limits are released
  b.test_pack_answered();
  ASSERT_TRUE(s.until([&] { return Bat::bus_state_shown == twingo_bus::BUS_FAHRT; }, 60000));
  s.run(1500);
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
  EXPECT_FALSE(b.test_db_hold_zero());
  // not again in the same minute (the checkbox is still on, tomorrow it runs again)
  s.run(20000);
  EXPECT_EQ(b.test_db_phase(), DB_IDLE);
}

TEST(TwingoDailyBalanceAbort, InTheShutdownWalkEndsTheRunLongBeforeTheNormalSilence) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  b.clock_secs = 3 * 3600 + 10;
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_SLEEPING; }, 20000));
  s.run(20000);  // in the shutdown walk (the silence would only start after about 190 s)
  ASSERT_NE(b.test_nvrol_state(), 5);
  const uint64_t t_abort = s.now;
  datalayer_extended.twingoGen1.daily_balance_abort = true;
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_RELEASE; }, 60000));
  EXPECT_LT(s.now - t_abort, 20000u);
  EXPECT_STREQ(b.test_db_status(), "aborted by user");
  EXPECT_FALSE(b.test_nvrol_request());
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_LADELOG2);
}

TEST(TwingoDailyBalanceAbort, WhileAwakeBetweenTwoCyclesEndsTheRunAndTheNextCycleDoesNotStart) {
  BalanceTwingo b;
  Sim s(b);
  prepare(b, s);
  b.clock_secs = 3 * 3600 + 10;
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_AWAKE; }, 600000));
  s.run(2000);
  datalayer_extended.twingoGen1.daily_balance_abort = true;
  s.run(1100);
  EXPECT_EQ(b.test_db_phase(), DB_RELEASE);
  EXPECT_STREQ(b.test_db_status(), "aborted by user");
  EXPECT_FALSE(b.test_db_hold_awake()) << "the bus goes back to the normal wake-up walk";
  b.test_bal_answer(PLAN_96);  // even a plan does not start the next cycle any more
  b.test_pack_answered();
  s.run(15000);
  EXPECT_FALSE(b.test_nvrol_request());
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_LADELOG2);
  ASSERT_TRUE(s.until([&] { return b.test_db_phase() == DB_IDLE; }, 60000));
  EXPECT_FALSE(b.test_db_hold_zero());
}
