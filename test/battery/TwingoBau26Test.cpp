#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 26 (15.10.): the raw balancing bytes of the last 0x912B answer under the 0x155 line of the Advanced page, and the extra
// request of 0x912B every BAL_POLL_INTERVAL_MS (20 s) besides the round-robin.

namespace {
class BalTwingo : public RenaultTwingoGen1Battery {
 public:
  BalTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.bus_state_manual = false;
    t.simulator_enabled_mask = 0x10;
    t.sleep_failsafe_minutes = 30;
    t.cell_source_broadcast = false;
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

CAN_frame reply(std::initializer_list<uint8_t> v) {
  CAN_frame f = {};
  f.ID = 0x18DAF1DB;
  f.ext_ID = true;
  f.DLC = 8;
  size_t i = 0;
  for (uint8_t b : v) {
    f.data.u8[i++] = b;
  }
  return f;
}

// The answer of 0x912B: 62 91 2B and 32 data bytes, as a first frame and five consecutive frames (35 bytes).
void feed_912b(BalTwingo& b, const uint8_t data[32]) {
  uint8_t all[35] = {0x62, 0x91, 0x2B};
  for (int i = 0; i < 32; i++) {
    all[3 + i] = data[i];
  }
  b.handle_incoming_can_frame(reply({0x10, 0x23, all[0], all[1], all[2], all[3], all[4], all[5]}));
  size_t pos = 6;
  for (uint8_t sn = 1; sn <= 5; sn++) {
    CAN_frame f = reply({(uint8_t)(0x20 | sn)});
    for (int k = 0; k < 7; k++) {
      f.data.u8[1 + k] = pos < 35 ? all[pos] : 0xAA;
      pos++;
    }
    b.handle_incoming_can_frame(f);
  }
}

int count_912b_requests(const std::vector<CAN_frame>& frames) {
  int n = 0;
  for (const CAN_frame& f : frames) {
    if (f.ID == 0x18DADBF1 && f.data.u8[1] == 0x22 && f.data.u8[2] == 0x91 && f.data.u8[3] == 0x2B) {
      n++;
    }
  }
  return n;
}
}  // namespace

TEST(TwingoBau26, ThePageHasTheLineUnderThe0x155LineAndItSaysNotYetRead) {
  BalTwingo b;
  b.setup();
  const std::string page = b.get_uds_info_html().c_str();
  const size_t rx = page.find("<span id='twingoRx155'>");
  const size_t bal = page.find("<span id='twingoBalBits'>");
  ASSERT_NE(rx, std::string::npos);
  ASSERT_NE(bal, std::string::npos);
  EXPECT_GT(bal, rx) << "under the 0x155 line";
  EXPECT_NE(page.find("0x912B raw (data bytes 21-32, first bit = cell 96): not yet read"), std::string::npos);
  EXPECT_NE(page.find("fetch('/twingoBalBits')"), std::string::npos) << "the page polls it";
}

TEST(TwingoBau26, ShowsTheRawBytesOfTheAnswerAsTheyAreInTheDataBytes) {
  BalTwingo b;
  b.setup();
  set_millis64(1000);
  // data bytes 21 to 32 = the balancing bits; the others (1 to 20) are something else and must not show
  uint8_t data[32] = {0};
  for (int i = 0; i < 20; i++) {
    data[i] = 0x11;
  }
  const uint8_t bits[12] = {0x00, 0x00, 0x00, 0xFF, 0xF0, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x81};
  for (int i = 0; i < 12; i++) {
    data[20 + i] = bits[i];
  }
  feed_912b(b, data);
  set_millis64(13000);
  const std::string t = b.bal_bits_text().c_str();
  EXPECT_NE(t.find("00 00 00 FF F0 00 00 08 00 00 00 81"), std::string::npos) << t;
  EXPECT_EQ(t.find("11 11"), std::string::npos) << "bytes 1 to 20 are not shown";
  EXPECT_NE(t.find("(read 12 s ago)"), std::string::npos) << t;
  // the page carries the same text
  const std::string page = b.get_uds_info_html().c_str();
  EXPECT_NE(page.find("00 00 00 FF F0 00 00 08 00 00 00 81"), std::string::npos);
  // and the decoded bits of the driver are still the same: 0xFF 0xF0 = cells 96 ... 85 (the byte 24 is the 4th of the 12)
  EXPECT_EQ(datalayer.battery.status.balancing_status, BALANCING_STATUS_ACTIVE);
}

TEST(TwingoBau26, AllZeroBitsAreShownToo) {
  BalTwingo b;
  b.setup();
  set_millis64(1000);
  uint8_t data[32] = {0};
  feed_912b(b, data);
  const std::string t = b.bal_bits_text().c_str();
  EXPECT_NE(t.find("00 00 00 00 00 00 00 00 00 00 00 00"), std::string::npos) << t;
  EXPECT_EQ(t.find("not yet read"), std::string::npos);
}

TEST(TwingoBau26Poll, TheBalancingSwitchesAreAskedEvery20SecondsBesideTheRoundRobin) {
  static_assert(BAL_POLL_INTERVAL_MS == 20000, "Bau 26: every 20 s");
  BalTwingo b;
  b.setup();
  int n912b = 0;
  int requests = 0;
  std::vector<uint64_t> at;
  for (uint64_t t = 1000; t < 1000 + 130000; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    const std::vector<CAN_frame>& fr = get_transmitted_frames();
    const int k = count_912b_requests(fr);
    for (const CAN_frame& f : fr) {
      if (f.ID == 0x18DADBF1 && f.data.u8[1] == 0x22) {
        requests++;
      }
    }
    if (k > 0) {
      at.push_back(t - 1000);
    }
    n912b += k;
  }
  // the cells come first in the round-robin; the entry 0x912B of the list is reached after about 100 s: it is skipped when the
  // extra request was a moment ago, so there are never two requests close together; in 130 s there are 6 requests (20 ... 120 s)
  ASSERT_GE(at.size(), 6u);
  for (size_t i = 1; i < at.size(); i++) {
    EXPECT_NEAR((double)(at[i] - at[i - 1]), 20000.0, 1100.0) << "request " << i;
  }
  EXPECT_NEAR((double)at[0], 20000.0, 1100.0);
  // the pace of one request per second stays (130 requests in 130 s)
  EXPECT_NEAR((double)requests, 130.0, 3.0);
  (void)n912b;
}
