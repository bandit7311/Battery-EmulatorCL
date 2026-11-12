# Build list and description: Zoe Ph2 additions for the original dala Battery-Emulator

Status: **nothing built. Build starts only when the owner says "freigeben".** One patch containing everything below.
Written 12.11.2026. Language: English on request. Labels: **measured** = read from code/logs/photos, **inferred** = derived, **assumption** = not confirmed.

## 0. Scope and rules
- Base: `dalathegreat/battery-emulator` `main` = `191de3594` (09.11.2026). Not `battery-emulatorcl` (production system stays on `7ab39c0`).
- Work happens in a clone of dala; delivery as a patch (`git apply`). No access to the owner's local folder.
- Style (measured, `.clang-format`, `CONTRIBUTING.md`): Google style, `IndentWidth 2`, `ColumnLimit 120`, pointer left (`CAN_frame* f`), two spaces before trailing `//` comments, every `if` with braces. Run `clang-format`; the project enforces it via `pre-commit`.
- `AI_POLICY.md` (measured): AI code is allowed, but the owner must understand and explain it; the PR text and answers to maintainers must be written by the owner; no autonomous agents. This list is internal, not PR text.
- Touched dala files: `RENAULT-ZOE-GEN2-BATTERY.h`, `RENAULT-ZOE-GEN2-BATTERY.cpp`, and **two lines** in `Software/src/devboard/webserver/webserver.cpp`. New files: one header and one source for the routes. Nothing else.
- Known conflict: `main` still sends the old `0x376` (base 255). PR `#3242` (`de81065f`) rewrites the same lines. Expect merge work later.

## 1. Decisions (all confirmed by the owner unless marked)
| # | Decision |
|---|---|
| 1 | UI placed directly under the `Temporisation` line, inside `get_uds_info_html()` of the Zoe `.cpp` (above the DTC table). No auto-refresh. |
| 2 | `0x4CE` and `0x4FB` are **individually** switchable (two radio boxes), default **off** at first boot, stored in flash. |
| 3 | `0x4CE` is sent as a fixed "driving" frame (B0 `7F`). `0x4FB` B2 = `04` when the main relay is closed, `02` when open; `00` is never sent. |
| 4 | Sleep: mode selection, seconds input (1 to 600, default 10), buttons Go sleep / Wake up. No protection, no confirm dialog. |
| 5 | "Sleep 0x9281=1" does **not** run the NVROL reset (no routine `B009`). It sends session `10 03` and writes `0x01` to `0x9281`. |
| 6 | Sleep keeps dala from raising a battery-missing fault for up to 600 s (counter keep-alive). |
| 7 | Sleep lead-in: 3 s with `0x4FB` B2 = `02` (relay "off") toward the pack only, then total silence. Both sleep modes. |
| 8 | `0x376` time and distance from the pack, plausibility checked, not sent when implausible. Manual override fields for time (minutes only) and distance (km). |
| 9 | Display shows full 32-bit `Time`, `Pack time`, `Mileage` (+ km). |
| 10 | Time check has **no** upper limit (build date is not a limit). Distance limit is 1,000,000 km. |
| 11 | `0x440` and `0x133` are **not** built (watch list). |

## 2. Work package 1 - frames `0x4CE` and `0x4FB`
Files: Zoe `.h` (frame objects, switches), Zoe `.cpp` (100 ms block).

Frame objects, same style as the existing ones (`CAN_frame ZOE_0F5 = {//Current ...}`):
```cpp
CAN_frame ZOE_4CE = {//HEVC charging type (fixed driving)
                     .FD = false, .ext_ID = false, .DLC = 8, .ID = 0x4CE,
                     .data = {0x7F, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xE0, 0x00}};
CAN_frame ZOE_4FB = {//Relay 4
                     .FD = false, .ext_ID = false, .DLC = 6, .ID = 0x4FB,
                     .data = {0x80, 0x00, 0x04, 0x00, 0x00, 0x00}};
```
(lay the initializers out the way the existing frames in the file are laid out; `clang-format` decides the final form).

| Frame | Rate | Bytes | CRC |
|---|---|---|---|
| `0x4CE` | 100 ms | B0 `7F`, B1-B5 `FF FF FF DF FF`, B6 `0xE0 | counter`, B7 CRC | CRC-8 poly `0x1D`, init 0, XOR `0x0A`, over B0-B6 |
| `0x4FB` (DLC 6) | 100 ms | B0 `0x80 | counter`, B1 `00`, B2 `04` or `02`, B3 `00`, B4 `00`, B5 CRC | XOR `0x82`, computed over 7 bytes (B0-B4 plus two zero bytes), result stored in B5 |

- Counter: 4-bit, 0 to 15, incremented per sent frame (new member next to `counter_10ms`, e.g. `counter_100ms`).
- Use the existing helper `calculate_crc_zoe(CAN_frame&, xor, length, crc_index)`. Expected calls (**must be verified by unit test against log frames**): `0x4CE`: `(ZOE_4CE, 0x0A, 8, 7)`; `0x4FB`: `(ZOE_4FB, 0x82, 8, 5)` so that B6/B7 (zero) count as the two padding bytes.
- `0x4FB` B2: `04` if `datalayer.system.status.contactors_engaged == 1`, else `02`. During the sleep lead-in B2 is forced to `02` (WP2).
- Each frame is sent only if its own switch is **on**.
- Evidence (measured, 11 logs of ljames28): `0x4CE` B0 `5F` 22,369x, `1F` 14,214x, `7F` 539x; `0x4FB` B0 = `0x80 + counter`, B2 always `04` (37,026 frames); CRC checks passed on the logs (`0x4CE` 7214/7214, `0x4FB` 7196/7196).
- **Not verified:** that the pack accepts `02`; that these frames remove the DTCs `U214381`, `U214361`, `P1B2081` (assumption from the Zoe table).

## 3. Work package 2 - sleep
Files: Zoe `.h/.cpp`, UI in `get_uds_info_html()`, routes (WP5).

State machine (members in the Zoe class): `IDLE` -> `LEAD_IN` (3000 ms) -> [`WRITE` only for mode "Sleep 0x9281=1"] -> `SILENT` -> `IDLE`.
- **Go sleep:** enters `LEAD_IN`. All normal frames keep running; `0x4FB` B2 is forced to `02` (if the `0x4FB` switch is on).
- **WRITE (mode 2 only):** send `02 10 03 AA AA AA AA AA`, 100 ms later `04 2E 92 81 01 AA AA AA` (the frame objects already exist in the file, `ZOE_NVROL_*` / `ZOE_UDS_18DADBF1`). No routine `B009`. Same pattern as dala's `transmit_reset_nvrol_frames`.
- **SILENT:** `transmit_can()` returns at the top: no `0x373`, `0x376`, `0x0EE`, `0x0F5`, `0x4CE`, `0x4FB`, `0x5F8`, `0x6BF`, and no UDS polling (`transmit_uds_can`). Ends after N seconds (1 to 600, default 10) or on **Wake up**.
- **Keep-alive:** while in `LEAD_IN`/`WRITE`/`SILENT`, once per second set `datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;` (e.g. in `update_values()`, which runs every second). Reason (measured): the safety layer decrements this counter every second (`safety.cpp:47-61`, `Software.cpp:583`); at 0 it raises `EVENT_CAN_BATTERY_MISSING` (level ERROR) about 61 s after the last pack frame, which sets `system_status = FAULT` (`events.cpp:855`); with contactor control on, the contactors open 10 s later and stay latched until power cycle (`comm_contactorcontrol.cpp:221-241`). `CAN_STILL_ALIVE` (60, `uint8_t`) cannot simply be raised to 600.
- **Wake up:** leaves any state immediately and resumes all frames.
- Status line on the page: `awake` / `lead-in` / `sleeping, wakes in N s`.
- Consequence (inferred, not tested on hardware): while silent, the inverter keeps the last known pack values with closed contactors; a real pack fault is invisible during that time. No generic stale-value check exists in dala (only in two BMW drivers).
- **Assumptions to confirm:** order `lead-in -> session/write -> silence`; "Go sleep" is ignored while `UserRequestNVROLReset` is running; before the first silent second the code waits until no UDS request is in flight (max 500 ms).
- After writing `0x01`, dala's page may show `Temporisation: 1 Disabled!` (dala reads `1` as "Disabled"; its own command text says "Enable"). Contradiction in dala, not solved.

## 4. Work package 3 - `0x376` time and distance
Files: Zoe `.h/.cpp` (`transmit_can_frame_376`, `handle_pid` for `POLL_TIME`, `POLL_PACK_TIME`, `POLL_MILEAGE`).
- Encoding (measured on the Zoe logs): B0-B2 = **24-bit binary minutes**, big endian, zero point **02.09.2021 11:32:44 UTC** (differences matched real time within +0.1 and +0.3 min). B3-B5 = distance as **24-bit binary in 0.1 km** (unit is dala's convention, **inferred**). B6 `0x4A`. B7 `0x60` for the first 10 s after the first send, then `0x54` (as in PR `#3242`). Example: 31,000 km = `04 BA F0`.
- Time source: first valid `$9261` value + 2 minutes (`RESUME_GAP_MINUTES = 2`), then counted with `millis()` (one minute per 60,000 ms). The counter runs only while the emulator runs (no clock in dala). A manual value replaces the base at once.
- Time plausibility (pack value): not `0xFFFFFF`, not `0`, not lower than the pack time `$91C1` (if read). **No upper limit.**
- Distance plausibility (pack value `$91CF`): raw must satisfy `0x80000000 <= raw < 0x81E84800` (= below 1,000,000 km at 1/32 km per count). Conversion: `km = (raw - 0x80000000) / 32`; for `0x376` use `(raw - 0x80000000) * 10 / 32` in 0.1 km units. Example: 31,000 km = raw `0x800F2300`.
- If time **or** distance is not available (implausible and no manual value): **`0x376` is not sent** and the page shows a message.
- Manual override: **time** in minutes (accepted 1 to 16,777,214), kept in RAM only (assumption: not stored, otherwise the stored value would reset the running time after every restart; the pack stores what it receives and dala reads it back at the next start). **Distance** in km (1 to 1,000,000), stored in flash (assumption); `0` removes the override. A manual value wins over the pack value.
- Unknown: whether and where the pack stores the distance it receives in `0x376` (`$91CF` or `$925F`). Mention in a code comment.
- Reference data (measured, photos): Pack 2 `Time` 2,200,322 = 08.11.2025 11:34 UTC (bytes `21 93 02`), `Pack time` 7,517 (low 16 bits only), `Mileage` raw `0xA6D575EF` (invalid, would be 20,360,111 km).

## 5. Work package 4 - display
- Members `battery_time`, `battery_pack_time`, `battery_mileage` become `uint32_t` and keep the full value (today `(uint16_t)(value)`).
- Lines: `Time: <minutes>`, `Pack time: <minutes>`, `Mileage: <km> km (raw <raw>)`.
- Under the manual fields: pack value plausible (green) / not plausible (orange) with the reason ("limit 1000000 km", "0x376 is not sent until a distance is set").

## 6. Work package 5 - web UI and routes
UI block (English labels, directly under `Temporisation`):
1. `HEVC frame 0x4CE:` ( ) off ( ) on, note `Fixed "driving" (7F). Stored.`
2. `HEVC frame 0x4FB:` ( ) off ( ) on, note `Follows the main relay: 04 closed, 02 open. Stored.`
3. `Sleep mode:` ( ) Sleep ( ) Sleep 0x9281=1
4. `Auto wake after: [ 10 ] s [Set]`, note `1 to 600 s. ...`
5. `[Go sleep] [Wake up]`, status line.
6. `Vehicle time (0x376): [ ] min [Set]` and note.
7. `Distance (0x376): [ ] km [Set]` and note.
Style: plain `<h4>`, `<label><input type='radio'>`, `<input type='number'>`, buttons from dala's page CSS (no new CSS). Mockup images were sent on 12.11.2026 (`zoe_ansicht_*.png`).

New files: `Software/src/devboard/webserver/webserver_zoe_gen2.h` and `.cpp` (pattern of `webserver_can_streaming.*`):
```cpp
void register_zoe_gen2_routes(AsyncWebServer& server);
```
Two lines in `webserver.cpp`: `#include "webserver_zoe_gen2.h"` and `register_zoe_gen2_routes(server);` next to `register_dump_can_route(server);` (around line 1133).

Routes: `def_route_with_auth(..., HTTP_GET, ...)` reading `value`, like the existing `/editBydAtto3AutoCalEnabled` (writes RAM and NVM, answers `OK`). Assumed names:
| Route | Value | Action |
|---|---|---|
| `/editZoeGen2Frame4CE` | 0/1 | switch + flash |
| `/editZoeGen2Frame4FB` | 0/1 | switch + flash |
| `/editZoeGen2SleepMode` | 0/1 | mode + flash |
| `/editZoeGen2SleepSeconds` | 1..600 | seconds + flash |
| `/zoeGen2GoSleep` | - | start sleep |
| `/zoeGen2WakeUp` | - | end sleep |
| `/editZoeGen2TimeMinutes` | 1..16777214 | RAM only |
| `/editZoeGen2DistanceKm` | 0..1000000 | flash |
Invalid values: HTTP 400, no change. The routes call static functions of `RenaultZoeGen2Battery` (state lives in the class, no change to dala's `datalayer*.h`). Assumption: the page reloads once after Set / Go sleep / Wake up (like dala's buttons with `reload_after`); no polling.

## 7. Work package 6 - flash storage
Precedent in dala (measured): the Atto3 routes use `Preferences prefs; prefs.begin("batterySettings", false); prefs.putBool("BYDAUTOCALEN", ...); prefs.end();`. Follow it: namespace `batterySettings`, own keys (max 15 characters), e.g. `ZOE4CE`, `ZOE4FB`, `ZOESLPMODE`, `ZOESLPSEC`, `ZOEKM01`. Defaults: off, off, 0, 10, 0. Load in the Zoe `setup()` (read-only open). Guard with `#ifndef UNIT_TEST` because the Zoe `.cpp` is compiled into the test binary (`test/CMakeLists.txt:159`).

## 8. Tests and checks
- gtest (new test file in `test/battery/`): CRC of `0x4CE` and `0x4FB` against sample log frames; `0x376` encoding (31,000 km -> `04 BA F0`, minute to byte conversion); plausibility functions (including raw `0xA6D575EF` -> rejected, `0x800F2300` -> 31,000 km); sleep state machine (3 s lead-in, silence, wake, keep-alive set every second); input range checks.
- Build `stark_330` and `lilygo_2CAN_330` with PlatformIO Core 6.2.0 (local setup notes in the German plan, section 7).
- Bench checks (not automated): CAN logger shows `0x4CE`/`0x4FB` with correct CRC at 100 ms; after enabling, read the DTC list (`U214381`, `U214361`, `P1B2081`); Go sleep: lead-in with `0x4FB` `02`, then silence on the bus, no `EVENT_CAN_BATTERY_MISSING`, contactors stay closed for the full time; Wake up resumes all frames; `0x376` not sent with an implausible pack value, sent after a manual value.
- Flash only to a test device. **Never to the production system.**

## 9. Out of scope (watch list)
`0x440` (HEVC relay/12 V; B7 unknown), `0x133` (vehicle speed; commented out in dala), NTP/RTC/GPS time, any Twingo change, the Twingo build list items ("Time in 0x436" persistence, label fixes).

## 10. Open points
1. Manual **time not stored**, manual **distance stored** (assumption, not confirmed by the owner).
2. Assumptions in WP2: sequence order, ignore Go sleep during NVROL, wait for idle UDS before silence.
3. Reload after Set (assumption). Route names are proposals.
4. Behaviour of the pack with `0x4FB` `02`, with `0x01` written to `0x9281`, and with a 600 s silence is **not tested**.
5. Distance unit 0.1 km in `0x376` (dala's convention) is not verified.
