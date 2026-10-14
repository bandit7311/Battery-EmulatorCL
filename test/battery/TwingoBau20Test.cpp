#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 20 (14.10.): Cellwatch only with TWINGO_CELLWATCH. This file is built in both variants
// (cmake -DTWINGO_CELLWATCH=ON / OFF).

namespace {

class CwTwingo : public RenaultTwingoGen1Battery {
 public:
  CwTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.cellwatch_enabled = false;
    t.cellwatch_cell = 1;
    t.bus_state_manual = false;
  }
  ~CwTwingo() { datalayer_extended.twingoGen1.cellwatch_enabled = false; }
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

// Requests on the extended channel (0x18DADBF1) during ms.
int requests(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms) {
  int n = 0;
  for (uint64_t end = t + ms; t < end; t += 10) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
    for (const CAN_frame& f : get_transmitted_frames()) {
      if (f.ext_ID && f.ID == 0x18DADBF1) {
        n++;
      }
    }
  }
  return n;
}

}  // namespace

TEST(TwingoCellwatchSwitch, AdvancedPageRowAndButtonOnlyExistWithTheSwitch) {
  CwTwingo b;
  b.setup();
  const std::string page = b.get_uds_info_html().c_str();
#ifdef TWINGO_CELLWATCH
  EXPECT_NE(page.find("Cellwatch (fast single-cell poll)"), std::string::npos);
  EXPECT_NE(page.find("id='twingoCellwatchEnable'"), std::string::npos);
  EXPECT_NE(page.find("id='twingoCellwatchCell'"), std::string::npos);
  EXPECT_NE(page.find("window.open('/cellwatch'"), std::string::npos);
  EXPECT_NE(page.find("function twingoSetCellwatchEnable"), std::string::npos);
  EXPECT_NE(page.find("function twingoSetCellwatchCell"), std::string::npos);
#else
  EXPECT_EQ(page.find("Cellwatch"), std::string::npos);
  EXPECT_EQ(page.find("cellwatch"), std::string::npos);
  EXPECT_EQ(page.find("/cellwatch"), std::string::npos);
#endif
  // what stays in both variants
  EXPECT_NE(page.find("id='twingoVidEn'"), std::string::npos);
  EXPECT_NE(page.find("id='twingoQueryHex'"), std::string::npos);
}

TEST(TwingoCellwatchSwitch, TheNormalRoundRobinRunsOncePerSecondWithoutCellwatchActive) {
  CwTwingo b;
  b.setup();
  uint64_t t = 1000;
  const int n = requests(b, t, 5000);
  EXPECT_GE(n, 3);
  EXPECT_LE(n, 7) << "one request per second (EXT_POLL_INTERVAL_MS)";
}

TEST(TwingoCellwatchSwitch, ASwitchedOnCellwatchOnlyHasAnEffectWhenItIsBuilt) {
  CwTwingo b;
  b.setup();
  datalayer_extended.twingoGen1.cellwatch_enabled = true;  // a leftover flag, e.g. from a build with Cellwatch
  uint64_t t = 1000;
  const int n = requests(b, t, 2000);
#ifdef TWINGO_CELLWATCH
  EXPECT_GE(n, 20) << "the fast poll of one cell: a request every 50 ms";
#else
  EXPECT_LE(n, 4) << "without the switch the flag does nothing, the round-robin goes on";
#endif
}
