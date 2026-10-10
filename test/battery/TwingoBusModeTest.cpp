#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-LOGIC.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "../../Software/src/devboard/webserver/simulator_html.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Tests of the 10.10. build: real bus frames of /simulator block 1, 128 bit row mask, reception tracking, cell
// broadcast (Prio 2). Frames in the comments come from the car log 22aaf176 and the bench log 25.09.

namespace {

using Bat = RenaultTwingoGen1Battery;

class BusTwingo : public RenaultTwingoGen1Battery {
 public:
  BusTwingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.bus_format_zoe_old = false;
    t.bus_state = twingo_bus::BUS_WACH;
    t.cell_source_broadcast = false;
    t.simulator_enabled_mask = 0x3FF;
    t.simulator_enabled_mask_hi = 0;
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

std::vector<Tx> with_id(const std::vector<Tx>& v, uint32_t id, bool ext = false) {
  std::vector<Tx> out;
  for (const Tx& x : v) {
    if (x.f.ID == id && x.f.ext_ID == ext) {
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

std::string hex_of(const uint8_t* d, int n) {
  std::string s;
  char b[4];
  for (int i = 0; i < n; i++) {
    snprintf(b, sizeof(b), i ? " %02X" : "%02X", d[i]);
    s += b;
  }
  return s;
}

void only_rows(std::initializer_list<uint32_t> ids) {
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0;
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
  for (uint32_t id : ids) {
    Bat::sim_row_set((uint8_t)row_of(id), true);
  }
}

CAN_frame rx11(uint32_t id, std::initializer_list<uint8_t> d) {
  CAN_frame f = {};
  f.ext_ID = false;
  f.DLC = 8;
  f.ID = id;
  int i = 0;
  for (uint8_t v : d) {
    f.data.u8[i++] = v;
  }
  return f;
}

}  // namespace

// ---------------------------------------------------------------------------
// Cell broadcast decoding (Prio 2)
// ---------------------------------------------------------------------------

TEST(TwingoCellBroadcast, BenchFrameOf5F7DecodesTo4162And4164) {
  const uint8_t d[8] = {0x87, 0x28, 0x74, 0x87, 0x48, 0x74, 0x87, 0x4F};  // bench log, 0x5F7
  uint16_t mv[5];
  EXPECT_EQ(twingo_bus::decode_cell_frame(0, d, mv), 5);
  EXPECT_EQ(mv[0], 4162);
  EXPECT_EQ(mv[1], 4164);
  EXPECT_EQ(mv[2], 4164);
  EXPECT_EQ(mv[3], 4164);
  EXPECT_EQ(mv[4], 4164);
}

TEST(TwingoCellBroadcast, CarFrameOf5F7DecodesTo3539ForAllFiveCells) {
  const uint8_t d[8] = {0x60, 0x36, 0x03, 0x60, 0x36, 0x03, 0x60, 0x3F};  // car log 22aaf176, T+96.68 s
  uint16_t mv[5];
  twingo_bus::decode_cell_frame(0, d, mv);
  for (int k = 0; k < 5; k++) {
    EXPECT_EQ(mv[k], 3539) << k;
  }
}

TEST(TwingoCellBroadcast, PlaceholdersOfTheWakeUpAreIgnored) {
  const uint8_t ff[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // car log T+84.67 s
  const uint8_t zero[8] = {0, 0, 0, 0, 0, 0, 0, 0x0F};                      // car log T+87.67 s
  uint16_t mv[5];
  twingo_bus::decode_cell_frame(0, ff, mv);
  for (int k = 0; k < 5; k++) {
    EXPECT_EQ(mv[k], 0) << k;
  }
  twingo_bus::decode_cell_frame(0, zero, mv);
  for (int k = 0; k < 5; k++) {
    EXPECT_EQ(mv[k], 0) << k;
  }
}

TEST(TwingoCellBroadcast, FrameOf5DDCarriesOneCell) {
  const uint8_t d[8] = {0x87, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint16_t mv[5];
  EXPECT_EQ(twingo_bus::decode_cell_frame(19, d, mv), 1);
  EXPECT_EQ(mv[0], 4164);  // 0x874 + 2000
  for (int k = 1; k < 5; k++) {
    EXPECT_EQ(mv[k], 0);
  }
}

TEST(TwingoCellBroadcast, CellNumbersFollowTheFrameOrder) {
  EXPECT_EQ(twingo_bus::cell_frame_index(0x5F7), 0);
  EXPECT_EQ(twingo_bus::first_cell(0), 0);  // cells 1-5
  EXPECT_EQ(twingo_bus::cell_frame_index(0x5D9), 7);
  EXPECT_EQ(twingo_bus::first_cell(7), 35);  // cells 36-40
  EXPECT_EQ(twingo_bus::cell_frame_index(0x5EC), 8);
  EXPECT_EQ(twingo_bus::first_cell(8), 40);  // cells 41-45 (stands in for A18)
  EXPECT_EQ(twingo_bus::cell_frame_index(0x5A1), 18);
  EXPECT_EQ(twingo_bus::first_cell(18), 90);  // cells 91-95
  EXPECT_EQ(twingo_bus::cell_frame_index(0x5DD), 19);
  EXPECT_EQ(twingo_bus::first_cell(19), 95);  // cell 96
  EXPECT_EQ(twingo_bus::cell_frame_index(0x155), -1);
  for (int i = 0; i < twingo_bus::CELL_FRAME_COUNT; i++) {
    EXPECT_EQ(Bat::RX_CELL_IDS[i], twingo_bus::CELL_FRAME_IDS[i]) << i;
  }
}

TEST(TwingoCellBroadcast, BroadcastModeFillsTheDatalayerAndThePackVoltage) {
  BusTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.cell_source_broadcast = true;
  for (int i = 0; i < 96; i++) {
    datalayer.battery.status.cell_voltages_mV[i] = 0;
  }
  for (int k = 0; k < twingo_bus::CELL_FRAME_COUNT; k++) {
    b.handle_incoming_can_frame(rx11(twingo_bus::CELL_FRAME_IDS[k], {0x87, 0x28, 0x74, 0x87, 0x48, 0x74, 0x87, 0x4F}));
  }
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[0], 4162);
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[1], 4164);
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[94], 4164);
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[95], 4162);  // 0x5DD: one cell, first value of the frame
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.cell_min_voltage_mV, 4162);
  EXPECT_EQ(datalayer.battery.status.cell_max_voltage_mV, 4164);
  EXPECT_EQ(datalayer.battery.status.voltage_dV, 3997);  // 4162 * 2 + 4164 * 94 mV = 399,640 mV -> 3996 dV
}

TEST(TwingoCellBroadcast, UdsModeIgnoresTheCellFrames) {
  BusTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.cell_source_broadcast = false;
  for (int i = 0; i < 96; i++) {
    datalayer.battery.status.cell_voltages_mV[i] = 0;
  }
  b.handle_incoming_can_frame(rx11(0x5F7, {0x87, 0x28, 0x74, 0x87, 0x48, 0x74, 0x87, 0x4F}));
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[0], 0);
}

TEST(TwingoCellBroadcast, PlaceholderFrameLeavesTheOldValues) {
  BusTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.cell_source_broadcast = true;
  datalayer.battery.status.cell_voltages_mV[0] = 3700;
  b.handle_incoming_can_frame(rx11(0x5F7, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}));
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[0], 3700);
}

TEST(TwingoCellBroadcast, BroadcastModeSkipsTheUdsCellPolls) {
  for (int broadcast = 0; broadcast < 2; broadcast++) {
    BusTwingo b;
    b.setup();
    datalayer_extended.twingoGen1.cell_source_broadcast = (broadcast == 1);
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 30000, 10, log);  // more than one full poll cycle of 136 PIDs
    int cell_polls = 0, other_polls = 0;
    for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
      if (x.f.data.u8[1] != 0x22) {
        continue;
      }
      const uint16_t pid = (uint16_t)((x.f.data.u8[2] << 8) | x.f.data.u8[3]);
      (pid >= 0x9021 && pid <= 0x9083) ? cell_polls++ : other_polls++;
    }
    EXPECT_GT(other_polls, 10) << "broadcast=" << broadcast;
    if (broadcast) {
      EXPECT_EQ(cell_polls, 0);
    } else {
      EXPECT_GT(cell_polls, 0);
    }
  }
  datalayer_extended.twingoGen1.cell_source_broadcast = false;
}

// ---------------------------------------------------------------------------
// Vehicle format of the five bus frames
// ---------------------------------------------------------------------------

TEST(TwingoBusFormat, Frame426InTheVehicleFormatCarriesStateAndKm) {
  const uint32_t save_km = Bat::odo_426_km;
  Bat::odo_426_km = 6844;  // 0x1ABC
  const struct {
    uint8_t state;
    const char* bytes;
  } cases[] = {{twingo_bus::BUS_ZU, "00 00 02 01 1A BC 00 40"}, {twingo_bus::BUS_WACH, "00 00 06 01 1A BC 00 40"}};
  for (const auto& c : cases) {
    BusTwingo b;
    b.setup();
    datalayer_extended.twingoGen1.bus_state = c.state;
    only_rows({0x426});
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 500, 10, log);
    auto f = with_id(log, 0x426);
    ASSERT_FALSE(f.empty());
    EXPECT_EQ(hex_of(f[0].f.data.u8, 8), c.bytes) << (int)c.state;
  }
  Bat::odo_426_km = save_km;
}

TEST(TwingoBusFormat, ZoeOldFormatKeepsTheOldFrame) {
  const uint32_t save_km = Bat::odo_426_km;
  Bat::odo_426_km = 6844;
  BusTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.bus_format_zoe_old = true;
  only_rows({0x426});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 500, 10, log);
  auto f = with_id(log, 0x426);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(hex_of(f[0].f.data.u8, 8), "00 60 01 00 1A BC 00 40");
  datalayer_extended.twingoGen1.bus_format_zoe_old = false;
  Bat::odo_426_km = save_km;
}

TEST(TwingoBusFormat, Frame19FRunsEvery10msInTheVehicleFormat) {
  BusTwingo b;
  b.setup();
  only_rows({0x19F});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 1, log);
  auto f = with_id(log, 0x19F);
  EXPECT_GE(f.size(), 95u);
  EXPECT_LE(f.size(), 102u);
  for (size_t i = 1; i < f.size(); i++) {
    EXPECT_GE(f[i].t - f[i - 1].t, 10u);
  }
}

TEST(TwingoBusFormat, Frame436CarriesTheVehicleAgeInTheVehicleFormat) {
  BusTwingo b;
  b.setup();
  only_rows({0x436});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 500, 10, log);
  auto f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(f[0].f.data.u8[0], 0x80);
  EXPECT_EQ(hex_of(&f[0].f.data.u8[1], 3), "0F A5 15");  // 1,025,301 min
}

// ---------------------------------------------------------------------------
// R rows
// ---------------------------------------------------------------------------

TEST(TwingoBusRows, TableHas25RRowsAndFiveBusRowsOfTheOldTable) {
  int r = 0;
  for (int i = 0; i < Bat::SIM_SIGNAL_COUNT; i++) {
    if (Bat::sim_signals[i].tag == 'R') {
      EXPECT_GE(i, 46);
      r++;
    }
  }
  EXPECT_EQ(r, 25);
  EXPECT_EQ((int)Bat::SIM_SIGNAL_COUNT, 71);
  for (int i = 46; i < Bat::SIM_SIGNAL_COUNT; i++) {
    EXPECT_NE(twingo_bus::find_frame((uint16_t)Bat::sim_signals[i].id), nullptr) << i;
  }
}

TEST(TwingoBusRows, RRowsAreOffByDefault) {
  BusTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x387;  // the default of the old rows
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 10, log);
  for (int i = 46; i < Bat::SIM_SIGNAL_COUNT; i++) {
    EXPECT_TRUE(with_id(log, Bat::sim_signals[i].id).empty()) << std::hex << Bat::sim_signals[i].id;
  }
}

TEST(TwingoBusRows, Frame500OnlyRunsWhileTheIgnitionIsOn) {
  for (uint8_t state : {(uint8_t)twingo_bus::BUS_WACH, (uint8_t)twingo_bus::BUS_ZUENDUNG1}) {
    BusTwingo b;
    b.setup();
    datalayer_extended.twingoGen1.bus_state = state;
    only_rows({0x500});
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 1000, 10, log);
    EXPECT_EQ(!with_id(log, 0x500).empty(), state == twingo_bus::BUS_ZUENDUNG1) << (int)state;
  }
}

TEST(TwingoBusRows, Counters511StepLikeInTheCar) {
  BusTwingo b;
  b.setup();
  only_rows({0x511});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1000, 10, log);
  auto f = with_id(log, 0x511);
  ASSERT_GE(f.size(), 3u);
  const uint8_t step[6] = {63, 201, 185, 107, 13, 227};
  for (size_t i = 1; i < f.size(); i++) {
    for (int k = 0; k < 6; k++) {
      EXPECT_EQ((uint8_t)(f[i].f.data.u8[1 + k] - f[i - 1].f.data.u8[1 + k]), step[k]) << i << " byte " << k + 1;
    }
  }
}

// ---------------------------------------------------------------------------
// 128 bit row mask
// ---------------------------------------------------------------------------

TEST(TwingoRowMask, RowsAbove63LiveInTheSecondWord) {
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0;
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
  Bat::sim_row_set(70, true);
  EXPECT_TRUE(Bat::sim_row_enabled(70));
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask_hi, 1ULL << 6);
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 0ULL);
  Bat::sim_row_set(63, true);
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 1ULL << 63);
  Bat::sim_row_set(70, false);
  EXPECT_FALSE(Bat::sim_row_enabled(70));
  EXPECT_FALSE(Bat::sim_row_enabled(200));
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
}

TEST(TwingoRowMask, AllOffAndRestoreCoverBothWords) {
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0x45;
  Bat::sim_mask_saved_valid = false;
  Bat::sim_all_off();
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 0ULL);
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask_hi, 0ULL);
  Bat::sim_restore();
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 0x3FFULL);
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask_hi, 0x45ULL);
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
}

// ---------------------------------------------------------------------------
// Reception tracking and status JSON
// ---------------------------------------------------------------------------

TEST(TwingoRxTracking, FirstLastAndCountPerRow) {
  BusTwingo b;
  b.setup();
  Bat::rx_reset(1000);
  const int row = row_of(0x155);
  ASSERT_GE(row, 46);
  set_millis64(2000);
  b.handle_incoming_can_frame(rx11(0x155, {0}));
  b.handle_incoming_can_frame(rx11(0x155, {0}));
  set_millis64(2500);
  b.handle_incoming_can_frame(rx11(0x155, {0}));
  EXPECT_EQ(Bat::rx_row[row].count, 3u);
  EXPECT_EQ(Bat::rx_row[row].first_ms, 2000u);
  EXPECT_EQ(Bat::rx_row[row].last_ms, 2500u);
}

TEST(TwingoRxTracking, CellFramesAndOtherIdsAreSeparate) {
  BusTwingo b;
  b.setup();
  Bat::rx_reset(0);
  set_millis64(5000);
  b.handle_incoming_can_frame(rx11(0x5F4, {0}));
  b.handle_incoming_can_frame(rx11(0x7AB, {0}));
  b.handle_incoming_can_frame(rx11(0x7AB, {0}));
  EXPECT_EQ(Bat::rx_cell[1].count, 1u);
  ASSERT_EQ(Bat::rx_other_count, 1);
  EXPECT_EQ(Bat::rx_other_id[0], 0x7AB);
  EXPECT_EQ(Bat::rx_other[0].count, 2u);
}

TEST(TwingoRxTracking, ExtendedFramesAreNotCounted) {
  BusTwingo b;
  b.setup();
  Bat::rx_reset(0);
  CAN_frame f = rx11(0x155, {0});
  f.ext_ID = true;
  b.handle_incoming_can_frame(f);
  EXPECT_EQ(Bat::rx_row[row_of(0x155)].count, 0u);
  EXPECT_EQ(Bat::rx_other_count, 0);
}

TEST(TwingoRxTracking, OnlyAllRowsOffSetsTimeZero) {
  BusTwingo b;
  b.setup();
  Bat::rx_reset(100);
  set_millis64(3000);
  b.handle_incoming_can_frame(rx11(0x155, {0}));
  Bat::note_action("test action");
  String j = Bat::rx_status_json(4000);
  EXPECT_NE(std::string(j.c_str()).find("\"now\":3900"), std::string::npos);
  EXPECT_NE(std::string(j.c_str()).find("\"acttxt\":\"test action\""), std::string::npos) << j.c_str();
  Bat::sim_mask_saved_valid = false;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  Bat::sim_all_off();  // T+0 and every "first seen" cleared
  EXPECT_EQ(Bat::rx_row[row_of(0x155)].count, 0u);
  Bat::sim_restore();
}

TEST(TwingoRxTracking, StatusJsonListsRowsCellsAndOthers) {
  BusTwingo b;
  b.setup();
  Bat::rx_reset(0);
  set_millis64(1000);
  b.handle_incoming_can_frame(rx11(0x155, {0}));
  b.handle_incoming_can_frame(rx11(0x5F7, {0}));
  b.handle_incoming_can_frame(rx11(0x7AB, {0}));
  std::string j = Bat::rx_status_json(1500).c_str();
  EXPECT_NE(j.find("\"r\":[[" + std::to_string(row_of(0x155)) + ",1000,1000,1]]"), std::string::npos) << j;
  EXPECT_NE(j.find("\"c\":[[0,1000,1000,1]]"), std::string::npos) << j;
  EXPECT_NE(j.find("\"o\":[[" + std::to_string(0x7AB) + ",1000,1000,1]]"), std::string::npos) << j;
  EXPECT_EQ(j.find('%'), std::string::npos);
}

// ---------------------------------------------------------------------------
// Defaults and page
// ---------------------------------------------------------------------------

TEST(TwingoBusDefaults, AgeSeedAndKilometres) {
  EXPECT_EQ(twingo::AGE_SEED_VALUE, 1025301u);
  EXPECT_EQ(twingo::AGE_SAFETY_MIN, 0u);
  EXPECT_FALSE(twingo::AGE_RAISE_FROM_PACK);
  EXPECT_EQ((uint32_t)Bat::ODO_426_MAX_KM, 65535u);
}

TEST(TwingoBusPage, HasBothBlocksTheControlsAndAllCellFrames) {
  BusTwingo b;
  b.setup();
  std::string page(simulator_processor(String("X")).c_str());
  EXPECT_NE(page.find("Block 1"), std::string::npos);
  EXPECT_NE(page.find("Block 2"), std::string::npos);
  EXPECT_NE(page.find("/twingoRxStatus"), std::string::npos);
  EXPECT_NE(page.find("/twingoBusFormat"), std::string::npos);
  EXPECT_NE(page.find("/twingoBusState"), std::string::npos);
  EXPECT_NE(page.find("/twingoCellSource"), std::string::npos);
  EXPECT_NE(page.find("Weitere empfangene IDs"), std::string::npos);
  EXPECT_NE(page.find("value='1025301'"), std::string::npos);
  EXPECT_EQ(page.find("1314935"), std::string::npos);
  for (int k = 0; k < twingo_bus::CELL_FRAME_COUNT; k++) {
    char id[12];
    snprintf(id, sizeof(id), "id='Fc%d'", k);
    EXPECT_NE(page.find(id), std::string::npos) << id;
  }
}

TEST(TwingoBusPage, Block1ShowsThirtyRowsAndBlock2FortyOne) {
  std::string page(simulator_processor(String("X")).c_str());
  const size_t b2 = page.find("Block 2: simulator alt");
  ASSERT_NE(b2, std::string::npos);
  int in1 = 0, in2 = 0;
  for (int i = 0; i < Bat::SIM_SIGNAL_COUNT; i++) {
    const size_t p = page.find("<tr id='Rr" + std::to_string(i) + "'>");
    ASSERT_NE(p, std::string::npos) << i;
    (p < b2 ? in1 : in2)++;
  }
  EXPECT_EQ(in1, 30);
  EXPECT_EQ(in2, 41);
}

// ---------------------------------------------------------------------------
// Wake-up form of 0x0EC / 0x0ED (car log 22aaf176, identical in all three sessions)
// ---------------------------------------------------------------------------

TEST(TwingoWakeForm, Frame0ECStartsWithSixFramesOf10ThenTheStateValue) {
  const twingo_bus::FrameDef* def = twingo_bus::find_frame(0x0EC);
  ASSERT_NE(def, nullptr);
  const char* real[8] = {"10 01 17", "10 11 DA", "10 21 90", "10 31 5D", "10 41 04", "10 51 C9", "1C 61 F4", "1C 71 39"};
  for (uint32_t seq = 0; seq < 8; seq++) {
    uint8_t out[8];
    twingo_bus::build_frame(*def, twingo_bus::BUS_WACH, seq, 0, 0, out);
    EXPECT_EQ(hex_of(out, 3), real[seq]) << seq;
  }
}

TEST(TwingoWakeForm, Frame0EDStartsWith10xA3FF00And21xA3FF80) {
  const twingo_bus::FrameDef* def = twingo_bus::find_frame(0x0ED);
  ASSERT_NE(def, nullptr);
  for (uint32_t seq = 0; seq < 40; seq++) {
    uint8_t out[8];
    twingo_bus::build_frame(*def, twingo_bus::BUS_WACH, seq, 0, 0, out);
    const char* want = seq < 10 ? "A3 FF 00" : seq < 31 ? "A3 FF 80" : "63 FF 80";
    EXPECT_EQ(hex_of(out, 3), want) << seq;
  }
}

TEST(TwingoWakeForm, SwitchingTheRowOffAndOnStartsANewSession) {
  BusTwingo b;
  b.setup();
  only_rows({0x0EC});
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 1, log);
  auto f = with_id(log, 0x0EC);
  ASSERT_GT(f.size(), 10u);
  EXPECT_EQ(f[0].f.data.u8[0], 0x10);
  EXPECT_EQ(f[5].f.data.u8[0], 0x10);
  EXPECT_EQ(f[6].f.data.u8[0], 0x1C);
  only_rows({});
  run(b, t, 50, 1, log);
  only_rows({0x0EC});
  log.clear();
  run(b, t, 100, 1, log);
  f = with_id(log, 0x0EC);
  ASSERT_GT(f.size(), 6u);
  EXPECT_EQ(hex_of(f[0].f.data.u8, 2), "10 01");
  EXPECT_EQ(f[6].f.data.u8[0], 0x1C);
}

TEST(TwingoBusPage, FourPartsTogetherAreTheWholePageAndEachIsSmall) {
  BusTwingo b;
  b.setup();
  std::string all(simulator_processor(String("X")).c_str());
  std::string parts;
  for (const char* name : {"A", "B", "C", "D"}) {
    std::string p(simulator_processor(String(name)).c_str());
    EXPECT_GT(p.size(), 1000u) << name;
    EXPECT_LT(p.size(), 30000u) << name;  // no String of the whole 60 kB page is needed on the device
    EXPECT_EQ(p.find('%'), std::string::npos) << name;
    parts += p;
  }
  EXPECT_EQ(parts, all);
}
