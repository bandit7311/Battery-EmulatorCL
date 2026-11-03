#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Golden run (Daily balance build, 03.11.2026): the complete stream of frames the driver sends in a Sleep run (the three
// ways long / short / Ladelog 2, the three run modes Sleep / Sleep 0x9281=1 / NVROL reset) is hashed (FNV-1a 64 bit over time,
// ID, extended flag, length and data of every frame). The expected values were taken on the unchanged state 90af77b. A change
// that must not touch the existing sleep runs (Daily balance switched off) has to leave every hash as it is.

namespace {
using Bat = RenaultTwingoGen1Battery;

class GoldenTwingo : public RenaultTwingoGen1Battery {
 public:
  GoldenTwingo() {
    Bat::age_manual_clear();
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = 1025301;
    t.age_pack_unix = 1791590400;
    t.age_last_sent = 0;
    t.age_source = 0;
    t.cell_source_broadcast = false;
    t.age_minute_factor = 1;
    t.bus_format_zoe_old = false;
    t.bus_state_manual = false;
    t.bus_target_restart = false;
    t.bus_wake_target = twingo_bus::TARGET_FAHREN;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.sleep_failsafe_minutes = 1;  // silence of one minute
    t.daily_sleep_enabled = false;
    t.sleep_button_enabled = false;
    t.simulator_enabled_mask = SIM_DEFAULT_MASK;
    t.simulator_enabled_mask_hi = 0;
    Bat::shutdown_like_car = false;
  }
  ~GoldenTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.bus_wake_target = twingo_bus::TARGET_FAHREN;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.bus_state_manual = false;
    t.bus_target_restart = false;
    t.simulator_enabled_mask = 0x10;
    t.sleep_failsafe_minutes = 30;
    Bat::shutdown_like_car = false;
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

uint64_t fnv(uint64_t h, uint8_t b) {
  return (h ^ b) * 1099511628211ULL;
}

uint64_t golden_hash(uint8_t path, uint8_t mode) {
  GoldenTwingo b;
  datalayer_extended.twingoGen1.bus_sleep_path = path;  // after the constructor, which resets it to the long way
  b.setup();
  uint64_t h = 1469598103934665603ULL;
  uint64_t now = 1000;
  uint32_t frames = 0;
  auto run = [&](uint64_t ms) {
    for (uint64_t end = now + ms; now < end; now += 10) {
      set_millis64(now);
      clear_transmitted_frames();
      b.transmit_can((unsigned long)now);
      for (const CAN_frame& f : get_transmitted_frames()) {
        frames++;
        for (int i = 0; i < 8; i++) {
          h = fnv(h, (uint8_t)(now >> (8 * i)));
        }
        for (int i = 0; i < 4; i++) {
          h = fnv(h, (uint8_t)(f.ID >> (8 * i)));
        }
        h = fnv(h, f.ext_ID ? 1 : 0);
        h = fnv(h, f.DLC);
        for (int i = 0; i < 8; i++) {
          h = fnv(h, f.data.u8[i]);
        }
      }
    }
  };
  run(30000);  // the wake-up sequence of driving has run
  if (mode == 0) {
    b.reset_NVROL();
  } else if (mode == 1) {
    b.request_sleep();
  } else {
    b.request_sleep_temporisation();
  }
  run(330000);  // shutdown walk, true silence of one minute, wake-up and the driving walk after it
  h = fnv(h, (uint8_t)(frames >> 24));
  h = fnv(h, (uint8_t)(frames >> 16));
  h = fnv(h, (uint8_t)(frames >> 8));
  h = fnv(h, (uint8_t)frames);
  std::printf("GOLDEN path=%u mode=%u frames=%u hash=0x%016llx\n", (unsigned)path, (unsigned)mode, (unsigned)frames,
              (unsigned long long)h);
  return h;
}
}  // namespace

// Not part of the normal run (the sleep runs leave static state of the driver behind that other tests depend on). Run it in its
// own process, so that the state is fresh and the order of the cases is fixed:
//   ./tests --gtest_also_run_disabled_tests --gtest_filter='TwingoGoldenRun.*'
TEST(TwingoGoldenRun, DISABLED_SleepRunsAllWaysAndModes) {
  struct Case {
    uint8_t path;
    uint8_t mode;
    uint64_t expected;
  };
  const Case cases[] = {
      {twingo_bus::SLEEP_LANG, 0, 0x308ea5bd417590c3ULL},     {twingo_bus::SLEEP_LANG, 1, 0xfc4204822c037743ULL},     {twingo_bus::SLEEP_LANG, 2, 0x3c3160e00ab9acb4ULL},
      {twingo_bus::SLEEP_KURZ, 0, 0xd5ab2323f78133ecULL},     {twingo_bus::SLEEP_KURZ, 1, 0x3f652fa553949cd3ULL},     {twingo_bus::SLEEP_KURZ, 2, 0xbc80c781ca8f6c53ULL},
      {twingo_bus::SLEEP_LADELOG2, 0, 0xf70375ecf5f06facULL}, {twingo_bus::SLEEP_LADELOG2, 1, 0x6eebad231235addaULL}, {twingo_bus::SLEEP_LADELOG2, 2, 0xe12a5fd692c1440fULL},
  };
  for (const Case& c : cases) {
    EXPECT_EQ(golden_hash(c.path, c.mode), c.expected) << "path " << (int)c.path << " mode " << (int)c.mode;
  }
}
