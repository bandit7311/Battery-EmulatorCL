#include <gtest/gtest.h>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "../../Software/src/devboard/webserver/simulator_html.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// 13.10.: the three ways the bus walks down at Sleep (long, short, end of the charge log 445a1527), the order of the
// last frames of the way "Ladelog 2" (0x157 and 0x0ED last), the state "charging" and the wake-up through the plug.
// Times from the car logs: 22aaf176 (PuTTY, gaps) and 445a1527 (Tera Term, complete).

namespace {

using Bat = RenaultTwingoGen1Battery;

class PathTwingo : public RenaultTwingoGen1Battery {
 public:
  PathTwingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.bus_format_zoe_old = false;
    t.bus_state_manual = true;
    t.bus_state = twingo_bus::BUS_WACH;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.bus_wake_target = twingo_bus::TARGET_FAHREN;
    t.cell_source_broadcast = false;
    t.simulator_enabled_mask = 0x3FF;
    t.simulator_enabled_mask_hi = 0;
  }
  ~PathTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.bus_wake_target = twingo_bus::TARGET_FAHREN;
    t.bus_state_manual = true;
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

struct Tx {
  uint64_t t;
  CAN_frame f;
};

void run(Bat& b, uint64_t& t, uint64_t ms, uint64_t step, std::vector<Tx>& log) {
  for (uint64_t end = t + ms; t < end; t += step) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      log.push_back({t, f});
    }
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

int row_of(uint32_t id) {
  for (int i = 0; i < Bat::SIM_SIGNAL_COUNT; i++) {
    if (Bat::sim_signals[i].id == id) {
      return i;
    }
  }
  return -1;
}

void only_rows(std::initializer_list<uint32_t> ids) {
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0;
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
  for (uint32_t id : ids) {
    Bat::sim_row_set((uint8_t)row_of(id), true);
  }
}

std::string pair_of(const Tx& x) {
  char b[8];
  snprintf(b, sizeof(b), "%02X/%02X", x.f.data.u8[1], x.f.data.u8[2]);
  return b;
}

struct Step {
  std::string pair;
  uint8_t b7;
  int64_t ms;  // since t0
};

// Changes of (B1/B2, B7) of the 0x426 frames, ms since t0.
std::vector<Step> steps_426(const std::vector<Tx>& v, uint64_t t0) {
  std::vector<Step> out;
  for (const Tx& x : with_id(v, 0x426)) {
    const std::string p = pair_of(x);
    const uint8_t b7 = x.f.data.u8[7];
    if (out.empty() || out.back().pair != p || out.back().b7 != b7) {
      out.push_back({p, b7, (int64_t)x.t - (int64_t)t0});
    }
  }
  return out;
}

int64_t last_of(const std::vector<Tx>& v, uint32_t id, uint64_t t0) {
  int64_t last = -1;
  for (const Tx& x : with_id(v, id)) {
    last = (int64_t)x.t - (int64_t)t0;
  }
  return last;
}

}  // namespace

// ---------------------------------------------------------------------------
// Pure logic (RENAULT-TWINGO-GEN1-BUS.h)
// ---------------------------------------------------------------------------

TEST(TwingoLaden, StateHasItsOwnNameAndTheTableHasEightStates) {
  EXPECT_EQ(twingo_bus::BUS_STATE_COUNT, 8);
  EXPECT_EQ(twingo_bus::BUS_LADEN, 7);
  EXPECT_STREQ(twingo_bus::state_name(twingo_bus::BUS_LADEN), "charging");
}

TEST(TwingoLaden, Frame426InTheStateChargingIsA041WithByte7Of80) {
  const twingo_bus::FrameDef* d = twingo_bus::find_frame(0x426);
  ASSERT_NE(d, nullptr);
  uint8_t out[8];
  twingo_bus::build_frame(*d, twingo_bus::BUS_LADEN, 0, 0, 0x016A71, out);  // the caller's byte 7 stays 40
  EXPECT_EQ(out[1], 0xA0);
  EXPECT_EQ(out[2], 0x41);
  EXPECT_EQ(out[3], 0x01);
  EXPECT_EQ(out[4], 0x6A);
  EXPECT_EQ(out[5], 0x71);
  EXPECT_EQ(out[7], 0x80) << "the car sends 80 all the time while it charges";
  // other states keep the byte of the caller
  twingo_bus::build_frame(*d, twingo_bus::BUS_FAHRT, 0, 0, 0x016A71, out);
  EXPECT_EQ(out[7], 0x40);
  // the transitional step with the flag sets byte 7 to 80 in any state
  twingo_bus::build_frame(*d, twingo_bus::BUS_WACH, 0, 0, 0x016A71, out, 0, 0x6061 | twingo_bus::O426_B7_80);
  EXPECT_EQ(out[1], 0x60);
  EXPECT_EQ(out[2], 0x61);
  EXPECT_EQ(out[7], 0x80);
}

TEST(TwingoLaden, Frame436ChargingIsADAndItsByte0CanBeOverridden) {
  const twingo_bus::FrameDef* d = twingo_bus::find_frame(0x436);
  ASSERT_NE(d, nullptr);
  uint8_t out[8];
  twingo_bus::build_frame(*d, twingo_bus::BUS_LADEN, 0, 0x2D5AD9, 0, out);
  EXPECT_EQ(out[0], 0xAD);
  EXPECT_EQ(out[1], 0x2D);
  EXPECT_EQ(out[2], 0x5A);
  EXPECT_EQ(out[3], 0xD9);
  twingo_bus::build_frame(*d, twingo_bus::BUS_LADEN, 0, 0x2D5AD9, 0, out, 0, -1, 0x40, 0x9D);
  EXPECT_EQ(out[0], 0x9D);
}

TEST(TwingoLaden, ChargingRowsComeFromTheChargeLog) {
  uint8_t out[8];
  const twingo_bus::FrameDef* d1c7 = twingo_bus::find_frame(0x1C7);
  ASSERT_NE(d1c7, nullptr);
  twingo_bus::build_frame(*d1c7, twingo_bus::BUS_LADEN, 0, 0, 0, out);
  EXPECT_EQ(out[0], 0xAA);
  EXPECT_EQ(out[3], 0x04);  // car log: AA 23 00 04 34 00
  EXPECT_EQ(out[4], 0x34);
  const twingo_bus::FrameDef* d428 = twingo_bus::find_frame(0x428);
  ASSERT_NE(d428, nullptr);
  twingo_bus::build_frame(*d428, twingo_bus::BUS_LADEN, 0, 0, 0, out);
  EXPECT_EQ(out[5], 0x04);
  EXPECT_GE(out[6], 0x8E);  // 8E / 8F in the two charge logs
  EXPECT_LE(out[6], 0x8F);
  // 0x500 is not sent while charging
  const twingo_bus::FrameDef* d500 = twingo_bus::find_frame(0x500);
  ASSERT_NE(d500, nullptr);
  EXPECT_FALSE(twingo_bus::active_in_state(*d500, twingo_bus::BUS_LADEN));
}

TEST(TwingoLaden, WakeUpThroughThePlugKeepsTheTimesOfTheChargeLog) {
  struct Case {
    uint32_t t;
    uint8_t state;
    int32_t o426;
  };
  const Case cases[] = {{0, twingo_bus::BUS_ZU, -1},
                        {250, twingo_bus::BUS_ZU, 0x0802},
                        {1350, twingo_bus::BUS_ZU, -1},
                        {1950, twingo_bus::BUS_ZU, 0x0003},
                        {2050, twingo_bus::BUS_WACH, 0x6063},
                        {2550, twingo_bus::BUS_WACH, 0x6061},
                        {2650, twingo_bus::BUS_WACH, 0x6061 | twingo_bus::O426_B7_80},
                        {7850, twingo_bus::BUS_WACH, 0x6061 | twingo_bus::O426_B7_80},
                        {7950, twingo_bus::BUS_LADEN, -1},
                        {11850, twingo_bus::BUS_LADEN, 0xA841},
                        {12950, twingo_bus::BUS_LADEN, -1},
                        {60000, twingo_bus::BUS_LADEN, -1}};
  for (const Case& c : cases) {
    int32_t o = 0;
    EXPECT_EQ(twingo_bus::wake_state(c.t, o, twingo_bus::TARGET_LADEN), c.state) << c.t;
    EXPECT_EQ(o, c.o426) << c.t;
  }
}

TEST(TwingoLaden, TheTargetsDrivingAndSleepingKeepTheDrivingWakeUp) {
  for (uint32_t t : {0u, 1500u, 14000u, 25000u}) {
    int32_t a = 0, b = 0, c = 0;
    const uint8_t sa = twingo_bus::wake_state(t, a);
    const uint8_t sb = twingo_bus::wake_state(t, b, twingo_bus::TARGET_FAHREN);
    const uint8_t sc = twingo_bus::wake_state(t, c, twingo_bus::TARGET_SCHLAFEN);
    EXPECT_EQ(sa, sb);
    EXPECT_EQ(sa, sc);
    EXPECT_EQ(a, b);
    EXPECT_EQ(a, c);
  }
  int32_t o = 0;
  EXPECT_EQ(twingo_bus::wake_state(60000, o), twingo_bus::BUS_FAHRT);
}

TEST(TwingoLaden, Byte0Of436RampsUpAfterTheWakeUp) {
  // car: 80 x27, 83, 86, 8A, 8C ..., 89 frames in all, then AD (charge log 445a1527)
  EXPECT_EQ(twingo_bus::laden_wake_b0(0), 0x80);
  EXPECT_EQ(twingo_bus::laden_wake_b0(2650), 0x80);
  EXPECT_EQ(twingo_bus::laden_wake_b0(2700), 0x83);
  EXPECT_EQ(twingo_bus::laden_wake_b0(2800), 0x86);
  EXPECT_EQ(twingo_bus::laden_wake_b0(2900), 0x8A);
  EXPECT_EQ(twingo_bus::laden_wake_b0(3000), 0x8C);
  EXPECT_EQ(twingo_bus::laden_wake_b0(8800), 0xAC);
  EXPECT_EQ(twingo_bus::laden_wake_b0(8900), 0xAD);
  EXPECT_EQ(twingo_bus::laden_wake_b0(60000), 0xAD);
  uint8_t prev = 0;
  for (uint32_t t = 0; t < 10000; t += 100) {
    const uint8_t v = twingo_bus::laden_wake_b0(t);
    EXPECT_GE(v, prev) << t;
    prev = v;
  }
}

TEST(TwingoLaden, Byte0Of436RampsDownAtTheEnd) {
  EXPECT_EQ(twingo_bus::laden_end_b0(0), 0xAD);
  EXPECT_EQ(twingo_bus::laden_end_b0(100), 0xA9);
  EXPECT_EQ(twingo_bus::laden_end_b0(200), 0xA6);
  EXPECT_EQ(twingo_bus::laden_end_b0(500), 0x9D);
  EXPECT_EQ(twingo_bus::laden_end_b0(4500), 0x81);
  EXPECT_EQ(twingo_bus::laden_end_b0(4600), 0x80);
  EXPECT_EQ(twingo_bus::laden_end_b0(60000), 0x80);
  uint8_t prev = 0xFF;
  for (uint32_t t = 0; t < 6000; t += 100) {
    const uint8_t v = twingo_bus::laden_end_b0(t);
    EXPECT_LE(v, prev) << t;
    prev = v;
  }
}

TEST(TwingoSleepPaths, LongWayIsTheOldOneAndTheDefault) {
  int32_t o = 0;
  EXPECT_EQ(twingo_bus::sleep_state(0, twingo_bus::BUS_FAHRT, o), twingo_bus::BUS_ZUENDUNG1);
  EXPECT_EQ(o, 0x7065);
  EXPECT_EQ(twingo_bus::sleep_state(150, twingo_bus::BUS_FAHRT, o, twingo_bus::SLEEP_LANG), twingo_bus::BUS_ZUENDUNG1);
  EXPECT_EQ(twingo_bus::sleep_state(250, twingo_bus::BUS_FAHRT, o), twingo_bus::BUS_AUS);
  EXPECT_EQ(o, -1);
  EXPECT_EQ(twingo_bus::sleep_state(2200, twingo_bus::BUS_FAHRT, o), twingo_bus::BUS_AUS);
  EXPECT_EQ(o, 0x0007);
  EXPECT_EQ(twingo_bus::sleep_state(4800, twingo_bus::BUS_FAHRT, o), twingo_bus::BUS_WACH);
  EXPECT_EQ(o, -1);
}

TEST(TwingoSleepPaths, ShortWayGoesFromDrivingStraightTo00_07ThenAwake) {
  int32_t o = 0;
  EXPECT_EQ(twingo_bus::sleep_state(0, twingo_bus::BUS_FAHRT, o, twingo_bus::SLEEP_KURZ), twingo_bus::BUS_AUS);
  EXPECT_EQ(o, 0x0007);
  EXPECT_EQ(twingo_bus::sleep_state(500, twingo_bus::BUS_FAHRT, o, twingo_bus::SLEEP_KURZ), twingo_bus::BUS_AUS);
  EXPECT_EQ(o, 0x0007);
  EXPECT_EQ(twingo_bus::sleep_state(600, twingo_bus::BUS_FAHRT, o, twingo_bus::SLEEP_KURZ), twingo_bus::BUS_WACH);
  EXPECT_EQ(o, -1);
  // from another state the short way is the long one
  int32_t o2 = 0;
  EXPECT_EQ(twingo_bus::sleep_state(1000, twingo_bus::BUS_WACH, o, twingo_bus::SLEEP_KURZ),
            twingo_bus::sleep_state(1000, twingo_bus::BUS_WACH, o2, twingo_bus::SLEEP_LANG));
  EXPECT_EQ(o, o2);
}

TEST(TwingoSleepPaths, ChargeLog2WayKeepsTheTimesOfTheLog) {
  struct Case {
    uint32_t t;
    int32_t o426;
  };
  // 00/01 B7=80 0.5 s, 00/01 1.5 s, 00/07 1.1 s, 00/06 0.4 s, 08/06 1.1 s, then awake (00/06)
  const Case cases[] = {{0, 0x0001 | twingo_bus::O426_B7_80},
                        {450, 0x0001 | twingo_bus::O426_B7_80},
                        {500, 0x0001},
                        {1950, 0x0001},
                        {2000, 0x0007},
                        {3050, 0x0007},
                        {3100, -1},
                        {3450, -1},
                        {3500, 0x0806},
                        {4550, 0x0806},
                        {4600, -1},
                        {60000, -1}};
  for (const Case& c : cases) {
    int32_t o = 0;
    for (uint8_t start : {twingo_bus::BUS_FAHRT, twingo_bus::BUS_LADEN}) {
      EXPECT_EQ(twingo_bus::sleep_state(c.t, start, o, twingo_bus::SLEEP_LADELOG2), twingo_bus::BUS_WACH) << c.t;
      EXPECT_EQ(o, c.o426) << c.t;
    }
  }
}

TEST(TwingoSleepPaths, ClosingStageIsElevenSecondsOnTheChargeLog2WayAndFiftyOneSecondsOtherwise) {
  // Bau 7: the closing stage of the long and short way is as long as in the drive log (00/02 from 697.96 s to 748.72 s)
  EXPECT_EQ(twingo_bus::closing_stage_ms(twingo_bus::SLEEP_LANG, false), 50820u);
  EXPECT_EQ(twingo_bus::closing_stage_ms(twingo_bus::SLEEP_LANG, true), 50820u);
  EXPECT_EQ(twingo_bus::closing_stage_ms(twingo_bus::SLEEP_KURZ, false), 50820u);
  EXPECT_EQ(twingo_bus::closing_stage_ms(twingo_bus::SLEEP_LADELOG2, false), 11100u);
  EXPECT_EQ(twingo_bus::closing_stage_ms(twingo_bus::SLEEP_LADELOG2, true), 11100u);
}

TEST(TwingoSleepPaths, Frame157AndFrame0EDEndLastOnTheChargeLog2Way) {
  const uint32_t L = twingo_bus::SLEEP_LADELOG2;
  const uint32_t e157 = twingo_bus::closing_end_ms(0x157, false, L);
  const uint32_t eED = twingo_bus::closing_end_ms(0x0ED, false, L);
  EXPECT_EQ(e157, eED);
  for (uint16_t id :
       {0x155, 0x424, 0x1A1, 0x1C7, 0x419, 0x428, 0x42F, 0x4F7, 0x511, 0x69F, 0x426, 0x435, 0x436, 0x423}) {
    EXPECT_GT(e157, twingo_bus::closing_end_ms(id, false, L)) << std::hex << id;
  }
  EXPECT_EQ(twingo_bus::closing_end_ms(0x0EC, true, L), 0u);
  EXPECT_LT(twingo_bus::closing_end_ms(0x1A1, false, L), twingo_bus::closing_end_ms(0x69F, false, L));
  EXPECT_LT(twingo_bus::closing_end_ms(0x69F, false, L), twingo_bus::closing_end_ms(0x426, false, L));
  EXPECT_LT(twingo_bus::closing_end_ms(0x426, false, L), twingo_bus::closing_end_ms(0x423, false, L));
  EXPECT_LT(twingo_bus::closing_end_ms(0x423, false, L), e157);
  EXPECT_LE(e157, twingo_bus::CLOSING_LADELOG2_MS);
  // the other ways (drive log): 0x157/0x0ED also end last, inside the stage of 50.82 s
  for (uint8_t path : {twingo_bus::SLEEP_LANG, twingo_bus::SLEEP_KURZ}) {
    EXPECT_EQ(twingo_bus::closing_end_ms(0x157, false, path), 50760u);
    EXPECT_EQ(twingo_bus::closing_end_ms(0x0ED, false, path), 50760u);
    EXPECT_LE(twingo_bus::closing_end_ms(0x157, false, path), twingo_bus::closing_stage_ms(path, false));
    for (uint16_t id : {0x1A1, 0x69F, 0x426, 0x435, 0x436, 0x423}) {
      EXPECT_GT(50760u, twingo_bus::closing_end_ms(id, false, path)) << std::hex << id;
    }
  }
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------

TEST(TwingoLadenDriver, WakeUpThroughThePlugOnTheBus) {
  PathTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.bus_state_manual = false;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  only_rows({0x426, 0x436, 0x0EC});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 16000, 10, log);
  auto f426 = with_id(log, 0x426);
  ASSERT_FALSE(f426.empty());
  const uint64_t t0 = f426[0].t;
  const auto st = steps_426(log, t0);
  const struct {
    const char* pair;
    uint8_t b7;
    int64_t ms;
  } want[] = {{"00/02", 0x40, 0},     {"08/02", 0x40, 200},  {"00/02", 0x40, 1300}, {"00/03", 0x40, 1900},
              {"60/63", 0x40, 2000},  {"60/61", 0x40, 2500}, {"60/61", 0x80, 2600}, {"A0/41", 0x80, 7900},
              {"A8/41", 0x80, 11800}, {"A0/41", 0x80, 12900}};
  ASSERT_GE(st.size(), 10u);
  for (size_t i = 0; i < 10; i++) {
    EXPECT_EQ(st[i].pair, want[i].pair) << i;
    EXPECT_EQ(st[i].b7, want[i].b7) << i;
    const int64_t d = st[i].ms - want[i].ms;
    EXPECT_LE(d < 0 ? -d : d, 60) << want[i].pair << " at " << st[i].ms;
  }
  // km of the frame stay in bytes 3-5, the state stays at charging
  EXPECT_EQ(Bat::bus_state_shown, twingo_bus::BUS_LADEN);
  // 0x436: B0 80, then the ramp, then AD
  auto f436 = with_id(log, 0x436);
  ASSERT_FALSE(f436.empty());
  auto b0_at = [&](int64_t ms) {
    uint8_t v = 0;
    for (const Tx& x : f436) {
      if ((int64_t)x.t - (int64_t)t0 <= ms) {
        v = x.f.data.u8[0];
      }
    }
    return v;
  };
  EXPECT_EQ(b0_at(2000), 0x80);
  EXPECT_EQ(b0_at(2650), 0x80);
  EXPECT_EQ(b0_at(3050), 0x8C);
  EXPECT_EQ(b0_at(12000), 0xAD);
  // 0x0EC starts 2.05 s after the first frame (wake-up form first)
  auto fec = with_id(log, 0x0EC);
  ASSERT_FALSE(fec.empty());
  const int64_t first_ec = (int64_t)fec[0].t - (int64_t)t0;
  EXPECT_GE(first_ec, 2000);
  EXPECT_LE(first_ec, 2150);
  EXPECT_EQ(fec[0].f.data.u8[0], 0x10);
}

TEST(TwingoLadenDriver, DrivingTargetKeepsTheOldWakeUp) {
  PathTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.bus_state_manual = false;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_FAHREN;
  only_rows({0x426, 0x0EC});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 25000, 10, log);
  auto f426 = with_id(log, 0x426);
  ASSERT_FALSE(f426.empty());
  const auto st = steps_426(log, f426[0].t);
  EXPECT_EQ(st.back().pair, "70/69");
  for (const Step& s : st) {
    EXPECT_EQ(s.b7, 0x40) << s.pair;
  }
  auto fec = with_id(log, 0x0EC);
  ASSERT_FALSE(fec.empty());
  EXPECT_LE((int64_t)fec[0].t - (int64_t)f426[0].t, 100);  // 0x0EC at once, no hold-back
}

TEST(TwingoLadenDriver, ManualStateChargingSendsTheChargingFrame) {
  PathTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.bus_state_manual = true;
  datalayer_extended.twingoGen1.bus_state = twingo_bus::BUS_LADEN;
  only_rows({0x426, 0x436});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 10, log);
  auto f426 = with_id(log, 0x426);
  ASSERT_FALSE(f426.empty());
  EXPECT_EQ(pair_of(f426[0]), "A0/41");
  EXPECT_EQ(f426[0].f.data.u8[7], 0x80);
  auto f436 = with_id(log, 0x436);
  ASSERT_FALSE(f436.empty());
  EXPECT_EQ(f436[0].f.data.u8[0], 0xAD);
}

namespace {
// Runs the Sleep button on the given way until the true silence and returns the log; t0 = the start of the Sleep run.
struct PathRun {
  std::vector<Tx> log;
  uint64_t t0 = 0;
  uint64_t t_closing = 0;  // first 0x426 frame in "closed"
};

PathRun run_path(PathTwingo& b, uint8_t path, std::initializer_list<uint32_t> rows, uint64_t total_ms = 200000) {
  datalayer_extended.twingoGen1.bus_sleep_path = path;
  datalayer_extended.twingoGen1.bus_state_manual = false;
  only_rows(rows);
  PathRun r;
  uint64_t t = 1000;
  std::vector<Tx> up;
  run(b, t, 25000, 10, up);
  b.request_sleep();
  r.t0 = t;
  run(b, t, total_ms, 10, r.log);
  // the start of the closing stage: the first 0x426 in "closed" (00/02); since Bau 7 the closing form of 0x0ED
  // (60 FF 00) comes only about 0.1 s before the 04 of 0x423, so it is the fallback
  for (const Tx& x : with_id(r.log, 0x426)) {
    if (x.f.data.u8[2] == 0x02) {
      r.t_closing = x.t;
      break;
    }
  }
  if (r.t_closing == 0) {
    for (const Tx& x : with_id(r.log, 0x0ED)) {
      if (x.f.data.u8[0] == 0x60 && x.f.data.u8[1] == 0xFF && x.f.data.u8[2] == 0x00) {
        r.t_closing = x.t;
        break;
      }
    }
  }
  return r;
}
}  // namespace

TEST(TwingoSleepPathsDriver, ChargeLog2WayWalksTheSequenceOfTheLog) {
  PathTwingo b;
  b.setup();
  PathRun r = run_path(b, twingo_bus::SLEEP_LADELOG2, {0x426, 0x436});
  const auto st = steps_426(r.log, r.t0);
  const struct {
    const char* pair;
    uint8_t b7;
    int64_t ms;
  } want[] = {{"00/01", 0x80, 0},    {"00/01", 0x40, 500},  {"00/07", 0x40, 2000},
              {"00/06", 0x40, 3100}, {"08/06", 0x40, 3500}, {"00/06", 0x40, 4600}};
  ASSERT_GE(st.size(), 6u);
  // the first frame after the request may come up to 100 ms later
  const int64_t shift = st[0].ms;
  for (size_t i = 0; i < 6; i++) {
    EXPECT_EQ(st[i].pair, want[i].pair) << i;
    EXPECT_EQ(st[i].b7, want[i].b7) << i;
    const int64_t d = (st[i].ms - shift) - want[i].ms;
    EXPECT_LE(d < 0 ? -d : d, 60) << want[i].pair << " at " << st[i].ms;
  }
  // 0x436 byte 0 walks down to 80 in the same time
  auto f436 = with_id(r.log, 0x436);
  ASSERT_FALSE(f436.empty());
  EXPECT_EQ(f436[0].f.data.u8[0], 0xAD);
  uint8_t prev = 0xFF;
  for (const Tx& x : f436) {
    if ((int64_t)x.t - (int64_t)r.t0 > 6000) {
      break;
    }
    EXPECT_LE(x.f.data.u8[0], prev);
    prev = x.f.data.u8[0];
  }
  EXPECT_EQ(prev, 0x80);
}

TEST(TwingoSleepPathsDriver, ChargeLog2ClosingLastsElevenSecondsAndEndsWith157And0ED) {
  PathTwingo b;
  b.setup();
  PathRun r = run_path(b, twingo_bus::SLEEP_LADELOG2, {0x157, 0x0ED, 0x426, 0x435, 0x436, 0x423, 0x1A1, 0x69F});
  ASSERT_GT(r.t_closing, 0u) << "0x426 never went to closed";
  const uint64_t tc = r.t_closing;
  const int64_t l157 = last_of(r.log, 0x157, tc);
  const int64_t led = last_of(r.log, 0x0ED, tc);
  const int64_t l426 = last_of(r.log, 0x426, tc);
  const int64_t l435 = last_of(r.log, 0x435, tc);
  const int64_t l436 = last_of(r.log, 0x436, tc);
  const int64_t l423 = last_of(r.log, 0x423, tc);
  const int64_t l1a1 = last_of(r.log, 0x1A1, tc);
  const int64_t l69f = last_of(r.log, 0x69F, tc);
  // the rows that run until the end of the log
  EXPECT_GE(l1a1, 9800);
  EXPECT_LE(l1a1, 10000);
  EXPECT_GE(l69f, 9000);
  EXPECT_LE(l69f, 10500);
  for (int64_t v : {l426, l435, l436}) {
    EXPECT_GE(v, 10800);
    EXPECT_LE(v, 11000);
  }
  EXPECT_LE(l423, 11030);
  EXPECT_GE(l423, 10800);
  EXPECT_GE(l157, 10900);
  EXPECT_LE(l157, 11040);
  EXPECT_GE(led, 10900);
  EXPECT_LE(led, 11040);
  // 0x157 and 0x0ED are the last frames of the whole bus
  for (int64_t v : {l426, l435, l436, l423, l1a1, l69f}) {
    EXPECT_GT(l157, v);
    EXPECT_GT(led, v);
  }
  // the closing frames: 0x426 in closed, 0x0ED in the closing form
  bool ed_closing = false;
  for (const Tx& x : with_id(r.log, 0x0ED)) {
    if ((int64_t)x.t - (int64_t)tc > 5000) {
      ed_closing |= (x.f.data.u8[0] == 0x60 && x.f.data.u8[1] == 0xFF && x.f.data.u8[2] == 0x00);
    }
  }
  EXPECT_TRUE(ed_closing);
  // true silence afterwards
  for (const Tx& x : r.log) {
    if (x.f.ID == 0x426 || x.f.ID == 0x157 || x.f.ID == 0x0ED) {
      EXPECT_LE((int64_t)x.t - (int64_t)tc, 11100);
    }
  }
}

TEST(TwingoSleepPathsDriver, LongAndShortWaysCloseInFiftyOneSecondsLikeTheDriveLog) {
  for (uint8_t path : {twingo_bus::SLEEP_LANG, twingo_bus::SLEEP_KURZ}) {
    PathTwingo b;
    b.setup();
    PathRun r = run_path(b, path, {0x157, 0x0ED, 0x426, 0x1A1, 0x423});  // t_closing = first 0x426 in closed
    ASSERT_GT(r.t_closing, 0u) << (int)path;
    const int64_t l157 = last_of(r.log, 0x157, r.t_closing);
    const int64_t led = last_of(r.log, 0x0ED, r.t_closing);
    const int64_t l426 = last_of(r.log, 0x426, r.t_closing);
    const int64_t l423 = last_of(r.log, 0x423, r.t_closing);
    EXPECT_GE(l157, 50600) << (int)path;
    EXPECT_LE(l157, 50800) << (int)path;
    EXPECT_GE(led, 50600) << (int)path;
    EXPECT_LE(led, 50800) << (int)path;
    EXPECT_GE(l426, 50300) << (int)path;
    EXPECT_LE(l426, 50700) << (int)path;
    EXPECT_GT(l157, l426) << (int)path;
    EXPECT_GT(l157, l423) << (int)path;
  }
}

TEST(TwingoSleepPathsDriver, ShortWayFromDrivingGoesStraightTo00_07) {
  PathTwingo b;
  b.setup();
  PathRun r = run_path(b, twingo_bus::SLEEP_KURZ, {0x426});
  const auto st = steps_426(r.log, r.t0);
  ASSERT_GE(st.size(), 3u);
  const int64_t shift = st[0].ms;
  EXPECT_EQ(st[0].pair, "00/07");
  EXPECT_EQ(st[1].pair, "00/06");
  EXPECT_LE(std::llabs((st[1].ms - shift) - 600), 60);
}

// ---------------------------------------------------------------------------
// Page
// ---------------------------------------------------------------------------

TEST(TwingoSleepPathsPage, HasTheRadioButtonsForWayAndMode) {
  PathTwingo b;
  b.setup();
  std::string page(simulator_processor(String("X")).c_str());
  for (int k = 0; k < 3; k++) {
    EXPECT_NE(page.find("id='slpath" + std::to_string(k) + "'"), std::string::npos) << k;
    EXPECT_NE(page.find("id='bustgt" + std::to_string(k) + "'"), std::string::npos) << k;
    EXPECT_NE(page.find("/twingoSleepPath?value=" + std::to_string(k)), std::string::npos) << k;
    EXPECT_NE(page.find("/twingoBusTarget?value=" + std::to_string(k)), std::string::npos) << k;
  }
  EXPECT_NE(page.find("Einschlafweg"), std::string::npos);
  EXPECT_NE(page.find("Betriebsart"), std::string::npos);
  EXPECT_NE(page.find("<option value='7'"), std::string::npos) << "the state field offers charging";
  // default: long way and driving are checked
  EXPECT_NE(page.find("id='slpath0' checked"), std::string::npos);
  EXPECT_NE(page.find("id='bustgt0' checked"), std::string::npos);
  datalayer_extended.twingoGen1.bus_sleep_path = twingo_bus::SLEEP_LADELOG2;
  datalayer_extended.twingoGen1.bus_wake_target = twingo_bus::TARGET_LADEN;
  std::string page2(simulator_processor(String("X")).c_str());
  EXPECT_NE(page2.find("id='slpath2' checked"), std::string::npos);
  EXPECT_NE(page2.find("id='bustgt1' checked"), std::string::npos);
}
