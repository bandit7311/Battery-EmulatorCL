#ifndef RENAULT_TWINGO_GEN1_LOGIC_H
#define RENAULT_TWINGO_GEN1_LOGIC_H

// Pure logic of the Twingo Gen1 emulator (08.10.): no Arduino, no CAN, no globals, so every function can be
// tested on the host. The driver (RENAULT-TWINGO-GEN1-BATTERY.cpp) only calls these.

#include <climits>
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

// ---------------------------------------------------------------------------------------------------------
// HV state model (point 15, 08.10.). 0x350 is the clock: the driver notes when it sends the stages C4 (connect),
// C5/C7 (0x57F bytes 3-4 switch), C7 (+2.0 s inverter on) and, in the shutdown sequence, C3 (disconnect). From these
// four times the contents of 0x57F, 0x599, 0x62D and 0x1FD follow (measured in the vehicle logs of 02.10. and
// 04.10., times are the averages, spread +-1 s). All times in ms, HV_NONE = has not happened.
constexpr int64_t HV_NONE = INT64_MIN;  // "has not happened" (times may be negative: "since long before the start")
struct HvTimes {
  int64_t connect = HV_NONE;  // C4 (or the first C7 after a wake-up)
  int64_t b34 = HV_NONE;      // C5 (or the first C7)
  int64_t c7 = HV_NONE;       // first C7
  int64_t disc = HV_NONE;     // C3 of the shutdown sequence
};
inline bool hv_is_connected(const HvTimes& t) {
  return t.connect != HV_NONE && (t.disc == HV_NONE || t.disc < t.connect);
}

struct HvOut {
  bool connected;     // between connect and disconnect
  bool relay_a0;      // 0x1FD byte 5: A0 (HV closed) instead of 50
  bool power_idle;    // 0x1FD byte 0: 45 (HV closed) instead of FE
  uint8_t phase_62d;  // 0x62D byte 3: 04 open, 06 transition, 02 closed
  uint8_t b5_62d;     // 0x62D byte 5: 80 / 40 inverter on / 00 after the disconnect
  bool b34;           // 0x57F bytes 3-4: CF A8 instead of 7F 80
  bool inverter_on;   // 0x599 byte 1: 08 instead of 04
  uint16_t volt_dV;   // 0x57F voltage in 0.1 V
};

constexpr int64_t HV_RELAY_MS = 400;         // 0x1FD byte 5 to A0
constexpr int64_t HV_PHASE_06_MS = 200;      // 0x62D byte 3 to 06
constexpr int64_t HV_PHASE_02_MS = 1200;     // 0x62D byte 3 to 02
constexpr int64_t HV_POWER_IDLE_MS = 900;    // 0x1FD byte 0 to 45
constexpr int64_t HV_RAMP_MS = 900;          // voltage 11.5 V -> pack voltage
constexpr int64_t HV_INVERTER_ON_MS = 2000;  // after C7, fixed (decision 08.10.)
constexpr int64_t HV_INVERTER_OFF_MS = 1000;
constexpr int64_t HV_B34_OFF_MS = 2000;
constexpr int64_t HV_PHASE_UP_MS = 1500;  // 0x62D to 06 after the disconnect
constexpr int64_t HV_OPEN_MS = 2200;      // HV opens: bits fall, voltage decays

// Voltage after the HV opened (x ms after the opening): 69 V at 0.55 s, 36 V at 1.5 s, 28 V at 2.5 s, then
// exponential with tau 22 s down to 0.5 V (log of 02.10.).
inline uint16_t hv_decay_dV(uint16_t pack_dV, int64_t x_ms) {
  if (x_ms < 0) {
    x_ms = 0;
  }
  double v;
  if (x_ms < 550) {
    v = pack_dV + (690.0 - pack_dV) * x_ms / 550.0;
  } else if (x_ms < 1500) {
    v = 690.0 + (360.0 - 690.0) * (x_ms - 550) / 950.0;
  } else if (x_ms < 2500) {
    v = 360.0 + (280.0 - 360.0) * (x_ms - 1500) / 1000.0;
  } else {
    v = 280.0 * std::exp(-(double)(x_ms - 2500) / 22000.0);
  }
  if (v < 5.0) {
    v = 5.0;
  }
  return (uint16_t)(v + 0.5);
}

inline HvOut hv_compute(const HvTimes& t, int64_t now, uint16_t pack_dV) {
  HvOut o = {};
  o.phase_62d = 0x04;
  o.b5_62d = 0x80;
  o.volt_dV = 5;  // 0.5 V, the first 0x57F frame after the wake-up
  if (t.connect == HV_NONE) {
    return o;
  }
  const bool disconnected = !hv_is_connected(t);
  const bool inv_before = t.c7 != HV_NONE && (disconnected ? t.disc : now) - t.c7 >= HV_INVERTER_ON_MS;
  if (!disconnected) {
    const int64_t dt = now - t.connect < 0 ? 0 : now - t.connect;
    o.connected = true;
    o.relay_a0 = dt >= HV_RELAY_MS;
    o.power_idle = dt >= HV_POWER_IDLE_MS;
    o.phase_62d = dt < HV_PHASE_06_MS ? 0x04 : (dt < HV_PHASE_02_MS ? 0x06 : 0x02);
    o.b34 = t.b34 != HV_NONE;
    o.inverter_on = inv_before;
    o.b5_62d = inv_before ? 0x40 : 0x80;
    if (dt >= HV_RAMP_MS || pack_dV <= 115) {
      o.volt_dV = pack_dV < 5 ? 5 : pack_dV;
    } else {
      o.volt_dV = (uint16_t)(115 + (int64_t)(pack_dV - 115) * dt / HV_RAMP_MS);
    }
    return o;
  }
  const int64_t d3 = now - t.disc < 0 ? 0 : now - t.disc;
  o.relay_a0 = d3 < HV_OPEN_MS;
  o.phase_62d = d3 < HV_PHASE_UP_MS ? 0x02 : (d3 < HV_OPEN_MS ? 0x06 : 0x04);
  o.b34 = t.b34 != HV_NONE && d3 < HV_B34_OFF_MS;
  o.inverter_on = inv_before && d3 < HV_INVERTER_OFF_MS;
  o.b5_62d = d3 < HV_INVERTER_OFF_MS ? (inv_before ? 0x40 : 0x80) : 0x00;
  o.volt_dV = d3 < HV_OPEN_MS ? pack_dV : hv_decay_dV(pack_dV, d3 - HV_OPEN_MS);
  if (o.volt_dV < 5) {
    o.volt_dV = 5;
  }
  return o;
}

// 0x57F (7 bytes): byte 0 = (2*A + 800) >> 3, byte 1 = low 3 bits of that sum in bits 7-5 + voltage bits 12-8, byte 2 =
// voltage bits 7-0 (13 bit, 0.1 V), bytes 3-4 7F 80 / CF A8, bytes 5-6 0. A positive = discharge. Check against the log:
// 64 0D 3E 7F 80 00 00 = 339.0 V, 0 A.
inline void frame_57f(double amps, uint16_t volt_dV, bool b34, uint8_t* d) {
  if (amps > 400.0) {
    amps = 400.0;
  }
  if (amps < -400.0) {
    amps = -400.0;
  }
  const uint16_t raw = (uint16_t)(amps * 2.0 + 800.0 + 0.5);  // 0 .. 1600, 11 bit
  const uint16_t v = volt_dV & 0x1FFF;
  d[0] = (uint8_t)(raw >> 3);
  d[1] = (uint8_t)(((raw & 7) << 5) | (v >> 8));
  d[2] = (uint8_t)(v & 0xFF);
  d[3] = b34 ? 0xCF : 0x7F;
  d[4] = b34 ? 0xA8 : 0x80;
  d[5] = 0;
  d[6] = 0;
}

// Pack current in A as 0x57F carries it: the emulator's current_dA is negative while discharging, 0x57F is positive
// then. Invalid values (more than 400 A) and an open HV give 0.
inline double hv_57f_amps(int32_t current_dA, bool hv_closed) {
  const double a = -(double)current_dA / 10.0;
  return (hv_closed && a >= -400.0 && a <= 400.0) ? a : 0.0;
}

// 0x5D7 (8 bytes): bytes 0-1 speed (0.01 km/h), bytes 2-5 odometer = (km * 100) << 4, byte 6 counter C0, C2 .. FE (step 2,
// 32 values), byte 7 0 (08 in the very first frame together with speed FF FF).
inline void frame_5d7(uint32_t km, uint8_t counter, bool first, uint8_t* d) {
  const uint32_t odo = (uint32_t)(((uint64_t)km * 100ULL) << 4);
  d[0] = first ? 0xFF : 0x00;
  d[1] = first ? 0xFF : 0x00;
  d[2] = (uint8_t)(odo >> 24);
  d[3] = (uint8_t)(odo >> 16);
  d[4] = (uint8_t)(odo >> 8);
  d[5] = (uint8_t)odo;
  d[6] = (uint8_t)(0xC0 + 2 * (counter & 0x1F));
  d[7] = first ? 0x08 : 0x00;
}

// 0x376 (Zoe Gen2 driver): the vehicle time as minutes in three base-255 digits (year, hour, minute), sent twice in
// bytes 0-2 and 3-5, bytes 6-7 = 0A 00. Here the minutes are the vehicle age (point 16, 09.10.), not the time since
// the Zoe production.
inline void frame_376(uint32_t minutes, uint8_t* d) {
  const uint8_t y = (uint8_t)((minutes / 65025UL) & 0xFF);
  const uint8_t h = (uint8_t)((minutes / 255UL) % 255UL);
  const uint8_t m = (uint8_t)(minutes % 255UL);
  d[0] = d[3] = y;
  d[1] = d[4] = h;
  d[2] = d[5] = m;
  d[6] = 0x0A;
  d[7] = 0x00;
}

}  // namespace twingo

#endif  // RENAULT_TWINGO_GEN1_LOGIC_H
