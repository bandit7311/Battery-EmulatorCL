# Build list draft: Zoe Ph2 "More Battery/Cell Info" time and distance block (18.11.2026)

**BUILT 18.11.2026 as patch v8** (owner: "ja los bauen"). The draft below is what was built. Result: dala test suite on `v13.6.0` plus patch v8: 706 tests, 705 passed, 1 skipped (`OtaConfirmPlacement...`, was skipped before), clang-format clean, no warnings with `-Wall -Wextra` in the two Zoe files. **Not built for the ESP32** (`-Werror`) yet, the owner does that on the PC.

Build notes (what Claude decided without asking): new routes `/editZoeGen2Days` (1 to 11650) and `/editZoeGen2Distance` (1 to 1000000, unit of LBC1), the old `/editZoeGen2TimeMinutes` and `/editZoeGen2DistanceKm` are gone. NVM keys: `ZOEDAYS` (new), `ZOEDISTDKM` (kept, 0.1 of the LBC1 unit). The frame 0x376 is only sent when days and distance are set (before: from the pack). The distance from the pack (`$91CF`) is no longer used as a fallback, it is only shown. Not set means: no boot counting.

(Former header:) **DRAFT.** Build only after the owner says "bauen" / "freigeben". Dala style (.clang-format). Show the raw value in brackets behind every converted value. No auto-refresh on pages. Only Zoe Gen2 files are touched (`RENAULT-ZOE-GEN2-BATTERY.cpp/.h`, `webserver_zoe_gen2.cpp/.h`, tests).

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
- **Weight (owner, 18.11.): everything on these lines is normal weight, only the numbers and the units are bold.** In the six checked lines the main value and its unit are bold (`2201920 min`, `1530`, `9 days 2 month 4 years`, `225100.02 mi`, `360160.04 km`); labels, `full days total:`, the arrows and the `(raw ...)` part stay normal. The unit of the LBC1 distance is bold only when the rule of item 9 decides it. Headings, hints and box titles are normal weight, except the bold NVROL sentence under the 0x376 hint.
- LBC2 raw shows its offset: `(raw 2183499652 = 0x80000000 + 36016004)` (owner: yes).

### 3b. NVROL hint (owner, 18.11.)
- Directly **under the Pack distance line**, inside the box with the six checked lines, in brackets: `(If you see an invalid mark on one position, consider NVROL, but you will lose all usage values!)` ("loose" corrected to "lose"). Normal weight.

### 3a. Frame switches
- **Second hint line, bold, below the bracket hint of 0x376** (owner, 18.11.): **After NVROL Reset, this frame will be initially turned off again and has to be restarted!** (written "NVROL" in capitals). It belongs to item 10 (the switch is set back to off at an NVROL).
- The other frames (`0x4CE`, `0x4FB`, `0x133`, `0x440`), Sleep mode, Auto wake, Go sleep, Wake up and the TX/RX counters **stay where they are now, further down**.
- The `HEVC frame 0x376` switch (off/on) sits right below the six checked lines. The hint under it (owner text, "alreay" corrected): `(after setting valid days above (days are already rounded up by 1 day) and a valid distance --> turn on frame, not before!)`. The NVROL sentence is **not** part of this hint any more, it moved under the Pack distance line (see 3b). Background (measured on Twingo packs, not on a Zoe pack): NVROL reset zeroes pack mileage `$91CF` and also the balancing counters, cycles and times (`BEFUND_Echter_Bus_LBC_EVC_09-10.md:265`). It cannot be undone. It replaces today's hint "Time and distance, see the two fields below. Stored.".
- The old input fields "Vehicle time (0x376)" and "Vehicle distance (0x376)" are replaced by the two new boxes.

### 4. Time lines
- Each time line: minutes with raw in brackets, then `full days total: N`, then `XX days XX month XX years` (days first, years last). 1 year = 365 days, 1 month = 30 days (approximation, leap years ignored, agreed).
- `full days total` (only for LBC1 time and LBC2 time) = `ceil(minutes / 1440)`. Pack time has no full days total.
- Example: Time 2201920 min = `1530` full days = `9 days 2 month 4 years`. Pack time 1923803 min = `0 days 8 month 3 years`.
- The owner example (3 years 1 month 1 day 1000 min = 397) does not match: 1 year 1 month 1 day 1000 min = 397; 3 years 1 month 1 day 1000 min = 1127.

### 5. Box "Time since first installation" (days + minutes)
- Fields: **days** (input with Set in front) and **minutes** (0 to 1439), then the hint `(see LBC1 time $9261 full days total, press Set)` (wording by Claude; **without "add 1"**, owner 18.11.).
- **At every boot** (stays, owner: "very important"): minutes = 0, days + 1, days stored in NVM **before** 0x376 is sent.
- While running: the minutes counter counts with the vehicle clock; at 1440 days + 1, days stored in NVM, minutes restart at 0.
- **Every Set click writes the typed value to the NVM at once.**
- **Warning only (owner, 18.11.):** if the typed days are lower than `ceil(LBC1 minutes / 1440)`, the page shows a warning next to the field (proposal for the text: `Warning: days are lower than the pack time, the time may run backwards.`). The value is stored anyway, nothing is rejected.
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
| LBC2 time | as above and at most **60 min** from LBC1 (owner, 18.11.) | ✓ | not read |
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
- Emphasis of the unit = **bold** (owner, 18.11.). It is visible because the rest of the line is normal weight (see item 3).

### 10. NVROL and the frame 0x376 switch (owner, 18.11.)
- When an NVROL reset is started, the frame **0x376 is switched off and the switch is set back to default off** (also stored in the NVM, key `ZOE376`), so that sensible days and distance can be entered again before the frame is turned on.
- Today: `reset_NVROL()` (`RENAULT-ZOE-GEN2-BATTERY.h:26`) only sets `UserRequestNVROLReset`. `transmit_can()` sends the NVROL frames (`.cpp:985-988`) and **keeps sending 0x376** if it is on. `frame_376_enabled` is not touched, so after the reset the old clock and distance would go straight into the freshly reset pack.
- Where: in `reset_NVROL()` set `frame_376_enabled = false` and call `save_settings()`. The radio box then shows off after the next page reload.
- Dala's own note in the driver (`.cpp:14-16`): after resetting the NVROL you must enable "temporisation before sleep" and then stop streaming 0x373. This is not part of this item.
- **Stored days and stored distance stay in the NVM after NVROL** (owner, 18.11.). The frame is off then, so they have no effect until the owner turns it on again. (NVROL zeroes the pack's own values, `BEFUND_Echter_Bus_LBC_EVC_09-10.md:265`.)

## Patch v9, owner 18.11.2026, **BUILT 18.11.2026** (owner: "bauen v9"): dala test suite on `v13.6.0` plus v9: 710 tests, 709 passed, 1 skipped (as before), clang-format clean, no warnings with `-Wall -Wextra` in the two Zoe files, patch applies to a fresh `v13.6.0` and gives the same 11 files. **Not built for the ESP32** yet. Built as listed below: grey minutes field (`background:#c8c8c8;color:#000`, `readonly`), the NVROL sentence ending `checked and restarted again!`, and the new safety line (PID `0x9216` added after `$91C1` in the scan list, `battery_safety_cause`, `safety_cause_read`, line under the Pack distance without a check mark, raw bytes by the length of the answer). Four new tests.
- The **minutes field** in the box "Time since first installation" gets a **light grey background** (`background:#c8c8c8`) so that it is clear that it cannot be changed. It is already `readonly` (`RENAULT-ZOE-GEN2-BATTERY.cpp`, field `zoeMinutes`). One style attribute plus a small test. The mock-up shows it.
- Facts about the minutes (checked in the code): they start at 0 at every start of the emulator (`count_boot()`), at every Set of the days and after every 1440 minutes. As long as no days were set they show 0 and nothing counts. The page does not refresh itself, the field shows the value of the moment the page was loaded.
- The **bold NVROL sentence** under the 0x376 hint changes (owner, 18.11.) from `After NVROL Reset, this frame will be initially turned off again and has to be restarted!` to **`After NVROL Reset, this frame will be initially turned off again and has to be checked and restarted again!`** (read as: the ending is replaced; tell if it should be a separate line). In the code it is in `get_uds_info_html()` after the 0x376 hint, and in the test `The_hints_stand_where_the_owner_wanted_them`. Not built.

- **Safety mode line (Zoe Ph2 only, owner 18.11.):** a new line directly **under "Pack distance ($91CF)"** in the checked block: `Safety mode 1 root cause ($9216): <b>10</b> (raw 0A)`, normal weight, only the number bold. Read `22 92 16` (DID `$9216`, "Carrier to save SafetyMode1 root cause", `Ztx_sfty_apv`, readable and writable per the DDT file). **No check mark** (empty mark column), because what a value means is not known: both dumps (`DID_Liste_Original-Pack_Dump_24-07-2025.md:314`, `DID_Liste_MCPU_Dump_11-10.md:669`) show `0A` = 10. Before the first answer: `Not read yet`. Needs one new entry in the PID scan list (about 100 ms more per round) and a `case` in `handle_pid()`. Mock-up shows it. Reset routine for later, not part of this item: `31 01 B0 04 00` ("ResetSM1", after `10 03`).
- Open for the owner: what value counts as "safety mode active" can only be found by reading `$9216` on a pack that is surely in a safety mode and comparing.

## Decided 18.11.2026 (answers to the former open questions)
1. N for LBC1/LBC2 time in the check rule: **60 minutes**.
2. Time hint: **without "add 1"**.
3. Set of days below the pack value: **warn only**, do not reject.
4. Label **"Time since first installation" stays** as it is (no reference date added).
5. The unreadable number in the sketch (`1 day ... 6334 minutes`) is **ignored** (owner: slip of the pen).
6. Double count of +5: **fine**. The firmware adds at boot, and the frame is switched on without a boot, so the owner's manual +5 is not counted twice.
7. After NVROL: stored days and distance **stay** in the NVM, no effect while the frame is off.
8. Emphasis of the unit: bold (see item 3). Only numbers and units are bold.
9. The NVROL sentence moved from the 0x376 hint to under the Pack distance line, in brackets (see 3b).

## Open (owner to decide)
None at the moment. All questions are answered (18.11.2026). Details of wording may still change.

## Facts from the Renault DDT file of the Zoe Ph2 MCPU (18.11.2026, `QUELLE_DDT_MCPU_Zoe_Ph2_Auszug_18-11.md`, read, not tested on a device)
- `$925F` "Distance Totalizer (vehicle)" is **km, step 0.01** (32 bit signed). So the register holds **360160.04 km** for Pack 3 (raw 36016004); our "225100.02 mi" in the LBC1 line is **raw / 160**, a conversion (km / 1.6). The frame 0x376 value 225100 became exactly 360160 km (factor 1.6).
- `$91CF` "Battery mileage": km, step 1/32, offset 0x80000000 (formula confirmed). `$9261` = absolute time of vehicle **saved** (min), `$91C1` = pack time **since 1st power-up** (min).
- Open for the owner: the LBC1 distance line shows `mi` (a conversion). The register unit is km. Keep the `mi` line as decided, or show `km` with the raw value and the `mi` in brackets? Not changed.
- The Ph2 has a reset for Safety Mode 1: `31 01 B0 04 00` ("ResetSM1") after `10 03`; the reason is in `$9216`. Not part of this build list.

## Not verified
- Compiles on the ESP32 (`-Werror`) and passes the tests on `v13.6.0`.
- Whether the pack takes a time or distance that is lower than its own (never tested).
- That the pack unit really is miles for what 0x376 carries (taken as given).
- Drift: every boot counts one day, 20 boots are 20 days ahead of real time. Does not break "never backwards".

## Related findings (separate, not part of this list)
- Zoe Gen2 and Bolt/Ampera lost `battery_allows_contactor_closing` with the UDS superclass commits (`240a3859e`, `25ca84749`). `v13.6.0` no longer needs it for FoxESS EP and SMA (commit `d9b42fa84`).
- Zoe Ph1: DTC erase uses `10 03` (inherited), read uses `10 C0`. See `MERKLISTE_Zoe_Ph1_DTC_17-11.md`.

## v10 (18.11., auf Wunsch gebaut): Knopf "Erase Safety Mode 1"
- Unter der Zeile "Safety mode 1 root cause ($9216)" auf der Zoe-Ph2-Seite, nur Zoe-Dateien (Route `/zoeGen2ResetSafetyMode` in `webserver_zoe_gen2.cpp`).
- Ablauf (`handle_safety_reset`): warten bis keine UDS-Anfrage laeuft (max. 500 ms), `02 10 03`, nach 100 ms `05 31 01 B0 04 00` (aus der DDT "ResetSM1", **ungetestet**; NVROL sendet dagegen Laenge 4 ohne Option `00`), 1 s Antwort abwarten, dann laeuft das Polling weiter und liest `$9216` neu. Waehrend des Ablaufs ruht das Polling.
- Die Antwort des Packs steht hinter dem Knopf: `accepted (71)`, `refused (7F), code NN` (NRC) oder `no answer`, jeweils mit raw.
- Startet nicht waehrend Sleep oder NVROL-Reset. Bestaetigungsdialog vor dem Senden.
- Tests: 715 (714 bestanden, 1 uebersprungen), 5 neue (Frames und Reihenfolge, Polling ruht/laeuft weiter, Anzeige, Platz, Sleep). Patch: `zoe_komplett_v10.patch` (11 Dateien gegen dala v13.6.0). ESP32-Build nicht gelaufen.

## v11 (18.11., auf Wunsch gebaut): zweiter Versuch fuer Erase Safety Mode 1 und Warnung korrigiert
- **Pack 4 mit v10 (Messung, Raul 21:41):** `31 01 B0 04 00` wurde mit `03 7F 31 31` abgelehnt (NRC 0x31 = requestOutOfRange nach UDS-Norm). Sitzung und Adresse funktionieren, das Pack antwortet.
- **Zweiter Versuch:** bei genau diesem Code sendet der Treiber sofort `04 31 01 B0 04` (Laenge 4, ohne das letzte `00`, wie der NVROL-Befehl). Beide Antworten stehen hinter dem Knopf (`... -> second try (31 01 B0 04): answer: ...`). Bei einem anderen Code oder ohne Antwort kein zweiter Versuch.
- **Warnung "days are lower":** statt mit den aufgerundeten Tagen der LBC1-Zeit wird jetzt mit der Zeit verglichen, die wir gerade senden (`clock_days*1440 + Minute des Tages`). Vorher kam die Warnung im Normalbetrieb, sobald die Minuten ueber 0 liefen (Pack 4, 1550 Tage + 29 min). Text: "Warning: days are lower than the LBC1 time, the time may run backwards." Hinweis: der Vorschlag "abgerundet vergleichen" haette den Fall 1529 Tage getippt gegen 1529 Tage + 160 min im Pack nicht erkannt, deshalb dieser Vergleich.
- Tests: 718 (717 bestanden, 1 uebersprungen), 3 neue. Patch `zoe_komplett_v11.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v10 (nicht oben drauf legen). ESP32-Build nicht gelaufen.

## v12 (18.11., auf Wunsch gebaut): zwei Knoepfe "Erase Safety Mode v1" und "v2"
- **Pack 4 mit v11 (Messung, Raul 21:58):** beide Formen werden mit `03 7F 31 31` (NRC 0x31) abgelehnt, der automatische zweite Versuch brachte nichts.
- **v12:** der automatische zweite Versuch ist entfernt. Zwei Knoepfe nebeneinander unter der Zeile `$9216`, jeder sendet nur seine Form nach `10 03`: **v1** = `31 01 B0 04 00` (5 Bytes, wie die DDT), **v2** = `31 01 B0 04` (4 Bytes, wie NVROL). Route `/zoeGen2ResetSafetyMode?value=1|2`. Darunter eine graue Zeile mit beiden Befehlen und `last: vN answer: ...` (accepted 71 / refused 7F mit Code / no answer, mit raw).
- Tests: 719 (718 bestanden, 1 uebersprungen). Patch `zoe_komplett_v12.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v11. ESP32-Build nicht gelaufen.

## v13 (19.11., auf Wunsch gebaut, "bauen"): Software-Zeilen auf der Zoe-Seite
- **Neue Zeilen** auf `/advanced`, direkt **unter "Temporisation"**, ueber "Time since first installation":
  `Software ($F194 / $F195): RL00 / 0620`, `Calibration ($F182): 09 81 · Spare part ($F187): 293A09578R`, `Hardware ($F191): 293A01958R`. Text fett, wenn alle Bytes Buchstaben/Ziffern sind, dahinter `(raw ..)`; sonst nur Hex; "Not read yet" bis das Pack antwortet. Serial (`$F18C`) bewusst nicht (Pack-Kennung).
- **Lesen:** nur vom ersten Steuergeraet (LBC1), je **ein Register pro Runde** der Abfrageliste (Umweg nach `$9216` wie beim Strom), jedes hoechstens **3 Mal** (`IDENT_MAX_ATTEMPTS`), dann Schluss bis zum naechsten Start. Alle fuenf gelesen: danach nur noch der normale Strom-Umweg. Nicht beantwortete Register kosten damit nur wenige Anfragen.
- Nicht belegt: ob die Packs 4 und 5 diese DIDs beantworten (in den Dumps ja, `DID_Liste_MCPU_Dump_11-10.md:1850-1865`). Zweck: pruefen, ob die DDT "SW5XX V1.5" zur Software der Packs passt (Routine B004 mit Code 31 abgelehnt).
- Tests: 725 (724 bestanden, 1 uebersprungen), 6 neue. Patch `zoe_komplett_v13.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v12. ESP32-Build nicht gelaufen.

## v14 (19.11., auf Wunsch gebaut): dritter Knopf "Erase Safety Mode v3" mit Supplier-Sitzung
- **Anlass:** Pack 4 und Pack 5 (`$9216` = 10, BMS state 4) lehnen v1 und v2 mit `7F 31 31` (NRC 0x31) ab. Die DDT `RBMS_MCPU_SW5XX_ZL_V1.5` enthaelt unsere Software (Zeile 130 `Soft="543" Version="ZL00"`), B004 steht dort als `RoutineControl_Start_ResetSM1` ("SM1 flag reset for full UDS LGC"), und der einzige "SM1"-Bildschirm ("SM1-CANdev") sendet die Supplier-Sitzung `10 60`. Das ist ein Hinweis, kein Beweis. Ein Security Access (`27`) ist in der Datei nicht definiert.
- **v3:** `10 60` (Supplier), nach 100 ms `31 01 B0 04 00` (5 Bytes wie v1), 1 s Antwort abwarten, dann wieder **`10 03`** (damit das Pack nicht in der Supplier-Sitzung bleibt), 100 ms, danach laeuft die Abfrage weiter. Route `/zoeGen2ResetSafetyMode?value=3`.
- **Anzeige:** graue Zeile mit allen drei Befehlen, dahinter `last: v3 session 10 60: accepted (50) / refused (7F), code NN / no answer (raw ..) · routine answer: ...`. So sieht man, ob schon die Sitzung scheitert. Bei v1 und v2 unveraendert.
- Tests: 728 (727 bestanden, 1 uebersprungen), 3 neue plus Anpassungen. Patch `zoe_komplett_v14.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v13. clang-format sauber. ESP32-Build nicht gelaufen.

## v15 (19.11., auf Wunsch gebaut): dritte Sleep-Mode-Option "Sleep instantly"
- **Anlass:** Vergleich mit byggsmurfens/Dibblah1s Zoe-Gen1-Patch ("timed balancing sleep"), der den Bus hart und sofort stilllegt statt eines gestuften Vorlaufs. Testweise fuer Zoe Ph2 uebernommen, um zu pruefen, ob ein harter Stopp etwas an den Balancing-Zaehlern aendert (nicht belegt, nur ein Experiment).
- **Dritte Option in derselben Gruppe "Sleep mode"** (nicht als eigener Radio-Block): Sleep / Sleep 0x9281=1 / **Sleep instantly** (`sleep_mode` jetzt 0/1/2, Route `/editZoeGen2SleepMode?value=0|1|2`, in der NVM gespeichert).
- **Wirkung von Mode 2:** beim Druck auf "Go sleep" wird der 7,2-s-Vorlauf (gestufte Frame-Aenderungen wie bei Mode 0/1) uebersprungen; die Uebertragung stoppt **sofort beim naechsten Tick**, kein 0x9281-Schreiben. Technisch minimal: `handle_sleep` springt im Idle-Zustand direkt nach `WaitIdle` statt nach `LeadIn`; da jeder Zustand ueber `LeadIn` ohnehin zu vollstaendiger Stille fuehrt, reicht das aus.
- **Default bleibt "Sleep"** (Mode 0), nichts am bisherigen Verhalten geaendert.
- Tests: 731 (730 bestanden, 1 uebersprungen), 3 neue (kein Vorlauf/sofortige Stille, keine gestuften Frame-Aenderungen, Anzeige/Auswahl auf der Seite). Patch `zoe_komplett_v15.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v14 (**nicht** den vom Nutzer selbst ergaenzten "Write $9216"-Knopf, der wird separat gepflegt). ESP32-Build nicht gelaufen.

## v16 (19.11., auf Wunsch gebaut): Antwort auf das 0x9281-Schreiben anzeigen
- **Anlass:** Beim "Sleep 0x9281=1"-Test stand `$9281` nach dem Aufwachen wieder auf 0. Offen war, ob das Pack den Schreibvorgang ueberhaupt angenommen hatte (kein Rueckmeldewert wurde bisher je abgefangen) oder ob das Schreiben nie wirkte.
- **Jetzt:** Nach dem Senden von `2E 92 81 01` wartet der Treiber bis zu 400 ms (`SLEEP_WRITE_ANSWER_MS`, vorher 100 ms fest) und faengt die Antwort des Packs ab (wie beim Safety-Mode-Knopf). Zeile auf der Seite: `Sleep 0x9281=1 first sends 10 03 and 2E 92 81 01, answer: <b>accepted (6E)</b>/<b>refused (7F), code NN</b>/<b>no answer</b> (raw ..)`, davor "not sent yet" bis zum ersten Lauf.
- Gilt nur fuer Sleep-Modus 1; bei Modus 0/2 bleibt die Zeile "not sent yet", da kein Schreiben erfolgt.
- Tests: 734 (733 bestanden, 1 uebersprungen), 3 neue. Patch `zoe_komplett_v16.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v15 (**nicht** den vom Nutzer selbst gepflegten "Write $9216"-Knopf). ESP32-Build nicht gelaufen.

## v17 (19.11., auf Wunsch gebaut): Sleep 0x9281=1 -- Sitzung/Schreiben zuerst, dann der Vorlauf
- **Anlass:** Bisher lief bei Modus 1 erst der 7,2-s-Vorlauf, erst danach Sitzung + Schreiben von `$9281`. Keine technische Notwendigkeit dafuer, nur gewachsene Reihenfolge aus dem Original-Treiber. Ziel: die "Temporisation"-Zeile hat so schon vor der Stille eine Chance, den neuen Wert zu sehen (bei Neuladen der Seite), und man muss nicht 7+ Sekunden auf die Schreib-Antwort warten.
- **Neue Reihenfolge (nur Modus 1):** Go sleep -> Sitzung `10 03` -> Schreiben `2E 92 81 01` -> (Antwort bis 400 ms abwarten) -> **danach erst** der normale 7,2-s-Vorlauf -> Stille. Modus 0 und 2 unveraendert.
- Technisch: neues internes Flag `sleep_write_done_this_cycle` (nicht auf der Seite sichtbar), verhindert, dass der zweite Durchlauf durch `WaitIdle` (nach dem Vorlauf) das Schreiben wiederholt.
- Tests: 735 (734 bestanden, 1 uebersprungen), bestehende Zeit-Tests fuer Modus 1 an die neue Reihenfolge angepasst, 2 neue (Reihenfolge Sitzung/Schreiben, kein zweites Schreiben nach dem Vorlauf). Patch `zoe_komplett_v17.patch`, 11 Dateien gegen dala v13.6.0, ersetzt v16. ESP32-Build nicht gelaufen.

## Zum Bauen vorgemerkt (19.11., Beobachtung von v17, nichts gebaut, wartet auf "bauen")
- **Beobachtung:** Mit v17 kommt die Schreib-Antwort auf `2E 92 81 01` schnell (`accepted (6E)`), aber die Zeile "Temporisation" oben auf der Seite zeigt weiterhin den alten Wert (`0 Activated!`). Grund (Schluss, nicht geprueft): Diese Zeile haengt an der normalen Hintergrund-Abfrage, die durch ueber 90 Register zyklisch durchlaeuft; ein Durchlauf dauert vermutlich laenger als die 7,2 s des anschliessenden Vorlaufs, sodass `$9281` darin oft nicht erneut an die Reihe kommt, bevor die naechste Stille die Abfrage wieder pausiert.
- **Idee:** direkt nach dem Schreiben gezielt `22 92 81` zuruecklesen (eigene kleine Anfrage, unabhaengig von der langsamen Abfrageliste) und das Ergebnis in derselben Zeile wie die Schreib-Antwort anzeigen -- dann sieht man den tatsaechlichen Pack-Wert sofort, nicht erst (zufaellig) beim naechsten Durchlauf der langen Liste.

## Zum Bauen vorgemerkt (20.11., Fund aus echten ljames28-Logs zum Schlaf-/Wach-Verhalten, nichts gebaut, wartet auf "bauen")
- **Quelle:** `ljames28`-Repo-Checkout im Scratchpad (`Canbus Logs/charge_to_100.log`, identisch `open_flap_charge.log`), echter candump-Log mit einer 723,6-s-Stille auf dem Batteriebus (1704750229.967 bis 1704750953.561 s Unix-Zeit) -- ein echter Schlaf-Wach-Zyklus, keine Vermutung.
- **Fund 1, wichtig -- `0x4FB` bleibt bis zum letzten Frame auf `04` (Relais geschlossen):** In den letzten 10 s vor der Stille (gemessen, jede einzelne Aenderung geprueft) bleibt Byte 2 von `0x4FB` durchgaengig `04`. Es gibt **keinen** Wechsel auf `02` ("Relais offen") vor der Stille. Das widerspricht der aktuellen Code-Annahme (`RENAULT-ZOE-GEN2-BATTERY.cpp`, Kommentar "0x4FB reports the relay as open" waehrend `LeadIn`, B2 wird dort auf `02` erzwungen, wenn der Schalter `frame_4fb_enabled` an ist). **Zu aendern:** B2 wird waehrend des gesamten Vorlaufs weiter per `is_main_relay_closed()` gesetzt (wie im Normalbetrieb), nicht fest auf `02`.
- **Fund 2 -- `0x440` an den realen Ablauf anpassen:** Gemessen (jede Aenderung, letzte 10 s vor der Stille):
  - B1 wechselt `31` -> `33` ca. **6,2 s** vor der Stille (bei `31` laeuft B4 auf `1F`).
  - B4 wechselt `1F` -> `9F` rund **100 ms** nach dem B1-Wechsel auf `33` (passt zur Groessenordnung von `SLEEP_440_OPEN_MS`).
  - B1 erreicht `32` erst in den letzten rund **3 s** vor der Stille (nicht exakt bei den bisherigen 3100 ms, aber in der Naehe -- Wert aus dem Log neu pruefen statt des bisherigen Annahme-Werts).
  - B7 im "offenen" Zustand zeigt im Log `78`, nicht `74` wie im bisherigen Code-Kommentar vermutet -- Byte-Werte am Log nachpruefen, nicht raten.
- **Fund 3 -- Aufwach-Reihenfolge ist gestuft, nicht gleichzeitig:** Gemessen (erster Frame je ID nach Ende der Stille, relativ zum Ende der Stille):
  1. `0x373` -- +64 ms
  2. `0x0F5`, `0x0EE`, `0x133` -- +77 bis +84 ms
  3. `0x4FB` -- +100 ms
  4. `0x375`, `0x376`, `0x440`, `0x4CE` -- +107 bis +108 ms
  Gesamtspanne ca. 44 ms. Der aktuelle Code (`transmit_can()`, `wake_up_requested` setzt `sleep_state` sofort auf `Idle`) laesst stattdessen alle Frames im selben Tick gleichzeitig wieder anlaufen (0 ms Versatz). **Zu bauen:** eine kurze gestufte Wake-Sequenz (neuer `SleepState`-Zustand oder Verzoegerungen pro Frame-Gruppe), die diese vier Stufen und ihre ungefaehren Abstaende (64 / 80 / 100 / 108 ms ab Wake-Beginn) nachbildet, analog zum bereits vorhandenen gestuften Sleep-Vorlauf.
- **Wichtig fuer die Umsetzung:** vor dem Bauen den Log selbst noch einmal pruefen (Datei liegt im Scratchpad, nicht dauerhaft -- bei Bedarf erneut aus dem ljames28-Repo holen), exakte Byte-Werte Stelle fuer Stelle gegen den Code abgleichen, nicht aus dem Gedaechtnis uebernehmen.
