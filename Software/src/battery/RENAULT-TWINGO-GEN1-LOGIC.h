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

// ---------------------------------------------------------------------------------------------------------
// Vehicle age (0x350 bytes 1-3, 0x523, 0x376), 24 bit minutes (point 17, 08.10.). The age never comes from the clock
// any more (the clock value of 2.97 million is 3.15 years above the 1,311,344 the bench pack holds). It counts
// +1 per minute from a reference (value + Unix time) and carries a safety lead of one day.
constexpr uint32_t AGE_SAFETY_MIN = 1440;  // one day, added once (not every minute)
constexpr uint32_t AGE_MAX_24 = 0xFFFFFF;
constexpr int64_t UNIX_PLAUSIBLE_MIN = 1700000000LL;  // 14.11.2023; anything below is "clock not set"
// Seed reference of the bench pack: 9261 = 91C1 = 1,311,344 min, first seen 06.10.2026 20:40:21 UTC.
constexpr uint32_t AGE_SEED_VALUE = 1311344;
constexpr int64_t AGE_SEED_UNIX = 1791319221LL;

inline uint32_t age_clamp24(uint64_t v) {
  return v > AGE_MAX_24 ? AGE_MAX_24 : (uint32_t)v;
}

// Whole minutes between two Unix times (0 if `now` is not after `from`, so a clock step back never lowers the age).
inline uint32_t age_elapsed_min(int64_t from_unix, int64_t now_unix) {
  return now_unix > from_unix ? (uint32_t)((now_unix - from_unix) / 60) : 0;
}

// Reference value carried to `now` without the safety lead.
inline uint32_t age_plain(uint32_t ref_value, int64_t ref_unix, int64_t now_unix) {
  return age_clamp24((uint64_t)ref_value + age_elapsed_min(ref_unix, now_unix));
}

// What is sent in the automatic mode: reference carried to `now` plus the safety lead.
inline uint32_t age_auto(uint32_t ref_value, int64_t ref_unix, int64_t now_unix) {
  return age_clamp24((uint64_t)age_plain(ref_value, ref_unix, now_unix) + AGE_SAFETY_MIN);
}

// Value read from the pack: 9261 (vehicle time) if it is not 0, otherwise 91C1 (pack time), 24 bit. 0 = nothing usable.
inline uint32_t age_pack_candidate(uint32_t raw_9261, uint8_t len_9261, uint32_t raw_91c1, uint8_t len_91c1) {
  const uint32_t a = len_9261 ? (raw_9261 & AGE_MAX_24) : 0;
  if (a != 0) {
    return a;
  }
  return len_91c1 ? (raw_91c1 & AGE_MAX_24) : 0;
}

// The reference is raised only if the pack holds MORE than the age we currently send. A pack that merely stores what
// we sent can therefore never push the age up (no feedback loop of +1 day per reading).
inline bool age_should_raise(uint32_t candidate, uint32_t currently_sent) {
  return candidate != 0 && candidate > currently_sent;
}

}  // namespace twingo

#endif  // RENAULT_TWINGO_GEN1_LOGIC_H
