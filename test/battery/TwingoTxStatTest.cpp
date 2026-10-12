#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-TXSTAT.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// 13.10.: measurement of the own send intervals of 0x426, 0x435 and 0x436 (microseconds, shown on the page).

using twingo_txstat::Measure;

TEST(TwingoTxStat, IntervalsOfOneFrameMinMaxMeanAndOutside) {
  Measure m;
  uint32_t t = 5000;
  for (uint32_t dt : {100000u, 100000u, 99000u, 101000u, 98500u, 103000u}) {
    m.note(0x426, t);
    t += dt;
  }
  m.note(0x426, t);
  const auto& v = m.iv[0];
  EXPECT_EQ(v.count, 6u) << "seven frames are six intervals";
  EXPECT_EQ(v.min_us, 98500u);
  EXPECT_EQ(v.max_us, 103000u);
  EXPECT_EQ(v.mean_us(), (100000u + 100000u + 99000u + 101000u + 98500u + 103000u) / 6u);
  EXPECT_EQ(v.outside, 2u) << "98.5 ms and 103 ms are outside 100 ms +-1 ms, 99 and 101 ms are inside";
  EXPECT_EQ(m.iv[1].count, 0u);
  EXPECT_EQ(m.iv[2].count, 0u);
}

TEST(TwingoTxStat, AGapOfMoreThanOneSecondIsAPauseNotAnInterval) {
  Measure m;
  m.note(0x435, 0);
  m.note(0x435, 100000);
  m.note(0x435, 100000 + 5000000);
  m.note(0x435, 100000 + 5100000);
  const auto& v = m.iv[1];
  EXPECT_EQ(v.count, 2u);
  EXPECT_EQ(v.pauses, 1u);
  EXPECT_EQ(v.max_us, 100000u);
  EXPECT_EQ(v.outside, 0u);
}

TEST(TwingoTxStat, TheMicrosecondCounterMayWrap) {
  Measure m;
  m.note(0x436, 0xFFFFFFFFu - 40000u);
  m.note(0x436, 60000u);  // 100001 us later, over the wrap
  EXPECT_EQ(m.iv[2].count, 1u);
  EXPECT_EQ(m.iv[2].min_us, 100001u);
}

TEST(TwingoTxStat, OffsetOf435And436ToThe426SentBefore) {
  Measure m;
  uint32_t t = 0;
  for (int i = 0; i < 4; i++) {
    m.note(0x426, t);
    m.note(0x435, t + 200);
    m.note(0x436, t + 500 + (uint32_t)i * 100);
    t += 100000;
  }
  EXPECT_EQ(m.off[0].count, 4u);
  EXPECT_EQ(m.off[0].min_us, 200u);
  EXPECT_EQ(m.off[0].max_us, 200u);
  EXPECT_EQ(m.off[1].min_us, 500u);
  EXPECT_EQ(m.off[1].max_us, 800u);
  EXPECT_EQ(m.off[1].mean_us(), 650u);
}

TEST(TwingoTxStat, OtherIdsAreIgnoredAndResetClearsEverything) {
  Measure m;
  m.note(0x19F, 10);
  m.note(0x19F, 20);
  EXPECT_FALSE(m.have_426);
  EXPECT_EQ(m.iv[0].count + m.iv[1].count + m.iv[2].count, 0u);
  m.note(0x426, 0);
  m.note(0x426, 100000);
  ASSERT_EQ(m.iv[0].count, 1u);
  m.reset();
  EXPECT_EQ(m.iv[0].count, 0u);
  EXPECT_FALSE(m.iv[0].have_last);
  EXPECT_FALSE(m.have_426);
}

TEST(TwingoTxStat, TheTextHasOneLinePerFrameAndTheOffsets) {
  Measure m;
  m.note(0x426, 0);
  m.note(0x426, 100000);
  m.note(0x435, 100050);
  m.note(0x436, 100300);
  char text[640];
  m.format(text, sizeof(text));
  const std::string s = text;
  EXPECT_NE(s.find("0x426: n=1 min=100000 max=100000 mean=100000 us, outside 100 ms +-1 ms: 0, pauses: 0\n"),
            std::string::npos)
      << s;
  EXPECT_NE(s.find("0x435: n=0"), std::string::npos);
  EXPECT_NE(s.find("0x436: n=0"), std::string::npos);
  EXPECT_NE(s.find("0x435 after 0x426: n=1 min=50 max=50 mean=50 us\n"), std::string::npos) << s;
  // a small buffer is never overrun
  char small[40];
  memset(small, 'x', sizeof(small));
  m.format(small, sizeof(small));
  EXPECT_LT(strlen(small), sizeof(small));
}

namespace {
using Bat = RenaultTwingoGen1Battery;

class StatTwingo : public RenaultTwingoGen1Battery {
 public:
  StatTwingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.bus_format_zoe_old = false;
    t.bus_state_manual = true;
    t.bus_state = twingo_bus::BUS_WACH;
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
}  // namespace

TEST(TwingoTxStatDriver, TheDriverMeasuresItsOwn426And436AndTheRouteTextShowsIt) {
  StatTwingo b;
  b.setup();
  b.tx_measure_reset();
  uint64_t t = 1000;
  for (; t < 21000; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
  }
  char text[640];
  b.tx_measure_text(text, sizeof(text));
  const std::string s = text;
  // rows 4 (0x426) and 5 (0x436) are on in the default mask: about 190 intervals of 100 ms in 20 s
  EXPECT_EQ(s.rfind("0x426: n=", 0), 0u) << s;
  EXPECT_NE(s.rfind("0x426: n=0 ", 0), 0u) << "0x426 was measured: " << s;
  EXPECT_NE(s.find("min=100000 max=100000 mean=100000 us, outside 100 ms +-1 ms: 0"), std::string::npos) << s;
  b.tx_measure_reset();
  b.tx_measure_text(text, sizeof(text));
  EXPECT_EQ(std::string(text).rfind("0x426: n=0 ", 0), 0u);
}

// ---------------------------------------------------------------------------
// 0x155 of the battery on the page (byte 7 to byte 0, decimal)
// ---------------------------------------------------------------------------

TEST(TwingoRx155Display, ShowsTheLastFrameByte7ToByte0InDecimal) {
  StatTwingo b;
  b.setup();
  const std::string none = b.rx_155_text().c_str();
  EXPECT_NE(none.find("no frame yet"), std::string::npos) << none;
  CAN_frame f = {};
  f.DLC = 8;
  f.ID = 0x155;
  const uint8_t bytes[8] = {0x49, 0x87, 0xD0, 0x54, 0x0F, 0x5A, 0x02, 0xA7};
  memcpy(f.data.u8, bytes, 8);
  b.handle_incoming_can_frame(f);
  EXPECT_STREQ(b.rx_155_text().c_str(), "0x155 (B7..B0): 167 2 90 15 84 208 135 73");
  // a frame that the driver drops as invalid (current raw 0xFFF) is shown too
  f.data.u8[1] = 0x8F;
  f.data.u8[2] = 0xFF;
  b.handle_incoming_can_frame(f);
  EXPECT_STREQ(b.rx_155_text().c_str(), "0x155 (B7..B0): 167 2 90 15 84 255 143 73");
}

TEST(TwingoRx155Display, TheFieldStandsRightOfTheDtcDetailsButtonWithItsPoll) {
  StatTwingo b;
  b.setup();
  const std::string page = b.get_uds_info_html().c_str();
  const size_t btn = page.find("subfunction 0x06)</button>");
  ASSERT_NE(btn, std::string::npos);
  const size_t span = page.find("<span id='twingoRx155'>", btn);
  ASSERT_NE(span, std::string::npos);
  EXPECT_LT(span - btn, 60u) << "directly after the button";
  EXPECT_NE(page.find("fetch('/twingoRx155')"), std::string::npos);
}
