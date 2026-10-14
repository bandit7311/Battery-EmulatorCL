#include <gtest/gtest.h>
#include <cstring>
#include <set>
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

// Bau 17 (14.10.): parameter block of BATTERY.h: the start values of the row switches as bools, the page switch
// TWINGO_SIM_PAGE. This file is built in both variants (cmake -DTWINGO_SIM_PAGE=ON / OFF).

namespace {
using Bat = RenaultTwingoGen1Battery;

const uint32_t DEFAULT_IDS[12] = {0x157, 0x0EC, 0x0ED, 0x19F, 0x426, 0x436, 0x423, 0x419, 0x428, 0x42F, 0x435, 0x69F};

bool is_default_id(uint32_t id) {
  for (uint32_t d : DEFAULT_IDS) {
    if (d == id) {
      return true;
    }
  }
  return false;
}

class ParamTwingo : public RenaultTwingoGen1Battery {
 public:
  ParamTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.simulator_enabled_mask = SIM_DEFAULT_MASK;
    t.simulator_enabled_mask_hi = 0;
    t.bus_format_zoe_old = false;  // the format of the real bus
    t.bus_state_manual = true;
    t.bus_state = twingo_bus::BUS_FAHRT;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.sleep_button_enabled = false;
    t.vehicle_id_custom = false;
  }
  ~ParamTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.bus_state_manual = false;
    t.simulator_enabled_mask = 0x10;
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
}  // namespace

TEST(TwingoParameterBlock, DefaultMaskIsTheTwelveFramesOfTheAuthorsBench) {
  EXPECT_EQ(SIM_DEFAULT_MASK, 0x0007391Fu);
  int on = 0;
  for (int i = 0; i < Bat::SIM_SIGNAL_COUNT; i++) {
    const bool bit = (SIM_DEFAULT_MASK >> i) & 1u;
    EXPECT_EQ(bit, is_default_id(Bat::sim_signals[i].id)) << "row " << i << " 0x" << std::hex << Bat::sim_signals[i].id;
    on += bit ? 1 : 0;
  }
  EXPECT_EQ(on, 12);
  EXPECT_EQ(SIM_DEFAULT_MASK >> Bat::SIM_SIGNAL_COUNT, 0u) << "no bit beyond the table";
}

TEST(TwingoParameterBlock, TheFlagsAreInTheOrderOfTheTable) {
  // spot checks of the named flags against the row indices
  EXPECT_TRUE(SIM_ON_19F);
  EXPECT_EQ(Bat::sim_signals[0].id, 0x19Fu);
  EXPECT_FALSE(SIM_ON_155);
  EXPECT_EQ(Bat::sim_signals[5].id, 0x155u);
  EXPECT_TRUE(SIM_ON_157);
  EXPECT_EQ(Bat::sim_signals[8].id, 0x157u);
  EXPECT_TRUE(SIM_ON_435);
  EXPECT_EQ(Bat::sim_signals[18].id, 0x435u);
  EXPECT_FALSE(SIM_ON_659);
  EXPECT_EQ(Bat::sim_signals[29].id, 0x659u);
}

TEST(TwingoParameterBlock, StartMaskRulesWithAndWithoutThePage) {
  using twingo::initial_row_mask;
  const uint32_t def = SIM_DEFAULT_MASK;
  // without the page: always the default, whatever the flash holds
  EXPECT_EQ(initial_row_mask(false, true, 0x1234u, true, 0x55u, def), def);
  EXPECT_EQ(initial_row_mask(false, false, 0, false, 0, def), def);
  // with the page: the mask stored by the page wins, then the one carried over from the old numbering, then the default
  EXPECT_EQ(initial_row_mask(true, true, 0x1234u, true, 0x55u, def), 0x1234u);
  EXPECT_EQ(initial_row_mask(true, true, 0u, false, 0u, def), 0u) << "all rows off, stored on purpose, stays off";
  EXPECT_EQ(initial_row_mask(true, false, 0, true, 0x55u, def), 0x55u);
  EXPECT_EQ(initial_row_mask(true, false, 0, false, 0, def), def);
}

TEST(TwingoParameterBlock, DefaultMaskSendsOnlyTheTwelveFramesAndTheBusRowsAreStillSent) {
  ParamTwingo b;
  b.setup();
  std::set<uint32_t> ids;
  for (uint64_t t = 1000; t < 1000 + 8000; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      if (!f.ext_ID && f.ID != 0x79B) {
        ids.insert(f.ID);
      }
    }
  }
  for (uint32_t id : ids) {
    EXPECT_TRUE(is_default_id(id)) << "0x" << std::hex << id << " is sent but its flag is false";
  }
  // the Zoe-form rows
  for (uint32_t id : {0x19Fu, 0x423u, 0x426u, 0x436u, 0x69Fu}) {
    EXPECT_TRUE(ids.count(id)) << "0x" << std::hex << id;
  }
  // and at least some of the rows of the bus (send_simulator_signals is no longer behind TWINGO_FAST_VEHICLE_FRAMES)
  EXPECT_TRUE(ids.count(0x419) || ids.count(0x428) || ids.count(0x42F) || ids.count(0x435) || ids.count(0x0EC) ||
              ids.count(0x0ED))
      << "no bus row was sent";
}

TEST(TwingoParameterBlock, SimulatorPageAndItsButtonOnlyExistWithTheSwitch) {
  ParamTwingo b;
  b.setup();
  const std::string page = b.get_uds_info_html().c_str();
#ifdef TWINGO_SIM_PAGE
  EXPECT_NE(page.find("Open CAN Signal Simulator page"), std::string::npos);
  EXPECT_NE(page.find("window.open('/simulator'"), std::string::npos);
#else
  EXPECT_EQ(page.find("Open CAN Signal Simulator page"), std::string::npos);
  EXPECT_EQ(page.find("/simulator"), std::string::npos);
#endif
  // the control block and the vehicle ID field are part of the Advanced page in both variants
  EXPECT_NE(page.find("id='busfmt0'"), std::string::npos);
  EXPECT_NE(page.find("id='twingoVidEn'"), std::string::npos);
}
