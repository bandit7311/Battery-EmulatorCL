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
// raw 0xFFF and/or SOC raw 0xFFF8..0xFFFB (65528..65531 = 163.82 %, seen 06.10. and 09.10.; 918 frames of the car logs
// have a normal current and only the SOC marker). A full pack reports a little above 100 % (UDS 0x91BA up to 102 %),
// so the limit lies well above 40000 (= 100 %, 0.0025 % per bit) and far below the marker: 45000 = 112.5 %. Frames
// above 100 % were dropped completely before 24.10.2026 (current, power and SOC froze at a full pack).
inline constexpr uint16_t SOC_RAW_100_PERCENT = 40000;
inline constexpr uint16_t SOC_RAW_MAX_VALID = 45000;
inline bool frame_155_valid(uint16_t current_raw, uint16_t soc_raw) {
  return current_raw != 0x0FFF && soc_raw <= SOC_RAW_MAX_VALID;
}

// SOC of 0x155 in 0.01 % units (pptt) for the datalayer, capped at 100.00 %: the pack may report up to 102 %, the
// safety rule "full" (reported_soc / real_soc == 10000) and the inverter must never see more than 100 %.
inline uint16_t soc_pptt_capped(uint16_t soc_raw) {
  const float pptt = soc_raw * 0.25f;  // 0.0025 % per bit -> 0.01 % units
  return pptt >= 10000.0f ? (uint16_t)10000 : (uint16_t)pptt;
}

// ---------------------------------------------------------------------------------------------------------
// Charge power limit towards the inverter (Bau 28, 24.10.2026). The pack names two limits: 0x424 byte 2 x 500 W
// (follows "Max Generated Power" 0x900E, i.e. the regeneration of the car) and UDS 0x9018 "Max Charge Power" (raw x
// 10 W, the charging of the pack; 0 W at 8:16 on 23.10. while the broadcast still allowed 4 kW and the inverter
// charged with 3.7 A). The smaller one counts. 0x9018 older than 30 s (or never read) counts as 0 W: if the pack
// cannot be asked, nothing is charged.
inline constexpr unsigned long CHARGE_LIMIT_MAX_AGE_MS = 30000;
inline uint32_t charge_power_limit_W(uint32_t regen_W, bool ext_valid, uint32_t ext_raw, unsigned long age_ms) {
  if (!ext_valid || age_ms > CHARGE_LIMIT_MAX_AGE_MS) {
    return 0;
  }
  const uint32_t pack_W = ext_raw * 10U;
  return regen_W < pack_W ? regen_W : pack_W;
}

// SOC for the inverter: never above 100.00 %, but 99.99 % as long as the pack still allows charging. At 100.00 % the
// rule "battery full" of safety.cpp sets the charge power to 0; the display SOC of the pack reaches 100 % at about
// 97.9 % internal SOC, the pack itself still charges then.
inline uint16_t soc_pptt_for_inverter(uint16_t soc_pptt_capped_value, uint32_t charge_limit_W) {
  return (soc_pptt_capped_value >= 10000 && charge_limit_W > 0) ? (uint16_t)9999 : soc_pptt_capped_value;
}

// Battery current (UDS 0x900D, display only): raw * 0.025 - 1200, negated (see the driver). A pack never carries
// more than a few hundred amperes; the invalid raw value of 06.10. showed -2895.875 A.
inline bool battery_current_plausible(double amps) {
  return amps > -500.0 && amps < 500.0;
}

// ---------------------------------------------------------------------------------------------------------
// Vehicle age (0x350 bytes 1-3, 0x523, 0x376, 0x436), 24 bit minutes (point 17, 08.10.). The age never comes from the
// clock (the clock value of 2.97 million is far above what this bench pack holds). It counts +1 per minute from a
// reference (value + Unix time). Since 10.10. (decision of the user) there is no safety lead any more and the pack
// reading higher no longer raises the reference: exactly the pack time of THIS bench pack is sent.
constexpr uint32_t AGE_SAFETY_MIN = 0;       // was one day until 09.10.
constexpr bool AGE_RAISE_FROM_PACK = false;  // was true until 09.10. (age_note_pack_read)
constexpr uint32_t AGE_MAX_24 = 0xFFFFFF;
constexpr int64_t UNIX_PLAUSIBLE_MIN = 1700000000LL;  // 14.11.2023; anything below is "clock not set"
// Seed reference of the bench pack (10.10.2026): its pack time $91C1 was 912,981 min on 24.07.2026 (Zoe Gen2 run,
// before the NVROL reset); 78 days x 1,440 min later (10.10.2026 00:00:00 UTC) that is 1,025,301 min. Until 09.10. the
// seed was 1,311,344 (value read on 06.10.).
constexpr uint32_t AGE_SEED_VALUE = 1025301;
constexpr int64_t AGE_SEED_UNIX = 1791590400LL;

inline uint32_t age_clamp24(uint64_t v) {
  return v > AGE_MAX_24 ? AGE_MAX_24 : (uint32_t)v;
}

// Row switches of the simulator page (Bau 13, 14.10.): the table had 71 rows, 30 are left. The switches stored in the
// flash by the old firmware (keys TWINGOSIMMASK/HI/HI2/HI3, bit n = old row n) are carried over to the new numbering:
// the five Zoe-form rows (old 3-7: 0x19F, 0x426, 0x436, 0x423, 0x69F) become 0-4, the 25 rows of the bus (old 46-70) become
// 5-29. All other old rows are gone. `lo` = old bits 0-63, `hi` = old bits 64-127.
inline uint32_t migrate_old_row_mask(uint64_t lo, uint64_t hi) {
  auto old_bit = [&](uint8_t n) -> bool {
    return n < 64 ? ((lo >> n) & 1ULL) != 0 : ((hi >> (n - 64)) & 1ULL) != 0;
  };
  uint32_t out = 0;
  for (uint8_t i = 0; i < 5; i++) {
    if (old_bit((uint8_t)(3 + i))) {
      out |= 1UL << i;
    }
  }
  for (uint8_t j = 0; j < 25; j++) {
    if (old_bit((uint8_t)(46 + j))) {
      out |= 1UL << (5 + j);
    }
  }
  return out;
}

// Row switches at the start (Bau 17): which mask the driver starts with.
//  - `page_built` false (TWINGO_SIM_PAGE off): always the default of the parameter block, a mask stored in the flash by an
//    earlier version with the page is ignored (and not deleted);
//  - page built: the mask stored in the flash by the page (key TWINGOSIMB1) wins; else the one carried over from the old
//    71-row numbering; else the default of the parameter block.
inline uint32_t initial_row_mask(bool page_built, bool has_new_key, uint32_t new_key_mask, bool has_old_keys,
                                 uint32_t migrated_old_mask, uint32_t default_mask) {
  if (!page_built) {
    return default_mask;
  }
  if (has_new_key) {
    return new_key_mask;
  }
  if (has_old_keys) {
    return migrated_old_mask;
  }
  return default_mask;
}

// Vehicle ID of 0x69F (Bau 18): the 4 data bytes as typed on the Advanced page, "46 13 88 6F" or "4613886F" (spaces and
// upper/lower case do not matter). Exactly 8 hex digits; 00000000 and FFFFFFFF are refused (not a vehicle ID).
inline bool parse_vehicle_id(const char* text, uint8_t out[4]) {
  if (text == nullptr) {
    return false;
  }
  uint8_t digits[8];
  uint8_t n = 0;
  for (const char* p = text; *p != '\0'; p++) {
    const char c = *p;
    if (c == ' ') {
      continue;
    }
    int v;
    if (c >= '0' && c <= '9') {
      v = c - '0';
    } else if (c >= 'a' && c <= 'f') {
      v = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
      v = c - 'A' + 10;
    } else {
      return false;
    }
    if (n >= 8) {
      return false;
    }
    digits[n++] = (uint8_t)v;
  }
  if (n != 8) {
    return false;
  }
  uint8_t b[4];
  for (uint8_t i = 0; i < 4; i++) {
    b[i] = (uint8_t)((digits[2 * i] << 4) | digits[2 * i + 1]);
  }
  const bool all00 = b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 0;
  const bool allFF = b[0] == 0xFF && b[1] == 0xFF && b[2] == 0xFF && b[3] == 0xFF;
  if (all00 || allFF) {
    return false;
  }
  for (uint8_t i = 0; i < 4; i++) {
    out[i] = b[i];
  }
  return true;
}

// Minute factor (Bau 9): 1 = real time, N = the age counts +1 only every N minutes (0 is treated as 1).
inline uint32_t age_factor_sane(uint32_t factor) {
  return factor < 1 ? 1 : factor;
}

// Whole minutes between two Unix times (0 if `now` is not after `from`, so a clock step back never lowers the age),
// divided by the minute factor and rounded down: the age steps up once per `factor` minutes.
inline uint32_t age_elapsed_min(int64_t from_unix, int64_t now_unix, uint32_t factor = 1) {
  return now_unix > from_unix ? (uint32_t)(((now_unix - from_unix) / 60) / age_factor_sane(factor)) : 0;
}

// Reference value carried to `now` without the safety lead.
inline uint32_t age_plain(uint32_t ref_value, int64_t ref_unix, int64_t now_unix, uint32_t factor = 1) {
  return age_clamp24((uint64_t)ref_value + age_elapsed_min(ref_unix, now_unix, factor));
}

// What is sent in the automatic mode: reference carried to `now` plus the safety lead.
inline uint32_t age_auto(uint32_t ref_value, int64_t ref_unix, int64_t now_unix, uint32_t factor = 1) {
  return age_clamp24((uint64_t)age_plain(ref_value, ref_unix, now_unix, factor) + AGE_SAFETY_MIN);
}

// Daily sleep (Bau 9/10): fires in the minute of the day `time_min` (0..1439, local time) if it is enabled (checkbox)
// and the duration is at least 1 minute. `secs_of_day` is the local time of day in seconds.
inline bool daily_sleep_due(bool enabled, uint16_t time_min, uint16_t duration_min, uint32_t secs_of_day) {
  return enabled && duration_min > 0 && time_min < 1440 && (secs_of_day / 60) == time_min;
}

// ---------------------------------------------------------------------------------------------------------
// Daily balance (03.11.2026, replaces the daily sleep): at the configured time of day the pack sleeps in cycles so that it
// makes (and works off) its balancing plan. Cycle 0 is the test cycle (short, `test_min` minutes, 0 = no test): afterwards the
// plan in 0x912B is checked, no plan = the whole run is aborted. Cycles 1..N sleep `dur_min` minutes each.
inline constexpr uint8_t DB_CYCLES_MIN = 1;
inline constexpr uint8_t DB_CYCLES_MAX = 5;
inline constexpr uint16_t DB_DUR_MAX_MIN = 1440;
inline constexpr uint16_t DB_TEST_DEFAULT_MIN = 10;
inline constexpr uint16_t DB_CHECK_DEFAULT_S = 120;
inline constexpr uint16_t DB_CHECK_MIN_S = 10;
inline constexpr uint16_t DB_CHECK_MAX_S = 3600;
inline constexpr unsigned long DB_ZERO_WAIT_MS = 5000;     // after the current is set to zero, before the sleep starts
inline constexpr unsigned long DB_AWAKE_HOLD_MS = 10000;   // awake after a wake-up, before the next sleep
inline constexpr unsigned long DB_BMS_QUIET_MS = 30000;    // no frame of the BMS for this long = the BMS sleeps for sure
inline constexpr unsigned long DB_RELEASE_AFTER_DRIVE_MS = 10000;  // after the wake-up walk reached driving: release anyway
inline constexpr unsigned long DB_RELEASE_HARD_CAP_MS = 180000;    // after the wake-up: release in any case
inline constexpr uint8_t DB_PLAN_MIN_ANSWERS = 3;          // "no plan" needs at least this many answers after the wake-up

inline uint8_t db_cycles_sane(uint16_t v) {
  return v < DB_CYCLES_MIN ? DB_CYCLES_MIN : (v > DB_CYCLES_MAX ? DB_CYCLES_MAX : (uint8_t)v);
}
// Run mode of the sleep of the cycles: 1 = "Sleep" (nothing is written to the pack, default), 2 = "Sleep 0x9281=1".
inline uint8_t db_mode_sane(uint32_t v) {
  return v == 2 ? 2 : 1;
}
inline uint16_t db_check_sane(uint32_t v) {
  return v < DB_CHECK_MIN_S ? DB_CHECK_MIN_S : (v > DB_CHECK_MAX_S ? DB_CHECK_MAX_S : (uint16_t)v);
}

// The first cycle: 0 (the test) if a test time is set, else 1.
inline uint8_t db_first_cycle(uint16_t test_min) {
  return test_min > 0 ? 0 : 1;
}
// Minutes of the sleep in this cycle.
inline uint16_t db_cycle_min(uint8_t cycle, uint16_t dur_min, uint16_t test_min) {
  return cycle == 0 ? test_min : dur_min;
}
// After the wake-up of this cycle the plan is checked unless it was the last cycle.
inline bool db_check_plan_after(uint8_t cycle, uint8_t total) {
  return cycle < total;
}

// Why the run does or does not start.
enum DbStart : uint8_t {
  DBS_NOT_DUE = 0,   // switched off, no clock or not the minute: nothing happens, no message
  DBS_OK,
  DBS_BUSY,          // another sleep run is going
  DBS_NOT_DRIVE,     // the bus state is not driving
  DBS_NO_TYPED_TIME  // the typed time of 0x436 is not active
};
inline DbStart db_start_check(bool enabled, bool clock_ok, bool due, bool busy, bool in_drive, bool typed_time_active) {
  if (!enabled || !clock_ok || !due) {
    return DBS_NOT_DUE;
  }
  if (busy) {
    return DBS_BUSY;
  }
  if (!in_drive) {
    return DBS_NOT_DRIVE;
  }
  if (!typed_time_active) {
    return DBS_NO_TYPED_TIME;
  }
  return DBS_OK;
}

// Number of balancing bits set in the 96 bits of 0x912B (data bytes 21 to 32).
inline uint8_t plan_bits_count(const uint8_t* raw, uint8_t len) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < len; i++) {
    for (uint8_t b = raw[i]; b != 0; b >>= 1) {
      n += (uint8_t)(b & 1);
    }
  }
  return n;
}

enum PlanVerdict : uint8_t { PLAN_WAIT = 0, PLAN_YES, PLAN_NO };
// Verdict of the plan check after a wake-up: `answers` = answers of 0x912B since the wake-up, `bits` = bits of the latest one,
// `elapsed_ms` = time since the wake-up. At once "plan" with one answer that has at least one bit; "no plan" only after
// `check_ms` and at least DB_PLAN_MIN_ANSWERS answers.
inline PlanVerdict plan_verdict(uint16_t answers, uint8_t bits, unsigned long elapsed_ms, unsigned long check_ms) {
  if (answers >= 1 && bits >= 1) {
    return PLAN_YES;
  }
  if (answers >= DB_PLAN_MIN_ANSWERS && elapsed_ms >= check_ms) {
    return PLAN_NO;
  }
  return PLAN_WAIT;
}

// The time of 0x436 after the booking: the value that is sent now plus the minutes of the cycle, capped.
inline uint32_t db_booked_time(uint32_t current, uint16_t add_min, uint32_t max_value) {
  const uint64_t v = (uint64_t)current + add_min;
  return v > max_value ? max_value : (uint32_t)v;
}

// After the last wake-up the current limits towards the inverter stay at zero until the state is as before (the wake-up walk
// has reached driving) and the pack has answered (the priority readings are in), at the latest DB_RELEASE_AFTER_DRIVE_MS
// after reaching driving, in any case after DB_RELEASE_HARD_CAP_MS since the wake-up.
inline bool db_release_ready(bool state_reached, bool pack_answered, unsigned long since_state_ms,
                             unsigned long since_wake_ms) {
  if (since_wake_ms >= DB_RELEASE_HARD_CAP_MS) {
    return true;
  }
  if (!state_reached) {
    return false;
  }
  return pack_answered || since_state_ms >= DB_RELEASE_AFTER_DRIVE_MS;
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
