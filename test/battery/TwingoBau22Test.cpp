#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 22 (15.10.): the "Result" lines of the silence page were cut off at 119 characters (char b[120]) - the text ended
// in the middle of a word and the <br> was lost, so the next line stuck to it. Two texts are longer than 119 characters.

namespace {
class QuietTwingo : public RenaultTwingoGen1Battery {
 public:
  QuietTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.sleep_failsafe_minutes = 10;
    t.daily_sleep_enabled = false;
    t.bus_state_manual = false;
    t.bus_format_zoe_old = false;
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

CAN_frame bms_frame() {
  CAN_frame f = {};
  f.ID = 0x424;
  f.DLC = 8;
  f.ext_ID = false;
  f.data.u8[0] = 0x11;
  f.data.u8[1] = 0x40;
  f.data.u8[2] = 0x56;
  f.data.u8[3] = 0x90;
  f.data.u8[4] = 0x39;
  f.data.u8[5] = 0x64;
  f.data.u8[6] = 0x55;
  f.data.u8[7] = 0x39;
  return f;
}

// Runs a Sleep run: the BMS sends frames until `bms_until_ms` after the start, then nothing; wake-up after `total_ms`.
std::string run_and_page(uint64_t bms_until_ms, uint64_t total_ms) {
  QuietTwingo b;
  b.setup();
  b.request_sleep();
  const uint64_t t0 = 1000;
  uint64_t t = t0;
  for (; t < t0 + total_ms; t += 100) {
    set_millis64(t);
    if (t < t0 + bms_until_ms) {
      b.handle_incoming_can_frame(bms_frame());
    }
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  b.request_wake_up();
  for (int i = 0; i < 400; i++, t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  return std::string(b.get_uds_info_html().c_str());
}
}  // namespace

TEST(TwingoBau22, ResultTextBeforeTheQuietPhaseIsCompleteAndEndsWithBreak) {
  // the BMS stops 60 s into the shutdown sequence (which lasts more than two minutes): "BEFORE the quiet phase began"
  const std::string page = run_and_page(60000, 400000);
  const std::string tail = "(that alone does not prove the pack slept).<br>";
  const size_t at = page.find("Result: BMS's last frame was ");
  ASSERT_NE(at, std::string::npos) << page;
  const size_t end = page.find(tail, at);
  ASSERT_NE(end, std::string::npos) << "the text is cut off: " << page.substr(at, 300);
  // the line is long (more than the old 119 characters) and the next line starts after the break
  EXPECT_GT(end + tail.size() - at, 120u);
  EXPECT_EQ(page.substr(end, tail.size()), tail);
}

TEST(TwingoBau22, ResultTextWentSilentAfterIsCompleteAndEndsWithBreak) {
  // the BMS keeps sending 90 s into the silence, then stops: "BMS went silent after ..."
  const std::string page = run_and_page(250000, 400000);
  const std::string tail = "(that alone does not prove the pack slept).<br>";
  const size_t at = page.find("Result: BMS went silent after ");
  ASSERT_NE(at, std::string::npos) << page;
  const size_t end = page.find(tail, at);
  ASSERT_NE(end, std::string::npos) << "the text is cut off: " << page.substr(at, 300);
  EXPECT_GT(end + tail.size() - at, 120u);
}

// Bau 23 (15.10.): the box refreshes itself once per second during a Sleep run and swapped the "Own send intervals" block
// (and the 0x155 line) for the placeholder "-" every time, so they blinked. The refresh script now keeps the text.
TEST(TwingoBau23, RefreshScriptKeepsTheTextOfTheSelfFilledBlocks) {
  QuietTwingo b;
  b.setup();
  b.request_sleep();
  uint64_t t = 1000;
  for (; t < 1000 + 5000; t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  const std::string page = b.get_uds_info_html().c_str();
  const size_t script = page.find("function tick()");
  ASSERT_NE(script, std::string::npos) << "the box refreshes itself during the run";
  const std::string js = page.substr(script, 1400);
  EXPECT_NE(js.find("'twingoTxStat'"), std::string::npos);
  EXPECT_NE(js.find("'twingoRx155'"), std::string::npos);
  // the text is read BEFORE the box is replaced and put back AFTER it, only where the fresh copy has the placeholder
  const size_t read = js.find("keep[i]=e.textContent");
  const size_t swap = js.find("cur.innerHTML=nb.innerHTML");
  const size_t back = js.find("e.textContent=keep[i]");
  ASSERT_NE(read, std::string::npos);
  ASSERT_NE(swap, std::string::npos);
  ASSERT_NE(back, std::string::npos);
  EXPECT_LT(read, swap);
  EXPECT_LT(swap, back);
  EXPECT_NE(js.find("e.textContent==='-'"), std::string::npos);
  // the blocks themselves are still in the page, with the ids the poll script uses
  EXPECT_NE(page.find("<pre id='twingoTxStat'>"), std::string::npos);
  EXPECT_NE(page.find("<span id='twingoRx155'>"), std::string::npos);
}

TEST(TwingoBau23, NoRefreshScriptWithoutARun) {
  QuietTwingo b;
  b.setup();
  uint64_t t = 1000;
  for (; t < 1000 + 2000; t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  const std::string page = b.get_uds_info_html().c_str();
  EXPECT_EQ(page.find("function tick()"), std::string::npos);
}
