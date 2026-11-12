# Plan Zoe Ph2 (dala-Original): Frames, Sleep, Zeit, Strecke - nichts gebaut, wartet auf "freigeben" (12.11.2026)

Kennzeichnung: **gemessen** = aus Log/Foto/Code gelesen, **Schluss** = abgeleitet, **Annahme** = nicht belegt, **entschieden** = Nutzer.
Ziel: Aenderungen am **Original von dala** (Stand `191de3594`, 09.11.2026), nicht an `battery-emulatorcl`. Das Produktivsystem bleibt auf `7ab39c0`.
Gebaut wird **alles zusammen in einem Patch**, erst wenn der Nutzer "freigeben" sagt. Arbeit in einem Klon von dala (kein Zugriff auf den lokalen Ordner des Nutzers), Lieferung als Patch.

## 1. Umfang (entschieden)
Alles direkt **unter der Zeile "Temporisation"** der Advanced-Seite (in `get_uds_info_html()` der Zoe-`.cpp`, also ueber der DTC-Tabelle).

| Teil | Inhalt |
|---|---|
| Frames | Radiobox fuer `0x4CE` (fest "Fahren", B0 `7F`) und `0x4FB` (`04` Relais an, `02` aus, B0 = `0x80` + Zaehler). Standard beim Erststart **aus**, danach im Flash gespeichert |
| Sleep | Auswahl "Sleep" / "Sleep 0x9281=1", Eingabe "auto wake after" in **Sekunden** (Standard **10 s**), Knopf Set, Knopf **Go sleep** (danach kein Frame mehr, auch keine UDS-Abfragen), Knopf **Wake up**. **Kein Schutz** gegen angezogene Schuetze, keine Rueckfrage angenommen |
| Zeit | `0x376` binaer, Startwert aus `$9261` plus 2 Minuten, danach mit `millis()` weiter (nur waehrend der Emulator laeuft) |
| Strecke | `0x376` binaer in 0,1 km, aus `$91CF` |
| Pruefung | Fallen Zeit oder Strecke durch, wird `0x376` **nicht gesendet** (Entscheidung a), die Seite zeigt eine Meldung |
| Set-Felder | Zeit und Strecke einstellbar, damit ein unplausibler Wert zurueckgebracht werden kann; eingetragener Wert gilt vor dem Pack-Wert |
| Anzeige | `Time`, `Pack time`, `Mileage` mit allen Bits, Strecke in km umgerechnet |
| Technik | Route in neuen Dateien (Variante 2: `...-ROUTES.h` und `.cpp`), zwei Zeilen in dalas `webserver.cpp` (`#include` und Aufruf, Muster `register_dump_can_route`, `webserver.cpp:1133`). **Kein Auto-Refresh** der Felder. Speichern mit `Preferences` in eigenem Namensraum, `#ifndef UNIT_TEST` |

### Noch nicht bestaetigt (Annahme: Vorschlaege des Assistenten gelten, wenn der Nutzer nichts anderes sagt)
1. Pruefgrenzen: Zeit nicht `0xFFFFFF`, nicht 0, nicht spaeter als Baudatum plus 1 Jahr, nicht unter der Pack-Zeit `$91C1` (falls gelesen); Strecke: Rohwert beginnt mit `0x80`, unter 1 Mio. km.
2. Eingetragene Zeit gilt nur im laufenden Betrieb und wird nicht im Flash gespeichert (sonst Ruecksprung nach jedem Neustart, wie bei "Time in 0x436"); die Strecke wird im Flash gespeichert.
3. Eingabe der Zeit als Datum mit Uhrzeit, daneben die Minuten.

## 2. Stand in dala (gemessen, `origin/main` = `191de3594`)
- `0x0F5` ist in `main` (Commit `ebb020c76`, 08.11.2026, "Add sending of 0x0F5 to reduce DTC amount"). `0x133` ist nur auskommentiert (`.cpp:454`), `0x4CE`, `0x440`, `0x4FB` fehlen.
- Die Zoe Gen2 hat nur noch **zwei** Dateien (`RENAULT-ZOE-GEN2-BATTERY.h/.cpp`). `...-HTML.h` gibt es nicht mehr. Die Seite entsteht aus `UdsBatteryHtmlRenderer::get_status_html()` = `get_uds_info_html()` der Zoe plus DTC-Abschnitt; danach kommen die Knoepfe der Befehlstabelle (`advanced_battery_html.cpp`). Unter die Knoepfe waere nur mit einer Aenderung an dalas Seitencode moeglich.
- `0x376` ist in `main` noch das **alte** Verfahren (Ziffern zur Basis 255, fester Start). Die bessere Fassung (binaer, Zeit aus `$9261` + 2 min, Strecke aus `$91CF`, volle 32 Bit in der Anzeige) steckt im PR-Zweig `#3242` (`de81065f`) und ist nicht in `main`. Der Patch aendert dieselben Zeilen: Konflikte moeglich.
- Hauptrelais bei dala: `datalayer.system.status.contactors_engaged` (`datalayer.h:444`): 0 Start, 1 angezogen, 2 von Battery-Emulator geoeffnet. Wird von anderen Dateien gelesen; keine Aenderung an dalas Dateien noetig.
- NVROL-Ablauf bei dala (`transmit_reset_nvrol_frames`): Session `10 03`, Routine `B009` (`04 31 01 B0 09 00`), 1 s, Session `10 03`, `04 2E 92 81 01`, dann 30 s mit `0x373` B0 `01`. Fuer "Sleep 0x9281=1" die beiden letzten Schritte wiederverwenden.

## 3. Frame-Inhalte (Beispielskript ljames28 und 11 Logs, gemessen)
Prueffsumme: CRC-8, Polynom `0x1D`, Start 0, XOR je Frame, CRC-Byte nicht eingerechnet. Takt laut Skript: `0x133` 10 ms, `0x440`/`0x4CE`/`0x4FB` 100 ms (in den Logs nicht nachgemessen).

| Frame | XOR | Logs | Abweichung zum Skript |
|---|---|---|---|
| `0x4CE` | `0x0A` | B0 `5F` (22.369), `1F` (14.214), `7F` (539); B1-B5 `FF FF FF DF FF`; B6 `E0..EF`; B7 CRC | Skript nennt `2F` fuer Laden, kommt in den Logs nicht vor |
| `0x4FB` (DLC 6) | `0x82`, auf 7 Byte mit Nullen aufgefuellt | B0 `0x80` + Zaehler; B1 `00`; B2 **immer `04`** (37.026 Frames); B3/B4 `00`; B5 CRC | Skript sendet B0 ohne `0x80`. `02` und `00` stehen nur in der Tabelle des Autors, nie in einem Log |
| `0x440` | `0xAB` | B1 `31` (35.790) / `32` (510); B4 `1F` / `9F`; B5/B6 `FE FF`; **B7 wechselt** (`7C`, `82`, `94`, `81`, `7D`, ueber 49 weitere) | Aussage "B7 = `94`" im Befund vom 08.11. war zu vereinfacht. Frame ist noch nicht in dalas Plan |
| `0x133` | `0x4E` | B0/B1 `B1 A6` oder `00 7F`; B5 `06` oder `03`; B6/B7 meist `FF FF` | Skript sendet `05 05` |

- Entscheidung Nutzer: `0x4CE` nur "Fahren" (`7F`), `0x4FB` B0 folgt den Logs, B2 `04` bei `contactors_engaged == 1`, sonst `02`; `00` wird nie gesendet.
- **Annahme/Vorbehalt (im Code kennzeichnen):** ob der Pack `02` wie erwartet versteht, ist nicht belegt (nur `04` in den Logs). Ob die Frames die Fehler `U214381`, `U214361`, `P1B2081` beseitigen, ist nicht getestet.

## 4. Zeit und Strecke (gemessen, korrigiert)
- `0x376` B0-B2 = **24-Bit-Zahl in Minuten, binaer**, Nullpunkt **02.09.2021 11:32:44 UTC** (geprueft an den Logs: +0,1 und +0,3 min). B3-B5 = Strecke binaer, **0,1 km** je Zaehler (Einheit nach dalas Annahme). 31.000 km = `04 BA F0`. B6 `4A`, B7 `54` im Betrieb.
- **Berichtigung der fruehen Notizen:** Die Rechnung mit "Basis 255" und der Bezugspunkt `1614454107` war falsch fuer die echte Zoe. Das Datum zu `1614454107` ist **27.02.2021 19:28:27 UTC** (der Kommentar in dalas altem Code nennt 25.02.2021 08:08:27).
- Pack-Zeit-Startwert (PR `#3242`): `$9261` + 2 min, dann `millis()`. Waehrend der Emulator aus ist, laeuft keine Zeit (dala hat keine Uhr).
- Raw-Kilometer `$91CF` muss mit `0x80` beginnen (alle Dumps): `0x80000000 + km * 32`. Fuer 31.000 km: `0x800F2300` (der Wert `0x800F2D00` in einer frueheren Antwort war falsch).

## 5. Messwerte der beiden Zoe-Packs (Fotos 08. und 09.11., gemessen)
Seite zeigt in der alten Fassung nur die unteren 16 Bit.

| | Pack 1 | Pack 2 |
|---|---|---|
| Time | 23.332 | 37.634, spaeter 37.633; neuer Stand voll: **2.200.322** = 08.11.2025 11:34 UTC |
| Pack time | 28.277 | 7.517 |
| Mileage | 49.616 | 30.191; neuer Stand: raw `2799007215` = `0xA6D575EF` (unmoeglich, 20.360.111 km) |
| SoC min / max | 7.200 / 7.370 | 5.991 (spaeter 5.982) / 6.113 |

- Pack 2 hat die Zeit roh gespeichert, die dala nach dem Start gesendet hat (Bytes `21 93 02`). Pack 1: oberes Byte unbekannt (Annahme `22` ergibt 14.12.2025).
- Zurueckrechnen der Pack-Zeit (`7.517 + 65.536 * n` min) ab 08.11.2025: fuer n = 29 der 24.03.2022. n ist unbekannt (Pruefung: `22 91 C1` und `22 92 61` roh lesen).

## 6. Stil und Regeln fuer dala
- `.clang-format`: Google-Stil, `IndentWidth 2`, `ColumnLimit 120`, Zeiger links, zwei Leerzeichen vor Zeilenkommentaren; jedes `if` mit Klammern. `pre-commit` prueft das, Tests laufen mit gtest.
- Frames als `CAN_frame ZOE_xxx = {...}` in der `.h`, Senden mit `transmit_can_frame(&ZOE_xxx)`, Pruefsumme mit `calculate_crc_zoe(frame, xor, Laenge, Index)`.
- `AI_POLICY.md` von dala: KI-Code erlaubt, ein Mensch muss ihn verstehen und erklaeren koennen; PR-Text und Antworten an Maintainer schreibt der Nutzer selbst; keine autonomen Agenten.

## 7. Lokales Bauen unter Windows (Stand 12.11., gemessen am Nutzer-PC)
- Core 6.2.0 liegt in `C:\Users\nb\.platformio620` (Python-Umgebung und PlatformIO-Daten im selben Ordner). Die Erweiterung von VS Code benutzt den alten Core 6.1.19 und scheitert an der Plattform `55.03.312-1`.
- Vor jedem Build im selben Fenster setzen: `PLATFORMIO_CORE_DIR`, `PIP_CONSTRAINT` und `UV_CONSTRAINT` (Datei `C:\Users\nb\pio-constraints.txt` mit `platformio==6.2.0` und `pioarduino==6.2.0`, wie in dalas Workflow). Ohne die Pin-Datei holt die innere Umgebung `6.2.0-post.1` und die Plattform lehnt ab.
- Build: `C:\Users\nb\.platformio620\Scripts\pio.exe run -e stark_330`. Erster Lauf erfolgreich nach Aufraeumen des Ordners (Stand `191de3594`). Versionsnummer wird `v13.5.0dev-191de35 (main)`, wenn die Tags geholt sind (`git fetch upstream --tags`).
