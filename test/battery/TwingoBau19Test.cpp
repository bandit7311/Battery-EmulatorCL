#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 19 (14.10.): label of the answer to "Erase DTC" (the positive answer is the single byte 54).

namespace {

class EraseTwingo : public RenaultTwingoGen1Battery {
 public:
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

void step(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms) {
  for (uint64_t end = t + ms; t < end; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
  }
}

CAN_frame reply(std::initializer_list<uint8_t> bytes) {
  CAN_frame f = {};
  f.ext_ID = true;
  f.ID = 0x18DAF1DB;
  f.DLC = 8;
  uint8_t i = 0;
  for (uint8_t v : bytes) {
    f.data.u8[i++] = v;
  }
  return f;
}

// "DTC Erase (ext. protocol, Erase DTC button): <text>" of the page
std::string erase_line(EraseTwingo& b) {
  const std::string page = b.get_uds_info_html().c_str();
  const std::string key = "DTC Erase (ext. protocol, Erase DTC button): ";
  const size_t p = page.find(key);
  if (p == std::string::npos) {
    return "<missing>";
  }
  const size_t e = page.find("<br>", p);
  return page.substr(p + key.size(), e - (p + key.size()));
}

// Starts Erase DTC, lets the session and the request go out and feeds one answer.
std::string erase_and_answer(std::initializer_list<uint8_t> bytes) {
  EraseTwingo b;
  b.setup();
  uint64_t t = 1000;
  step(b, t, 700);
  b.reset_DTC();
  step(b, t, 300);  // session sent, gap over, 14 FF FF FF sent
  b.handle_incoming_can_frame(reply(bytes));
  return erase_line(b);
}

}  // namespace

TEST(TwingoEraseAnswer, PositiveResponse54IsShownAsOk) {
  EXPECT_EQ(erase_and_answer({0x01, 0x54, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}), "OK (positive response 54)");
}

TEST(TwingoEraseAnswer, OtherSingleByteAndTwoByteAnswersShowTheirBytes) {
  EXPECT_EQ(erase_and_answer({0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}), "short reply (1 byte: AA)");
  EXPECT_EQ(erase_and_answer({0x02, 0x54, 0x12, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}), "short reply (2 bytes: 54 12)");
}

TEST(TwingoEraseAnswer, NegativeResponseIsAsBefore) {
  EXPECT_EQ(erase_and_answer({0x03, 0x7F, 0x14, 0x22, 0xAA, 0xAA, 0xAA, 0xAA}), "NEGATIVE SID=0x14 NRC=0x22");
}

TEST(TwingoEraseAnswer, TheStateGoesBackToIdleAfterTheAnswer) {
  EraseTwingo b;
  b.setup();
  uint64_t t = 1000;
  step(b, t, 700);
  b.reset_DTC();
  step(b, t, 300);
  b.handle_incoming_can_frame(reply({0x01, 0x54, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}));
  // a new Erase can start at once (it would be refused while the first one still waits)
  b.reset_DTC();
  EXPECT_EQ(erase_line(b), "requested");
}
