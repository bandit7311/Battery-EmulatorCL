#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-LOGIC.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "../../Software/src/devboard/utils/events.h"

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
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.bus_format_zoe_old = true;  // the legacy tests check the old Zoe form
    t.bus_state_manual = true;    // the tests choose the state themselves
  }
  bool network_ready() override { return false; }
  void start_ntp() override {}
  bool get_unix_time(time_t& t) override {
    t = 1791590400;  // the seed reference: age 1,312,784
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

// ---------------------------------------------------------------------------------------------------------
// Points 13 and 14: 0x5D7 with kilometres, 0x426 editable
namespace {
struct OdoGuard {
  ~OdoGuard() {
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

TEST(TwingoBuild0810, Frame426KilometresAndByte7AreEditable) {
  OdoGuard guard;
  RenaultTwingoGen1Battery::odo_426_km = 19400;
  RenaultTwingoGen1Battery::odo_426_b7 = 0x40;
  TestTwingo b;
  b.setup();
  auto f = collect(b, 0x426, 1000, 1300);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(hex_of(f[0].data.u8, 8), "00 60 01 00 4B C8 00 40");  // 19,400 km, byte 7 = 0x40
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

TEST(TwingoBuild0810, ZoeRowDescriptionsCarryTheFindings) {
  auto info = [](uint32_t id) {
    return std::string(RenaultTwingoGen1Battery::sim_signals[row_of(id)].info);
  };
  EXPECT_NE(info(0x426).find("mileage"), std::string::npos);
  EXPECT_NE(info(0x426).find("E14381"), std::string::npos);
  EXPECT_NE(info(0x426).find("1B0715"), std::string::npos);
  EXPECT_NE(info(0x423).find("wakes"), std::string::npos);
  EXPECT_NE(info(0x436).find("minute counter"), std::string::npos);
}

namespace {
// Runs a "Sleep 0x9281=1" run (no reset routine) and returns the frames sent on the MCPU request ID.
std::vector<CAN_frame> sleep_temporisation_frames(TestTwingo& b, uint64_t& t, uint64_t ms) {
  std::vector<CAN_frame> out;
  for (uint64_t e = t + ms; t < e; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      out.push_back(f);
    }
  }
  return out;
}
}  // namespace

TEST(TwingoBuild0810, Write9281CanBe0x80AndTheLabelShowsTheSentValue) {
  struct Guard {
    ~Guard() { datalayer_extended.twingoGen1.nvrol_temporisation_write_value = 0; }
  } guard;
  for (uint8_t v : {0x00, 0x01, 0x80}) {
    datalayer_extended.twingoGen1.nvrol_temporisation_write_value = v;
    TestTwingo b;
    b.setup();
    uint64_t t = 1000;
    sleep_temporisation_frames(b, t, 700);
    b.request_sleep_temporisation();
    auto frames = sleep_temporisation_frames(b, t, 6000);
    bool seen = false;
    for (const CAN_frame& f : frames) {
      if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[1] == 0x2E && f.data.u8[2] == 0x92 && f.data.u8[3] == 0x81) {
        EXPECT_EQ(f.data.u8[4], v);
        seen = true;
      }
    }
    EXPECT_TRUE(seen) << "value " << (int)v;
    char want[40];
    snprintf(want, sizeof(want), "Write 9281=0x%02X", v);
    EXPECT_NE(std::string(b.get_uds_info_html().c_str()).find(want), std::string::npos) << want;
  }
}

TEST(TwingoBuild0810, EventsAreAcknowledgedWhenThePollingResumes) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  sleep_temporisation_frames(b, t, 700);
  b.request_sleep_temporisation();
  sleep_temporisation_frames(b, t,
                             200000);  // write, shutdown sequence (about 137 s), closing (50.8 s), then the silence
  EXPECT_TRUE(sleep_temporisation_frames(b, t, 500).empty());
  set_event(EVENT_CAN_BATTERY_MISSING, 0);  // raised during the silence
  EXPECT_EQ(get_event_pointer(EVENT_CAN_BATTERY_MISSING)->state, EVENT_STATE_ACTIVE);
  b.request_wake_up();
  sleep_temporisation_frames(b, t, 8000);  // wake-up burst, then the polling resumes
  EXPECT_EQ(get_event_pointer(EVENT_CAN_BATTERY_MISSING)->state, EVENT_STATE_INACTIVE);
}

TEST(TwingoBuild0810, ReadRequestsInTheSilenceOnlyWithTheSwitch) {
  struct Guard {
    ~Guard() { RenaultTwingoGen1Battery::diag_in_silence = false; }
  } guard;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  sleep_temporisation_frames(b, t, 700);
  b.request_sleep_temporisation();
  sleep_temporisation_frames(b, t,
                             200000);  // write, shutdown sequence (about 137 s), closing (50.8 s), then the silence
  // the silence: nothing is on the bus any more
  EXPECT_TRUE(sleep_temporisation_frames(b, t, 500).empty());
  // switch off (default): refused
  EXPECT_FALSE(RenaultTwingoGen1Battery::diag_in_silence);
  EXPECT_STRNE(b.start_user_query("22 92 61"), "OK");
  EXPECT_TRUE(sleep_temporisation_frames(b, t, 300).empty());
  // switch on: the read goes out, the answer is shown, writes stay refused
  RenaultTwingoGen1Battery::diag_in_silence = true;
  EXPECT_STRNE(b.start_user_query("2E 92 61 00 00 03"), "OK");
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("22 92 61"), "OK");
  bool seen = false;
  for (const CAN_frame& f : get_transmitted_frames()) {  // a read goes out at once
    if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[1] == 0x22 && f.data.u8[2] == 0x92 && f.data.u8[3] == 0x61) {
      seen = true;
    }
  }
  EXPECT_TRUE(seen);
  CAN_frame r = {};
  r.ext_ID = true;
  r.DLC = 8;
  r.ID = 0x18DAF1DB;
  const uint8_t data[8] = {0x06, 0x62, 0x92, 0x61, 0x14, 0x02, 0x70, 0xAA};
  memcpy(r.data.u8, data, 8);
  b.handle_incoming_can_frame(r);
  EXPECT_NE(std::string(b.user_query_result()).find("62 92 61 14 02 70"), std::string::npos) << b.user_query_result();
  // and it is silent again afterwards
  sleep_temporisation_frames(b, t, 1000);
  EXPECT_TRUE(sleep_temporisation_frames(b, t, 500).empty());
}

TEST(TwingoBuild0810, QueryPageHasTheDirectButtonsTheTargetAndTheSilenceSwitch) {
  TestTwingo b;
  b.setup();
  const std::string html = b.get_uds_info_html().c_str();
  for (const char* want : {"DTC 19 02 09", "Time 22 92 61", "Session 10 03", "Set time now", "twingoQueryTarget",
                           "DC (safety CPU)", "twingoSilenceDiag", "twingoWrite80", "localStorage"}) {
    EXPECT_NE(html.find(want), std::string::npos) << want;
  }
}

// ---------------------------------------------------------------------------------------------------------
// 0x0EE, 0x5F8, 0x6BF (Zoe Gen2 driver)
namespace {
uint8_t crc8_j1850_start0(const uint8_t* d, int n) {  // independent bitwise check of the table version
  uint8_t crc = 0;
  for (int i = 0; i < n; i++) {
    crc ^= d[i];
    for (int b = 0; b < 8; b++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x1D) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}
}  // namespace

// ---------------------------------------------------------------------------------------------------------
// Time in 0x436 (bytes 1-3), input field like 0x426
namespace {
struct T436Guard {
  ~T436Guard() {
    RenaultTwingoGen1Battery::time_436_active = false;
    datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  }
};
}  // namespace

TEST(TwingoBuild0810, Time436DefaultIs1400WithTheMinuteCounter) {
  T436Guard guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL;  // 0x436 is row 5
  TestTwingo b;
  b.setup();
  auto f = collect(b, 0x436, 1000, 1300);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(hex_of(f[0].data.u8, 6), "86 14 00 01 FF DC");
  auto g = collect(b, 0x436, 62000, 62300);  // after the first 60 s step
  ASSERT_FALSE(g.empty());
  EXPECT_EQ(hex_of(g.back().data.u8, 6), "86 14 00 02 FF DC");
}

TEST(TwingoBuild0810, Time436CarriesTheEnteredValueAndCountsOnePerMinute) {
  T436Guard guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL;
  TestTwingo b;
  b.setup();
  collect(b, 0x436, 1000, 1100);
  b.time_436_set(1315021);  // 0x1410CD
  auto f = collect(b, 0x436, 1100, 1400);
  ASSERT_FALSE(f.empty());
  const uint32_t v = (f[0].data.u8[1] << 16) | (f[0].data.u8[2] << 8) | f[0].data.u8[3];
  EXPECT_EQ(v, 1315021u);
  EXPECT_EQ(f[0].data.u8[0], 0x86);
  EXPECT_EQ(f[0].data.u8[4], 0xFF);
  auto g = collect(b, 0x436, 62000, 62300);
  ASSERT_FALSE(g.empty());
  const uint32_t w = (g.back().data.u8[1] << 16) | (g.back().data.u8[2] << 8) | g.back().data.u8[3];
  EXPECT_EQ(w, 1315022u);
  b.time_436_clear();
  auto i = collect(b, 0x436, 62600, 62900);
  ASSERT_FALSE(i.empty());
  EXPECT_EQ(i[0].data.u8[1], 0x14);  // back to 14 00 xx
}

TEST(TwingoBuild0810, Time436ValueIsLimitedTo24Bit) {
  T436Guard guard;
  TestTwingo b;
  b.setup();
  b.time_436_set(0xFFFFFFFFu);
  EXPECT_EQ(RenaultTwingoGen1Battery::time_436_value, 0xFFFFFFu);
}

// ---------------------------------------------------------------------------------------------------------
// DTC buttons for MCPU and SCPU, 14 FF FF FF, decoding of 19 02
namespace {
CAN_frame reply_on(uint32_t id, std::initializer_list<uint8_t> bytes) {
  CAN_frame f = {};
  f.ext_ID = true;
  f.DLC = 8;
  f.ID = id;
  uint8_t i = 0;
  for (uint8_t b : bytes) {
    f.data.u8[i++] = b;
  }
  return f;
}

std::vector<CAN_frame> ext_frames_since_clear() {
  std::vector<CAN_frame> out;
  for (const CAN_frame& f : get_transmitted_frames()) {
    if (f.ext_ID) {
      out.push_back(f);
    }
  }
  return out;
}

void settle(TestTwingo& b, uint64_t& t, uint64_t ms) {
  for (uint64_t e = t + ms; t < e; t += 10) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
}
}  // namespace

TEST(TwingoBuild0810, ScpuButtonsExistAndTheMcpuButtonsAreLabelled) {
  TestTwingo b;
  b.setup();
  EXPECT_TRUE(b.supports_read_DTC());
  EXPECT_TRUE(b.supports_reset_DTC());
  EXPECT_TRUE(b.supports_read_DTC_scpu());
  EXPECT_TRUE(b.supports_reset_DTC_scpu());
  EXPECT_STREQ(b.dtc_title_suffix(), " MCPU");
}

TEST(TwingoBuild0810, ReadDtcScpuGoesToTheSafetyCpuAndDecodesTwoEntries) {
  struct Guard {
    ~Guard() { RenaultTwingoGen1Battery::uq_target = 0; }
  } guard;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  settle(b, t, 700);
  clear_transmitted_frames();
  b.read_DTC_scpu();
  auto tx = ext_frames_since_clear();
  ASSERT_FALSE(tx.empty());
  EXPECT_EQ(tx[0].ID, 0x18DADCF1u);  // the session goes to the SCPU as well
  EXPECT_EQ(tx[0].data.u8[1], 0x10);
  settle(b, t, 300);
  clear_transmitted_frames();
  settle(b, t, 100);
  bool seen_request = false;
  // the request 19 02 09 was sent by the state machine on the SCPU ID (the gap has passed after settle)
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x02, 0x50, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}));
  (void)seen_request;
  // two entries: 59 02 FF E1 43 81 68 1B B1 08 28 (11 bytes, multi-frame)
  clear_transmitted_frames();
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x10, 0x0B, 0x59, 0x02, 0xFF, 0xE1, 0x43, 0x81}));
  auto fc = ext_frames_since_clear();
  ASSERT_FALSE(fc.empty());
  EXPECT_EQ(fc[0].ID, 0x18DADCF1u);  // the flow control also goes to the SCPU
  EXPECT_EQ(fc[0].data.u8[0], 0x30);
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x21, 0x68, 0x1B, 0xB1, 0x08, 0x28, 0xAA, 0xAA}));
  EXPECT_STREQ(b.user_query_result(), "[DC] 19 02 09: OK 59 02 FF E1 43 81 68 1B B1 08 28 | DTC: E14381=68 1BB108=28");
}

// 13.10.: own row "DTC Read SCPU" on the page: the last result of the SCPU read stays until the next SCPU read.
namespace {
void answer_dtc_scpu_with_one_entry(TestTwingo& b, uint64_t& t) {
  settle(b, t, 700);
  clear_transmitted_frames();
  b.read_DTC_scpu();
  settle(b, t, 300);
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x02, 0x50, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}));
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x07, 0x59, 0x02, 0xFF, 0x1B, 0xB5, 0x87, 0x2F}));
}
}  // namespace

TEST(TwingoBuild1013, ScpuListIsFilledFromTheAnswerAndKeptUntilTheNextScpuRead) {
  struct Guard {
    ~Guard() { RenaultTwingoGen1Battery::uq_target = 0; }
  } guard;
  TestTwingo b;
  b.setup();
  DATALAYER_BATTERY_DTC_TYPE* scpu = b.get_dtc_second();
  ASSERT_NE(scpu, nullptr);
  EXPECT_EQ(scpu->dtc_last_read_millis, 0u);  // "Not read yet"
  uint64_t t = 1000;
  answer_dtc_scpu_with_one_entry(b, t);
  // the field "Answer" carries the text, the list of the SCPU the decoded entry (code 1BB587, status 2F)
  EXPECT_STREQ(b.user_query_result(), "[DC] 19 02 09: OK 59 02 FF 1B B5 87 2F | DTC: 1BB587=2F");
  EXPECT_NE(scpu->dtc_last_read_millis, 0u);
  EXPECT_FALSE(scpu->dtc_read_failed);
  ASSERT_EQ(scpu->dtc_count, 1);
  EXPECT_EQ(scpu->dtc_codes[0], 0x1BB587u);
  EXPECT_EQ(scpu->dtc_status[0], 0x2F);
  // the list of the MCPU is not touched
  EXPECT_EQ(b.dtc->dtc_count, 0);
  // another free request overwrites "Answer", not the list
  settle(b, t, 700);
  ASSERT_STREQ(b.start_user_query("22 92 5E", 0), "OK");
  settle(b, t, 100);
  b.handle_incoming_can_frame(reply_on(0x18DAF1DB, {0x06, 0x62, 0x92, 0x5E, 0x13, 0x88, 0x6F, 0xAA}));
  EXPECT_STREQ(b.user_query_result(), "22 92 5E: OK 62 92 5E 13 88 6F");
  EXPECT_EQ(scpu->dtc_count, 1);
  EXPECT_EQ(scpu->dtc_codes[0], 0x1BB587u);
}

TEST(TwingoBuild1013, ScpuListShowsNoResponseAndRefusalsAsFailed) {
  struct Guard {
    ~Guard() { RenaultTwingoGen1Battery::uq_target = 0; }
  } guard;
  TestTwingo b;
  b.setup();
  DATALAYER_BATTERY_DTC_TYPE* scpu = b.get_dtc_second();
  uint64_t t = 1000;
  settle(b, t, 700);
  b.read_DTC_scpu();
  EXPECT_FALSE(scpu->dtc_read_failed);
  settle(b, t, 4000);  // nobody answers
  EXPECT_TRUE(scpu->dtc_read_failed);
  EXPECT_NE(scpu->dtc_last_read_millis, 0u);
  // a second read while another exchange runs is refused and counts as failed, too
  scpu->dtc_read_failed = false;
  ASSERT_STREQ(b.start_user_query("22 92 5E", 0), "OK");
  b.read_DTC_scpu();
  EXPECT_TRUE(scpu->dtc_read_failed);
}

TEST(TwingoBuild1013, NegativeAnswerOfTheScpuCountsAsFailed) {
  TestTwingo b;
  b.setup();
  DATALAYER_BATTERY_DTC_TYPE* scpu = b.get_dtc_second();
  uint64_t t = 1000;
  settle(b, t, 700);
  b.read_DTC_scpu();
  settle(b, t, 300);
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x03, 0x7F, 0x19, 0x31, 0xAA, 0xAA, 0xAA, 0xAA}));
  EXPECT_TRUE(scpu->dtc_read_failed);
}

TEST(TwingoBuild1013, PageHasNoInfoRowOfTheScpuAndNoDetailsAndNoFaultCounters) {
  TestTwingo b;
  b.setup();
  std::string page(b.get_uds_info_html().c_str());
  EXPECT_EQ(page.find("twingoScpuDtc"), std::string::npos);
  EXPECT_EQ(page.find("/triggerTwingoDtcScpu"), std::string::npos);
  EXPECT_EQ(page.find("DTC Read SCPU"), std::string::npos);
  EXPECT_EQ(page.find("Read DTC details"), std::string::npos);
  EXPECT_EQ(page.find("DTC details -"), std::string::npos);
  EXPECT_EQ(page.find("fault counters"), std::string::npos);
  EXPECT_EQ(page.find("twingoFdc"), std::string::npos);
  EXPECT_NE(page.find("DTC Read (ext. protocol"), std::string::npos);                // stays
  EXPECT_NE(page.find("DTC Erase (ext. protocol"), std::string::npos);               // stays
  EXPECT_NE(page.find("Answer: <span id='twingoQueryResult'>"), std::string::npos);  // free request stays
}

TEST(TwingoBuild1013, StatusHtmlShowsTwoColumnsMcpuAndScpu) {
  TestTwingo b;
  b.setup();
  std::string html(BatteryHtmlRenderer::render_dtc_columns_html(*b.dtc, b.get_dtc_second(), "x.json", false).c_str());
  EXPECT_NE(html.find("<b>MCPU</b>"), std::string::npos);
  EXPECT_NE(html.find("<b>SCPU</b>"), std::string::npos);
  EXPECT_NE(html.find("use the Read DTC MCPU button"), std::string::npos);
  EXPECT_NE(html.find("use the Read DTC SCPU button"), std::string::npos);
  EXPECT_EQ(html.find("id='dtcJsonStatus'"), std::string::npos);  // no table, no loader
  // one heading only
  EXPECT_EQ(html.find("Diagnostic Trouble Codes"), html.rfind("Diagnostic Trouble Codes"));
  // an SCPU code appears in the SCPU column with the loader once
  DATALAYER_BATTERY_DTC_TYPE* scpu = b.get_dtc_second();
  scpu->dtc_codes[0] = 0x1BB587;
  scpu->dtc_status[0] = 0x2F;
  scpu->dtc_count = 1;
  scpu->dtc_last_read_millis = 5000;
  html = BatteryHtmlRenderer::render_dtc_columns_html(*b.dtc, b.get_dtc_second(), "x.json", false).c_str();
  EXPECT_NE(html.find("1BB587"), std::string::npos);
  EXPECT_EQ(html.find("id='dtcJsonStatus'"), html.rfind("id='dtcJsonStatus'"));  // one loader only
  EXPECT_NE(html.find("id='dtcJsonStatus'"), std::string::npos);
}

TEST(TwingoBuild0810, ClearAllDtcsOnTheScpu) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  settle(b, t, 700);
  clear_transmitted_frames();
  b.reset_DTC_scpu();
  auto tx = ext_frames_since_clear();
  ASSERT_FALSE(tx.empty());
  EXPECT_EQ(tx[0].ID, 0x18DADCF1u);
  EXPECT_EQ(tx[0].data.u8[1], 0x10);  // session first
  clear_transmitted_frames();
  settle(b, t, 300);
  bool sent = false;
  for (const CAN_frame& f : ext_frames_since_clear()) {
    if (f.data.u8[1] == 0x14) {
      EXPECT_EQ(f.ID, 0x18DADCF1u);
      EXPECT_EQ(f.data.u8[0], 0x04);
      EXPECT_EQ(f.data.u8[2], 0xFF);
      EXPECT_EQ(f.data.u8[3], 0xFF);
      EXPECT_EQ(f.data.u8[4], 0xFF);
      sent = true;
    }
  }
  EXPECT_TRUE(sent);
  b.handle_incoming_can_frame(reply_on(0x18DAF1DC, {0x01, 0x54, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}));
  EXPECT_STREQ(b.user_query_result(), "[DC] 14 FF FF FF: OK 54");
  // back on the MCPU ID afterwards
  clear_transmitted_frames();
  settle(b, t, 700);
  for (const CAN_frame& f : ext_frames_since_clear()) {
    EXPECT_NE(f.ID, 0x18DADCF1u);
  }
}

TEST(TwingoBuild0810, FreeRequestTargetCanBeGivenPerCall) {
  struct Guard {
    ~Guard() { RenaultTwingoGen1Battery::uq_target = 0; }
  } guard;
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  settle(b, t, 700);
  RenaultTwingoGen1Battery::uq_target = 1;  // the page select stands on DC
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("22 92 61", 0), "OK");  // but this call asks the MCPU
  for (const CAN_frame& f : ext_frames_since_clear()) {
    EXPECT_EQ(f.ID, 0x18DADBF1u);
  }
}

TEST(TwingoBuild0810, TimeoutOnTheScpuIsMarked) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  settle(b, t, 700);
  ASSERT_STREQ(b.start_user_query("22 92 61", 1), "OK");
  settle(b, t, 3000);
  EXPECT_STREQ(b.user_query_result(), "[DC] no response");
}

TEST(TwingoBuild0810, QueryPageHasTheButtonsForBothCpus) {
  TestTwingo b;
  b.setup();
  const std::string html = b.get_uds_info_html().c_str();
  for (const char* want : {"DTC 19 02 09 MCPU", "DTC 19 02 09 SCPU", "Time 22 92 61 MCPU", "Time 22 92 61 SCPU",
                           "twingoQuick('19 02 09',1)", "twingoQuick('22 92 61',0)"}) {
    EXPECT_NE(html.find(want), std::string::npos) << want;
  }
}

// ---------------------------------------------------------------------------------------------------------
// Point 19: state / relay / km buttons, decoding, 0x436 follows the vehicle age, all rows off at once
TEST(TwingoBuild0810, QueryPageHasStateRelayAndKmButtonsForBothCpus) {
  TestTwingo b;
  b.setup();
  const std::string html = b.get_uds_info_html().c_str();
  for (const char* want : {"State 22 92 59 MCPU", "State 22 92 59 SCPU", "Relay 22 92 5C MCPU", "Relay 22 92 5C SCPU",
                           "Km 22 92 5F MCPU", "Km 22 92 62 SCPU", "twingoQuick('22 92 62',1)"}) {
    EXPECT_NE(html.find(want), std::string::npos) << want;
  }
}

namespace {
const char* query_and_answer(TestTwingo& b, uint64_t& t, const char* hex, int8_t target, uint32_t did,
                             std::initializer_list<uint8_t> data) {
  settle(b, t, 700);
  EXPECT_STREQ(b.start_user_query(hex, target), "OK");
  CAN_frame r = {};
  r.ext_ID = true;
  r.DLC = 8;
  r.ID = target == 1 ? 0x18DAF1DC : 0x18DAF1DB;
  r.data.u8[0] = (uint8_t)(3 + data.size());
  r.data.u8[1] = 0x62;
  r.data.u8[2] = (uint8_t)(did >> 8);
  r.data.u8[3] = (uint8_t)did;
  uint8_t i = 4;
  for (uint8_t v : data) {
    r.data.u8[i++] = v;
  }
  b.handle_incoming_can_frame(r);
  return b.user_query_result();
}
}  // namespace

TEST(TwingoBuild0810, TimeAnswersAreShownInDaysHoursMinutes) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  // 1,311,344 min = 910 d 15 h 44 min
  EXPECT_STREQ(query_and_answer(b, t, "22 92 61", 0, 0x9261, {0x14, 0x02, 0x70}),
               "22 92 61: OK 62 92 61 14 02 70 | 1311344 min = 910 d 15 h 44 min");
}

TEST(TwingoBuild0810, KilometreAnswersAreShownInKm) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  EXPECT_STREQ(query_and_answer(b, t, "22 92 5F", 0, 0x925F, {0x00, 0x1D, 0x9C, 0x14}),
               "22 92 5F: OK 62 92 5F 00 1D 9C 14 | 19405.00 km");
  EXPECT_STREQ(query_and_answer(b, t, "22 92 62", 1, 0x9262, {0x80, 0x1D, 0x9C, 0x14}),
               "[DC] 22 92 62: OK 62 92 62 80 1D 9C 14 | 19405.00 km");
  // 9262 on the MCPU is the balancing counter: no km shown
  EXPECT_STREQ(query_and_answer(b, t, "22 92 62", 0, 0x9262, {0x80, 0x00, 0x00, 0x00}),
               "22 92 62: OK 62 92 62 80 00 00 00");
}

namespace {
struct FollowGuard {
  ~FollowGuard() {
    RenaultTwingoGen1Battery::time_436_follow_age = false;
    RenaultTwingoGen1Battery::time_436_active = false;
    datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
  }
};
}  // namespace

TEST(TwingoBuild0810, Time436CanFollowTheVehicleAge) {
  FollowGuard guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL;  // 0x436 is row 5
  TestTwingo b;
  b.setup();
  b.time_436_follow_set(true);
  auto f = collect(b, 0x436, 1000, 1300);
  ASSERT_FALSE(f.empty());
  const uint32_t v = (f.back().data.u8[1] << 16) | (f.back().data.u8[2] << 8) | f.back().data.u8[3];
  EXPECT_EQ(v, 1025301u);  // the age of the seed at the frozen test clock, the same as 0x350
  b.time_436_follow_set(false);
  auto g = collect(b, 0x436, 1300, 1600);
  ASSERT_FALSE(g.empty());
  EXPECT_EQ(g.back().data.u8[1], 0x14);  // back to 14 00 xx
  EXPECT_EQ(g.back().data.u8[2], 0x00);
}

TEST(TwingoBuild0810, Time436FollowingSendsNothingWithoutAnAge) {
  struct NoClock : public TestTwingo {
    bool get_unix_time(time_t&) override { return false; }
  };
  FollowGuard guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL;
  NoClock b;
  b.setup();
  b.time_436_follow_set(true);
  auto f = collect(b, 0x436, 1000, 2000);
  EXPECT_TRUE(f.empty());
}

TEST(TwingoBuild0810, ManualValueBeatsFollowingTheAge) {
  FollowGuard guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL;
  TestTwingo b;
  b.setup();
  b.time_436_follow_set(true);
  b.time_436_set(1311364);
  auto f = collect(b, 0x436, 1000, 1300);
  ASSERT_FALSE(f.empty());
  const uint32_t v = (f.back().data.u8[1] << 16) | (f.back().data.u8[2] << 8) | f.back().data.u8[3];
  EXPECT_EQ(v, 1311364u);
}

TEST(TwingoBuild0810, AllRowsOffAtOnceAndBack) {
  struct Guard {
    ~Guard() {
      RenaultTwingoGen1Battery::sim_mask_saved_valid = false;
      datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FF;
    }
  } guard;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0x3FFULL | (1ULL << row_of(0x436)) | (1ULL << row_of(0x57F));
  const uint64_t before = datalayer_extended.twingoGen1.simulator_enabled_mask;
  TestTwingo b;
  b.setup();
  RenaultTwingoGen1Battery::sim_all_off();
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 0ULL);
  RenaultTwingoGen1Battery::sim_all_off();  // pressed twice: the first state stays saved
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, 0ULL);
  auto f = collect(b, 0x350, 1000, 1500);
  EXPECT_TRUE(f.empty());  // even 0x350 is gone
  RenaultTwingoGen1Battery::sim_restore();
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, before);
  RenaultTwingoGen1Battery::sim_restore();  // nothing saved: unchanged
  EXPECT_EQ(datalayer_extended.twingoGen1.simulator_enabled_mask, before);
}

// Bau 13 (14.10.): the table has 30 rows; the default switches only 0x69F on; the switches of the old firmware (71 rows)
// are carried over to the new numbering.
TEST(TwingoBuild0810, DefaultMaskSwitchesOnlyTheRow0x69FOn) {
  std::unique_ptr<DataLayerExtended> d(new DataLayerExtended());
  EXPECT_EQ(d->twingoGen1.simulator_enabled_mask, 1ULL << RenaultTwingoGen1Battery::SIM_ROW_69F);
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    const bool on = (d->twingoGen1.simulator_enabled_mask >> i) & 1ULL;
    EXPECT_EQ(on, RenaultTwingoGen1Battery::sim_signals[i].id == 0x69F) << i;
  }
}

TEST(TwingoBuild0810, RowIndicesOfTheZoeFormRowsMatchTheTable) {
  using Bat = RenaultTwingoGen1Battery;
  EXPECT_EQ((int)Bat::SIM_SIGNAL_COUNT, 30);
  EXPECT_EQ(Bat::sim_signals[Bat::SIM_ROW_19F].id, 0x19Fu);
  EXPECT_EQ(Bat::sim_signals[Bat::SIM_ROW_426].id, 0x426u);
  EXPECT_EQ(Bat::sim_signals[Bat::SIM_ROW_436].id, 0x436u);
  EXPECT_EQ(Bat::sim_signals[Bat::SIM_ROW_423].id, 0x423u);
  EXPECT_EQ(Bat::sim_signals[Bat::SIM_ROW_69F].id, 0x69Fu);
  for (int i = 5; i < Bat::SIM_SIGNAL_COUNT; i++) {
    EXPECT_EQ(Bat::sim_signals[i].tag, 'R') << i;
  }
}

TEST(TwingoBuild0810, SwitchesOfTheOldFirmwareAreCarriedOverToTheNewNumbering) {
  // old rows: 0 = 0x090, 1 = 0x242, 2 = 0x350, 3-7 = 0x19F 0x426 0x436 0x423 0x69F, 46-70 = the bus
  const uint64_t old_lo =
      (1ULL << 0) | (1ULL << 1) | (1ULL << 2) | (1ULL << 3) | (1ULL << 7) | (1ULL << 8) | (1ULL << 9);
  // bus rows: old 46 (0x155) and old 70 (0x659) -> bits 46 and 70 of the 128 bit word
  const uint64_t old_lo_with_46 = old_lo | (1ULL << 46);
  const uint64_t old_hi = 1ULL << (70 - 64);
  const uint32_t m = twingo::migrate_old_row_mask(old_lo_with_46, old_hi);
  EXPECT_TRUE(m & (1u << 0)) << "0x19F (old 3)";
  EXPECT_FALSE(m & (1u << 1)) << "0x426 was off";
  EXPECT_FALSE(m & (1u << 2));
  EXPECT_FALSE(m & (1u << 3));
  EXPECT_TRUE(m & (1u << 4)) << "0x69F (old 7)";
  EXPECT_TRUE(m & (1u << 5)) << "0x155 (old 46)";
  EXPECT_TRUE(m & (1u << 29)) << "0x659 (old 70)";
  EXPECT_EQ(m & ~((1u << 0) | (1u << 4) | (1u << 5) | (1u << 29)), 0u) << "the removed rows leave no trace";
  EXPECT_EQ(twingo::migrate_old_row_mask(0, 0), 0u);
  // the same rows through the table: the IDs agree
  using Bat = RenaultTwingoGen1Battery;
  EXPECT_EQ(Bat::sim_signals[5].id, 0x155u);
  EXPECT_EQ(Bat::sim_signals[29].id, 0x659u);
}
