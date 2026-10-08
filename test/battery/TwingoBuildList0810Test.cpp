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

// Tests of the build of 08.10. (points 1 to 17 of the Twingo build list). Reference values come from the real vehicle
// logs of 02.10. and 04.10., the Lade-Log of 20.11.2025 and the bench readings of 06.10.; the numbers are explained
// at the code they test.

namespace {

class TestTwingo : public RenaultTwingoGen1Battery {
 public:
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t&) override { return false; }
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
