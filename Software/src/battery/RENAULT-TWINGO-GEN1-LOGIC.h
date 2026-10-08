#ifndef RENAULT_TWINGO_GEN1_LOGIC_H
#define RENAULT_TWINGO_GEN1_LOGIC_H

// Pure logic of the Twingo Gen1 emulator (08.10.): no Arduino, no CAN, no globals, so every function can be
// tested on the host. The driver (RENAULT-TWINGO-GEN1-BATTERY.cpp) only calls these.

#include <cmath>
#include <cstdint>

namespace twingo {

// ---------------------------------------------------------------------------------------------------------
// 0x155 (battery -> vehicle, 10 ms): current raw (12 bit, byte 1 low nibble + byte 2) and SOC raw (bytes 4/5).
// A frame carries the "invalid" markers right after a wake-up or while the vehicle frames are missing: current
// raw 0xFFF, SOC raw above 40000 (0.0025 % per bit, so 100 %). Seen 06.10.: SOC raw 0xFFF8 = 163.82 %.
inline bool frame_155_valid(uint16_t current_raw, uint16_t soc_raw) {
  return current_raw != 0x0FFF && soc_raw <= 40000;
}

// Battery current (UDS 0x900D, display only): raw * 0.025 - 1200, negated (see the driver). A pack never carries
// more than a few hundred amperes; the invalid raw value of 06.10. showed -2895.875 A.
inline bool battery_current_plausible(double amps) {
  return amps > -500.0 && amps < 500.0;
}

}  // namespace twingo

#endif  // RENAULT_TWINGO_GEN1_LOGIC_H
