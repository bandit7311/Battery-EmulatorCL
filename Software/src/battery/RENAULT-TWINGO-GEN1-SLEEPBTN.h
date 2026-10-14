#ifndef RENAULT_TWINGO_GEN1_SLEEPBTN_H
#define RENAULT_TWINGO_GEN1_SLEEPBTN_H

// Hardware push button "Sleep" of the Twingo emulator (Bau 12, GPIO15 of the T-2CAN): pure logic without Arduino or
// GPIO, so it can be tested on the host. The driver feeds it the raw level of the pin (true = pressed, the button
// connects the pin to GND against the internal pull-up) and starts the Sleep run when it returns true.

#include <cstdint>

namespace twingo_sleepbutton {

constexpr uint32_t DEBOUNCE_MS = 50;      // a level must stay this long to count
constexpr uint32_t LONG_PRESS_MS = 2000;  // the button must be held this long (a short touch does nothing)

struct LongPress {
  bool raw_last = false;
  uint32_t raw_change_ms = 0;
  bool stable = false;  // debounced level (true = pressed)
  uint32_t press_start_ms = 0;
  bool fired = false;  // this press has already delivered its event
  bool armed = false;  // a released level was seen: a pin that is shorted to GND at the start never fires

  // Returns true exactly once per press, at the moment the debounced press has lasted LONG_PRESS_MS.
  bool update(bool raw_pressed, uint32_t now_ms) {
    if (raw_pressed != raw_last) {
      raw_last = raw_pressed;
      raw_change_ms = now_ms;
    }
    if (now_ms - raw_change_ms >= DEBOUNCE_MS && stable != raw_last) {
      stable = raw_last;
      if (stable) {
        press_start_ms = raw_change_ms;
      } else {
        fired = false;
      }
    }
    if (!stable && now_ms - raw_change_ms >= DEBOUNCE_MS) {
      armed = true;
    }
    if (armed && stable && !fired && now_ms - press_start_ms >= LONG_PRESS_MS) {
      fired = true;
      return true;
    }
    return false;
  }

  void reset() { *this = LongPress(); }
};

}  // namespace twingo_sleepbutton

#endif  // RENAULT_TWINGO_GEN1_SLEEPBTN_H
