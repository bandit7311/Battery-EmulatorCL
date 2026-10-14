#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <vector>
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../Software/src/battery/RENAULT-TWINGO-GEN1-SLEEPBTN.h"
#include "../../Software/src/datalayer/datalayer.h"
#include "../../Software/src/datalayer/datalayer_extended.h"
#include "Arduino.h"

void clear_transmitted_frames();
const std::vector<CAN_frame>& get_transmitted_frames();

// Bau 12 (14.10.): push button "Sleep" (GPIO15), the shared failsafe time, the total time of the display, the line break
// of "Frames per 10 s" and the verdict "Result" of the silence page.

using twingo_sleepbutton::LongPress;

// ---------------------------------------------------------------------------
// Logic of the button
// ---------------------------------------------------------------------------

namespace {
// Runs the button with a level that is `pressed` from t0 for `ms`, in steps of 5 ms. Returns the number of events.
int hold(LongPress& lp, uint32_t& t, bool pressed, uint32_t ms) {
  int events = 0;
  for (uint32_t end = t + ms; t < end; t += 5) {
    events += lp.update(pressed, t) ? 1 : 0;
  }
  return events;
}
}  // namespace

TEST(TwingoSleepButtonLogic, ShortPressDoesNothing) {
  LongPress lp;
  uint32_t t = 1000;
  EXPECT_EQ(hold(lp, t, false, 200), 0);
  EXPECT_EQ(hold(lp, t, true, 1900), 0);  // just under 2 s
  EXPECT_EQ(hold(lp, t, false, 500), 0);
}

TEST(TwingoSleepButtonLogic, LongPressGivesOneEventAtTwoSecondsAndNotAgainWhileHeld) {
  LongPress lp;
  uint32_t t = 1000;
  hold(lp, t, false, 200);
  const uint32_t t_press = t;
  int events = 0;
  uint32_t t_event = 0;
  for (uint32_t end = t + 6000; t < end; t += 5) {
    if (lp.update(true, t)) {
      events++;
      t_event = t;
    }
  }
  EXPECT_EQ(events, 1);
  EXPECT_GE(t_event - t_press, 2000u);
  EXPECT_LE(t_event - t_press, 2100u);
}

TEST(TwingoSleepButtonLogic, AfterTheReleaseTheNextLongPressCountsAgain) {
  LongPress lp;
  uint32_t t = 1000;
  hold(lp, t, false, 200);
  EXPECT_EQ(hold(lp, t, true, 2500), 1);
  EXPECT_EQ(hold(lp, t, false, 300), 0);
  EXPECT_EQ(hold(lp, t, true, 2500), 1);
}

TEST(TwingoSleepButtonLogic, BouncingContactsAreIgnored) {
  LongPress lp;
  uint32_t t = 1000;
  hold(lp, t, false, 200);
  // 1.5 s of bouncing (20 ms on, 20 ms off) never reaches the debounce time, no event, even after a long time
  int events = 0;
  for (int i = 0; i < 75; i++) {
    events += hold(lp, t, true, 20);
    events += hold(lp, t, false, 20);
  }
  EXPECT_EQ(events, 0);
  // a bounce inside a press restarts nothing: bounce at the start, then held 2 s
  hold(lp, t, true, 15);
  hold(lp, t, false, 15);
  EXPECT_EQ(hold(lp, t, true, 2300), 1);
}

TEST(TwingoSleepButtonLogic, APinShortedToGroundFromTheStartNeverFires) {
  LongPress lp;
  uint32_t t = 1000;
  EXPECT_EQ(hold(lp, t, true, 10000), 0);
  EXPECT_EQ(hold(lp, t, false, 300), 0);
  EXPECT_EQ(hold(lp, t, true, 2500), 1);  // after one released level it works
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------

namespace {

using Bat = RenaultTwingoGen1Battery;

class ButtonTwingo : public RenaultTwingoGen1Battery {
 public:
  bool pressed = false;
  bool pin_ok = true;
  int init_calls = 0;
  ButtonTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.sleep_button_enabled = true;
    t.sleep_failsafe_minutes = 7;
    t.daily_sleep_enabled = false;
    t.daily_sleep_duration_min = 5;
    t.bus_sleep_path = twingo_bus::SLEEP_KURZ;  // the button must use the long way and give this back
    t.bus_state_manual = false;
    t.bus_format_zoe_old = false;
    t.simulator_enabled_mask = 0x10;
  }
  ~ButtonTwingo() {
    auto& t = datalayer_extended.twingoGen1;
    t.sleep_button_enabled = false;
    t.sleep_failsafe_minutes = 30;
    t.bus_sleep_path = twingo_bus::SLEEP_LANG;
    t.daily_sleep_enabled = false;
  }
  bool sleep_button_init() override {
    init_calls++;
    return pin_ok;
  }
  bool sleep_button_raw_pressed() override { return pressed; }
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

// 0 = no sleep run, 1 = shutdown sequence, 2 = silence (what the display shows)
int phase() {
  return datalayer.system.status.sleep_display_phase;
}

void step(RenaultTwingoGen1Battery& b, uint64_t& t, uint64_t ms, uint64_t dt = 10) {
  for (uint64_t end = t + ms; t < end; t += dt) {
    set_millis64(t);
    clear_transmitted_frames();
    b.transmit_can((unsigned long)t);
  }
}

void press_for(ButtonTwingo& b, uint64_t& t, uint64_t ms) {
  b.pressed = true;
  step(b, t, ms);
  b.pressed = false;
  step(b, t, 200);
}

}  // namespace

TEST(TwingoSleepButtonDriver, LongPressStartsSleep9281OnTheLongWayAndRestoresThePathAfterTheRun) {
  ButtonTwingo b;
  b.setup();
  uint64_t t = 1000;
  step(b, t, 500);
  EXPECT_EQ(phase(), 0);
  press_for(b, t, 2300);
  step(b, t, 5000);
  EXPECT_EQ(phase(), 1) << "the run started: the shutdown sequence";
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_LANG) << "the long way is used for the run";
  EXPECT_EQ(datalayer.system.status.sleep_display_total_s, 7u * 60u)
      << "the end time is the failsafe time of the field";
  // through the shutdown sequence into the silence, then the wake-up by the failsafe time of the field (7 min)
  step(b, t, 200000);
  EXPECT_EQ(phase(), 2) << "silent";
  EXPECT_EQ(datalayer.system.status.sleep_display_total_s, 7u * 60u) << "the display shows the same time";
  step(b, t, 7UL * 60UL * 1000UL + 30000);
  EXPECT_EQ(phase(), 0) << "woken by the failsafe time";
  EXPECT_EQ(datalayer_extended.twingoGen1.bus_sleep_path, twingo_bus::SLEEP_KURZ) << "the path of the page is back";
}

TEST(TwingoSleepButtonDriver, ShortPressAndCheckboxOffDoNothing) {
  ButtonTwingo b;
  b.setup();
  uint64_t t = 1000;
  step(b, t, 500);
  press_for(b, t, 1500);
  step(b, t, 3000);
  EXPECT_EQ(phase(), 0);
  datalayer_extended.twingoGen1.sleep_button_enabled = false;
  press_for(b, t, 3000);
  step(b, t, 3000);
  EXPECT_EQ(phase(), 0);
  EXPECT_EQ(b.init_calls, 1) << "the pin was set up once (checkbox was on for the first press)";
  // the press that began while the box was off does not count when the box is switched on during it
  b.pressed = true;
  step(b, t, 1000);
  datalayer_extended.twingoGen1.sleep_button_enabled = true;
  step(b, t, 3000);
  b.pressed = false;
  step(b, t, 5000);
  EXPECT_EQ(phase(), 0);
}

TEST(TwingoSleepButtonDriver, APressDuringTheRunAndInTheSilenceIsIgnored) {
  ButtonTwingo b;
  b.setup();
  uint64_t t = 1000;
  step(b, t, 500);
  press_for(b, t, 2300);
  step(b, t, 5000);
  ASSERT_EQ(phase(), 1);
  step(b, t, 20000);
  press_for(b, t, 2500);  // while the shutdown sequence runs: no second start, the run goes on
  EXPECT_EQ(phase(), 1);
  step(b, t, 200000);
  ASSERT_EQ(phase(), 2);
  const uint32_t before = datalayer.system.status.sleep_display_elapsed_s;
  press_for(b, t, 2500);  // in the silence: no waking, the counter of the silence goes on
  EXPECT_EQ(phase(), 2);
  EXPECT_GE(datalayer.system.status.sleep_display_elapsed_s, before + 2u);
}

TEST(TwingoSleepButtonDriver, AnUnusablePinIsTriedOnceAndNothingStarts) {
  ButtonTwingo b;
  b.pin_ok = false;
  b.setup();
  uint64_t t = 1000;
  step(b, t, 500);
  press_for(b, t, 3000);
  press_for(b, t, 3000);
  step(b, t, 5000);
  EXPECT_EQ(phase(), 0);
  EXPECT_EQ(b.init_calls, 1);
}

TEST(TwingoSleepButtonDriver, PageHasTheNewLabelAndTheCheckbox) {
  ButtonTwingo b;
  b.setup();
  const std::string page = b.get_uds_info_html().c_str();
  EXPECT_NE(page.find("Sleep failsafe manual / push button hardware (IO15) --> auto wake after"), std::string::npos);
  EXPECT_NE(page.find("id='twingoSleepButton' onclick='twingoSetSleepButton(this.checked)' checked>"),
            std::string::npos);
  EXPECT_NE(page.find("/editTwingoSleepButton?value="), std::string::npos);
  datalayer_extended.twingoGen1.sleep_button_enabled = false;
  const std::string page2 = b.get_uds_info_html().c_str();
  EXPECT_NE(page2.find("id='twingoSleepButton' onclick='twingoSetSleepButton(this.checked)' >"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Silence page: line of 10 s values, "Result"
// ---------------------------------------------------------------------------

namespace {
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
}  // namespace

TEST(TwingoBau12Page, TenSecondValuesBreakAfterTenValues) {
  ButtonTwingo b;
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 30;
  b.setup();
  b.request_sleep();
  uint64_t t = 1000;
  for (; t < 1000 + 600000; t += 100) {  // 10 min: the shutdown sequence (about 3 min) and 7 min of silence
    set_millis64(t);
    b.handle_incoming_can_frame(bms_frame());
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  const std::string page = b.get_uds_info_html().c_str();
  const size_t from = page.find("Frames per 10 s (from 01:00): ");
  ASSERT_NE(from, std::string::npos);
  size_t end = page.find("Frames by CAN ID", from);
  ASSERT_NE(end, std::string::npos);
  // 54 values (01:00 to 10:00): a break after every group of ten = five inside the line, one at its end. (The host String
  // prints a char as its number, so the commas show as 44 here - counted by the breaks only.)
  size_t breaks = 0;
  for (size_t p = from; p < end; p++) {
    if (page.compare(p, 4, "<br>") == 0) {
      breaks++;
    }
  }
  EXPECT_EQ(breaks, 6u) << page.substr(from, 700);
}

TEST(TwingoBau12Page, ResultIgnoresTheFramesOfTheWakeUpBurst) {
  ButtonTwingo b;
  datalayer_extended.twingoGen1.sleep_failsafe_minutes = 10;
  b.setup();
  b.request_sleep();
  uint64_t t = 1000;
  for (; t < 1000 + 100000; t += 100) {  // the BMS keeps sending during the first 100 s, then stops for good
    set_millis64(t);
    b.handle_incoming_can_frame(bms_frame());
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  for (; t < 1000 + 400000; t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  ASSERT_EQ(phase(), 2) << "silent (10 min failsafe not over yet)";
  b.request_wake_up();
  // the wake-up burst: now the BMS answers with frames again, as it does in the car
  for (int i = 0; i < 400; i++, t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);  // first the step of the state machine (the wake-up begins), then the answers
    b.handle_incoming_can_frame(bms_frame());
  }
  for (int i = 0; i < 20; i++, t += 100) {
    set_millis64(t);
    b.transmit_can((unsigned long)t);
  }
  const std::string page = b.get_uds_info_html().c_str();
  EXPECT_EQ(page.find("kept sending until the end"), std::string::npos)
      << "the burst after the wake-up is not 'sending until the end'";
  EXPECT_NE(page.find("Result: "), std::string::npos);
}
