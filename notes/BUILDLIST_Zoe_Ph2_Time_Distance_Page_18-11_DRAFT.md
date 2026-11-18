# Build list draft: Zoe Ph2 "More Battery/Cell Info" time and distance block (18.11.2026)

**DRAFT. Nothing is built.** Build only after the owner says "bauen" / "freigeben". Dala style (.clang-format). Show the raw value in brackets behind every converted value. No auto-refresh on pages. Only Zoe Gen2 files are touched (`RENAULT-ZOE-GEN2-BATTERY.cpp/.h`, `webserver_zoe_gen2.cpp/.h`, tests).

Base: dala `v13.6.0` plus patch v7 (applies without conflict, checked 17.11.). Source: owner sketch (handwritten page), explanations of 18.11.2026, and the mock-up (artifact "Zoe Ph2 Zeitblock", PNG sent 18.11.).

## Decided (owner)

### 1. No 0x376 switch on the Settings page
- Reason: saving there needs a reboot (`webserver.cpp:83`). The switch stays on the "More Battery/Cell Info" page, where it acts at once. Default off, stored as `ZOE376`.
- No change to `settings_html.cpp` and `webserver.cpp`.

### 2. New order of the block on "More Battery/Cell Info"
Today (`RENAULT-ZOE-GEN2-BATTERY.cpp:499-539`): Mileage, Vehicle distance, LBC2 distance, Fan lines, Time, LBC2 time, Pack time.
New order, one group:
1. Time since first installation (box, see 4)
2. Distance since first installation (box, see 5)
3. LBC1 time `$9261`
4. LBC2 time `$9261`
5. Pack time `$91C1`
6. LBC1 distance `$925F`
7. LBC2 distance `$9262`
8. Pack distance `$91CF`

### 3. Look of the block
- **All texts left aligned**, including the check mark or cross. The symbol sits in a fixed column in front of each line, so the texts start at the same place.
- Heading above the six lines: **`✓ valid / ✗ invalid`** (no word "Valid?" any more, owner 18.11.). Green check = plausible, red cross = not plausible. A grey dash for "not read yet" (proposal).
- LBC2 raw shows its offset: `(raw 2183499652 = 0x80000000 + 36016004)` (owner: yes).

### 3a. Frame switches
- **Second hint line, bold, below the bracket hint of 0x376** (owner, 18.11.): **After NVROL Reset, this frame will be initially turned off again and has to be restarted!** (written "NVROL" in capitals). It belongs to item 10 (the switch is set back to off at an NVROL).
- The other frames (`0x4CE`, `0x4FB`, `0x133`, `0x440`), Sleep mode, Auto wake, Go sleep, Wake up and the TX/RX counters **stay where they are now, further down**.
- The `HEVC frame 0x376` switch (off/on) sits right below the six checked lines. The hint under it (owner text, "alreay" corrected): `(after setting valid days above (days are already rounded up by 1 day) and a valid distance --> turn on frame, not before! If you see an invalid mark on one position, consider NVROL, but you will lose all usage values!)`. The NVROL sentence is **inside the same brackets** (owner, 18.11.; "loose" corrected to "lose", capital I after the exclamation mark). Background (measured on Twingo packs, not on a Zoe pack): NVROL reset zeroes pack mileage `$91CF` and also the balancing counters, cycles and times (`BEFUND_Echter_Bus_LBC_EVC_09-10.md:265`). It cannot be undone. It replaces today's hint "Time and distance, see the two fields below. Stored.".
- The old input fields "Vehicle time (0x376)" and "Vehicle distance (0x376)" are replaced by the two new boxes.

### 4. Time lines
- Each time line: minutes with raw in brackets, then `full days total: N`, then `XX days XX month XX years` (days first, years last). 1 year = 365 days, 1 month = 30 days (approximation, leap years ignored, agreed).
- `full days total` (only for LBC1 time and LBC2 time) = `ceil(minutes / 1440)`. Pack time has no full days total.
- Example: Time 2201920 min = `1530` full days = `9 days 2 month 4 years`. Pack time 1923803 min = `0 days 8 month 3 years`.
- The owner example (3 years 1 month 1 day 1000 min = 397) does not match: 1 year 1 month 1 day 1000 min = 397; 3 years 1 month 1 day 1000 min = 1127.

### 5. Box "Time since first installation" (days + minutes)
- Fields: **days** (input with Set in front) and **minutes** (0 to 1439), then the hint `(see LBC1 time $9261 full days total, press Set)` (wording by Claude, open: "add 1"?).
- **At every boot** (stays, owner: "very important"): minutes = 0, days + 1, days stored in NVM **before** 0x376 is sent.
- While running: the minutes counter counts with the vehicle clock; at 1440 days + 1, days stored in NVM, minutes restart at 0.
- **Every Set click writes the typed value to the NVM at once.**
- Never backwards: at boot at most 1439 minutes are lost and a full day is added, so the sent time always grows.
- Start value of 0x376 = `days * 1440 + minutes`. Limit: 24 bit, maximum 16 777 214 min (about 11 650 days), `0xFFFFFF` invalid.
- Replaces today's start (`seed_376_time()`, pack `$9261` + 2 min, `.cpp:131-141`) and the RAM clock (`.cpp:1265`).

### 6. Box "Distance since first installation"
- Input field with Set in front, then the **unit of LBC1** (same unit as the LBC1 distance line), then the hint `(see LBC1 distance $925F add 5 press Set)` (owner wording).
- The user reads the LBC1 distance, adds 5, types it, presses Set. **Every Set click writes the value to the NVM at once.**
- **At every boot** (stays, owner: "very important"): +5 on the stored distance, saved in NVM.
- Used as start distance for 0x376 (replaces `manual_distance_dkm`, key `ZOEDISTDKM`).
- Measured side effect: the pack adds the difference times 1.6 at sleep (+100 sent = +160 km in `$91CF`), so +5 per boot is about +8 km on the pack side per boot.

### 7. Commissioning order
1. Boot the battery with 0x376 off.
2. Read and check LBC1 time, LBC2 time, Pack time, LBC1 distance, LBC2 distance, Pack distance (look at the ✓ and ✗).
3. Enter the total days in the time box and the distance in the distance box, press Set (each Set stores).
4. Switch 0x376 on.

### 8. Valid? rules (proposal, to be confirmed)

| Field | ✓ when | Pack 3 | Pack 2 |
|---|---|---|---|
| LBC1 time | not 0, not 0xFFFFFF (`is_time_plausible`, `.cpp:94`) | ✓ | ✓ |
| LBC2 time | as above and at most N min from LBC1 (N open, 10 min proposed) | ✓ | not read |
| Pack time | not 0, not 0xFFFFFF, **not larger than LBC1 time** | ✓ | ✗ (2628957 > 2200548) |
| LBC1 distance | not 0 and below 1 000 000 | ✓ | ✓ |
| LBC2 distance | as above and 1.6 x LBC1 or the same number (1 % tolerance) | ✓ | not read |
| Pack distance | valid range below 1 000 000 km (`.cpp:97-106`) | ✗ (5.7 Mio km) | ✗ (20.4 Mio km) |

### 9. Units
- **LBC2 distance** is always **km**.
- LBC1 distance unit by comparing the shown numbers, tolerance **1 %** (agreed): LBC2 = 1.6 x LBC1 gives **mi** emphasised; LBC2 = LBC1 gives **km** emphasised; neither gives no emphasis.
- The distance box takes the unit of LBC1.
- Basis: both DIDs hold the same counter (Pack 3: raw 36016004, LBC2 with offset `0x80000000`), LBC1 shown in 1/160 (miles), LBC2 in 1/100 (km). No odometer value needed (owner, taken as given).
- Remark: with today's scaling the "mi" case appears for Pack 2 and Pack 3 every time. Emphasis is a plausibility sign, not a measurement of the car's unit.
- Remark: every line on the real page is already bold, so a bold unit would not show. The mock-up underlines it. **Open: which emphasis.**

### 10. NVROL and the frame 0x376 switch (owner, 18.11.)
- When an NVROL reset is started, the frame **0x376 is switched off and the switch is set back to default off** (also stored in the NVM, key `ZOE376`), so that sensible days and distance can be entered again before the frame is turned on.
- Today: `reset_NVROL()` (`RENAULT-ZOE-GEN2-BATTERY.h:26`) only sets `UserRequestNVROLReset`. `transmit_can()` sends the NVROL frames (`.cpp:985-988`) and **keeps sending 0x376** if it is on. `frame_376_enabled` is not touched, so after the reset the old clock and distance would go straight into the freshly reset pack.
- Where: in `reset_NVROL()` set `frame_376_enabled = false` and call `save_settings()`. The radio box then shows off after the next page reload.
- Dala's own note in the driver (`.cpp:14-16`): after resetting the NVROL you must enable "temporisation before sleep" and then stop streaming 0x373. This is not part of this item.
- Open: do the stored days and the stored distance in the NVM stay after NVROL, or are they reset too? (NVROL zeroes the pack's own values, `BEFUND_Echter_Bus_LBC_EVC_09-10.md:265`.)

## Open (owner to decide)
1. Emphasis of the unit: bold (not visible on the real page), underline (as in the mock-up), colour or size.
2. N for LBC1/LBC2 time in the ✓ rule (Pack 3 showed 4 min after a 240 s sleep).
3. Time hint: with "add 1" or without.
4. SET of days below `ceil(LBC1 minutes / 1440)`: reject, warn or allow.
5. Label "Time since first installation": the 0x376 time counts minutes from 02.09.2021 11:32:44 UTC, not from the installation. Keep the name or add the reference date.
6. Sketch: example `1 day ... 6334 minutes` not readable (minutes box is 0 to 1439). Maybe 634.
7. Double count: the owner adds 5 by hand when setting, and the firmware adds 5 again at the next boot. Confirm that this is intended.
8. After NVROL: keep or reset the stored days and distance (see 10).

## Not verified
- Compiles on the ESP32 (`-Werror`) and passes the tests on `v13.6.0`.
- Whether the pack takes a time or distance that is lower than its own (never tested).
- That the pack unit really is miles for what 0x376 carries (taken as given).
- Drift: every boot counts one day, 20 boots are 20 days ahead of real time. Does not break "never backwards".

## Related findings (separate, not part of this list)
- Zoe Gen2 and Bolt/Ampera lost `battery_allows_contactor_closing` with the UDS superclass commits (`240a3859e`, `25ca84749`). `v13.6.0` no longer needs it for FoxESS EP and SMA (commit `d9b42fa84`).
- Zoe Ph1: DTC erase uses `10 03` (inherited), read uses `10 C0`. See `MERKLISTE_Zoe_Ph1_DTC_17-11.md`.
