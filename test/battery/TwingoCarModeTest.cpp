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
  time_t unix_now = 1791590400;
  TestTwingo() {  // the age state is global: every test starts from the seed, automatic mode, nothing sent yet
    RenaultTwingoGen1Battery::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.bus_format_zoe_old = true;  // the legacy tests check the old Zoe form
    t.bus_state_manual = true;    // the tests choose the state themselves
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
  run(b, t, 200000, step, r.log);
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
// Bau 7: after the 00 stage of 0x350 the closing stage of the bus goes on (long way: as in the drive log 50.82 s)
uint64_t end_of_bus(const SleepRun& r) {
  return r.t_00 + twingo_bus::closing_stage_ms(twingo_bus::SLEEP_LANG, true) + AFTER_LAST_350_MS;
}
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

// ---------------------------------------------------------------------------
// Wake-up "like the car" (log of 04.10., first wake-up)
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 0x214
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// End of the rows (SimEnd)
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, AllRowsOffMeansNoFrameAtAllNotEvenTheSleepProtocol) {
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
  // Bau 6: row 0x350 is off as well, so the sleep protocol of 0x350 is not sent either: the bus stays silent
  EXPECT_TRUE(ids.empty());
}

// ---------------------------------------------------------------------------
// The seven new rows
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Bau 13 (14.10.): the frames of the removed Block 2 never appear, the internal sleep / wake clock still runs
// ---------------------------------------------------------------------------

TEST(TwingoCarMode, RemovedFramesNeverAppearEvenWithAllRowsOnAndTheClockStillRuns) {
  Switches g(true);
  datalayer_extended.twingoGen1.simulator_enabled_mask = all_rows_mask();
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  SleepRun r = run_sleep(b, t, 100);
  std::vector<Tx> wake;
  b.request_wake_up();
  run(b, t, 15000, 100, wake);
  for (const std::vector<Tx>* log : {&r.log, &wake}) {
    for (const Tx& x : *log) {
      if (!x.f.ext_ID) {
        for (uint32_t id : {0x090u, 0x242u, 0x350u, 0x53Bu, 0x214u}) {
          EXPECT_NE(x.f.ID, id) << "removed frame 0x" << std::hex << id;
        }
      }
    }
  }
  // the sleep run ends in the true silence: the last 10 s of the 200 s run carry no frame (apart from the old poll)
  for (const Tx& x : r.log) {
    if (!is_old_kwp_poll(x.f)) {
      EXPECT_LT(x.t, r.start + 190000) << "frame in the silence: ID 0x" << std::hex << x.f.ID;
    }
  }
  // the wake-up brings the vehicle frames of the bus back
  EXPECT_FALSE(with_id(wake, 0x19F).empty());
  EXPECT_FALSE(with_id(wake, 0x426).empty());
}
