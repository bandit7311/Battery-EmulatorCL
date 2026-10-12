#ifndef SLEEP_DISPLAY_H
#define SLEEP_DISPLAY_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// The text for the OLED display while a battery goes to sleep (13.10., wish of the user): in the middle rows, where the
// events normally stand, row 4 shows what happens and row 5 the time. Pure formatting, unit-tested.
//
//   phase 1 (the shutdown sequence is running):   "Sleep requested"        and  "00:00:00 / 00:10:00"
//   phase 2 (the true silence, the pack sleeps):  "Sleep pls wait"         and  "00:04:34 / 00:10:00"
//
// The first time is the time that has passed since the start of the silence (0 in phase 1), the second one is the end time,
// the minutes set as "Sleep failsafe minutes". Both HH:MM:SS. The display has 21 characters per row, both rows are
// centred; every row is filled up to 21 characters with blanks so that nothing of an earlier text stays on the display.
namespace sleep_display {

inline constexpr uint8_t PHASE_NONE = 0;
inline constexpr uint8_t PHASE_REQUESTED = 1;
inline constexpr uint8_t PHASE_WAIT = 2;
inline constexpr int COLUMNS = 21;

// "HH:MM:SS" (at most 99:59:59) into out (9 bytes at least).
inline void format_hms(uint32_t seconds, char* out) {
  if (seconds > 99U * 3600U + 59U * 60U + 59U) {
    seconds = 99U * 3600U + 59U * 60U + 59U;
  }
  snprintf(out, 9, "%02u:%02u:%02u", (unsigned)(seconds / 3600U), (unsigned)((seconds / 60U) % 60U),
           (unsigned)(seconds % 60U));
}

// Writes text centred into row[22] (21 characters and the end of the string).
inline void centre(const char* text, char* row) {
  memset(row, ' ', COLUMNS);
  row[COLUMNS] = '\0';
  int n = (int)strlen(text);
  if (n > COLUMNS) {
    n = COLUMNS;
  }
  const int left = (COLUMNS - n) / 2;
  memcpy(row + left, text, (size_t)n);
}

// Row 4 and row 5 for the phase; false (and blank rows) when nothing is to be shown.
inline bool build_rows(uint8_t phase, uint32_t elapsed_s, uint32_t total_s, char row4[22], char row5[22]) {
  if (phase != PHASE_REQUESTED && phase != PHASE_WAIT) {
    centre("", row4);
    centre("", row5);
    return false;
  }
  centre(phase == PHASE_REQUESTED ? "Sleep requested" : "Sleep pls wait", row4);
  if (phase == PHASE_REQUESTED) {
    elapsed_s = 0;
  } else if (elapsed_s > total_s) {
    elapsed_s = total_s;
  }
  char a[9], b[9], line[24];
  format_hms(elapsed_s, a);
  format_hms(total_s, b);
  snprintf(line, sizeof(line), "%s / %s", a, b);
  centre(line, row5);
  return true;
}

}  // namespace sleep_display

#endif  // SLEEP_DISPLAY_H
