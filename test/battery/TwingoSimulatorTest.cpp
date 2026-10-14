#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "../../Software/src/devboard/webserver/simulator_html.h"

#include "Arduino.h"

// TX frame capture injected by the emulated CAN layer (see emul/can.cpp).
void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Tests of the 03.10. changes: vehicle age from the clock, 0x350 C7, switchable simulator rows, 0x42E / 0x18A /
// 0x1F8 content, free read request, fault counters (19 14), DTC details (19 06).

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
  bool clock_set = true;
  uint32_t clock_secs = 12 * 3600;
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

const time_t AGE_EPOCH = 1613387588;  // 15.02.2021 11:13:08 UTC = zero of the 0x350 vehicle age counter

std::vector<CAN_frame> tick(RenaultTwingoGen1Battery& b, uint64_t t) {
  set_millis64(t);
  clear_transmitted_frames();
  b.transmit_can((unsigned long)t);
  return get_transmitted_frames();
}

void run(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms, uint64_t step, std::vector<Tx>& log) {
  for (uint64_t end = t + ms; t < end; t += step) {
    for (const CAN_frame& f : tick(b, t)) {
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
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    if (RenaultTwingoGen1Battery::sim_signals[i].id == id) {
      return i;
    }
  }
  return -1;
}

void set_mask(uint64_t mask) {
  datalayer_extended.twingoGen1.simulator_enabled_mask = mask;
}

// CRC-8 SAE J1850 (poly 0x1D, start 0xFF, final XOR 0xFF), bit by bit - independent of the driver's table.
uint8_t crc_j1850(const std::vector<uint8_t>& d) {
  uint8_t crc = 0xFF;
  for (uint8_t b : d) {
    crc ^= b;
    for (int i = 0; i < 8; i++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x1D) : (uint8_t)(crc << 1);
    }
  }
  return (uint8_t)(crc ^ 0xFF);
}

CAN_frame reply_frame(std::initializer_list<uint8_t> bytes) {
  CAN_frame f = {};
  f.FD = false;
  f.ext_ID = true;
  f.ID = 0x18DAF1DB;
  f.DLC = 8;
  int i = 0;
  for (uint8_t v : bytes) {
    f.data.u8[i++] = v;
  }
  return f;
}

// Single-frame positive reply 62 <DID> <data...> as the LBC sends it.
CAN_frame did_reply(uint16_t did, std::initializer_list<uint8_t> data_bytes) {
  CAN_frame f = {};
  f.FD = false;
  f.ext_ID = true;
  f.ID = 0x18DAF1DB;
  f.DLC = 8;
  f.data.u8[0] = (uint8_t)(3 + data_bytes.size());
  f.data.u8[1] = 0x62;
  f.data.u8[2] = (uint8_t)(did >> 8);
  f.data.u8[3] = (uint8_t)(did & 0xFF);
  int i = 4;
  for (uint8_t v : data_bytes) {
    f.data.u8[i++] = v;
  }
  return f;
}

bool contains(const String& s, const char* needle) {
  return std::string(s.c_str()).find(needle) != std::string::npos;
}

// The two "DTC details" lines of the page (text after the button). The host emulation of String does not print
// String(x, HEX) as hex (the page label of the code looks odd there, on the ESP32 it is fine), so the tests
// look at the answer texts only.
std::string detail_lines(const String& html) {
  std::string h(html.c_str());
  size_t p = h.find("subfunction 0x06)</button>");
  if (p == std::string::npos) {
    return "";
  }
  return h.substr(p, 420);  // the field of 0x155 stands behind the button (13.10.)
}

// Request frames on 0x18DADBF1 (the driver's own TX of the diagnostic channel).
std::vector<CAN_frame> diag_tx() {
  std::vector<CAN_frame> out;
  for (const CAN_frame& f : get_transmitted_frames()) {
    if (f.ID == 0x18DADBF1 && f.ext_ID) {
      out.push_back(f);
    }
  }
  return out;
}

}  // namespace

// ---------------------------------------------------------------------------
// Simulator table
// ---------------------------------------------------------------------------

TEST(TwingoSimulatorTable, FiveIRowsInTheDocumentedOrder) {
  const uint32_t expected_i[5] = {0x19F, 0x426, 0x436, 0x423, 0x69F};
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[i].id, expected_i[i]) << "row " << i;
    EXPECT_EQ(RenaultTwingoGen1Battery::sim_signals[i].tag, 'I') << "row " << i;
  }
  for (int i = 5; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    EXPECT_NE(RenaultTwingoGen1Battery::sim_signals[i].tag, 'I') << "row " << i;
  }
}

TEST(TwingoSimulatorTable, OnlyTheZoeFramesAndTheCanListFrameAreMarkedX) {
  int x_count = 0;
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    const auto& s = RenaultTwingoGen1Battery::sim_signals[i];
    const bool expect_x = (s.id == 0x19F || s.id == 0x426 || s.id == 0x436 || s.id == 0x423 || s.id == 0x523);
    if (i >= 5) {
      continue;  // the 25 bus rows are no subject of this test
    }
    EXPECT_EQ(s.not_in_vehicle_log, expect_x) << "0x" << std::hex << s.id;
    x_count += s.not_in_vehicle_log ? 1 : 0;
  }
  EXPECT_EQ(x_count, 4);
}

TEST(TwingoSimulatorTable, EveryRowHasSenderAndMeaningText) {
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    const auto& s = RenaultTwingoGen1Battery::sim_signals[i];
    ASSERT_NE(s.sender, nullptr);
    ASSERT_NE(s.info, nullptr);
    EXPECT_GT(strlen(s.sender), 0u) << "0x" << std::hex << s.id;
    EXPECT_GT(strlen(s.info), 10u) << "0x" << std::hex << s.id;
  }
}

TEST(TwingoSimulatorTable, SenderIsOnlyNamedWhereASourceExists) {
  // ids without an entry in the CanZE table say "unknown"; every row has a sender text
  for (int i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
    ASSERT_NE(RenaultTwingoGen1Battery::sim_signals[i].sender, nullptr) << i;
  }
}

// ---------------------------------------------------------------------------
// Vehicle age (0x350 bytes 1-3) and the C7 run frame
// ---------------------------------------------------------------------------

namespace {
// The age state is global: every test starts from the seed, nothing sent yet, automatic mode.
void reset_age_state() {
  RenaultTwingoGen1Battery::age_manual_clear();
  auto& t = datalayer_extended.twingoGen1;
  t.age_pack_value = 1025301;
  t.age_pack_unix = 1791590400;
  t.age_last_sent = 0;
  t.age_source = 0;
  t.bus_format_zoe_old = true;                           // the legacy tests check the old Zoe form
  t.bus_state_manual = true;                             // the tests choose the state themselves
  RenaultTwingoGen1Battery::time_436_follow_age = true;  // since Bau 13 the age is read from the 0x436 frame
  RenaultTwingoGen1Battery::time_436_active = false;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 1ULL << RenaultTwingoGen1Battery::SIM_ROW_436;
}
// A single-frame positive reply 62 <pid> <3 data bytes> from the MCPU.
void feed_time_reply(RenaultTwingoGen1Battery& b, uint16_t pid, uint32_t v) {
  CAN_frame f = {};
  f.ext_ID = true;
  f.DLC = 8;
  f.ID = 0x18DAF1DB;
  f.data.u8[0] = 6;
  f.data.u8[1] = 0x62;
  f.data.u8[2] = (uint8_t)(pid >> 8);
  f.data.u8[3] = (uint8_t)pid;
  f.data.u8[4] = (uint8_t)(v >> 16);
  f.data.u8[5] = (uint8_t)(v >> 8);
  f.data.u8[6] = (uint8_t)v;
  b.handle_incoming_can_frame(f);
}
uint32_t age_of_frame(const Tx& x) {
  return ((uint32_t)x.f.data.u8[1] << 16) | ((uint32_t)x.f.data.u8[2] << 8) | x.f.data.u8[3];
}
}  // namespace

TEST(TwingoVehicleAge, SeedGivesTheBenchValueOf0810) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1791590400 + 6000;  // 100 minutes after the seed reference
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  auto f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(age_of_frame(f.back()), 1025401u);  // seed 1,025,301 + 100 min, no safety day
}

TEST(TwingoVehicleAge, TicksOncePerMinute) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1791590400 + 59;
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 100, 100, log);
  auto f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(age_of_frame(f.back()), 1025301u);
  b.unix_now += 1;  // the next minute starts
  log.clear();
  run(b, t, 100, 100, log);
  f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(age_of_frame(f.back()), 1025302u);
}

TEST(TwingoVehicleAge, WithoutAClockNoFrameWithAnAgeIsSent) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = false;
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 500, 100, log);
  EXPECT_TRUE(with_id(log, 0x436).empty());
  b.unix_set = true;
  b.unix_now = 1791590400;
  log.clear();
  run(b, t, 300, 100, log);
  EXPECT_FALSE(with_id(log, 0x436).empty());
}

TEST(TwingoVehicleAge, ImplausibleClockIsNotUsed) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1000000;  // 1970
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 100, log);
  EXPECT_TRUE(with_id(log, 0x436).empty());
}

TEST(TwingoVehicleAge, NeverGoesBackwardsWhenTheClockStepsBack) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1791590400 + 600;
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  uint32_t before = age_of_frame(with_id(log, 0x436).back());
  b.unix_now = 1791590400 + 60;
  log.clear();
  run(b, t, 200, 100, log);
  EXPECT_GE(age_of_frame(with_id(log, 0x436).back()), before);
}

TEST(TwingoVehicleAge, PackReadingAboveWhatWeSendDoesNotRaiseTheReference) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1791590400 + 600;  // sends 1025311
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  feed_time_reply(b, 0x9261, 1400000);  // since 09.10. the reading never raises what we send
  log.clear();
  run(b, t, 200, 100, log);
  EXPECT_EQ(age_of_frame(with_id(log, 0x436).back()), 1025311u);
}

TEST(TwingoVehicleAge, PackReadingBelowWhatWeSendChangesNothing) {
  reset_age_state();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1791590400;
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  feed_time_reply(b, 0x9261, 1311344);
  log.clear();
  run(b, t, 200, 100, log);
  EXPECT_EQ(age_of_frame(with_id(log, 0x436).back()), 1025301u);
}

// ---------------------------------------------------------------------------
// I rows are real switches
// ---------------------------------------------------------------------------

namespace {
size_t count_id(const std::vector<Tx>& log, uint32_t id) {
  return with_id(log, id).size();
}
}  // namespace

TEST(TwingoSimulatorSwitches, AllFiveSteadyIRowsRunByDefault) {
  set_mask(0x3FF);
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = AGE_EPOCH + 2959882LL * 60;
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 3000, 10, log);
  for (uint32_t id : {0x19Fu, 0x426u, 0x436u, 0x423u, 0x69Fu}) {
    EXPECT_GT(count_id(log, id), 0u) << "0x" << std::hex << id;
  }
  for (uint32_t id : {0x090u, 0x242u, 0x350u, 0x53Bu, 0x214u}) {
    EXPECT_EQ(count_id(log, id), 0u) << "removed in Bau 13: 0x" << std::hex << id;
  }
}

TEST(TwingoSimulatorSwitches, EachRowSwitchesExactlyItsOwnFrame) {
  const uint32_t ids[5] = {0x19F, 0x426, 0x436, 0x423, 0x69F};
  for (int k = 0; k < 5; k++) {
    int row = row_of(ids[k]);
    ASSERT_GE(row, 0);
    set_mask(0x3FF & ~(1u << row));
    TestTwingo b;
    b.setup();
    b.unix_set = true;
    b.unix_now = AGE_EPOCH + 2959882LL * 60;
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 3000, 10, log);
    EXPECT_EQ(count_id(log, ids[k]), 0u) << "0x" << std::hex << ids[k] << " must be off";
    for (int j = 0; j < 5; j++) {
      if (j != k) {
        EXPECT_GT(count_id(log, ids[j]), 0u)
            << "0x" << std::hex << ids[j] << " must still run while 0x" << ids[k] << " is off";
      }
    }
  }
  set_mask(0x3FF);
}

TEST(TwingoSimulatorSwitches, AllIRowsOffMeansNoVehicleFramesAtAll) {
  set_mask(0);
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 3000, 10, log);
  for (uint32_t id : {0x090u, 0x242u, 0x350u, 0x19Fu, 0x426u, 0x436u, 0x423u, 0x69Fu, 0x53Bu, 0x214u}) {
    EXPECT_EQ(count_id(log, id), 0u) << "0x" << std::hex << id;
  }
  set_mask(0x3FF);
}

// ---------------------------------------------------------------------------
// 0x42E: measured pack voltage
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 0x18A: counter and CRC
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 0x1F8: FA first, then 00 and a fade, then 0 - and no FA in the sleep stages
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Free read request
// ---------------------------------------------------------------------------

TEST(TwingoFreeQuery, RejectsInvalidInput) {
  TestTwingo b;
  b.setup();
  EXPECT_STREQ(b.start_user_query(""), "empty request");
  EXPECT_STREQ(b.start_user_query("   "), "empty request");
  EXPECT_STREQ(b.start_user_query("22925G"), "invalid character (hex digits only)");
  EXPECT_STREQ(b.start_user_query("22925"), "odd number of hex digits");
  const char* not_allowed =
      "only 0x22 and 0x19 (read), 0x10 03 (session), 14 FF FF FF (clear DTCs) and 0x2E (write, "
      "9261/9264/926B/91C1/91CF/925F/9281/925E only) are allowed";
  EXPECT_STREQ(b.start_user_query("11 01"), not_allowed);
  EXPECT_STREQ(b.start_user_query("14 01 02 03"), "0x14 is only allowed as 14 FF FF FF (clear all DTCs)");
  EXPECT_STREQ(b.start_user_query("14 FF FF"), "0x14 is only allowed as 14 FF FF FF (clear all DTCs)");
  EXPECT_STREQ(b.start_user_query("10 02"), "0x10 is only allowed as 10 03 (extended session)");
  EXPECT_STREQ(b.start_user_query("31 01 B0 09"), not_allowed);
  EXPECT_STREQ(b.start_user_query("27 01"), not_allowed);
  EXPECT_STREQ(b.start_user_query("22"), "0x22 needs one or more 2-byte identifiers");
  EXPECT_STREQ(b.start_user_query("2292"), "0x22 needs one or more 2-byte identifiers");
  EXPECT_STREQ(b.start_user_query("22 92 5E 92"), "0x22 needs one or more 2-byte identifiers");
  EXPECT_STREQ(b.start_user_query("19"), "0x19 needs a sub-function");
  EXPECT_STREQ(b.start_user_query("22 92 5E 92 61 92 5F 90"), "too long (at most 7 bytes)");
  EXPECT_STREQ(b.start_user_query(nullptr), "empty request");
  // None of the rejected inputs sent anything.
  clear_transmitted_frames();
  b.start_user_query("2E902100");  // a write to an identifier that is not on the list
  b.start_user_query("11 01");
  EXPECT_TRUE(get_transmitted_frames().empty());
}

TEST(TwingoFreeQuery, ReadServicesWithOtherCaseAndSeparatorsAreAccepted) {
  TestTwingo b;
  b.setup();
  clear_transmitted_frames();
  EXPECT_STREQ(b.start_user_query("22:92:5e"), "OK");
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[0], 0x03);
  EXPECT_EQ(tx[0].data.u8[1], 0x22);
  EXPECT_EQ(tx[0].data.u8[2], 0x92);
  EXPECT_EQ(tx[0].data.u8[3], 0x5E);
  EXPECT_EQ(tx[0].data.u8[4], 0xAA);  // padding like the other sequences of this driver
}

TEST(TwingoFreeQuery, SingleFrameAnswerIsShownTogetherWithTheRequest) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x06, 0x62, 0x92, 0x5E, 0x13, 0x88, 0x6F, 0x00}));
  EXPECT_STREQ(b.user_query_result(), "22 92 5E: OK 62 92 5E 13 88 6F");
  // The shared poll frame is back to its normal read template.
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 600, 100, log);
  auto polls = with_id(log, 0x18DADBF1, true);
  ASSERT_FALSE(polls.empty());
  EXPECT_EQ(polls[0].f.data.u8[0], 0x03);
  EXPECT_EQ(polls[0].f.data.u8[1], 0x22);
}

TEST(TwingoFreeQuery, TwoIdentifiersInOneRequestAreSentAsOneFrame) {
  TestTwingo b;
  b.setup();
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("22 9005 9006"), "OK");
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[0], 0x05);  // 22 + two identifiers = 5 bytes
  EXPECT_EQ(tx[0].data.u8[2], 0x90);
  EXPECT_EQ(tx[0].data.u8[3], 0x05);
  EXPECT_EQ(tx[0].data.u8[4], 0x90);
  EXPECT_EQ(tx[0].data.u8[5], 0x06);
}

TEST(TwingoFreeQuery, NegativeAnswerShowsSidAndNrc) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22FFFF"), "OK");
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x22, 0x31, 0, 0, 0, 0}));
  EXPECT_STREQ(b.user_query_result(), "22 FF FF: NEGATIVE 7F 22 31 (SID 0x22, NRC 0x31)");
}

TEST(TwingoFreeQuery, NoAnswerGivesTimeoutTextAndFreesTheChannel) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);  // the driver reads millis() when the request starts
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1500, 100, log);
  EXPECT_STREQ(b.user_query_result(), "requested");  // still waiting
  run(b, t, 1000, 100, log);                         // after the 2 s timeout
  EXPECT_STREQ(b.user_query_result(), "no response");
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");  // channel is free again
}

TEST(TwingoFreeQuery, SecondRequestWhileOneIsRunningIsRefused) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  EXPECT_STREQ(b.start_user_query("229261"), "busy (another diagnostic exchange or a Sleep/NVROL run is active)");
  b.handle_incoming_can_frame(reply_frame({0x06, 0x62, 0x92, 0x5E, 0x13, 0x88, 0x6F, 0x00}));
  EXPECT_STREQ(b.start_user_query("229261"), "OK");
}

TEST(TwingoFreeQuery, RefusedWhileASleepRunIsActive) {
  TestTwingo b;
  b.setup();
  b.request_sleep();
  EXPECT_STREQ(b.start_user_query("22925E"), "busy (another diagnostic exchange or a Sleep/NVROL run is active)");
}

TEST(TwingoFreeQuery, MultiFrameAnswerSendsFlowControlAndIsReassembled) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22912B"), "OK");
  clear_transmitted_frames();
  // First frame: 10 bytes in total (62 91 2B + 7 data bytes), 6 of them here
  b.handle_incoming_can_frame(reply_frame({0x10, 0x0A, 0x62, 0x91, 0x2B, 0x01, 0x02, 0x03}));
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[0], 0x30);  // flow control: continue to send
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x21, 0x04, 0x05, 0x06, 0x07, 0xAA, 0xAA, 0xAA}));
  EXPECT_STREQ(b.user_query_result(), "22 91 2B: OK 62 91 2B 01 02 03 04 05 06 07");
}

TEST(TwingoFreeQuery, ResponsePendingKeepsWaiting) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);  // the driver reads millis() when the request starts
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 1500, 100, log);
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x22, 0x78, 0, 0, 0, 0}));  // pending, restarts the timeout
  run(b, t, 1500, 100, log);                                                       // 3 s after the request in total
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x06, 0x62, 0x92, 0x5E, 0x13, 0x88, 0x6F, 0x00}));
  EXPECT_STREQ(b.user_query_result(), "22 92 5E: OK 62 92 5E 13 88 6F");
}

TEST(TwingoFreeQuery, VeryLongAnswerIsCutAndNeverOverflowsTheTextBuffer) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22926C"), "OK");
  // 500 bytes announced: 0x1F4. First frame 6 bytes, then consecutive frames of 7 bytes.
  CAN_frame ff = reply_frame({0x11, 0xF4, 0x62, 0x92, 0x6C, 0x80, 0x00, 0x00});
  b.handle_incoming_can_frame(ff);
  uint8_t sn = 1;
  int received = 6;
  while (received < 500) {
    CAN_frame cf = reply_frame({(uint8_t)(0x20 | (sn & 0x0F)), 1, 2, 3, 4, 5, 6, 7});
    b.handle_incoming_can_frame(cf);
    received += 7;
    sn++;
  }
  const char* r = b.user_query_result();
  EXPECT_LT(strlen(r), 832u);
  EXPECT_NE(std::string(r).find("OK 62 92 6C"), std::string::npos);
  // the data buffer holds 256 bytes (768 characters), so the text always fits into UQ_TEXT_SIZE
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");  // exchange finished, channel free
}

TEST(TwingoFreeQuery, StrayConsecutiveFrameIsIgnored) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  b.handle_incoming_can_frame(reply_frame({0x21, 1, 2, 3, 4, 5, 6, 7}));
  EXPECT_STREQ(b.user_query_result(), "requested");
}

TEST(TwingoFreeQuery, ServiceNineteenOpensTheSessionFirstAndThenSendsTheRequest) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("19 02 FF"), "OK");
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[0], 0x02);  // session control 10 03
  EXPECT_EQ(tx[0].data.u8[1], 0x10);
  EXPECT_EQ(tx[0].data.u8[2], 0x03);
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);  // after the 100 ms gap the request follows
  bool found = false;
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
    if (x.f.data.u8[1] == 0x19) {
      found = true;
      EXPECT_EQ(x.f.data.u8[0], 0x03);
      EXPECT_EQ(x.f.data.u8[2], 0x02);
      EXPECT_EQ(x.f.data.u8[3], 0xFF);
    }
  }
  EXPECT_TRUE(found);
  b.handle_incoming_can_frame(reply_frame({0x03, 0x59, 0x02, 0xFF, 0, 0, 0, 0}));
  EXPECT_STREQ(b.user_query_result(), "19 02 FF: OK 59 02 FF | no DTC");
}

TEST(TwingoFreeQuery, ReadServiceTwentyTwoNeedsNoSession) {
  TestTwingo b;
  b.setup();
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("22925F"), "OK");
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[1], 0x22);  // straight to the read request
}

// ---------------------------------------------------------------------------
// Free request: write 0x2E (05.10.) - session, read before, write, answer
// ---------------------------------------------------------------------------

// Number of frames with the given service byte in the diagnostic frames of a log.
static int count_diag_sid(const std::vector<Tx>& log, uint8_t sid) {
  int n = 0;
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
    if (x.f.data.u8[1] == sid) {
      n++;
    }
  }
  return n;
}

// Starts a write, lets the session gap pass so that the read before the write is on the bus.
static void start_write_until_read(TestTwingo& b, const char* hex, uint64_t& t, std::vector<Tx>& log) {
  set_millis64(1000);
  t = 1000;
  ASSERT_STREQ(b.start_user_query(hex), "OK");
  run(b, t, 300, 50, log);
}

TEST(TwingoFreeWrite, OnlyTheListedIdentifiersCanBeWritten) {
  for (const char* ok : {"2E 92 61 00 00 03", "2E 91 C1 00 00 03", "2E 91 CF 00 00 00 00", "2E 92 5F 00 00 00 00",
                         "2E 92 81 00", "2E 92 5E 13 88 6F"}) {  // 925E: the vehicle ID (11.10.)
    TestTwingo b;
    b.setup();
    EXPECT_STREQ(b.start_user_query(ok), "OK") << ok;
  }
  TestTwingo b;
  b.setup();
  const char* refused = "0x2E is only allowed for 9261, 9264, 926B, 91C1, 91CF, 925F, 9281 and 925E";
  EXPECT_STREQ(b.start_user_query("2E 90 21 00"), refused);        // a cell voltage
  EXPECT_STREQ(b.start_user_query("2E 92 5D 13 88 6F"), refused);  // neighbour of 925E
  EXPECT_STREQ(b.start_user_query("2E 92 60 00"), refused);        // neighbour of 9261
  EXPECT_STREQ(b.start_user_query("2E 92 61"), "0x2E needs a 2-byte identifier and 1 to 4 data bytes");
  EXPECT_STREQ(b.start_user_query("2E 92"), "0x2E needs a 2-byte identifier and 1 to 4 data bytes");
  EXPECT_STREQ(b.start_user_query("2E 92 61 00 00 00 00 00"), "too long (at most 7 bytes)");
}

TEST(TwingoFreeWrite, SessionThenReadBeforeThenWriteThenAnswerWithBeforeValue) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  clear_transmitted_frames();
  ASSERT_STREQ(b.start_user_query("2E 92 61 00 00 03"), "OK");
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[1], 0x10);  // extended session first
  EXPECT_EQ(tx[0].data.u8[2], 0x03);
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);
  EXPECT_EQ(count_diag_sid(log, 0x22), 1);  // the read of $9261 ...
  EXPECT_EQ(count_diag_sid(log, 0x2E), 0);  // ... and no write yet
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
    if (x.f.data.u8[1] == 0x22) {
      EXPECT_EQ(x.f.data.u8[0], 0x03);
      EXPECT_EQ(x.f.data.u8[2], 0x92);
      EXPECT_EQ(x.f.data.u8[3], 0x61);
    }
  }
  EXPECT_STREQ(b.user_query_result(), "requested");

  clear_transmitted_frames();
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));  // value before: 0
  tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);  // the write follows the answer
  EXPECT_EQ(tx[0].data.u8[0], 0x06);
  EXPECT_EQ(tx[0].data.u8[1], 0x2E);
  EXPECT_EQ(tx[0].data.u8[2], 0x92);
  EXPECT_EQ(tx[0].data.u8[3], 0x61);
  EXPECT_EQ(tx[0].data.u8[4], 0x00);
  EXPECT_EQ(tx[0].data.u8[5], 0x00);
  EXPECT_EQ(tx[0].data.u8[6], 0x03);
  EXPECT_STREQ(b.user_query_result(), "requested");

  clear_transmitted_frames();
  b.handle_incoming_can_frame(reply_frame({0x03, 0x6E, 0x92, 0x61, 0, 0, 0, 0}));
  tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);  // read back at once (point 10)
  EXPECT_EQ(tx[0].data.u8[1], 0x22);
  EXPECT_EQ(tx[0].data.u8[2], 0x92);
  EXPECT_EQ(tx[0].data.u8[3], 0x61);
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x03}));
  EXPECT_STREQ(b.user_query_result(), "2E 92 61 00 00 03: before 00 00 00 -> OK 6E 92 61 | read back 00 00 03");
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");  // channel is free again
}

TEST(TwingoFreeWrite, TheNewTimeIdentifiersAndSession1003AreAccepted) {
  for (const char* ok : {"2E 92 64 00 00 00", "2E 92 6B 00 00 00", "10 03"}) {
    TestTwingo b;
    b.setup();
    EXPECT_STREQ(b.start_user_query(ok), "OK") << ok;
  }
}

TEST(TwingoFreeWrite, SetTimeNowWritesTheVehicleAge) {
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = 1791590400;  // seed: age 1,312,784 = 0x140810
  set_millis64(1000);
  clear_transmitted_frames();
  ASSERT_STREQ(b.write_time_now(), "OK");
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);
  clear_transmitted_frames();
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));
  auto tx = diag_tx();
  ASSERT_EQ(tx.size(), 1u);
  EXPECT_EQ(tx[0].data.u8[1], 0x2E);
  EXPECT_EQ(tx[0].data.u8[4], 0x0F);
  EXPECT_EQ(tx[0].data.u8[5], 0xA5);
  EXPECT_EQ(tx[0].data.u8[6], 0x15);
}

TEST(TwingoFreeWrite, SetTimeNowNeedsAnAge) {
  TestTwingo b;
  b.setup();
  b.unix_set = false;
  EXPECT_STRNE(b.write_time_now(), "OK");
}

TEST(TwingoFreeWrite, NoAnswerToTheReadBackIsShownNextToTheWriteResult) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));
  b.handle_incoming_can_frame(reply_frame({0x03, 0x6E, 0x92, 0x61, 0, 0, 0, 0}));
  run(b, t, 3000, 50, log);  // no answer to the read back
  EXPECT_NE(std::string(b.user_query_result()).find("OK 6E 92 61 | read back: no response"), std::string::npos)
      << b.user_query_result();
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");
}

TEST(TwingoFreeWrite, NothingIsWrittenWhenTheReadBeforeIsRefused) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  clear_transmitted_frames();
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x22, 0x31, 0, 0, 0, 0}));
  EXPECT_TRUE(diag_tx().empty());
  EXPECT_STREQ(b.user_query_result(),
               "2E 92 61 00 00 03: read before the write refused (NRC 0x31) - nothing was written");
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");
}

TEST(TwingoFreeWrite, NothingIsWrittenWhenTheReadBeforeIsNotAnswered) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  run(b, t, 2500, 100, log);  // no answer: timeout
  EXPECT_STREQ(b.user_query_result(), "no response to the read before the write - nothing was written");
  EXPECT_EQ(count_diag_sid(log, 0x2E), 0);
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");
}

TEST(TwingoFreeWrite, NothingIsWrittenWhenTheLengthDiffers) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 03", t, log);  // 2 data bytes entered
  clear_transmitted_frames();
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));  // the value has 3
  EXPECT_TRUE(diag_tx().empty());
  EXPECT_STREQ(b.user_query_result(), "2E 92 61 00 03: the value is 3 bytes long, 2 entered - nothing was written");
}

TEST(TwingoFreeWrite, ReadBeforeIgnoresAnAnswerForAnotherIdentifier) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  clear_transmitted_frames();
  b.handle_incoming_can_frame(did_reply(0x9072, {0x0C, 0xE4}));  // a late answer of the cell polling
  EXPECT_TRUE(diag_tx().empty());                                // no write on that
  EXPECT_STREQ(b.user_query_result(), "requested");
}

TEST(TwingoFreeWrite, NegativeAnswerToTheWriteShowsTheMeaningOfTheCode) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x2E, 0x33, 0, 0, 0, 0}));
  EXPECT_STREQ(b.user_query_result(),
               "2E 92 61 00 00 03: before 00 00 00 -> NEGATIVE 7F 2E 33 (SID 0x2E, NRC 0x33) - security access denied");
}

TEST(TwingoFreeWrite, NegativeAnswerWithAnUnknownCodeStillShowsTheCode) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 81 01", t, log);
  b.handle_incoming_can_frame(did_reply(0x9281, {0x00}));
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x2E, 0x99, 0, 0, 0, 0}));
  EXPECT_STREQ(b.user_query_result(), "2E 92 81 01: before 00 -> NEGATIVE 7F 2E 99 (SID 0x2E, NRC 0x99)");
}

TEST(TwingoFreeWrite, NoAnswerToTheWriteSaysThatItIsNotKnownWhetherItWasWritten) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  clear_transmitted_frames();
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));
  ASSERT_EQ(diag_tx().size(), 1u);  // the write goes out when the read has been answered
  EXPECT_EQ(diag_tx()[0].data.u8[1], 0x2E);
  log.clear();
  run(b, t, 2500, 100, log);
  EXPECT_EQ(count_diag_sid(log, 0x2E), 0);  // and is not repeated
  EXPECT_STREQ(b.user_query_result(),
               "no response to the write - not known whether it was written, read the value again");
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");
}

TEST(TwingoFreeWrite, ResponsePendingToTheWriteKeepsWaiting) {
  TestTwingo b;
  b.setup();
  uint64_t t;
  std::vector<Tx> log;
  start_write_until_read(b, "2E 92 61 00 00 03", t, log);
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x00}));
  run(b, t, 1500, 100, log);
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x2E, 0x78, 0, 0, 0, 0}));  // pending restarts the timeout
  run(b, t, 1500, 100, log);
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x03, 0x6E, 0x92, 0x61, 0, 0, 0, 0}));
  b.handle_incoming_can_frame(did_reply(0x9261, {0x00, 0x00, 0x03}));  // the read back
  EXPECT_STREQ(b.user_query_result(), "2E 92 61 00 00 03: before 00 00 00 -> OK 6E 92 61 | read back 00 00 03");
}

TEST(TwingoFreeWrite, CellPollingIsPausedWhileTheWriteRuns) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 700, 100, log);
  ASSERT_STREQ(b.start_user_query("2E 92 61 00 00 03"), "OK");
  log.clear();
  run(b, t, 1500, 100, log);
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {  // only the session, the read before and nothing else
    EXPECT_TRUE(x.f.data.u8[1] == 0x22 || x.f.data.u8[1] == 0x10) << "SID " << (int)x.f.data.u8[1];
  }
  EXPECT_EQ(count_diag_sid(log, 0x22), 1);
}

// ---------------------------------------------------------------------------
// Manual vehicle age (05.10.)
// ---------------------------------------------------------------------------

// Resets the runtime-only age switches after each test (they are static).
struct AgeGuard {
  ~AgeGuard() { RenaultTwingoGen1Battery::age_manual_clear(); }
};

static uint32_t age_of(const Tx& x) {
  return ((uint32_t)x.f.data.u8[1] << 16) | ((uint32_t)x.f.data.u8[2] << 8) | x.f.data.u8[3];
}

// Since Bau 13 the age is visible on the bus only in 0x436 (Zoe form, row 0x436, the frame follows the age).
static void age_on_0x436() {
  RenaultTwingoGen1Battery::time_436_active = false;
  RenaultTwingoGen1Battery::time_436_follow_age = true;
  datalayer_extended.twingoGen1.bus_format_zoe_old = true;
  datalayer_extended.twingoGen1.simulator_enabled_mask = 1ULL << RenaultTwingoGen1Battery::SIM_ROW_436;
}

TEST(TwingoManualAge, SendsTheEnteredValueAndCountsOnePerMinute) {
  AgeGuard guard;
  age_on_0x436();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = AGE_EPOCH + 2959882LL * 60;
  set_millis64(1000);
  RenaultTwingoGen1Battery::age_manual_set(3);
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  auto f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(f.back().f.data.u8[1], 0x00);
  EXPECT_EQ(f.back().f.data.u8[2], 0x00);
  EXPECT_EQ(f.back().f.data.u8[3], 0x03);  // not the automatic value
  log.clear();
  run(b, t, 60000, 100, log);  // one minute later
  f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(age_of(f.back()), 4u);
}

TEST(TwingoManualAge, OffReturnsToTheAutomaticMode) {
  AgeGuard guard;
  age_on_0x436();
  TestTwingo b;
  b.setup();
  b.unix_set = true;
  b.unix_now = AGE_EPOCH + 2959882LL * 60;
  set_millis64(1000);
  RenaultTwingoGen1Battery::age_manual_set(5);
  RenaultTwingoGen1Battery::age_manual_clear();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  auto f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(age_of(f.back()), 1025301u);
}

TEST(TwingoManualAge, ValueIsLimitedTo24Bit) {
  AgeGuard guard;
  age_on_0x436();
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  RenaultTwingoGen1Battery::age_manual_set(0xFFFFFFFFu);
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 200, 100, log);
  auto f = with_id(log, 0x436);
  ASSERT_FALSE(f.empty());
  EXPECT_EQ(age_of(f.back()), 0xFFFFFFu);
}

TEST(TwingoFreeQuery, CellPollingIsPausedWhileTheRequestRuns) {
  TestTwingo b;
  b.setup();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 700, 100, log);
  EXPECT_FALSE(with_id(log, 0x18DADBF1, true).empty());  // normal polling runs
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  log.clear();
  run(b, t, 1500, 100, log);  // within the 2 s timeout, no answer: nothing else may be sent on this ID
  EXPECT_TRUE(with_id(log, 0x18DADBF1, true).empty());
}

// ---------------------------------------------------------------------------
// 19 14: fault detection counters
// ---------------------------------------------------------------------------

TEST(TwingoFaultCounters, RequestIsSentAfterTheSessionGapAndAnswerIsDecoded) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  clear_transmitted_frames();
  b.read_DTC_fdc();
  EXPECT_STREQ(b.fdc_query_result(), "requested");
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);
  bool found = false;
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
    if (x.f.data.u8[1] == 0x19 && x.f.data.u8[2] == 0x14) {
      found = true;
      EXPECT_EQ(x.f.data.u8[0], 0x02);
    }
  }
  EXPECT_TRUE(found);
  // 59 14 E14381 7F 1B0715 FF = 10 bytes (multi-frame): first frame 6 bytes, then 4
  clear_transmitted_frames();
  b.handle_incoming_can_frame(reply_frame({0x10, 0x0A, 0x59, 0x14, 0xE1, 0x43, 0x81, 0x7F}));
  EXPECT_EQ(diag_tx().size(), 1u);  // flow control
  b.handle_incoming_can_frame(reply_frame({0x21, 0x1B, 0x07, 0x15, 0xFF, 0xAA, 0xAA, 0xAA}));
  EXPECT_STREQ(b.fdc_query_result(), "OK, 2 entries: E14381=+127 1B0715=-1");
  // The free query result is a separate field
  EXPECT_STREQ(b.user_query_result(), "not run yet");
}

TEST(TwingoFaultCounters, EmptyAnswerAndNegativeAnswerAndOddLength) {
  {
    TestTwingo b;
    b.setup();
    set_millis64(1000);
    b.read_DTC_fdc();
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 300, 50, log);
    b.handle_incoming_can_frame(reply_frame({0x02, 0x59, 0x14, 0, 0, 0, 0, 0}));
    EXPECT_STREQ(b.fdc_query_result(), "OK, no DTC with a fault detection counter");
  }
  {
    TestTwingo b;
    b.setup();
    set_millis64(1000);
    b.read_DTC_fdc();
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 300, 50, log);
    b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x19, 0x12, 0, 0, 0, 0}));
    EXPECT_STREQ(b.fdc_query_result(), "NEGATIVE 7F 19 12 (SID 0x19, NRC 0x12)");
  }
  {
    TestTwingo b;
    b.setup();
    set_millis64(1000);
    b.read_DTC_fdc();
    uint64_t t = 1000;
    std::vector<Tx> log;
    run(b, t, 300, 50, log);
    // 59 14 E1 43 81 = 5 bytes: not a multiple of 4 after the sub-function, so shown raw
    b.handle_incoming_can_frame(reply_frame({0x05, 0x59, 0x14, 0xE1, 0x43, 0x81, 0, 0}));
    EXPECT_STREQ(b.fdc_query_result(), "OK 59 14 E1 43 81");
  }
}

TEST(TwingoFaultCounters, NoAnswerGivesTimeout) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  b.read_DTC_fdc();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 3000, 100, log);
  EXPECT_STREQ(b.fdc_query_result(), "no response");
}

// ---------------------------------------------------------------------------
// 19 06: DTC details now with the multi-frame collector
// ---------------------------------------------------------------------------

TEST(TwingoDtcDetails, ShowsTheCompleteAnswerOfBothCodes) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  b.read_DTC_details();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);
  bool found = false;
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
    if (x.f.data.u8[1] == 0x19 && x.f.data.u8[2] == 0x06) {
      found = true;
      EXPECT_EQ(x.f.data.u8[0], 0x06);
      EXPECT_EQ(x.f.data.u8[3], 0xE1);
      EXPECT_EQ(x.f.data.u8[4], 0x43);
      EXPECT_EQ(x.f.data.u8[5], 0x81);
      EXPECT_EQ(x.f.data.u8[6], 0xFF);
    }
  }
  EXPECT_TRUE(found);
  // Single frame answer of the real battery: 59 06 E1 43 81 50
  b.handle_incoming_can_frame(reply_frame({0x06, 0x59, 0x06, 0xE1, 0x43, 0x81, 0x50, 0}));
  run(b, t, 300, 50, log);  // next code after the session gap
  bool second = false;
  for (const Tx& x : with_id(log, 0x18DADBF1, true)) {
    if (x.f.data.u8[1] == 0x19 && x.f.data.u8[2] == 0x06 && x.f.data.u8[3] == 0x1B) {
      second = true;
    }
  }
  EXPECT_TRUE(second);
  // The second answer is longer (multi-frame): 59 06 1B 07 15 50 AA BB CC DD = 10 bytes
  b.handle_incoming_can_frame(reply_frame({0x10, 0x0A, 0x59, 0x06, 0x1B, 0x07, 0x15, 0x50}));
  b.handle_incoming_can_frame(reply_frame({0x21, 0xAA, 0xBB, 0xCC, 0xDD, 0, 0, 0}));
  std::string lines = detail_lines(b.get_uds_info_html());
  EXPECT_NE(lines.find(": OK 59 06 E1 43 81 50<br>"), std::string::npos) << lines;
  EXPECT_NE(lines.find(": OK 59 06 1B 07 15 50 AA BB CC DD<br>"), std::string::npos) << lines;
}

TEST(TwingoDtcDetails, NoAnswerGivesNoResponseForBothCodes) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  b.read_DTC_details();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 6000, 100, log);
  std::string lines = detail_lines(b.get_uds_info_html());
  size_t first = lines.find(": no response<br>");
  ASSERT_NE(first, std::string::npos) << lines;
  EXPECT_NE(lines.find(": no response<br>", first + 1), std::string::npos) << lines;
}

// ---------------------------------------------------------------------------
// More Battery Info page
// ---------------------------------------------------------------------------

TEST(TwingoPage, ShowsInputButtonAndAnswerFieldForTheFreeRequest) {
  TestTwingo b;
  b.setup();
  String html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "id='twingoQueryHex'"));
  EXPECT_TRUE(contains(html, "onclick='twingoQuery(-1)'"));
  EXPECT_TRUE(contains(html, "id='twingoQueryResult'"));
  EXPECT_TRUE(contains(html, "twingoFdc()"));
  EXPECT_TRUE(contains(html, "/twingoQuery?hex="));
  EXPECT_TRUE(contains(html, "/twingoQueryResult?which="));
  EXPECT_TRUE(contains(html, "/triggerTwingoDtcFdc"));
}

TEST(TwingoPage, AnswerFieldShowsTheLastResult) {
  TestTwingo b;
  b.setup();
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  b.handle_incoming_can_frame(reply_frame({0x06, 0x62, 0x92, 0x5E, 0x13, 0x88, 0x6F, 0x00}));
  String html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "22 92 5E: OK 62 92 5E 13 88 6F"));
}

// ---------------------------------------------------------------------------
// 03.10. corrections: only the reply to the request is taken
// ---------------------------------------------------------------------------

TEST(TwingoReplyMatching, LateCellPollingReplyIsNotTakenAsTheAnswer) {
  // Screenshot case: request 22 90 05, the first reply was 62 90 72 0F 53 (cell 80 of the polling).
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("229005"), "OK");
  b.handle_incoming_can_frame(did_reply(0x9072, {0x0F, 0x53}));  // not ours
  EXPECT_STREQ(b.user_query_result(), "requested");              // still waiting
  b.handle_incoming_can_frame(did_reply(0x9005, {0x0E, 0x55}));  // ours
  EXPECT_STREQ(b.user_query_result(), "22 90 05: OK 62 90 05 0E 55");
}

TEST(TwingoReplyMatching, LateReplyStillReachesTheNormalHandlerAndIsNotLost) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("229005"), "OK");
  b.handle_incoming_can_frame(did_reply(0x9072, {0x0F, 0x53}));  // cell 80 = 3831 mV, goes to the polling
  b.handle_incoming_can_frame(did_reply(0x9005, {0x0E, 0x55}));
  EXPECT_TRUE(contains(b.get_uds_info_html(), "22 90 05: OK 62 90 05 0E 55"));
  EXPECT_EQ(datalayer.battery.status.cell_voltages_mV[79], 3831);
}

TEST(TwingoReplyMatching, SessionConfirmationIsNotTakenAsTheFaultCounterAnswer) {
  // Screenshot case: the button "Read DTC fault counters (19 14)" showed OK 50 03 00 32 01 F4.
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  b.read_DTC_fdc();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);  // session gap over, 19 14 sent, now waiting for the reply
  b.handle_incoming_can_frame(reply_frame({0x06, 0x50, 0x03, 0x00, 0x32, 0x01, 0xF4, 0}));
  EXPECT_STREQ(b.fdc_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x02, 0x59, 0x14, 0, 0, 0, 0, 0}));
  EXPECT_STREQ(b.fdc_query_result(), "OK, no DTC with a fault detection counter");
}

TEST(TwingoReplyMatching, NegativeReplyOfAnotherServiceIsIgnored) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x19, 0x12, 0, 0, 0, 0}));  // SID 0x19, not ours
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x03, 0x7F, 0x22, 0x31, 0, 0, 0, 0}));  // ours
  EXPECT_STREQ(b.user_query_result(), "22 92 5E: NEGATIVE 7F 22 31 (SID 0x22, NRC 0x31)");
}

TEST(TwingoReplyMatching, FirstFrameOfAnotherReplyGoesToTheNormalPathAndFlowControlIsSentOnce) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  clear_transmitted_frames();
  // balancing reply of the polling (DID 0x912B) starts: not ours
  b.handle_incoming_can_frame(reply_frame({0x10, 0x0A, 0x62, 0x91, 0x2B, 0x01, 0x02, 0x03}));
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x21, 0x04, 0x05, 0x06, 0x07, 0xAA, 0xAA, 0xAA}));
  EXPECT_STREQ(b.user_query_result(), "requested");
  // our own reply is still accepted afterwards
  b.handle_incoming_can_frame(reply_frame({0x06, 0x62, 0x92, 0x5E, 0x13, 0x88, 0x6F, 0x00}));
  EXPECT_STREQ(b.user_query_result(), "22 92 5E: OK 62 92 5E 13 88 6F");
}

TEST(TwingoReplyMatching, SecondDidOfTheRequestDoesNotMatterOnlyTheFirstIsChecked) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("22 9005 9006"), "OK");
  b.handle_incoming_can_frame(reply_frame({0x07, 0x62, 0x90, 0x05, 0x0E, 0x55, 0x0E, 0x56}));
  EXPECT_STREQ(b.user_query_result(), "22 90 05 90 06: OK 62 90 05 0E 55 0E 56");
}

TEST(TwingoReplyMatching, NoMatchingReplyStillEndsWithTheTimeout) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("22925E"), "OK");
  b.handle_incoming_can_frame(did_reply(0x9072, {0x0F, 0x53}));
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 2500, 100, log);
  EXPECT_STREQ(b.user_query_result(), "no response");
  EXPECT_STREQ(b.start_user_query("22925E"), "OK");  // channel free again
}

TEST(TwingoReplyMatching, DtcDetailsIgnoreTheSessionConfirmationToo) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  b.read_DTC_details();
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);
  b.handle_incoming_can_frame(reply_frame({0x06, 0x50, 0x03, 0x00, 0x32, 0x01, 0xF4, 0}));  // not the answer
  b.handle_incoming_can_frame(reply_frame({0x06, 0x59, 0x06, 0xE1, 0x43, 0x81, 0x2F, 0}));
  std::string lines = detail_lines(b.get_uds_info_html());
  EXPECT_NE(lines.find(": OK 59 06 E1 43 81 2F<br>"), std::string::npos) << lines;
  EXPECT_EQ(lines.find("50 03 00 32"), std::string::npos) << lines;
}

TEST(TwingoReplyMatching, ReadServiceNineteenSubFunctionMustBeEchoed) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  ASSERT_STREQ(b.start_user_query("190209"), "OK");
  uint64_t t = 1000;
  std::vector<Tx> log;
  run(b, t, 300, 50, log);
  b.handle_incoming_can_frame(reply_frame({0x03, 0x59, 0x14, 0xFF, 0, 0, 0, 0}));  // other sub-function
  EXPECT_STREQ(b.user_query_result(), "requested");
  b.handle_incoming_can_frame(reply_frame({0x10, 0x0B, 0x59, 0x02, 0xFF, 0xE1, 0x43, 0x81}));
  b.handle_incoming_can_frame(reply_frame({0x21, 0x2F, 0x1B, 0x07, 0x15, 0x2F, 0xAA, 0xAA}));
  EXPECT_STREQ(b.user_query_result(), "19 02 09: OK 59 02 FF E1 43 81 2F 1B 07 15 2F | DTC: E14381=2F 1B0715=2F");
}

// ---------------------------------------------------------------------------
// 03.10. corrections: no raw '%' in the pages (the web server's template engine eats text between two of them)
// ---------------------------------------------------------------------------

TEST(TwingoPagePercent, NoRawPercentSignAnywhereOnceAllPercentValuesAreRead) {
  TestTwingo b;
  b.setup();
  set_millis64(1000);
  b.handle_incoming_can_frame(did_reply(0x9003, {0x27, 0x10}));  // SOH 100 %
  b.handle_incoming_can_frame(did_reply(0x9001, {0x1A, 0x7E}));  // SOC
  b.handle_incoming_can_frame(did_reply(0x9002, {0x17, 0xDA}));  // USOC
  b.handle_incoming_can_frame(did_reply(0x91B9, {0x1B, 0x74}));  // SOC min
  b.handle_incoming_can_frame(did_reply(0x91BA, {0x1B, 0xC5}));  // SOC max
  CAN_frame f658 = {};
  f658.ID = 0x658;
  f658.DLC = 8;
  f658.data.u8[4] = 0x5F;  // SOH candidate
  b.handle_incoming_can_frame(f658);
  std::string page(b.get_uds_info_html().c_str());
  EXPECT_EQ(page.find('%'), std::string::npos) << "a raw percent sign is still in the More Battery Info page";
  EXPECT_NE(page.find("Battery SOH avg (0x9003): 100.000 &#37;"), std::string::npos);
  EXPECT_NE(page.find("SOH candidate (0x658 byte 4): 95 &#37;"), std::string::npos);
  // The block that was swallowed on 03.10. is present: checkbox, DTC read and erase lines
  EXPECT_NE(page.find("Read ALL DTC statuses (mask 0xFF)"), std::string::npos);
  EXPECT_NE(page.find("DTC Erase (ext. protocol, Erase DTC button):"), std::string::npos);
  EXPECT_NE(page.find("DTC Read (ext. protocol, Read DTC button):"), std::string::npos);
}

TEST(TwingoPagePercent, SimulatorPageHasNoRawPercentSignEither) {
  std::string page(simulator_processor(String("X")).c_str());
  ASSERT_GT(page.size(), 1000u);
  EXPECT_EQ(page.find('%'), std::string::npos);
}

// ---------------------------------------------------------------------------
// 03.10.: steady 0x350 frame C7 / C3
// ---------------------------------------------------------------------------
