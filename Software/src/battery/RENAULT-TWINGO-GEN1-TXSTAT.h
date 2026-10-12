#ifndef RENAULT_TWINGO_GEN1_TXSTAT_H
#define RENAULT_TWINGO_GEN1_TXSTAT_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Measurement of the own send intervals of 0x426, 0x435 and 0x436 (13.10., order "bauen 1 bis 5"): the time between two
// calls of the send function in microseconds (min, max, mean, number outside 100 ms +-1 ms) and the offset of 0x435 and
// 0x436 to the 0x426 sent before. Pure logic, unit-tested; shown on the page.
//
// What is measured: the moment the driver hands a frame to the send function (micros()), not the moment the frame stands
// on the wire (the frame may wait in the send queue of the CAN driver). A gap of more than 1 s (silence, a pause of the
// row) is not an interval: it is counted as a pause and the next frame starts a new interval.
namespace twingo_txstat {

inline constexpr uint32_t NOMINAL_US = 100000;  // the three frames are sent every 100 ms
inline constexpr uint32_t TOLERANCE_US = 1000;  // +-1 ms
inline constexpr uint32_t PAUSE_US = 1000000;   // longer than this: a pause, not an interval
inline constexpr int FRAME_COUNT = 3;           // 0x426, 0x435, 0x436

struct Interval {
  bool have_last = false;
  uint32_t last_us = 0;
  uint32_t count = 0;    // intervals measured
  uint32_t outside = 0;  // intervals outside 100 ms +-1 ms
  uint32_t pauses = 0;   // gaps of more than 1 s
  uint32_t min_us = 0;
  uint32_t max_us = 0;
  uint64_t sum_us = 0;

  void note(uint32_t now_us) {
    if (have_last) {
      const uint32_t dt = now_us - last_us;  // unsigned: micros() may wrap
      if (dt > PAUSE_US) {
        pauses++;
      } else {
        if (count == 0 || dt < min_us) {
          min_us = dt;
        }
        if (count == 0 || dt > max_us) {
          max_us = dt;
        }
        sum_us += dt;
        count++;
        const uint32_t dev = dt > NOMINAL_US ? dt - NOMINAL_US : NOMINAL_US - dt;
        if (dev > TOLERANCE_US) {
          outside++;
        }
      }
    }
    have_last = true;
    last_us = now_us;
  }
  uint32_t mean_us() const { return count == 0 ? 0 : (uint32_t)(sum_us / count); }
};

// The offset of a frame to the 0x426 sent before it (0 = in the same moment, always >= 0).
struct Offset {
  uint32_t count = 0;
  uint32_t min_us = 0;
  uint32_t max_us = 0;
  uint64_t sum_us = 0;

  void note(uint32_t dt) {
    if (count == 0 || dt < min_us) {
      min_us = dt;
    }
    if (count == 0 || dt > max_us) {
      max_us = dt;
    }
    sum_us += dt;
    count++;
  }
  uint32_t mean_us() const { return count == 0 ? 0 : (uint32_t)(sum_us / count); }
};

struct Measure {
  Interval iv[FRAME_COUNT];  // 0 = 0x426, 1 = 0x435, 2 = 0x436
  Offset off[2];             // 0 = 0x435 after 0x426, 1 = 0x436 after 0x426
  bool have_426 = false;
  uint32_t last_426_us = 0;

  void reset() { *this = Measure(); }

  // Called for every frame that is sent; other IDs are ignored.
  void note(uint32_t id, uint32_t now_us) {
    int slot = -1;
    if (id == 0x426) {
      slot = 0;
    } else if (id == 0x435) {
      slot = 1;
    } else if (id == 0x436) {
      slot = 2;
    }
    if (slot < 0) {
      return;
    }
    iv[slot].note(now_us);
    if (slot == 0) {
      have_426 = true;
      last_426_us = now_us;
    } else if (have_426) {
      const uint32_t dt = now_us - last_426_us;
      if (dt <= PAUSE_US) {
        off[slot - 1].note(dt);
      }
    }
  }

  // Text for the page, one frame per line; out gets at most n bytes.
  void format(char* out, size_t n) const {
    static const char* const NAMES[FRAME_COUNT] = {"0x426", "0x435", "0x436"};
    size_t len = 0;
    out[0] = '\0';
    for (int i = 0; i < FRAME_COUNT && len < n; i++) {
      const Interval& v = iv[i];
      len += (size_t)snprintf(out + len, n - len,
                              "%s: n=%lu min=%lu max=%lu mean=%lu us, outside 100 ms +-1 ms: %lu, pauses: %lu\n",
                              NAMES[i], (unsigned long)v.count, (unsigned long)v.min_us, (unsigned long)v.max_us,
                              (unsigned long)v.mean_us(), (unsigned long)v.outside, (unsigned long)v.pauses);
      if (len >= n) {
        break;
      }
    }
    for (int i = 0; i < 2 && len < n; i++) {
      const Offset& o = off[i];
      len += (size_t)snprintf(out + len, n - len, "%s after 0x426: n=%lu min=%lu max=%lu mean=%lu us\n",
                              i == 0 ? "0x435" : "0x436", (unsigned long)o.count, (unsigned long)o.min_us,
                              (unsigned long)o.max_us, (unsigned long)o.mean_us());
    }
  }
};

}  // namespace twingo_txstat

#endif  // RENAULT_TWINGO_GEN1_TXSTAT_H
