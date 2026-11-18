# Build list draft: Zoe Ph2 "More Battery/Cell Info" time and distance block (18.11.2026)

**DRAFT. Nothing is built.** Build only after the owner says "bauen" / "freigeben". Dala style (.clang-format). Show the raw value in brackets behind every converted value. No auto-refresh on pages. Only Zoe Gen2 files are touched (`RENAULT-ZOE-GEN2-BATTERY.cpp/.h`, `webserver_zoe_gen2.cpp/.h`, tests).

Base: dala `v13.6.0` plus patch v7 (applies without conflict, checked 17.11.). Source of the idea: owner sketch (handwritten page) and explanation of 18.11.2026.

## Decided (owner)

### 1. No 0x376 switch on the Settings page
- Reason: saving there needs a reboot (`webserver.cpp:83`). The switch stays on the "More Battery/Cell Info" page, where it acts at once. Default off, stored as `ZOE376`.
- Consequence: no change to `settings_html.cpp` and `webserver.cpp`.

### 2. New order of the block on "More Battery/Cell Info"
Today (`RENAULT-ZOE-GEN2-BATTERY.cpp:499-539`): Mileage, Vehicle distance, LBC2 distance, Fan lines, Time, LBC2 time, Pack time.
New order, all in one group:
1. Time since first installation (new box, see 3)
2. Distance since first installation (new box, see 4)
3. LBC1 time `$9261`
4. LBC2 time `$9261`
5. Pack time `$91C1`
6. LBC1 distance `$925F`
7. LBC2 distance `$9262`
8. Pack distance `$91CF`

### 3. Time lines
- Each time line: minutes with raw in brackets, then `XX days XX month XX years` (**days first, years last**). 1 year = 365 days, 1 month = 30 days (approximation, no calendar date; leap years ignored).
- Example: Pack time 1923803 min = `0 days 8 month 3 years`; Time 2201920 min = `9 days 2 month 4 years`.
- LBC1 time and LBC2 time get in front of the days/month/years a **"full days total"**: `ceil(minutes / 1440)`.
- Note: the example in the owner text (3 years 1 month 1 day 1000 min = 397) does not match. 1 year 1 month 1 day 1000 min = 397; 3 years 1 month 1 day 1000 min = 1127.

### 4. Box "Time since first installation"
- Two fields: **days** (input with SET button in front) and **minutes** (0 to 1439).
- At every boot: minutes = 0, days + 1, days stored in NVM **before** 0x376 is sent.
- While running: the minutes counter counts with the vehicle clock; when it reaches 1440 days + 1, days stored in NVM, minutes restart at 0.
- Never backwards: at boot at most 1439 minutes are lost, and one full day is added, so the sent time always grows.
- Start value of 0x376 = `days * 1440 + minutes`.
- Limit: 24 bit, maximum 16 777 214 min (about 11 650 days), `0xFFFFFF` is invalid.
- Replaces today's start (`seed_376_time()`, pack `$9261` + 2 min, `.cpp:131-141`) and the RAM clock (`.cpp:1265`).

### 5. Box "Distance since first installation"
- Input/output field with SET. Takes the LBC1 distance (`$925F` / 160, unit mi), stored **separately** in NVM.
- +5 at every boot.
- Used as start distance for 0x376 (replaces `manual_distance_dkm`, key `ZOEDISTDKM`).
- Measured side effect: the pack adds the difference times 1.6 at sleep (+100 sent = +160 km in `$91CF`), so +5 per boot is about +8 km on the pack side per boot.

### 6. Commissioning order
1. Boot the battery with 0x376 off.
2. Read and check LBC1 time, LBC2 time, Pack time, LBC1 distance, LBC2 distance, Pack distance.
3. Enter the total days in the "Time since first installation" box, press SET.
4. Switch 0x376 on.

### 7. Units
- **LBC2 distance** is always **km** (the value that counts).
- LBC1 distance unit, decided by comparing the shown numbers, tolerance **1 %** (agreed):

| Comparison | LBC1 shows | LBC2 shows |
|---|---|---|
| LBC2 = 1.6 x LBC1 | **mi** (bold) | **km** (bold) |
| LBC2 = LBC1 (same number) | **km** (bold) | **km** (bold) |
| neither | no unit bold | km |

- Basis: both DIDs hold the same counter (Pack 3: raw 36016004 in both, LBC2 with offset `0x80000000`). LBC1 is shown in 1/160 (miles), LBC2 in 1/100 (km). The factor 1.6 is exact in the pack. No odometer value is needed (owner decision, taken as given).
- Remark: with today's scaling the bold "mi" shows for Pack 2 and Pack 3 every time. The "same number" case appears only for a pack whose LBC1 counter differs from the LBC2 counter. Bold is a plausibility sign, not a measurement of the car's unit.

## Proposed by Claude, not decided
- ~~Show the LBC2 raw with its offset~~ **decided yes (owner, 18.11.)**: `(raw 2183499652 = 0x80000000 + 36016004)`.
- SET of days below `ceil(LBC1 minutes / 1440)`: warn or reject (open).
- Name "Time since first installation": the 0x376 time counts minutes from 02.09.2021 11:32:44 UTC, not from the installation of the pack. Maybe add the reference date in the label (open).
- Tolerance for "LBC1 and LBC2 time nearly identical": Pack 3 showed LBC2 4 min behind LBC1 after a 240 s sleep, so zero tolerance would warn.
- Drift: every boot counts one day, 20 boots are 20 days ahead of real time. Does not break "never backwards".

## Not verified
- Compiles on the ESP32 (`-Werror`) and passes the tests on `v13.6.0`.
- Whether the Pack takes a time that is lower than its own (not tested).
- That the pack unit really is miles for what 0x376 carries (taken as given, no odometer).

## Related findings (separate, not part of this list)
- Zoe Gen2 and Bolt/Ampera lost `battery_allows_contactor_closing` with the UDS superclass commits (`240a3859e`, `25ca84749`). `v13.6.0` no longer needs it for FoxESS EP and SMA (commit `d9b42fa84`).
- Zoe Ph1: DTC erase uses `10 03` (inherited), read uses `10 C0`. See `MERKLISTE_Zoe_Ph1_DTC_17-11.md`.
