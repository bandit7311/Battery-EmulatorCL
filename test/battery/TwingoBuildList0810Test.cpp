#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-LOGIC.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"

#include "Arduino.h"

// TX frame capture injected by the emulated CAN layer (see emul/can.cpp).
void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Tests of the build of 08.10. (points 1 to 17 of the Twingo build list). Reference values come from the real vehicle
// logs of 02.10. and 04.10., the Lade-Log of 20.11.2025 and the bench readings of 06.10.; the numbers are explained
// at the code they test.

namespace {

class TestTwingo : public RenaultTwingoGen1Battery {
 public:
  TestTwingo() {  // the age state is global: every test starts from the seed
    RenaultTwingoGen1Battery::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1311344;
    t.age_pack_unix = 1791319221;
    t.age_last_sent = 0;
    t.age_source = 0;
  }
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t& t) override {
    t = 1791319221;  // the seed reference: age 1,312,784
    return true;
  }
  bool get_wall_clock_seconds_of_day(uint32_t& secs) override {
    secs = 12 * 3600;
    return true;
  }
};

CAN_frame frame_155(uint8_t power, uint16_t current_raw, uint16_t soc_raw) {
  CAN_frame f = {.FD = false, .ext_ID = false, .DLC = 8, .ID = 0x155};
  f.data.u8[0] = power;
  f.data.u8[1] = (uint8_t)((current_raw >> 8) & 0x0F);
  f.data.u8[2] = (uint8_t)(current_raw & 0xFF);
  f.data.u8[3] = 0;
  f.data.u8[4] = (uint8_t)(soc_raw >> 8);
  f.data.u8[5] = (uint8_t)(soc_raw & 0xFF);
  return f;
}

int row_of(uint32_t id) {
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    if (RenaultTwingoGen1Battery::sim_signals[i].id == id) {
      return i;
    }
  }
  return -1;
}

}  // namespace

// ---------------------------------------------------------------------------------------------------------
// Point 1: 0x42E temperature field fixed at 20 degC
TEST(TwingoBuild0810, Row42ETemperatureFieldIs20DegC) {
  const int row = row_of(0x42E);
  ASSERT_GE(row, 0);
  const uint8_t* d = RenaultTwingoGen1Battery::sim_signals[row].data;
  // the driver reads the temperature the same way (case 0x42E): ((b5 << 8 | b6) >> 5) & 0x7F, minus 40
  const int temp = (int)((((d[5] << 8) | d[6]) >> 5) & 0x7F) - 40;
  EXPECT_EQ(temp, 20);
  EXPECT_EQ(d[5], 0x07);
  EXPECT_EQ(d[6], 0x80);
}

// ---------------------------------------------------------------------------------------------------------
// Point 2: 0x155 filter
TEST(TwingoBuild0810, Frame155ValidityRule) {
  EXPECT_TRUE(twingo::frame_155_valid(0x800, 9184));     // the bench: 22.96 %
  EXPECT_TRUE(twingo::frame_155_valid(0x000, 0));        // empty pack is valid
  EXPECT_TRUE(twingo::frame_155_valid(0x7FF, 40000));    // exactly 100 % is valid
  EXPECT_FALSE(twingo::frame_155_valid(0xFFF, 9184));    // current marker
  EXPECT_FALSE(twingo::frame_155_valid(0x800, 40001));   // above 100 %
  EXPECT_FALSE(twingo::frame_155_valid(0x800, 0xFFF8));  // 163.82 % of 06.10.
}

TEST(TwingoBuild0810, InvalidFrame155KeepsTheLastValidValues) {
  datalayer_extended.twingoGen1.frame_155_dropped = 0;
  TestTwingo b;
  b.setup();
  b.handle_incoming_can_frame(frame_155(10, 0x800, 9184));
  b.update_values();
  const uint16_t soc_before = datalayer.battery.status.real_soc;
  const int32_t current_before = datalayer.battery.status.current_dA;
  EXPECT_EQ(soc_before, (uint16_t)(9184 * 0.25f));
  EXPECT_EQ(datalayer_extended.twingoGen1.frame_155_dropped, 0u);

  b.handle_incoming_can_frame(frame_155(30, 0x800, 0xFFF8));  // SOC marker
  b.handle_incoming_can_frame(frame_155(30, 0xFFF, 9184));    // current marker
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.real_soc, soc_before);
  EXPECT_EQ(datalayer.battery.status.current_dA, current_before);
  EXPECT_EQ(datalayer_extended.twingoGen1.frame_155_dropped, 2u);

  b.handle_incoming_can_frame(frame_155(10, 0x800, 9300));  // valid again: taken over
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.real_soc, (uint16_t)(9300 * 0.25f));
  EXPECT_EQ(datalayer_extended.twingoGen1.frame_155_dropped, 2u);
}

// ---------------------------------------------------------------------------------------------------------
// Point 3: 0x900D display
TEST(TwingoBuild0810, BatteryCurrentPlausibility) {
  EXPECT_TRUE(twingo::battery_current_plausible(0.275));
  EXPECT_TRUE(twingo::battery_current_plausible(-12.5));
  EXPECT_TRUE(twingo::battery_current_plausible(499.9));
  EXPECT_FALSE(twingo::battery_current_plausible(-2895.875));  // the invalid value of 06.10.
  EXPECT_FALSE(twingo::battery_current_plausible(500.0));
}

// ---------------------------------------------------------------------------------------------------------
// Point 15: HV state model and frame encodings
namespace {
std::string hex_of(const uint8_t* d, size_t n) {
  std::string out;
  char buf[4];
  for (size_t i = 0; i < n; i++) {
    snprintf(buf, sizeof(buf), "%02X", d[i]);
    out += buf;
    if (i + 1 < n) {
      out += ' ';
    }
  }
  return out;
}
}  // namespace

TEST(TwingoBuild0810, Frame57FEncodingMatchesTheVehicleLog) {
  uint8_t d[7];
  twingo::frame_57f(0.0, 3390, false, d);  // 02.10. log: 64 0D 3E 7F 80 00 00 = 339.0 V, 0 A
  EXPECT_EQ(hex_of(d, 7), "64 0D 3E 7F 80 00 00");
  twingo::frame_57f(1.0, 3385, false, d);  // 64 4D 39 ...
  EXPECT_EQ(hex_of(d, 7), "64 4D 39 7F 80 00 00");
  twingo::frame_57f(4.0, 3380, true, d);  // 65 0D 34 CF A8
  EXPECT_EQ(hex_of(d, 7), "65 0D 34 CF A8 00 00");
  twingo::frame_57f(0.0, 5, false, d);  // first frame after the wake-up: 0.5 V
  EXPECT_EQ(hex_of(d, 7), "64 00 05 7F 80 00 00");
}

TEST(TwingoBuild0810, Frame57FCurrentSignAndLimits) {
  EXPECT_DOUBLE_EQ(twingo::hv_57f_amps(-100, true), 10.0);  // the emulator: negative = discharge, 0x57F positive
  EXPECT_DOUBLE_EQ(twingo::hv_57f_amps(250, true), -25.0);
  EXPECT_DOUBLE_EQ(twingo::hv_57f_amps(-100, false), 0.0);   // HV open
  EXPECT_DOUBLE_EQ(twingo::hv_57f_amps(-28950, true), 0.0);  // invalid value (above 400 A)
}

TEST(TwingoBuild0810, Frame5D7EncodingAndCounter) {
  uint8_t d[8];
  twingo::frame_5d7(92591, 0, false, d);  // log: 00 00 08 D4 85 C0 C0 00 for 92591 km (the log shows 92591.12)
  EXPECT_EQ(hex_of(d, 8), "00 00 08 D4 85 C0 C0 00");
  twingo::frame_5d7(19400, 31, false, d);
  EXPECT_EQ(hex_of(d, 8), "00 00 01 D9 A2 00 FE 00");
  twingo::frame_5d7(19400, 32, false, d);  // wraps after 32 values
  EXPECT_EQ(d[6], 0xC0);
  twingo::frame_5d7(19400, 0, true, d);  // very first frame: speed FF FF, byte 7 = 08
  EXPECT_EQ(hex_of(d, 8), "FF FF 01 D9 A2 00 C0 08");
}

TEST(TwingoBuild0810, HvModelConnectSequence) {
  twingo::HvTimes t;
  t.connect = 1000;
  auto o = twingo::hv_compute(t, 1000, 3390);
  EXPECT_TRUE(o.connected);
  EXPECT_EQ(o.phase_62d, 0x04);
  EXPECT_FALSE(o.relay_a0);
  EXPECT_EQ(o.volt_dV, 115);  // 11.5 V at the start of the ramp
  o = twingo::hv_compute(t, 1300, 3390);
  EXPECT_EQ(o.phase_62d, 0x06);
  EXPECT_FALSE(o.relay_a0);
  o = twingo::hv_compute(t, 1500, 3390);
  EXPECT_TRUE(o.relay_a0);
  EXPECT_EQ(o.phase_62d, 0x06);
  o = twingo::hv_compute(t, 2300, 3390);
  EXPECT_EQ(o.phase_62d, 0x02);
  EXPECT_EQ(o.volt_dV, 3390);
  EXPECT_FALSE(o.b34);
  EXPECT_FALSE(o.inverter_on);
  t.b34 = 1500;
  t.c7 = 3000;
  EXPECT_TRUE(twingo::hv_compute(t, 3100, 3390).b34);
  EXPECT_FALSE(twingo::hv_compute(t, 4999, 3390).inverter_on);  // C7 + 2.0 s
  o = twingo::hv_compute(t, 5000, 3390);
  EXPECT_TRUE(o.inverter_on);
  EXPECT_EQ(o.b5_62d, 0x40);
}

TEST(TwingoBuild0810, HvModelDisconnectSequence) {
  twingo::HvTimes t;
  t.connect = 0;
  t.b34 = 100;
  t.c7 = 100;
  t.disc = 100000;
  auto o = twingo::hv_compute(t, 100000, 3390);
  EXPECT_TRUE(o.relay_a0);
  EXPECT_FALSE(o.power_idle);  // 0x1FD byte 0 falls to FE at once
  EXPECT_TRUE(o.inverter_on);
  EXPECT_TRUE(o.b34);
  o = twingo::hv_compute(t, 101100, 3390);  // +1.1 s: inverter off
  EXPECT_FALSE(o.inverter_on);
  EXPECT_EQ(o.b5_62d, 0x00);
  EXPECT_EQ(o.phase_62d, 0x02);
  EXPECT_TRUE(o.b34);
  o = twingo::hv_compute(t, 101600, 3390);  // +1.6 s: transition
  EXPECT_EQ(o.phase_62d, 0x06);
  o = twingo::hv_compute(t, 102100, 3390);  // +2.1 s: b34 off, HV still closed
  EXPECT_FALSE(o.b34);
  EXPECT_TRUE(o.relay_a0);
  o = twingo::hv_compute(t, 102200, 3390);  // +2.2 s: HV opens
  EXPECT_FALSE(o.relay_a0);
  EXPECT_EQ(o.phase_62d, 0x04);
  EXPECT_EQ(o.volt_dV, 3390);
  EXPECT_EQ(twingo::hv_compute(t, 102750, 3390).volt_dV, 690);  // 69 V after 0.55 s
  EXPECT_EQ(twingo::hv_compute(t, 103700, 3390).volt_dV, 360);  // 36 V after 1.5 s
  EXPECT_EQ(twingo::hv_compute(t, 104700, 3390).volt_dV, 280);  // 28 V after 2.5 s
  // tau 22 s: after another 22 s 28 V / e = 10.3 V
  EXPECT_NEAR(twingo::hv_compute(t, 104700 + 22000, 3390).volt_dV, 103, 2);
  EXPECT_EQ(twingo::hv_compute(t, 104700 + 600000, 3390).volt_dV, 5);  // floor 0.5 V
}

TEST(TwingoBuild0810, HvRowsFollowTheStagesWithoutAWakeUp) {
  datalayer.battery.status.voltage_dV = 3390;
  datalayer.battery.status.current_dA = 0;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL | (1ULL << row_of(0x57F)) | (1ULL << row_of(0x599)) |
                                                         (1ULL << row_of(0x62D)) | (1ULL << row_of(0x1FD)) |
                                                         (1ULL << row_of(0x523));
  TestTwingo b;
  b.setup();
  std::vector<CAN_frame> f57, f59, f62, f1f, f52;
  for (uint64_t t = 1000; t < 12000; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      if (f.ID == 0x57F)
        f57.push_back(f);
      if (f.ID == 0x599)
        f59.push_back(f);
      if (f.ID == 0x62D)
        f62.push_back(f);
      if (f.ID == 0x1FD)
        f1f.push_back(f);
      if (f.ID == 0x523)
        f52.push_back(f);
    }
  }
  ASSERT_GE(f62.size(), 3u);
  EXPECT_EQ(hex_of(f62[0].data.u8, 7), "01 45 E0 04 7F CC 00");  // the two invalid first frames
  EXPECT_EQ(hex_of(f62[1].data.u8, 7), "01 45 E0 04 00 00 00");
  EXPECT_EQ(hex_of(f62[2].data.u8, 7), "01 45 E0 02 06 40 00");  // HV closed since the start, inverter on
  ASSERT_GE(f59.size(), 2u);
  EXPECT_EQ(hex_of(f59[0].data.u8, 6), "00 07 FF FF FF E0");
  EXPECT_EQ(f59[1].data.u8[1], 0x08);
  ASSERT_GE(f57.size(), 2u);
  EXPECT_EQ(hex_of(f57[1].data.u8, 7), "64 0D 3E CF A8 00 00");  // 339.0 V, 0 A, bytes 3-4 switched
  ASSERT_GE(f1f.size(), 2u);
  EXPECT_EQ(hex_of(f1f[0].data.u8, 8), "FF 80 7F FF 7F FF FF 00");
  EXPECT_EQ(f1f[1].data.u8[0], 0x45);
  EXPECT_EQ(f1f[1].data.u8[5], 0xA0);
  ASSERT_GE(f52.size(), 1u);
  EXPECT_EQ(hex_of(f52[0].data.u8, 3), "14 08 10");  // 1,312,784 min = the vehicle age
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
}

TEST(TwingoBuild0810, Frame523NeedsAnAge) {
  struct NoClock : public TestTwingo {
    bool get_unix_time(time_t&) override { return false; }
  };
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL | (1ULL << row_of(0x523));
  NoClock b;
  b.setup();
  int n = 0;
  for (uint64_t t = 1000; t < 4000; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      n += (f.ID == 0x523) ? 1 : 0;
    }
  }
  EXPECT_EQ(n, 0);
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
}

// ---------------------------------------------------------------------------------------------------------
// Points 13 and 14: 0x5D7 with kilometres, 0x426 editable
namespace {
struct OdoGuard {
  ~OdoGuard() {
    RenaultTwingoGen1Battery::odo_5d7_km = 19400;
    RenaultTwingoGen1Battery::odo_426_km = 19400;
    RenaultTwingoGen1Battery::odo_426_b7 = 0x40;
    datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  }
};

std::vector<CAN_frame> collect(RenaultTwingoGen1Battery& b, uint32_t id, uint64_t from, uint64_t to) {
  std::vector<CAN_frame> out;
  for (uint64_t t = from; t < to; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      if (f.ID == id && !f.ext_ID) {
        out.push_back(f);
      }
    }
  }
  return out;
}
}  // namespace

TEST(TwingoBuild0810, Row5D7SendsTheOdometerWithCounter) {
  OdoGuard guard;
  const int row = row_of(0x5D7);
  ASSERT_GE(row, 0);
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[row].interval_ms, 100);
  EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[row].end_stage, RenaultTwingoGen1Battery::SIM_END_AT_C0);
  EXPECT_FALSE(RenaultTwingoGen1Battery::sim_row_enabled(row));  // off by default
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL | (1ULL << row);
  TestTwingo b;
  b.setup();
  auto f = collect(b, 0x5D7, 1000, 5000);
  ASSERT_GE(f.size(), 35u);
  EXPECT_EQ(hex_of(f[0].data.u8, 8), "FF FF 01 D9 A2 00 C0 08");  // first frame
  EXPECT_EQ(hex_of(f[1].data.u8, 8), "00 00 01 D9 A2 00 C2 00");
  EXPECT_EQ(f[32].data.u8[6], 0xC0);  // counter wraps after 32 values
  EXPECT_EQ(f[0].DLC, 8);
}

TEST(TwingoBuild0810, Row5D7FollowsTheEnteredKilometres) {
  OdoGuard guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL | (1ULL << row_of(0x5D7));
  RenaultTwingoGen1Battery::odo_5d7_km = 92591;
  TestTwingo b;
  b.setup();
  auto f = collect(b, 0x5D7, 1000, 1500);
  ASSERT_GE(f.size(), 2u);
  EXPECT_EQ(hex_of(f[1].data.u8, 8), "00 00 08 D4 85 C0 C2 00");
  RenaultTwingoGen1Battery::odo_5d7_km = 5000000;  // above the 32 bit limit: clamped, no overflow
  f = collect(b, 0x5D7, 1500, 1700);
  ASSERT_FALSE(f.empty());
  const uint32_t odo =
      ((uint32_t)f[0].data.u8[2] << 24) | (f[0].data.u8[3] << 16) | (f[0].data.u8[4] << 8) | f[0].data.u8[5];
  EXPECT_EQ(odo, (2684354u * 100u) << 4);
}

TEST(TwingoBuild0810, Frame426KilometresAndByte7AreEditable) {
  OdoGuard guard;
  TestTwingo b;
  b.setup();
  auto f = collect(b, 0x426, 1000, 1300);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(hex_of(f[0].data.u8, 8), "00 60 01 00 4B C8 00 40");  // unchanged default: 19,400 km
  RenaultTwingoGen1Battery::odo_426_km = 65535;
  RenaultTwingoGen1Battery::odo_426_b7 = 0x00;
  f = collect(b, 0x426, 1300, 1600);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(hex_of(f[0].data.u8, 8), "00 60 01 00 FF FF 00 00");
  RenaultTwingoGen1Battery::odo_426_km = 70000;  // above 65,535: clamped
  f = collect(b, 0x426, 1600, 1900);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(hex_of(f[0].data.u8, 8), "00 60 01 00 FF FF 00 00");
}

// ---------------------------------------------------------------------------------------------------------
// Point 16: Zoe Gen2 frames 0x373 / 0x375 / 0x376
TEST(TwingoBuild0810, Frame376CarriesTheAgeAsBase255Digits) {
  uint8_t d[8];
  twingo::frame_376(1312784, d);  // 20 * 65025 + 48 * 255 + 44
  EXPECT_EQ(hex_of(d, 8), "14 30 2C 14 30 2C 0A 00");
  twingo::frame_376(0, d);
  EXPECT_EQ(hex_of(d, 8), "00 00 00 00 00 00 0A 00");
  twingo::frame_376(254, d);
  EXPECT_EQ(d[2], 254);
  twingo::frame_376(255, d);
  EXPECT_EQ(hex_of(d, 3), "00 01 00");
}

TEST(TwingoBuild0810, ZoeRowsSendTheirFramesAndAreOffByDefault) {
  OdoGuard guard;
  for (uint32_t id : {0x373u, 0x375u, 0x376u}) {
    const int row = row_of(id);
    ASSERT_GE(row, 0);
    EXPECT_FALSE(RenaultTwingoGen1Battery::sim_row_enabled(row)) << std::hex << id;
    EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[row].interval_ms, 100);
  }
  datalayer_extended.twingoGen1.simulator_enabled_mask =
      0x3FFULL | (1ULL << row_of(0x373)) | (1ULL << row_of(0x375)) | (1ULL << row_of(0x376));
  TestTwingo b;
  b.setup();
  auto f373 = collect(b, 0x373, 1000, 2200);
  ASSERT_GE(f373.size(), 11u);
  EXPECT_EQ(hex_of(f373[0].data.u8, 8), "C1 40 5D B2 00 01 FF E3");
  EXPECT_EQ(hex_of(f373[4].data.u8, 8), "C1 40 5D B2 00 01 FF E3");
  EXPECT_EQ(hex_of(f373[5].data.u8, 8), "C1 40 B2 5D 00 01 FF E3");  // swapped after 5 frames
  EXPECT_EQ(hex_of(f373[10].data.u8, 8), "C1 40 5D B2 00 01 FF E3");
  auto f375 = collect(b, 0x375, 2200, 2500);
  ASSERT_FALSE(f375.empty());
  EXPECT_EQ(hex_of(f375[0].data.u8, 8), "02 29 00 BF FE 64 00 FF");
  auto f376 = collect(b, 0x376, 2500, 2800);
  ASSERT_FALSE(f376.empty());
  EXPECT_EQ(hex_of(f376[0].data.u8, 8), "14 30 2C 14 30 2C 0A 00");  // the age 1,312,784 of the seed
}

// ---------------------------------------------------------------------------------------------------------
// Point 12: target DB / DC of the free request
TEST(TwingoBuild0810, FreeRequestGoesToTheSelectedTarget) {
  struct TargetGuard {
    ~TargetGuard() { RenaultTwingoGen1Battery::uq_target = 0; }
  } guard;
  TestTwingo b;
  b.setup();
  EXPECT_EQ(RenaultTwingoGen1Battery::uq_target, 0);
  // DB (default): request on 0x18DADBF1
  std::vector<CAN_frame> tx;
  uint64_t t = 1000;
  set_millis64(t);
  for (; t < 1700; t += 10) {  // let the first polls pass
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  ASSERT_STREQ(b.start_user_query("22 92 5E"), "OK");
  for (const CAN_frame& f : get_transmitted_frames()) {
    if (f.ext_ID && f.data.u8[1] == 0x22 && f.data.u8[2] == 0x92) {
      EXPECT_EQ(f.ID, 0x18DADBF1u);
    }
  }
}

TEST(TwingoBuild0810, FreeRequestToDcUsesTheSafetyCpuIds) {
  struct TargetGuard {
    ~TargetGuard() { RenaultTwingoGen1Battery::uq_target = 0; }
  } guard;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  for (; t < 1700; t += 10) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  RenaultTwingoGen1Battery::uq_target = 1;
  EXPECT_STRNE(b.start_user_query("2E 92 61 00 00 03"), "OK");  // writes are refused for DC
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("22 92 5E"), "OK");
  bool seen = false;
  for (const CAN_frame& f : get_transmitted_frames()) {
    if (f.ext_ID && f.data.u8[1] == 0x22 && f.data.u8[2] == 0x92 && f.data.u8[3] == 0x5E) {
      EXPECT_EQ(f.ID, 0x18DADCF1u);
      seen = true;
    }
  }
  EXPECT_TRUE(seen);
  // reply from the safety CPU on 0x18DAF1DC is taken as the answer
  CAN_frame r = {};
  r.ext_ID = true;
  r.DLC = 8;
  r.ID = 0x18DAF1DC;
  const uint8_t data[8] = {0x04, 0x62, 0x92, 0x5E, 0x2A, 0xAA, 0xAA, 0xAA};
  memcpy(r.data.u8, data, 8);
  b.handle_incoming_can_frame(r);
  EXPECT_NE(std::string(b.user_query_result()).find("62 92 5E 2A"), std::string::npos) << b.user_query_result();
  // back to DB after the exchange: the polling frame has the MCPU ID again
  for (uint64_t e = t + 700; t < e; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      EXPECT_NE(f.ID, 0x18DADCF1u);
    }
  }
}
