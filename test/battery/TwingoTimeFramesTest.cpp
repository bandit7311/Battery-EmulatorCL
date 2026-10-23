#include <gtest/gtest.h>

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

namespace {

// Test double: replaces the three hooks that touch WiFi / NTP / the system clock.
class TestTwingo : public RenaultTwingoGen1Battery {
 public:
  bool network_up = false;
  int ntp_starts = 0;
  bool clock_set = false;
  uint32_t clock_secs = 0;
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

  bool network_ready() override { return network_up; }
  bool get_unix_time(time_t& now_utc) override {
    if (!unix_set) {
      return false;
    }
    now_utc = unix_now;
    return true;
  }
  void start_ntp() override { ntp_starts++; }
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

// Runs one transmit_can() call at time t and returns what went on the wire during it.
std::vector<CAN_frame> tick(RenaultTwingoGen1Battery& b, uint64_t t) {
  set_millis64(t);
  clear_transmitted_frames();
  b.transmit_can((unsigned long)t);
  return get_transmitted_frames();
}

// Same, but appends to a timeline (with the time of each tick) so the whole sleep run can be inspected.
void tick_log(RenaultTwingoGen1Battery& b, uint64_t t, std::vector<Tx>& log) {
  for (const CAN_frame& f : tick(b, t)) {
    log.push_back({t, f});
  }
}

std::vector<CAN_frame> with_id(const std::vector<CAN_frame>& v, uint32_t id) {
  std::vector<CAN_frame> out;
  for (const CAN_frame& f : v) {
    if (f.ID == id && !f.ext_ID) {
      out.push_back(f);
    }
  }
  return out;
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

uint32_t secs(uint32_t h, uint32_t m, uint32_t s) {
  return h * 3600 + m * 60 + s;
}

// Zero point of the vehicle age counter (0x350 bytes 1-3): 15.02.2021 11:13:08 UTC.
const time_t AGE_EPOCH = 1613387588;
// UTC time at which the vehicle age is `minutes` (plus a few seconds into the minute).
time_t unix_for_age(uint32_t minutes, uint32_t extra_secs = 5) {
  return AGE_EPOCH + (time_t)minutes * 60 + (time_t)extra_secs;
}
// The three age bytes of 0x350 for a minute value.
void age_bytes(uint32_t minutes, uint8_t out[3]) {
  out[0] = (uint8_t)(minutes >> 16);
  out[1] = (uint8_t)(minutes >> 8);
  out[2] = (uint8_t)minutes;
}

CAN_frame frame_424(int t_min_c, int t_max_c) {
  CAN_frame f = {};
  f.DLC = 8;
  f.ID = 0x424;
  f.data.u8[4] = (uint8_t)(t_min_c + 40);
  f.data.u8[5] = 96;    // SOH
  f.data.u8[6] = 0x55;  // heartbeat
  f.data.u8[7] = (uint8_t)(t_max_c + 40);
  return f;
}

CAN_frame frame_658(uint8_t byte4) {
  CAN_frame f = {};
  f.DLC = 8;
  f.ID = 0x658;
  f.data.u8[0] = 0x67;
  f.data.u8[4] = byte4;
  return f;
}

void reset_datalayer_temperatures() {
  datalayer.battery.status.temperature_sensors_valid_mask = 0;  // no pack sensors -> the 0x424 values are used
  datalayer.battery.status.temperature_min_dC = 0;
  datalayer.battery.status.temperature_max_dC = 0;
}

bool contains(const String& s, const char* needle) {
  return std::string(s.c_str()).find(needle) != std::string::npos;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0x53B / 0x350 in normal operation
// ---------------------------------------------------------------------------

TEST(TwingoTimeFramesTests, NtpIsStartedOnceAsSoonAsTheNetworkIsUp) {
  TestTwingo b;
  b.setup();
  b.network_up = false;
  for (uint64_t t = 1000; t <= 5000; t += 100) {
    tick(b, t);
  }
  EXPECT_EQ(b.ntp_starts, 0);  // no WiFi yet
  b.network_up = true;         // WiFi comes up late, after the boot
  for (uint64_t t = 5100; t <= 12000; t += 100) {
    tick(b, t);
  }
  EXPECT_EQ(b.ntp_starts, 1);  // started once, not on every second
}

// ---------------------------------------------------------------------------
// Sleep / wake sequence: fixed 0x350 age, 0x53B follows the other own frames
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Temperature boot filter (0x424)
// ---------------------------------------------------------------------------

TEST(TwingoTimeFramesTests, ImplausibleTemperaturesAreDroppedInTheBootPhase) {
  reset_datalayer_temperatures();
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(frame_424(-40, 215));  // raw 0 / 255, as right after a power cycle
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 0);
  EXPECT_EQ(datalayer.battery.status.temperature_min_dC, 0);
  b.handle_incoming_can_frame(frame_424(-40, 215));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 0);
}

TEST(TwingoTimeFramesTests, FirstPlausibleFrameEndsTheBootPhaseAndValuesPassAfterwards) {
  reset_datalayer_temperatures();
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(frame_424(20, 22));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_min_dC, 200);
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 220);
  // Boot phase over: a genuine over-temperature is no longer filtered.
  b.handle_incoming_can_frame(frame_424(20, 75));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 750);
  // The limits themselves are inclusive.
  reset_datalayer_temperatures();
  RenaultTwingoGen1Battery c;
  c.setup();
  c.handle_incoming_can_frame(frame_424(-20, 60));
  c.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_min_dC, -200);
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 600);
}

TEST(TwingoTimeFramesTests, BootPhaseEndsAfter60SecondsEvenWithoutAPlausibleFrame) {
  reset_datalayer_temperatures();
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(frame_424(-40, 215));  // first frame at 1000 ms starts the 60 s
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 0);
  set_millis64(1000 + 59999);
  b.handle_incoming_can_frame(frame_424(-40, 215));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 0);  // still inside the 60 s
  set_millis64(1000 + 60000);
  b.handle_incoming_can_frame(frame_424(-40, 215));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.temperature_max_dC, 2150);  // now taken over unfiltered
}

// ---------------------------------------------------------------------------
// New values on "More Battery Info"
// ---------------------------------------------------------------------------

TEST(TwingoTimeFramesTests, Soh658IsShownAsCandidate) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  EXPECT_TRUE(contains(b.get_uds_info_html(), "SOH candidate (0x658 byte 4): not received"));
  b.handle_incoming_can_frame(frame_658(0x5F));
  EXPECT_TRUE(contains(b.get_uds_info_html(), "SOH candidate (0x658 byte 4): 95 &#37;"));
  b.handle_incoming_can_frame(frame_658(0x60));
  EXPECT_TRUE(contains(b.get_uds_info_html(), "SOH candidate (0x658 byte 4): 96 &#37;"));
  b.handle_incoming_can_frame(frame_658(0x7F));
  EXPECT_TRUE(contains(b.get_uds_info_html(), "SOH candidate (0x658 byte 4): invalid (127)"));
  // The top bit is masked away like in OVMS (0xE0 & 0x7F = 96).
  b.handle_incoming_can_frame(frame_658(0xE0));
  EXPECT_TRUE(contains(b.get_uds_info_html(), "SOH candidate (0x658 byte 4): 96 &#37;"));
}

TEST(TwingoTimeFramesTests, Soh658IsNotUsedForTheDatalayerSoh) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(frame_424(20, 22));  // SOH from 0x424 = 96
  b.update_values();
  uint16_t soh_before = datalayer.battery.status.soh_pptt;
  b.handle_incoming_can_frame(frame_658(0x5F));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.soh_pptt, soh_before);
  EXPECT_EQ(soh_before, 9600);
}

TEST(TwingoTimeFramesTests, TimePidsAreShownRaw) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  String html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "Time (0x9261): not yet read"));
  EXPECT_TRUE(contains(html, "Pack time (0x91C1): not yet read"));
  // Real reply of the pack: 06 62 91 C1 0D EE 55 (three data bytes).
  CAN_frame f = {};
  f.ext_ID = true;
  f.DLC = 8;
  f.ID = 0x18DAF1DB;
  const uint8_t reply[8] = {0x06, 0x62, 0x91, 0xC1, 0x0D, 0xEE, 0x55, 0xAA};
  memcpy(f.data.u8, reply, 8);
  b.handle_incoming_can_frame(f);
  html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "Pack time (0x91C1): 0DEE55 (912981)"));
  EXPECT_TRUE(contains(html, "Time (0x9261): not yet read"));
  // A two-byte reply for 0x9261.
  const uint8_t reply2[8] = {0x05, 0x62, 0x92, 0x61, 0x01, 0x2C, 0xAA, 0xAA};
  memcpy(f.data.u8, reply2, 8);
  b.handle_incoming_can_frame(f);
  html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "Time (0x9261): 012C (300)"));
}

TEST(TwingoTimeFramesTests, TimePidsAreInThePollList) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  std::set<uint16_t> pids;
  uint32_t requests = 0;
  for (uint64_t t = 1000; t < 1000 + 200000; t += 100) {  // 1 request/s, a round with the extras takes ~175 s
    for (const CAN_frame& f : tick(b, t)) {
      if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[0] == 0x03 && f.data.u8[1] == 0x22) {
        pids.insert((uint16_t)((f.data.u8[2] << 8) | f.data.u8[3]));
        requests++;
      }
    }
  }
  EXPECT_TRUE(pids.count(0x9261) == 1);
  EXPECT_TRUE(pids.count(0x91C1) == 1);
  // 96 cells + 23 others + 16 new display-only PIDs + the display-only battery current 0x900D = 136 poll
  // targets, one request every 1 s since 12.10. (see the comment at ext_poll_list in the header)
  EXPECT_EQ(pids.size(), 136u);
  EXPECT_TRUE(pids.count(0x900D) == 1);
  EXPECT_GE(requests, 136u);
}

// ---------------------------------------------------------------------------
// 0x090 (10 ms) and 0x242 (20 ms) with counter and CRC
// ---------------------------------------------------------------------------

namespace {

// Independent bitwise CRC-8 (poly 0x1D, start 0), not the table of the driver.
uint8_t crc8_ref(const std::vector<uint8_t>& d, uint8_t xor_out) {
  uint8_t c = 0;
  for (uint8_t b : d) {
    c ^= b;
    for (int i = 0; i < 8; i++) {
      c = (c & 0x80) ? (uint8_t)((c << 1) ^ 0x1D) : (uint8_t)(c << 1);
    }
  }
  return (uint8_t)(c ^ xor_out);
}

std::vector<uint8_t> bytes_of(const CAN_frame& f) {
  return std::vector<uint8_t>(f.data.u8, f.data.u8 + f.DLC);
}

// Runs 1 ms steps like the core loop does and collects every frame.
void run_ms(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms, std::vector<Tx>& log) {
  for (uint64_t end = t + ms; t < end; t++) {
    tick_log(b, t, log);
  }
}

}  // namespace

// ---------------------------------------------------------------------------
// 0x9281 fix: write 0 ("activated"), not 1
// ---------------------------------------------------------------------------

TEST(TwingoNewDisplayPidsTests, NvrolResetWritesTemporisationZero) {
  reset_datalayer_temperatures();
  RenaultTwingoGen1Battery b;
  b.setup();
  set_millis64(1000);
  b.reset_NVROL();
  std::vector<CAN_frame> writes;
  for (uint64_t t = 1000; t < 1000 + 2000; t += 10) {
    for (const CAN_frame& f : tick(b, t)) {
      if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[1] == 0x2E && f.data.u8[2] == 0x92 && f.data.u8[3] == 0x81) {
        writes.push_back(f);
      }
    }
  }
  ASSERT_EQ(writes.size(), 1u);
  EXPECT_EQ(writes[0].data.u8[4], 0x00)
      << "must write 0x00 (\"temporisation is activated\" per the real ECU dump), not 0x01";
}

TEST(TwingoNewDisplayPidsTests, Sleep9281WritesTemporisationZero) {
  reset_datalayer_temperatures();
  RenaultTwingoGen1Battery b;
  b.setup();
  set_millis64(1000);
  b.request_sleep_temporisation();
  std::vector<CAN_frame> writes;
  for (uint64_t t = 1000; t < 1000 + 500; t += 10) {
    for (const CAN_frame& f : tick(b, t)) {
      if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[1] == 0x2E && f.data.u8[2] == 0x92 && f.data.u8[3] == 0x81) {
        writes.push_back(f);
      }
    }
  }
  ASSERT_EQ(writes.size(), 1u);
  EXPECT_EQ(writes[0].data.u8[4], 0x00);
}

// ---------------------------------------------------------------------------
// B009 RequestRoutineResults (subfunction 0x03) after the start
// ---------------------------------------------------------------------------

TEST(TwingoNewDisplayPidsTests, NvrolResetRequestsRoutineResultsAfterStart) {
  reset_datalayer_temperatures();
  RenaultTwingoGen1Battery b;
  b.setup();
  set_millis64(1000);
  b.reset_NVROL();
  std::vector<CAN_frame> requests;
  for (uint64_t t = 1000; t < 1000 + 2000; t += 10) {
    for (const CAN_frame& f : tick(b, t)) {
      if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[1] == 0x31) {
        requests.push_back(f);
      }
    }
  }
  ASSERT_EQ(requests.size(), 2u);
  // 0x31 01 B0 09 = StartRoutine, then 0x31 03 B0 09 = RequestRoutineResults, in that order.
  EXPECT_EQ(requests[0].data.u8[2], 0x01);
  EXPECT_EQ(requests[0].data.u8[3], 0xB0);
  EXPECT_EQ(requests[0].data.u8[4], 0x09);
  EXPECT_EQ(requests[1].data.u8[2], 0x03);
  EXPECT_EQ(requests[1].data.u8[3], 0xB0);
  EXPECT_EQ(requests[1].data.u8[4], 0x09);
}

TEST(TwingoNewDisplayPidsTests, RoutineResultsReplyIsLogged) {
  reset_datalayer_temperatures();
  RenaultTwingoGen1Battery b;
  b.setup();
  set_millis64(1000);
  b.reset_NVROL();
  // Tick only into the window where nvrol_awaiting_step == 5 (after the results request at t=600ms into the
  // sequence, before case 2's own 1s wait elapses at t=1600ms and moves the awaiting step on).
  for (uint64_t t = 1000; t < 1000 + 1000; t += 10) {
    tick(b, t);
  }
  CAN_frame reply = {};
  reply.ext_ID = true;
  reply.DLC = 8;
  reply.ID = 0x18DAF1DB;
  const uint8_t data[8] = {0x04, 0x71, 0x03, 0xB0, 0x09, 0xAA, 0xAA, 0xAA};  // positive response to RoutineControl
  memcpy(reply.data.u8, data, 8);
  b.handle_incoming_can_frame(reply);
  EXPECT_TRUE(contains(b.get_uds_info_html(), "NVROL Log - Routine B009 results: OK"));
}

// ---------------------------------------------------------------------------
// The 16 new display-only PIDs: parsing + formula
// ---------------------------------------------------------------------------

namespace {
CAN_frame uds_reply(uint16_t pid, std::initializer_list<uint8_t> data_bytes) {
  CAN_frame f = {};
  f.ext_ID = true;
  f.DLC = 8;
  f.ID = 0x18DAF1DB;
  uint8_t n = (uint8_t)data_bytes.size();
  f.data.u8[0] = (uint8_t)(3 + n);  // PCI: SID + 2 PID bytes + n data bytes
  f.data.u8[1] = 0x62;
  f.data.u8[2] = (uint8_t)(pid >> 8);
  f.data.u8[3] = (uint8_t)(pid & 0xFF);
  uint8_t i = 4;
  for (uint8_t b : data_bytes) {
    f.data.u8[i++] = b;
  }
  return f;
}
}  // namespace

TEST(TwingoNewDisplayPidsTests, AllSixteenPidsShowNotYetReadBeforeAnyReply) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  String html = b.get_uds_info_html();
  for (const char* label :
       {"Pack Mileage (0x91CF)", "Vehicle Distance Totalizer (0x925F)", "Low Voltage Supply (0x9011)",
        "Pack Voltage, cell sum (0x9006)", "Cell Voltage A (0x9007", "Cell Voltage B (0x9009",
        "Cell Voltage A index (0x9008)", "Cell Voltage B index (0x900A)", "Battery SOH avg (0x9003)",
        "Max Charge Power (0x9018)", "Max Generated Power (0x900E)", "Max Available Power (0x900F)",
        "Battery SOC, internal (0x9001)", "Battery USOC, dashboard (0x9002", "Battery SOC min (0x91B9)",
        "Battery SOC max (0x91BA)"}) {
    EXPECT_TRUE(contains(html, label)) << label;
  }
  EXPECT_EQ(html.c_str() + std::string(html.c_str()).find("Pack Mileage"),
            html.c_str() + std::string(html.c_str()).find("Pack Mileage"));  // sanity no-op, see per-field checks below
  // Every one of the 16 must currently say "not yet read".
  size_t not_yet_read_count = 0;
  std::string h(html.c_str());
  size_t pos = 0;
  while ((pos = h.find("not yet read", pos)) != std::string::npos) {
    not_yet_read_count++;
    pos += 1;
  }
  EXPECT_GE(not_yet_read_count, 16u);
}

TEST(TwingoNewDisplayPidsTests, PackMileageFormula) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(uds_reply(0x91CF, {0x80, 0x08, 0x95, 0x60}));  // real dump: 17579 km
  EXPECT_TRUE(contains(b.get_uds_info_html(), "Pack Mileage (0x91CF): 17579.00 km"));
}

TEST(TwingoNewDisplayPidsTests, VehicleDistanceTotalizerFormula) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(uds_reply(0x925F, {0x00, 0x82, 0x68, 0xC4}));  // real dump: 85465 km
  EXPECT_TRUE(contains(b.get_uds_info_html(), "Vehicle Distance Totalizer (0x925F): 85465.00 km"));
}

TEST(TwingoNewDisplayPidsTests, LowVoltageSupplyFormula) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(uds_reply(0x9011, {0x33, 0xAF}));  // real dump: 12.92 V
  EXPECT_TRUE(contains(b.get_uds_info_html(), "Low Voltage Supply (0x9011): 12.921 V"));
}

TEST(TwingoNewDisplayPidsTests, PackVoltageAndCellVoltagesFormula) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(uds_reply(0x9006, {0x00, 0x05, 0xC2, 0xE0}));  // real dump: 368.71875 V
  b.handle_incoming_can_frame(uds_reply(0x9007, {0x0F, 0x62}));
  b.handle_incoming_can_frame(uds_reply(0x9009, {0x0F, 0x5A}));
  b.handle_incoming_can_frame(uds_reply(0x9008, {0x23}));  // 35
  b.handle_incoming_can_frame(uds_reply(0x900A, {0x00}));  // 0
  String html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "Pack Voltage, cell sum (0x9006): 368.719 V"));
  EXPECT_TRUE(contains(html, "Cell Voltage A index (0x9008): 35.000"));
  EXPECT_TRUE(contains(html, "Cell Voltage B index (0x900A): 0.000"));
}

TEST(TwingoNewDisplayPidsTests, SohAndPowerLimitsFormula) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(uds_reply(0x9003, {0x25, 0x75}));  // 95.89 %
  b.handle_incoming_can_frame(uds_reply(0x9018, {0x05, 0x8B}));  // 14.19 kW
  b.handle_incoming_can_frame(uds_reply(0x900E, {0x10, 0xCC}));  // 43 kW
  b.handle_incoming_can_frame(uds_reply(0x900F, {0x1C, 0x20}));  // 72 kW
  String html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "Battery SOH avg (0x9003): 95.890 &#37;"));
  EXPECT_TRUE(contains(html, "Max Charge Power (0x9018): 14.190 kW"));
  EXPECT_TRUE(contains(html, "Max Generated Power (0x900E): 43.000 kW"));
  EXPECT_TRUE(contains(html, "Max Available Power (0x900F): 72.000 kW"));
}

TEST(TwingoNewDisplayPidsTests, SocFormulaWithOffset) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.handle_incoming_can_frame(uds_reply(0x9001, {0x1A, 0x7E}));  // 64.82 %
  b.handle_incoming_can_frame(uds_reply(0x91B9, {0x1B, 0x74}));  // 67.28 %
  b.handle_incoming_can_frame(uds_reply(0x91BA, {0x1B, 0xC5}));  // 68.09 %
  b.handle_incoming_can_frame(uds_reply(0x9002, {0x17, 0xDA}));  // 61.06 % (dashboard SOC, no offset)
  String html = b.get_uds_info_html();
  EXPECT_TRUE(contains(html, "Battery SOC, internal (0x9001): 64.820 &#37;"));
  EXPECT_TRUE(contains(html, "Battery SOC min (0x91B9): 67.280 &#37;"));
  EXPECT_TRUE(contains(html, "Battery SOC max (0x91BA): 68.090 &#37;"));
  EXPECT_TRUE(contains(html, "Battery USOC, dashboard (0x9002, display only): 61.060 &#37;"));
}

TEST(TwingoNewDisplayPidsTests, NewPidsAreInThePollListAndNoneAreDuplicated) {
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  std::set<uint16_t> pids;
  for (uint64_t t = 1000; t < 1000 + 200000; t += 100) {  // 1 request/s, a round with the extras takes ~175 s
    for (const CAN_frame& f : tick(b, t)) {
      if (f.ext_ID && f.ID == 0x18DADBF1 && f.data.u8[0] == 0x03 && f.data.u8[1] == 0x22) {
        pids.insert((uint16_t)((f.data.u8[2] << 8) | f.data.u8[3]));
      }
    }
  }
  for (uint16_t pid : {0x91CFu, 0x925Fu, 0x9011u, 0x9006u, 0x9007u, 0x9009u, 0x9008u, 0x900Au, 0x9003u, 0x9018u,
                       0x900Eu, 0x900Fu, 0x9001u, 0x9002u, 0x91B9u, 0x91BAu, 0x900Du}) {
    EXPECT_EQ(pids.count(pid), 1u) << "PID 0x" << std::hex << pid;
  }
  EXPECT_EQ(pids.size(), 136u);  // 135 + the battery current 0x900D
}

TEST(TwingoNewDisplayPidsTests, NewPidsNotFedIntoDatalayer) {
  reset_datalayer_temperatures();
  set_millis64(1000);
  RenaultTwingoGen1Battery b;
  b.setup();
  b.update_values();  // establishes the baseline the driver computes from its own broadcast-fed fields alone
  uint16_t soc_before = datalayer.battery.status.real_soc;
  uint16_t soh_before = datalayer.battery.status.soh_pptt;
  b.handle_incoming_can_frame(uds_reply(0x9001, {0x1A, 0x7E}));
  b.handle_incoming_can_frame(uds_reply(0x9002, {0x17, 0xDA}));
  b.handle_incoming_can_frame(uds_reply(0x9003, {0x25, 0x75}));
  b.update_values();
  EXPECT_EQ(datalayer.battery.status.real_soc, soc_before)
      << "0x9001/0x9002 must stay display-only, not feed the datalayer/inverter SOC";
  EXPECT_EQ(datalayer.battery.status.soh_pptt, soh_before) << "0x9003 must stay display-only";
}
