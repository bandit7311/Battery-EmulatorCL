#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"

#include "Arduino.h"

// TX frame capture injected by the emulated CAN layer (see emul/can.cpp).
void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Tests of the 04.10. changes: the mode "like the car" (shutdown sequence, wake-up, 0x214, end of the rows), the
// seven new rows, the vehicle ID, the smooth vehicle age. The reference values are copied from the real vehicle
// logs of 02.10. and 04.10. (canmitlog.log, canmitlog_tw2.log); the numbers are explained at the code they test
// (RENAULT-TWINGO-GEN1-BATTERY.h/.cpp: POWERDOWN_*_CAR, WAKE_STEPS_CAR, SimEnd, sim_signals).

namespace {

class TestTwingo : public RenaultTwingoGen1Battery {
 public:
  bool unix_set = true;  // the vehicle age needs a valid clock: default = frozen at the seed reference
  time_t unix_now = 1791319221;
  TestTwingo() {  // the age state is global: every test starts from the seed, automatic mode, nothing sent yet
    RenaultTwingoGen1Battery::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1311344;
    t.age_pack_unix = 1791319221;
    t.age_last_sent = 0;
    t.age_source = 0;
  }
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t& now_utc) override {
    if (!unix_set) {
      return false;
    }
    now_utc = unix_now;
    return true;
  }
  bool get_wall_clock_seconds_of_day(uint32_t& secs) override {
    secs = 12 * 3600;
    return true;
  }
};

// The two runtime switches and the simulator mask are global; every test puts them back (tests run in one process).
struct Switches {
  Switches(bool like_car) { RenaultTwingoGen1Battery::shutdown_like_car = like_car; }
  ~Switches() {
    RenaultTwingoGen1Battery::shutdown_like_car = false;
    datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  }
};

struct Tx {
  uint64_t t;
  CAN_frame f;
};

void tick_log(RenaultTwingoGen1Battery& b, uint64_t t, std::vector<Tx>& log) {
  set_millis64(t);
  clear_transmitted_frames();
  b.transmit_can((unsigned long)t);
  for (const CAN_frame& f : get_transmitted_frames()) {
    log.push_back({t, f});
  }
}

void run(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms, uint64_t step, std::vector<Tx>& log) {
  for (uint64_t end = t + ms; t < end; t += step) {
    tick_log(b, t, log);
  }
}

std::vector<Tx> with_id(const std::vector<Tx>& v, uint32_t id) {
  std::vector<Tx> out;
  for (const Tx& x : v) {
    if (x.f.ID == id && !x.f.ext_ID) {
      out.push_back(x);
    }
  }
  return out;
}

bool bytes_are(const CAN_frame& f, std::initializer_list<uint8_t> expected) {
  if (f.DLC != expected.size()) {
    return false;
  }
  uint8_t i = 0;
  for (uint8_t b : expected) {
    if (f.data.u8[i++] != b) {
      return false;
    }
  }
  return true;
}

// Bytes 4-7 of a 0x350 frame as one number, e.g. 0x14149445.
uint32_t tail350(const CAN_frame& f) {
  return ((uint32_t)f.data.u8[4] << 24) | ((uint32_t)f.data.u8[5] << 16) | ((uint32_t)f.data.u8[6] << 8) | f.data.u8[7];
}

int row_of(uint32_t id) {
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    if (RenaultTwingoGen1Battery::sim_signals[i].id == id) {
      return i;
    }
  }
  return -1;
}

uint64_t all_rows_mask() {
  return (1ULL << RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT) - 1;
}

// A complete "Sleep" run (all stages and the silence) in steps of `step` ms. Returns the log of the run.
struct SleepRun {
  std::vector<Tx> log;
  uint64_t start = 0;
  uint64_t t_first_c3 = 0, t_c2 = 0, t_c0 = 0, t_00 = 0, t_last_350 = 0;
};

SleepRun run_sleep(TestTwingo& b, uint64_t& t, uint64_t step) {
  SleepRun r;
  std::vector<Tx> warm;
  run(b, t, 500, step, warm);
  b.request_sleep();
  r.start = t;
  run(b, t, 140000, step, r.log);
  for (const Tx& x : with_id(r.log, 0x350)) {
    uint8_t s = x.f.data.u8[0];
    if (s == 0xC3 && r.t_first_c3 == 0) {
      r.t_first_c3 = x.t;
    }
    if (s == 0xC2 && r.t_c2 == 0) {
      r.t_c2 = x.t;
    }
    if (s == 0xC0 && r.t_c0 == 0) {
      r.t_c0 = x.t;
    }
    if (s == 0x00 && r.t_00 == 0) {
      r.t_00 = x.t;
    }
    r.t_last_350 = x.t;
  }
  return r;
}

const time_t AGE_EPOCH = 1613387588;  // 15.02.2021 11:13:08 UTC = zero of the 0x350 vehicle age counter

// The old KWP poll of the base class (0x79B, UdsCanBattery::transmit_uds_can() at the end of transmit_can()) is not
// part of the 04.10. changes: it keeps running through the shutdown in both modes and is left out of the checks of
// "which frames are on the bus". In the real car the last frames of the other ECUs come up to 60 ms after the last
// 0x350 (log of 02.10.: 0x350 +0.90 s, 0x5DE +0.93 s, 0x657 +0.95 s), so one 100 ms period is allowed.
const uint64_t AFTER_LAST_350_MS = 100;
bool is_old_kwp_poll(const CAN_frame& f) {
  return f.ID == 0x79B && !f.ext_ID;
}

uint8_t sum_complement(const std::vector<uint8_t>& d) {
  uint8_t sum = 0;
  for (uint8_t b : d) {
    sum = (uint8_t)(sum + b);
  }
  return (uint8_t)~sum;
}

}  // namespace

// ---------------------------------------------------------------------------
// Table and switches
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, TableHas35RowsAndTheSevenNewOnesAreAtTheEnd) {
  ASSERT_EQ((int)RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT, 40);
  const uint32_t expected[7] = {0x0C6, 0x12E, 0x29A, 0x29C, 0x2B7, 0x45C, 0x657};
  for (int i = 0; i < 7; i++) {
    const auto& s = RenaultTwingoGen1Battery::sim_signals[28 + i];
    EXPECT_EQ(s.id, expected[i]) << "row " << 28 + i;
    EXPECT_EQ(s.tag, 'A');
    EXPECT_FALSE(s.not_in_vehicle_log);
    EXPECT_GT(strlen(s.sender), 0u);
    EXPECT_GT(strlen(s.info), 10u);
  }
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[28].interval_ms, 10);
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[29].interval_ms, 10);
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[30].interval_ms, 20);
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[33].interval_ms, 100);
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[34].interval_ms, 100);
}

TEST(TwingoCarMode, NewRowsAreOffByDefaultAndEveryRowHasAnEndStage) {
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  for (int row = 28; row < 35; row++) {
    EXPECT_FALSE(RenaultTwingoGen1Battery::sim_row_enabled(row)) << "row " << row;
  }
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    uint8_t e = RenaultTwingoGen1Battery::sim_signals[i].end_stage;
    EXPECT_LE(e, (uint8_t)RenaultTwingoGen1Battery::SIM_END_BUS) << "row " << i;
  }
  // the four Zoe frames are not part of the car: they keep the old rule
  for (uint32_t id : {0x19Fu, 0x426u, 0x436u, 0x423u}) {
    EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[row_of(id)].end_stage, RenaultTwingoGen1Battery::SIM_END_LEGACY);
  }
}

TEST(TwingoCarMode, RowSwitchesWorkAbove32Bits) {
  Switches g(false);
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  RenaultTwingoGen1Battery::sim_row_set(34, true);
  RenaultTwingoGen1Battery::sim_row_set(32, true);
  EXPECT_TRUE(RenaultTwingoGen1Battery::sim_row_enabled(34));
  EXPECT_TRUE(RenaultTwingoGen1Battery::sim_row_enabled(32));
  EXPECT_FALSE(RenaultTwingoGen1Battery::sim_row_enabled(33));
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 0x3FFULL | (1ULL << 34) | (1ULL << 32));
  RenaultTwingoGen1Battery::sim_row_set(34, false);
  EXPECT_FALSE(RenaultTwingoGen1Battery::sim_row_enabled(34));
  EXPECT_TRUE(RenaultTwingoGen1Battery::sim_row_enabled(9));  // the old bits are untouched
}

TEST(TwingoCarMode, TheSwitchIsOffAtStart) {
  EXPECT_FALSE(RenaultTwingoGen1Battery::shutdown_like_car);
}

// ---------------------------------------------------------------------------
// Vehicle ID 0x69F
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, VehicleIdIsTheOneOfTheUsersCarInBothModes) {
  for (int like_car = 0; like_car < 2; like_car++) {
    Switches g(like_car == 1);
    datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
    TestTwingo b;
    b.setup();
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 3500, 10, log);
    auto f = with_id(log, 0x69F);
    ASSERT_GE(f.size(), 2u) << "mode " << like_car;
    for (const Tx& x : f) {
      EXPECT_TRUE(bytes_are(x.f, {0x46, 0x13, 0x88, 0x6F})) << "mode " << like_car;
    }
  }
}

// ---------------------------------------------------------------------------
// Shutdown sequence "like the car" (log of 04.10., second shutdown)
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, ShutdownStageTimesAndBytes) {
  Switches g(true);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 10);
  ASSERT_GT(r.t_first_c3, 0u);
  ASSERT_GT(r.t_c2, 0u);
  ASSERT_GT(r.t_c0, 0u);
  ASSERT_GT(r.t_00, 0u);

  // Stage lengths (the next stage starts with its first frame, up to one 100 ms slot after the switch).
  EXPECT_NEAR((double)(r.t_c2 - r.t_first_c3), 63200.0, 130.0) << "C3 = 63.2 s";
  EXPECT_NEAR((double)(r.t_c0 - r.t_c2), 60000.0, 130.0) << "C2 = 60.0 s";
  EXPECT_NEAR((double)(r.t_00 - r.t_c0), 10000.0, 130.0) << "C0 = 10.0 s";
  EXPECT_NEAR((double)(r.t_last_350 - r.t_00), 800.0, 110.0)
      << "00 = 0.9 s, last frame about 0.8-0.9 s after the first";

  auto f = with_id(r.log, 0x350);
  size_t c0_frames = 0, n00 = 0;
  for (const Tx& x : f) {
    uint8_t s = x.f.data.u8[0];
    if (s == 0xC3) {
      uint64_t dt = x.t - r.t_first_c3;
      if (dt < 3000) {
        EXPECT_EQ(tail350(x.f), 0x14149445u) << "C3 part 1 at +" << dt;
      } else if (dt > 3300) {
        EXPECT_EQ(tail350(x.f), 0x14149645u) << "C3 part 2 at +" << dt;
      }
    } else if (s == 0xC2) {
      EXPECT_EQ(tail350(x.f), 0x14149645u);
    } else if (s == 0xC0) {
      c0_frames++;
      if (c0_frames == 1) {
        EXPECT_EQ(tail350(x.f), 0x14109645u) << "first C0 frame";
      } else if (c0_frames <= 3) {
        EXPECT_EQ(tail350(x.f), 0x14709645u) << "C0 frame " << c0_frames;
      } else {
        EXPECT_EQ(tail350(x.f), 0x14709685u) << "C0 frame " << c0_frames;
      }
    } else if (s == 0x00) {
      n00++;
      EXPECT_EQ(tail350(x.f), 0x14709685u);
    }
  }
  EXPECT_GE(n00, 9u);
  EXPECT_LE(n00, 10u);

  // The three C0 entry frames come within about 30 ms, then the normal 100 ms rhythm.
  std::vector<Tx> c0;
  for (const Tx& x : f) {
    if (x.f.data.u8[0] == 0xC0) {
      c0.push_back(x);
    }
  }
  ASSERT_GT(c0.size(), 6u);
  EXPECT_LE(c0[1].t - c0[0].t, 20u);
  EXPECT_LE(c0[2].t - c0[1].t, 20u);
  EXPECT_LE(c0[2].t - c0[0].t, 30u);
  EXPECT_GE(c0[3].t - c0[2].t, 100u);
  EXPECT_LE(c0[3].t - c0[2].t, 130u);
  EXPECT_EQ(c0[4].t - c0[3].t, 100u);

  // Silence afterwards: nothing after the last 0x350 of the sequence (apart from the last frames of the 00 stage).
  for (const Tx& x : r.log) {
    if (!is_old_kwp_poll(x.f)) {
      EXPECT_LE(x.t, r.t_last_350 + AFTER_LAST_350_MS) << "frame after the sequence: ID 0x" << std::hex << x.f.ID;
    }
  }
}

TEST(TwingoCarMode, ShutdownAsBeforeIsUnchanged) {
  Switches g(false);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 100);
  EXPECT_NEAR((double)(r.t_c2 - r.t_first_c3), 66000.0, 130.0);
  EXPECT_NEAR((double)(r.t_c0 - r.t_c2), 60000.0, 130.0);
  EXPECT_NEAR((double)(r.t_00 - r.t_c0), 10000.0, 130.0);
  EXPECT_NEAR((double)(r.t_last_350 - r.t_00), 900.0, 110.0);
  std::vector<Tx> c0;
  for (const Tx& x : with_id(r.log, 0x350)) {
    uint8_t s = x.f.data.u8[0];
    if (s == 0xC3 || s == 0xC2) {
      EXPECT_EQ(tail350(x.f), 0x14149645u);
    } else {
      EXPECT_EQ(tail350(x.f), 0x14709685u);
    }
    if (s == 0xC0) {
      c0.push_back(x);
    }
  }
  ASSERT_GT(c0.size(), 3u);
  EXPECT_EQ(c0[1].t - c0[0].t, 100u) << "no entry burst in the mode as before";
}

// ---------------------------------------------------------------------------
// Wake-up "like the car" (log of 04.10., first wake-up)
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, WakeUpSequenceHasTheTwelveSteps) {
  Switches g(true);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 10);
  ASSERT_GT(r.t_00, 0u);
  b.request_wake_up();
  std::vector<Tx> w;
  run(b, t, 14000, 10, w);

  struct Step {
    uint8_t byte0;
    uint32_t tail;
    int count;
  };
  const Step steps[12] = {{0xC0, 0x14709685, 2},  {0xC3, 0x14709685, 2},  {0xC3, 0x14109645, 4},
                          {0xC3, 0x14109445, 7},  {0xC3, 0x1410A445, 10}, {0xC3, 0x1414A445, 20},
                          {0xC3, 0x14149445, 20}, {0xC4, 0x14149445, 3},  {0xC5, 0x14149445, 4},
                          {0xC6, 0x14109445, 4},  {0xC7, 0x14109445, 17}, {0xC7, 0x14909445, 6}};
  auto f = with_id(w, 0x350);
  size_t total = 0;
  for (const Step& s : steps) {
    total += s.count;
  }
  ASSERT_EQ(total, 99u);
  ASSERT_GE(f.size(), total + 5);
  size_t k = 0;
  for (int si = 0; si < 12; si++) {
    for (int i = 0; i < steps[si].count; i++, k++) {
      EXPECT_EQ(f[k].f.data.u8[0], steps[si].byte0) << "step " << si + 1 << " frame " << i;
      EXPECT_EQ(tail350(f[k].f), steps[si].tail) << "step " << si + 1 << " frame " << i;
    }
  }
  // Rhythm: the first four frames about 50 ms apart, then 100 ms.
  EXPECT_EQ(f[1].t - f[0].t, 50u);
  EXPECT_EQ(f[2].t - f[1].t, 50u);
  EXPECT_EQ(f[3].t - f[2].t, 50u);
  for (size_t i = 4; i < total; i++) {
    EXPECT_EQ(f[i].t - f[i - 1].t, 100u) << "frame " << i;
  }
  EXPECT_NEAR((double)(f[total - 1].t - f[0].t), 9650.0, 60.0) << "about 10 s in all";
  // Afterwards the normal run frame (C7 .. 14 98 94 45) every 100 ms, as before.
  for (size_t i = total; i < f.size(); i++) {
    EXPECT_EQ(f[i].f.data.u8[0], 0xC7);
    EXPECT_EQ(tail350(f[i].f), 0x14989445u);
  }
}

TEST(TwingoCarMode, WakeUpAsBeforeIsStillTheShortBurst) {
  Switches g(false);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 100);
  ASSERT_GT(r.t_00, 0u);
  b.request_wake_up();
  std::vector<Tx> w;
  run(b, t, 4000, 100, w);
  auto f = with_id(w, 0x350);
  ASSERT_GE(f.size(), 11u);
  EXPECT_EQ(f[0].f.data.u8[0], 0xC0);
  for (int i = 1; i <= 10; i++) {
    EXPECT_EQ(f[i].f.data.u8[0], 0xC3);
  }
  EXPECT_EQ(f[1].t - f[0].t, 200u);
}

// ---------------------------------------------------------------------------
// 0x214
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, Frame214StartSequenceAwakeAndShutdown) {
  Switches g(true);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> boot;
  run(b, t, 1000, 10, boot);
  auto b214 = with_id(boot, 0x214);
  ASSERT_GE(b214.size(), 14u);
  EXPECT_TRUE(bytes_are(b214[0].f, {0xFB, 0xFE})) << "first frame after the start";
  for (int i = 1; i <= 10; i++) {
    EXPECT_TRUE(bytes_are(b214[i].f, {0xF8, 0x3E})) << "frame " << i;
  }
  for (size_t i = 11; i < b214.size(); i++) {
    EXPECT_TRUE(bytes_are(b214[i].f, {0x08, 0x02})) << "awake, frame " << i;
  }
  for (size_t i = 1; i < b214.size(); i++) {
    EXPECT_EQ(b214[i].t - b214[i - 1].t, 20u) << "every 20 ms";
  }

  // Shutdown: 08 02 in C3 and C2, F8 3E in C0, nothing in 00.
  SleepRun r;
  b.request_sleep();
  r.start = t;
  run(b, t, 140000, 10, r.log);
  uint64_t t_c2 = 0, t_c0 = 0, t_00 = 0;
  for (const Tx& x : with_id(r.log, 0x350)) {
    uint8_t s = x.f.data.u8[0];
    if (s == 0xC2 && !t_c2)
      t_c2 = x.t;
    if (s == 0xC0 && !t_c0)
      t_c0 = x.t;
    if (s == 0x00 && !t_00)
      t_00 = x.t;
  }
  ASSERT_TRUE(t_c2 && t_c0 && t_00);
  size_t n_c3c2 = 0, n_c0 = 0;
  for (const Tx& x : with_id(r.log, 0x214)) {
    if (x.t + 40 < t_c0) {
      n_c3c2++;
      EXPECT_TRUE(bytes_are(x.f, {0x08, 0x02})) << "C3/C2 at " << x.t;
    } else if (x.t > t_c0 + 40 && x.t + 40 < t_00) {
      n_c0++;
      EXPECT_TRUE(bytes_are(x.f, {0xF8, 0x3E})) << "C0 at " << x.t;
    } else if (x.t >= t_00 + 40) {
      ADD_FAILURE() << "0x214 in stage 00 at " << x.t;
    }
  }
  EXPECT_GT(n_c3c2, 6000u);
  EXPECT_GT(n_c0, 400u);  // 10 s at 20 ms

  // Wake-up: starts again with FB FE (after the first two wake frames), ten times F8 3E, then 08 02.
  b.request_wake_up();
  std::vector<Tx> w;
  run(b, t, 3000, 10, w);
  auto w214 = with_id(w, 0x214);
  auto w350 = with_id(w, 0x350);
  ASSERT_GE(w214.size(), 12u);
  ASSERT_GE(w350.size(), 2u);
  EXPECT_TRUE(bytes_are(w214[0].f, {0xFB, 0xFE}));
  EXPECT_GE(w214[0].t, w350[1].t) << "not before the second wake frame";
  EXPECT_LE(w214[0].t, w350[1].t + 40);
  for (int i = 1; i <= 10; i++) {
    EXPECT_TRUE(bytes_are(w214[i].f, {0xF8, 0x3E}));
  }
  EXPECT_TRUE(bytes_are(w214[11].f, {0x08, 0x02}));
}

TEST(TwingoCarMode, Frame214SwitchOffMeansNoFrameAtAll) {
  Switches g(true);
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF & ~(1ULL << row_of(0x214));
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 100);
  EXPECT_TRUE(with_id(r.log, 0x214).empty());
  std::vector<Tx> w;
  b.request_wake_up();
  run(b, t, 12000, 100, w);
  EXPECT_TRUE(with_id(w, 0x214).empty());
}

TEST(TwingoCarMode, Frame214AsBeforeIsOnlyInTheShutdownAndKeepsItsOldBytes) {
  Switches g(false);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> boot;
  run(b, t, 3000, 100, boot);
  EXPECT_TRUE(with_id(boot, 0x214).empty());
  b.request_sleep();
  std::vector<Tx> sl;
  run(b, t, 140000, 100, sl);
  bool saw0800 = false, sawF83E = false;
  for (const Tx& x : with_id(sl, 0x214)) {
    saw0800 |= bytes_are(x.f, {0x08, 0x00});
    sawF83E |= bytes_are(x.f, {0xF8, 0x3E});
    EXPECT_FALSE(bytes_are(x.f, {0xFB, 0xFE}));
    EXPECT_FALSE(bytes_are(x.f, {0x08, 0x02}));
  }
  EXPECT_TRUE(saw0800);
  EXPECT_TRUE(sawF83E);
}

// ---------------------------------------------------------------------------
// End of the rows (SimEnd)
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, EveryRowEndsWhereItEndsInTheCar) {
  Switches g(true);
  datalayer_extended.twingoGen1.simulator_enabled_mask = all_rows_mask();
  datalayer.battery.status.voltage_dV = 3840;  // 0x42E only sends while the pack voltage is known
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 10);
  ASSERT_GT(r.t_00, 0u);

  const uint32_t at_c0[] = {0x090, 0x242, 0x1F6, 0x217, 0x0C6, 0x12E, 0x29A, 0x29C, 0x2B7};
  const uint32_t at_00[] = {0x17A, 0x17E, 0x186, 0x18A, 0x1F8, 0x211, 0x1FD, 0x427, 0x42E, 0x432, 0x634, 0x650, 0x53B};
  const uint32_t bus[] = {0x1B0, 0x55D, 0x5DE, 0x5DF, 0x45C, 0x657};

  auto last_t = [&](uint32_t id) {
    auto v = with_id(r.log, id);
    return v.empty() ? (uint64_t)0 : v.back().t;
  };
  for (uint32_t id : at_c0) {
    uint64_t l = last_t(id);
    ASSERT_GT(l, 0u) << "0x" << std::hex << id << " never sent";
    EXPECT_GT(l, r.t_c2) << "0x" << std::hex << id << " runs in C2";
    EXPECT_LE(l, r.t_c0 + 20) << "0x" << std::hex << id << " must end with the C0 stage";
  }
  for (uint32_t id : at_00) {
    uint64_t l = last_t(id);
    ASSERT_GT(l, 0u) << "0x" << std::hex << id << " never sent";
    EXPECT_GT(l, r.t_c0 + 5000) << "0x" << std::hex << id << " runs through C0";
    EXPECT_LE(l, r.t_00 + 20) << "0x" << std::hex << id << " must end at the change to 00";
  }
  for (uint32_t id : bus) {
    uint64_t l = last_t(id);
    ASSERT_GT(l, 0u) << "0x" << std::hex << id << " never sent";
    EXPECT_GT(l, r.t_00 + 400) << "0x" << std::hex << id << " runs through the 00 stage";
    EXPECT_LE(l, r.t_last_350 + AFTER_LAST_350_MS) << "0x" << std::hex << id << " must not outlast the bus";
  }
  // the Zoe frames and 0x214 stay out of the 00 stage
  for (uint32_t id : {0x19Fu, 0x426u, 0x436u, 0x423u, 0x214u}) {
    EXPECT_LE(last_t(id), r.t_00 + 20) << "0x" << std::hex << id;
  }
  for (const Tx& x : r.log) {
    if (!is_old_kwp_poll(x.f)) {
      EXPECT_LE(x.t, r.t_last_350 + AFTER_LAST_350_MS) << "frame after the end of the bus: ID 0x" << std::hex << x.f.ID;
    }
  }
}

TEST(TwingoCarMode, EndOfTheRowsAsBeforeIsUnchanged) {
  Switches g(false);
  datalayer_extended.twingoGen1.simulator_enabled_mask = all_rows_mask();
  datalayer.battery.status.voltage_dV = 3840;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 10);
  ASSERT_GT(r.t_00, 0u);
  auto last_t = [&](uint32_t id) {
    auto v = with_id(r.log, id);
    return v.empty() ? (uint64_t)0 : v.back().t;
  };
  // as before every simulator row and the two fast frames stop with the C0 stage ...
  for (uint32_t id :
       {0x090u, 0x242u, 0x17Au, 0x18Au, 0x1F8u, 0x211u, 0x1B0u, 0x55Du, 0x5DEu, 0x1FDu, 0x0C6u, 0x45Cu, 0x657u}) {
    EXPECT_LE(last_t(id), r.t_c0 + 20) << "0x" << std::hex << id;
  }
  // ... the I rows (0x53B, 0x69F, Zoe frames) run until the 00 stage, nothing in the 00 stage
  for (const Tx& x : r.log) {
    if (x.t > r.t_00 + 20 && x.f.ID != 0x350 && !is_old_kwp_poll(x.f)) {
      ADD_FAILURE() << "0x" << std::hex << x.f.ID << " in the 00 stage as before";
    }
  }
}

TEST(TwingoCarMode, AllRowsOffMeansOnlyTheSleepProtocolItself) {
  Switches g(true);
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 100);
  std::set<uint32_t> ids;
  for (const Tx& x : r.log) {
    if (!is_old_kwp_poll(x.f)) {
      ids.insert(x.f.ID);
    }
  }
  EXPECT_EQ(ids, std::set<uint32_t>({0x350}));
}

// ---------------------------------------------------------------------------
// The seven new rows
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, ChecksumFormulaMatchesRealVehicleFrames) {
  // Frames copied from canmitlog_tw2.log (02.10.): byte 7 = complement of the sum of bytes 0-6.
  EXPECT_EQ(sum_complement({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C}), 0xF3);  // 0x29A  00 .. 0C F3
  EXPECT_EQ(sum_complement({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09}), 0xF6);  // 0x29A  00 .. 09 F6 (last frame)
  EXPECT_EQ(sum_complement({0x89, 0x6A, 0x80, 0x00, 0x80, 0x01, 0xA4}), 0x67);  // 0x0C6
  EXPECT_EQ(sum_complement({0x89, 0x6A, 0x7F, 0xF8, 0x80, 0x01, 0xAA}), 0x6A);  // 0x0C6
  EXPECT_EQ(sum_complement({0x7F, 0xA4, 0x80, 0x00, 0x80, 0x01, 0xA4}), 0x37);  // 0x0C6 last frame of the log
}

TEST(TwingoCarMode, Frame0C6HasCounterAndChecksum) {
  Switches g(false);
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF | (1ULL << row_of(0x0C6));
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 10, log);
  auto f = with_id(log, 0x0C6);
  ASSERT_GE(f.size(), 95u);
  for (size_t i = 0; i < f.size(); i++) {
    EXPECT_EQ(f[i].f.DLC, 8);
    EXPECT_EQ(f[i].f.data.u8[0], 0x7F);
    EXPECT_EQ(f[i].f.data.u8[5], 0x01);
    EXPECT_EQ(f[i].f.data.u8[6], (uint8_t)(0xA0 + 2 * (i % 16))) << "counter A0, A2 ... BE, then A0 again, frame " << i;
    EXPECT_EQ(f[i].f.data.u8[7],
              sum_complement({f[i].f.data.u8[0], f[i].f.data.u8[1], f[i].f.data.u8[2], f[i].f.data.u8[3],
                              f[i].f.data.u8[4], f[i].f.data.u8[5], f[i].f.data.u8[6]}))
        << "frame " << i;
  }
  EXPECT_EQ(f[1].t - f[0].t, 10u);
}

TEST(TwingoCarMode, Frame29AHasCounterAndChecksum) {
  Switches g(false);
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF | (1ULL << row_of(0x29A));
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 10, log);
  auto f = with_id(log, 0x29A);
  ASSERT_GE(f.size(), 45u);
  for (size_t i = 0; i < f.size(); i++) {
    EXPECT_EQ(f[i].f.DLC, 8);
    for (int k = 0; k < 6; k++) {
      EXPECT_EQ(f[i].f.data.u8[k], 0x00);
    }
    EXPECT_EQ(f[i].f.data.u8[6], (uint8_t)(i % 16)) << "counter 0-15, frame " << i;
    EXPECT_EQ(f[i].f.data.u8[7], (uint8_t)~(i % 16)) << "frame " << i;
  }
  EXPECT_EQ(f[1].t - f[0].t, 20u);
}

TEST(TwingoCarMode, FixedRowsCarryTheStandstillValuesOfTheLog) {
  Switches g(false);
  uint64_t mask = 0x3FF;
  for (uint32_t id : {0x12Eu, 0x29Cu, 0x2B7u, 0x45Cu, 0x657u}) {
    mask |= 1ULL << row_of(id);
  }
  datalayer_extended.twingoGen1.simulator_enabled_mask = mask;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 10, log);
  for (const Tx& x : with_id(log, 0x12E)) {
    EXPECT_TRUE(bytes_are(x.f, {0xC3, 0x7F, 0xF9, 0x7F, 0xF0, 0xFF, 0xFF, 0x00}));
  }
  for (const Tx& x : with_id(log, 0x29C)) {
    EXPECT_TRUE(bytes_are(x.f, {0, 0, 0, 0, 0, 0, 0xFF, 0xFF}));
  }
  for (const Tx& x : with_id(log, 0x2B7)) {
    EXPECT_TRUE(bytes_are(x.f, {0x00, 0xE0, 0xFF, 0xFE, 0x11}));
  }
  for (const Tx& x : with_id(log, 0x45C)) {
    EXPECT_TRUE(bytes_are(x.f, {0, 0, 0, 0xFE, 0, 0, 0, 0}));
  }
  for (const Tx& x : with_id(log, 0x657)) {
    EXPECT_TRUE(bytes_are(x.f, {0xC0, 0x40, 0x00}));
  }
  EXPECT_GE(with_id(log, 0x12E).size(), 95u);
  EXPECT_GE(with_id(log, 0x29C).size(), 45u);
  EXPECT_GE(with_id(log, 0x45C).size(), 8u);
}

// ---------------------------------------------------------------------------
// Helper for the vehicle age frames
// ---------------------------------------------------------------------------

namespace {
uint32_t age_of(const CAN_frame& f) {
  return ((uint32_t)f.data.u8[1] << 16) | ((uint32_t)f.data.u8[2] << 8) | f.data.u8[3];
}
}  // namespace

// ---------------------------------------------------------------------------
// Point 15: the HV rows follow the shutdown sequence (C3 = disconnect)
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, HvRowsFollowTheShutdownSequence) {
  Switches g(true);
  datalayer.battery.status.voltage_dV = 3390;
  datalayer.battery.status.current_dA = 0;
  datalayer_extended.twingoGen1.simulator_enabled_mask =
      0x3FFULL | (1ULL << row_of(0x57F)) | (1ULL << row_of(0x599)) | (1ULL << row_of(0x62D)) | (1ULL << row_of(0x1FD));
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r;
  std::vector<Tx> warm;
  run(b, t, 12000, 10, warm);  // steady operation: HV closed, inverter on
  b.request_sleep();
  run(b, t, 30000, 10, r.log);
  for (const Tx& x : with_id(r.log, 0x350)) {
    if (x.f.data.u8[0] == 0xC3) {
      r.t_first_c3 = x.t;
      break;
    }
  }
  ASSERT_GT(r.t_first_c3, 0u);
  r.log.insert(r.log.begin(), warm.begin(), warm.end());
  auto f62 = with_id(r.log, 0x62D);
  auto f1f = with_id(r.log, 0x1FD);
  auto f57 = with_id(r.log, 0x57F);
  auto f59 = with_id(r.log, 0x599);
  ASSERT_FALSE(f62.empty());
  bool saw_closed = false, saw_open = false, saw_inv_off = false;
  for (const Tx& x : f62) {
    if (x.t > 3000 &&
        x.t + 100 < r.t_first_c3) {  // after the two first frames, before the disconnect: HV closed (02), inverter on
      EXPECT_EQ(x.f.data.u8[3], 0x02) << "at " << x.t;
      saw_closed = true;
    } else if (x.t > r.t_first_c3 + 2800) {  // after HV opened (C3 + 2.2 s): 04
      EXPECT_EQ(x.f.data.u8[3], 0x04) << "at " << x.t;
      EXPECT_EQ(x.f.data.u8[5], 0x00);
      saw_open = true;
    }
  }
  for (const Tx& x : f59) {
    if (x.t > r.t_first_c3 + 1200 && x.f.data.u8[1] != 0x07) {
      EXPECT_EQ(x.f.data.u8[1], 0x04);  // inverter off 1.0 s after C3
      saw_inv_off = true;
    }
  }
  EXPECT_TRUE(saw_closed);
  EXPECT_TRUE(saw_open);
  EXPECT_TRUE(saw_inv_off);
  for (const Tx& x : f1f) {
    if (x.t > r.t_first_c3 + 2800) {
      EXPECT_EQ(x.f.data.u8[0], 0xFE);
      EXPECT_EQ(x.f.data.u8[5], 0x50);
    }
  }
  // the voltage decays after the opening and never rises again
  uint16_t last = 0xFFFF;
  for (const Tx& x : f57) {
    if (x.t > r.t_first_c3 + 2800) {
      uint16_t v = (uint16_t)(((x.f.data.u8[1] & 0x1F) << 8) | x.f.data.u8[2]);
      EXPECT_LE(v, last);
      last = v;
    }
  }
  EXPECT_LT(last, 3390);
}
