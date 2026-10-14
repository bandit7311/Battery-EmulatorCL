#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-LOGIC.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 18 (14.10.): vehicle ID of 0x69F as an input field on the Advanced page (switch "use own vehicle ID", Set, NVM).

TEST(TwingoVehicleIdParse, AcceptsEightHexDigitsWithOrWithoutSpacesAndAnyCase) {
  uint8_t id[4] = {0, 0, 0, 0};
  EXPECT_TRUE(twingo::parse_vehicle_id("46 13 88 6F", id));
  EXPECT_EQ(id[0], 0x46);
  EXPECT_EQ(id[1], 0x13);
  EXPECT_EQ(id[2], 0x88);
  EXPECT_EQ(id[3], 0x6F);
  EXPECT_TRUE(twingo::parse_vehicle_id("4613886f", id));
  EXPECT_EQ(id[3], 0x6F);
  EXPECT_TRUE(twingo::parse_vehicle_id("  a1 b2c3   D4 ", id));
  EXPECT_EQ(id[0], 0xA1);
  EXPECT_EQ(id[1], 0xB2);
  EXPECT_EQ(id[2], 0xC3);
  EXPECT_EQ(id[3], 0xD4);
}

TEST(TwingoVehicleIdParse, RefusesEverythingElseAndKeepsTheOutput) {
  uint8_t id[4] = {1, 2, 3, 4};
  for (const char* bad : {"", "46 13 88", "46 13 88 6F 00", "46 13 88 6G", "46-13-88-6F", "0x46 13 88 6F",
                          "00 00 00 00", "FF FF FF FF", "4613886"}) {
    EXPECT_FALSE(twingo::parse_vehicle_id(bad, id)) << bad;
  }
  EXPECT_FALSE(twingo::parse_vehicle_id(nullptr, id));
  EXPECT_EQ(id[0], 1);
  EXPECT_EQ(id[3], 4);
}

namespace {

class IdTwingo : public RenaultTwingoGen1Battery {
 public:
  IdTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    memset(t.vehicle_id, 0, sizeof(t.vehicle_id));
    t.vehicle_id_custom = false;
    t.simulator_enabled_mask = 0x10;  // 0x69F only
    t.bus_state_manual = false;
  }
  ~IdTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    memset(t.vehicle_id, 0, sizeof(t.vehicle_id));
    t.vehicle_id_custom = false;
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

// The data bytes of every 0x69F frame sent during `ms` milliseconds.
std::vector<std::string> ids_sent(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms) {
  std::vector<std::string> out;
  for (uint64_t end = t + ms; t < end; t += 100) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      if (!f.ext_ID && f.ID == 0x69F) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02X %02X %02X %02X", f.data.u8[0], f.data.u8[1], f.data.u8[2], f.data.u8[3]);
        out.push_back(buf);
      }
    }
  }
  return out;
}

}  // namespace

TEST(TwingoVehicleId, DefaultIsTheIdOfTheCarEvenWithATypedIdWhileTheSwitchIsOff) {
  IdTwingo b;
  b.setup();
  uint64_t t = 1000;
  auto sent = ids_sent(b, t, 3500);
  ASSERT_GE(sent.size(), 2u);
  for (const auto& s : sent) {
    EXPECT_EQ(s, "46 13 88 6F");
  }
  auto& d = datalayer_extended.twingoGen1;
  const uint8_t typed[4] = {0x12, 0x34, 0x56, 0x78};
  memcpy(d.vehicle_id, typed, 4);  // typed and stored, switch off
  b.apply_vehicle_id();
  sent = ids_sent(b, t, 2500);
  ASSERT_GE(sent.size(), 1u);
  for (const auto& s : sent) {
    EXPECT_EQ(s, "46 13 88 6F");
  }
}

TEST(TwingoVehicleId, OwnIdIsSentWhenTheSwitchIsOnAndTheDefaultComesBackWhenItGoesOff) {
  IdTwingo b;
  b.setup();
  auto& d = datalayer_extended.twingoGen1;
  const uint8_t typed[4] = {0x12, 0x34, 0x56, 0x78};
  memcpy(d.vehicle_id, typed, 4);
  d.vehicle_id_custom = true;
  b.apply_vehicle_id();
  uint64_t t = 1000;
  auto sent = ids_sent(b, t, 3500);
  ASSERT_GE(sent.size(), 2u);
  for (const auto& s : sent) {
    EXPECT_EQ(s, "12 34 56 78");
  }
  uint8_t now[4];
  RenaultTwingoGen1Battery::effective_vehicle_id(now);
  EXPECT_EQ(now[0], 0x12);
  d.vehicle_id_custom = false;
  b.apply_vehicle_id();
  sent = ids_sent(b, t, 2500);
  ASSERT_GE(sent.size(), 1u);
  for (const auto& s : sent) {
    EXPECT_EQ(s, "46 13 88 6F");
  }
  EXPECT_EQ(d.vehicle_id[0], 0x12) << "the typed ID stays stored";
}

TEST(TwingoVehicleId, SwitchOnWithNothingTypedStillSendsTheDefault) {
  IdTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.vehicle_id_custom = true;  // box ticked, field never set (all zero)
  b.apply_vehicle_id();
  uint64_t t = 1000;
  for (const auto& s : ids_sent(b, t, 3500)) {
    EXPECT_EQ(s, "46 13 88 6F");
  }
}

TEST(TwingoVehicleId, AdvancedPageHasTheRowBetweenB009AndTheSleepFailsafe) {
  IdTwingo b;
  b.setup();
  const std::string page = b.get_uds_info_html().c_str();
  const size_t b009 = page.find("B009 diagnostic session");
  const size_t row = page.find("id='twingoVidEn'");
  const size_t failsafe = page.find("Sleep failsafe manual / push button hardware (IO15)");
  ASSERT_NE(b009, std::string::npos);
  ASSERT_NE(row, std::string::npos);
  ASSERT_NE(failsafe, std::string::npos);
  EXPECT_LT(b009, row);
  EXPECT_LT(row, failsafe);
  EXPECT_NE(page.find("id='twingoVid' size='11' maxlength='11' value='46 13 88 6F'"), std::string::npos);
  EXPECT_NE(page.find("<button onclick='twingoSetVehicleId(false)'>Set</button>"), std::string::npos);
  EXPECT_NE(page.find("sending 46 13 88 6F (default of the car)"), std::string::npos);
  EXPECT_NE(page.find("/editTwingoVehicleId?en="), std::string::npos);
  EXPECT_EQ(page.find("id='twingoVidEn' onclick='twingoSetVehicleId(true)' checked"), std::string::npos);
  // switch on with a typed ID: the box is ticked, the field shows the typed ID, the state says "own ID"
  auto& d = datalayer_extended.twingoGen1;
  const uint8_t typed[4] = {0xAB, 0xCD, 0xEF, 0x01};
  memcpy(d.vehicle_id, typed, 4);
  d.vehicle_id_custom = true;
  const std::string page2 = b.get_uds_info_html().c_str();
  EXPECT_NE(page2.find("id='twingoVidEn' onclick='twingoSetVehicleId(true)' checked>"), std::string::npos);
  EXPECT_NE(page2.find("value='AB CD EF 01'"), std::string::npos);
  EXPECT_NE(page2.find("sending AB CD EF 01 (own ID)"), std::string::npos);
}
