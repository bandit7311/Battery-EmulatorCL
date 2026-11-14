# Plan Zoe Ph2 (dala-Original): Frames, Sleep, Zeit, Strecke - nichts gebaut, wartet auf "freigeben" (12.11.2026)

Kennzeichnung: **gemessen** = aus Log/Foto/Code gelesen, **Schluss** = abgeleitet, **Annahme** = nicht belegt, **entschieden** = Nutzer.
Ziel: Aenderungen am **Original von dala** (Stand `191de3594`, 09.11.2026), nicht an `battery-emulatorcl`. Das Produktivsystem bleibt auf `7ab39c0`.
Gebaut wird **alles zusammen in einem Patch**, erst wenn der Nutzer "freigeben" sagt. Arbeit in einem Klon von dala (kein Zugriff auf den lokalen Ordner des Nutzers), Lieferung als Patch.

## 1. Umfang (entschieden)
Alles direkt **unter der Zeile "Temporisation"** der Advanced-Seite (in `get_uds_info_html()` der Zoe-`.cpp`, also ueber der DTC-Tabelle).

| Teil | Inhalt |
|---|---|
| Frames | **Zwei getrennte Radioboxen**, jeder Frame **einzeln** abschaltbar (Entscheidung Nutzer 12.11.): `0x4CE` (fest "Fahren", B0 `7F`) und `0x4FB` (`04` Relais an, `02` aus, B0 = `0x80` + Zaehler). Standard beim Erststart jeweils **aus**, danach je ein eigener Schluessel im Flash gespeichert |
| Sleep | Auswahl "Sleep" / "Sleep 0x9281=1", Eingabe "auto wake after" in **Sekunden** (Standard **10 s**), Knopf Set, Knopf **Go sleep** (danach kein Frame mehr, auch keine UDS-Abfragen), Knopf **Wake up**. **Kein Schutz** gegen angezogene Schuetze, keine Rueckfrage angenommen |
| Zeit | `0x376` binaer, Startwert aus `$9261` plus 2 Minuten, danach mit `millis()` weiter (nur waehrend der Emulator laeuft) |
| Strecke | `0x376` binaer in 0,1 km, aus `$91CF` |
| Pruefung | Fallen Zeit oder Strecke durch, wird `0x376` **nicht gesendet** (Entscheidung a), die Seite zeigt eine Meldung |
| Set-Felder | Zeit und Strecke einstellbar, damit ein unplausibler Wert zurueckgebracht werden kann; eingetragener Wert gilt vor dem Pack-Wert |
| Anzeige | `Time`, `Pack time`, `Mileage` mit allen Bits, Strecke in km umgerechnet |
| Technik | Route in neuen Dateien (Variante 2: `...-ROUTES.h` und `.cpp`), zwei Zeilen in dalas `webserver.cpp` (`#include` und Aufruf, Muster `register_dump_can_route`, `webserver.cpp:1133`). **Kein Auto-Refresh** der Felder. Speichern mit `Preferences` in eigenem Namensraum, `#ifndef UNIT_TEST` |

### Pruefgrenzen und Set-Felder (Punkt 1 bestaetigt, Punkt 3 entschieden; Punkt 2 ist noch Annahme des Assistenten)
1. Pruefgrenzen: Zeit nicht `0xFFFFFF`, nicht 0, nicht unter der Pack-Zeit `$91C1` (falls gelesen); **Baudatum ist keine Grenze (entschieden, Nutzer 12.11.)**; Strecke: Rohwert beginnt mit `0x80`, unter 1 Mio. km (**1 Mio. km bestaetigt, Nutzer 12.11.**).
2. Eingetragene Zeit gilt nur im laufenden Betrieb und wird nicht im Flash gespeichert (sonst Ruecksprung nach jedem Neustart, wie bei "Time in 0x436"); die Strecke wird im Flash gespeichert.
3. ~~Eingabe der Zeit als Datum mit Uhrzeit~~ **entschieden (Nutzer, 12.11.): Eingabe der Zeit nur in Minuten** (24-Bit-Zahl), kein Datum.

## 2. Stand in dala (gemessen, `origin/main` = `191de3594`)
- `0x0F5` ist in `main` (Commit `ebb020c76`, 08.11.2026, "Add sending of 0x0F5 to reduce DTC amount"). `0x133` ist nur auskommentiert (`.cpp:454`), `0x4CE`, `0x440`, `0x4FB` fehlen.
- Die Zoe Gen2 hat nur noch **zwei** Dateien (`RENAULT-ZOE-GEN2-BATTERY.h/.cpp`). `...-HTML.h` gibt es nicht mehr. Die Seite entsteht aus `UdsBatteryHtmlRenderer::get_status_html()` = `get_uds_info_html()` der Zoe plus DTC-Abschnitt; danach kommen die Knoepfe der Befehlstabelle (`advanced_battery_html.cpp`). Unter die Knoepfe waere nur mit einer Aenderung an dalas Seitencode moeglich.
- `0x376` ist in `main` noch das **alte** Verfahren (Ziffern zur Basis 255, fester Start). Die bessere Fassung (binaer, Zeit aus `$9261` + 2 min, Strecke aus `$91CF`, volle 32 Bit in der Anzeige) steckt im PR-Zweig `#3242` (`de81065f`) und ist nicht in `main`. Der Patch aendert dieselben Zeilen: Konflikte moeglich.
- Hauptrelais bei dala: `datalayer.system.status.contactors_engaged` (`datalayer.h:444`): 0 Start, 1 angezogen, 2 von Battery-Emulator geoeffnet. Wird von anderen Dateien gelesen; keine Aenderung an dalas Dateien noetig.
- NVROL-Ablauf bei dala (`transmit_reset_nvrol_frames`): Session `10 03`, Routine `B009` (`04 31 01 B0 09 00`), 1 s, Session `10 03`, `04 2E 92 81 01`, dann 30 s mit `0x373` B0 `01`. Fuer "Sleep 0x9281=1" die beiden letzten Schritte wiederverwenden.
- **Entscheidung/Klarstellung Nutzer 12.11.: "Sleep 0x9281=1" macht KEINEN NVROL-Reset** (keine Routine `B009`). Wie am Twingo (`BATTERY.cpp:1135-1145`, gemessen im Code): Session `10 03`, dann direkt `04 2E 92 81 <Wert>`, danach Stille. "Sleep" (ohne 9281) sendet nur nichts mehr.
- **Entschieden (Nutzer 12.11., Option a): "Sleep 0x9281=1" schreibt `0x01`**, wie der Name sagt und wie dalas NVROL-Ablauf und die README von ljames28 (`042E928101AAAAAA`). Kein waehlbarer Wert.
  - **Vorbehalt:** Dalas Anzeige liest `1` als "Disabled!" und `0` als "Activated!" (`.cpp:162-166`). Nach dem Schreiben kann die Zeile "Temporisation" also "1 Disabled!" zeigen. Den Widerspruch in dalas Text haben wir nicht aufgeloest. Am Twingo ist der Standard `0x00` (ECU-Dump vom 28.09.: `0x00` = activated); das wurde bei der Zoe nicht geprueft.

### Sleep: Verhalten von dala waehrend der Stille und die 600 Sekunden (Nutzer 12.11., gelesen im Code von `191de3594`)
- **Kette ohne Gegenmassnahme (gemessen im Code):** Der Zaehler `CAN_battery_still_alive` (`uint8_t`, Start `CAN_STILL_ALIVE` = 60, `types.h:78`) wird von jedem Frame des Packs auf 60 gesetzt und einmal pro Sekunde um 1 verringert (`safety.cpp:47-61`, Aufruf `:480`, Takt `Software.cpp:583`). Bei 0 wird `EVENT_CAN_BATTERY_MISSING` (Stufe ERROR, `events.cpp:178`) gesetzt, ca. **61 s nach dem letzten Frame**. ERROR setzt `system_status = FAULT` (`events.cpp:855`). Bei aktiver Schuetzsteuerung (`CNTCTRL`, am Stark vom Nutzer aktiv) oeffnen die Schuetze **10 s spaeter** (`comm_contactorcontrol.cpp:221-241`) und bleiben bis zum Neustart gesperrt.
- **Entscheidung Nutzer: Stille bis 600 Sekunden, ohne dass dala einen Fehler setzt.** `CAN_STILL_ALIVE` kann nicht auf 600 gestellt werden (`uint8_t`, max. 255; Konstante gilt auch fuer Wechselrichter und Lader; liegt in dalas Datei). Umsetzung ohne Eingriff in dalas Dateien: waehrend der Stille setzt der Zoe-Treiber `datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;` einmal pro Sekunde (Muster: dalas BMS-Reset, `comm_contactorcontrol.cpp:495`, und der Zoe-Treiber selbst beim Empfang, `.cpp:154-177`).
- Eingabefeld "auto wake after": bis **600 s** erlaubt, Standard 10 s; kein Schutz.
- **Folge (Schluss, nicht am Aufbau getestet):** Der Wechselrichter bekommt in der Stille die letzten bekannten Werte bei geschlossenen Schuetzen; ein echter Fehler im Pack waere in dieser Zeit nicht sichtbar. Eine allgemeine Pruefung auf eingefrorene Werte gibt es bei dala nicht (`EVENT_STALE_VALUE` nur in zwei BMW-Treibern). Nach Ende oder "Wake up" laeuft die normale Pruefung wieder.
- Offen: Wie lange der Pack nach Beginn der Stille noch selbst sendet, ist nicht gemessen.

### Sleep: "Schuetze aus" gegenueber dem Pack (Entscheidung Nutzer 12.11.)
- Nur **gegenueber dem Akku**: Bei "Go sleep" sendet der Treiber **3 Sekunden Vorlauf** lang `0x4FB` mit B2 = `02` ("Relais aus"), auch wenn die Schuetze real zu sind; danach beginnt die Stille. Gilt fuer **beide** Sleep-Arten ("Sleep" und "Sleep 0x9281=1").
- Dalas Schuetzsteuerung und `contactors_engaged` bleiben unberuehrt; dem Wechselrichter wird nichts gemeldet (Variante B verworfen: `contactors_engaged` wird von dala alle 10 ms geschrieben und von anderen Dateien gelesen).
- Der Vorlauf wirkt nur, wenn die Radiobox fuer `0x4FB` auf **on** steht (sonst wird der Frame nicht gesendet).
- **Annahme (nicht bestaetigt):** Reihenfolge bei "Sleep 0x9281=1": Vorlauf 3 s, dann Session `10 03` und `2E 92 81 01`, dann Stille.
- `0x440` (Relais/12 V, B1 `32`, B4 `9F`) ist **nicht** Teil des Plans.

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

## 8. Merkliste (nur gemerkt, **nichts bauen**, Nutzer 12.11.)
- **`0x440` (HEVC -> LBC, "Relay/12V Voltage" laut Beispielskript):** nicht Teil des Patches. Dalas Zoe sendet ihn nicht. Aufbau aus den 11 Logs (37.122 Frames, gemessen): B0 `00`; B1 `31` (eingerastet, 35.790) / `32` (ausgerastet fuer Schlafuebergang und Balancing, 510), selten `33`/`39`; B2 CRC (XOR `0xAB`); B3 Zaehler im oberen Halbbyte; B4 `1F` (Relais an, 36.275) / `9F` (Relais aus, 847); B5/B6 `FE FF`; **B7 wechselt** (`7C`, `82`, `94`, `81`, `7D`, ueber 49 weitere). Takt laut Skript 100 ms.
  - **Offen:** Bedeutung von B7 (Vermutung 12-V-Wert, nicht belegt); ob B1 `32` und B4 `9F` beim Einschlafen zusammen mit `0x4FB` B2 `02` noetig sind; Abhaengigkeit vom Zustand (Schlafen, Laden, Fahren) nicht Zeitpunkt fuer Zeitpunkt zugeordnet.
  - Zusammenhang: Beim echten Auto gehoeren `0x440` und `0x4FB` zum Zustand "Relais". Der 3-Sekunden-Vorlauf des Sleep sendet nur `0x4FB` mit `02`.
- **`0x133` (Geschwindigkeit):** in dalas `main` nur auskommentiert (`.cpp:454`), nicht im Patch (siehe Abschnitt 3 fuer die Inhalte).

### Nachtrag Merkliste 12.11. (Bench-Messungen am Zoe-Pack 2, Zweig `zoe-combined`, Patch v6)
Kennzeichnung: **gemessen**, **Schluss**, **Annahme/offen**.

- **Gebaut seit der Merkliste oben:** `0x133` und `0x440` sind jetzt je mit Radiobox eingebaut (Standard aus, im NVM, Schluessel `ZOE133`, `ZOE440`). Die beiden Punkte oben ("nicht im Patch") sind damit ueberholt.
- **`$925F` Vehicle distance (gemessen, 3 Punkte):** Der Pack uebernimmt die Strecke aus `0x376` und zeigt sie in `$925F`. Rohwert bei gesendet 31001 / 31011 / 31021: **4960164 / 4961764 / 4963364**. Je 10 Einheiten +1600, also **160 Zaehler pro gesendeter Einheit**, dazu konstant **+4**. `$91CF` (Pack-Speicher) bleibt unveraendert (`2799007215`).
- **Offen: Einheit (Annahme).** Rohwert / 100 = 1,6 x die gesendete Zahl. Bei Twingo und Kangoo ist `$925F` Rohwert x 0,01 km. **Vermutung (Nutzer, 12.11.):** Der Akku kommt aus England, der Pack versteht die gesendete Zahl als **Meilen** und legt sie in km (x 1,6) ab. Entscheidet sich am Tacho des zugehoerigen Autos (Meilen oder km). Wenn Meilen: Anzeige und Beschriftung aendern (v7), das Feld "Vehicle distance (0x376)" bekommt "mi". Bis dahin zeigt die Seite Rohwert / 160 mit dem Wort "km".
- **Offen:** Woher der Aufschlag +4 kommt (0,025 km).
- **Sleep wie im Fahrzeug (gemessen, ljames28-Logs, beide Einschlaf-Logs gleich):** Sekunden vor dem Ende: 7,2 s `0x440` B1 `31`->`33`, `0x376` B7 `54`->`00`, `0x373` B1 `80`->`40`; 7,0 s `0x440` B4 `1F`->`9F`, B3 unteres Nibble `F`; 4,1 s `0x440` B1 `33`->`32`; 1,1 s `0x4FB` hoert auf (B2 bleibt `04`); 1,0 s `0x373` B0 `C1`->`01`, `0x0F5` hoert auf; 0,8 s `0x373` B5 `FE`, B7 `E0`; 0,7 s `0x375` B1 `28`->`FF`; dann endet alles. Eingebaut als Vorlauf von 7,2 s (vorher 3 s mit `0x4FB` = `02`, das gab es im echten Auto nicht).
- **DTC (Bench, 12.11.):** Mit `0x440` verschwand `U214361` (LBC). Mit allen vier Frames waren beide Listen leer. Nach dem alten Sleep (3 s Vorlauf, `0x4FB` = `02`) kamen `P1B2081` (LBC) und `U214387` (LBC2) zurueck (Status "Confirmed"). **Schluss/Annahme:** abruptes Verstummen ohne die Ankuendigung. **Ergebnis mit dem neuen Vorlauf (7,2 s wie im Fahrzeug), vom Nutzer berichtet, 12.11.:** alle Fehler weg, **auch nach dem Sleep keine Fehler mehr**. Nicht von mir gesehen; Dauer des Sleeps und Sleep-Art (Sleep oder 0x9281=1) beim Test nicht festgehalten. Das stuetzt den Schluss, dass das abrupte Verstummen ohne Ankuendigung die Ursache war.
- **`$91CF` (offen):** Rohwert `0xA6D575EF` (20.360.111 km) ist kein moeglicher Kilometerstand. Untere 16 Bit = 30191. Ob der Pack nie beschrieben wurde oder ein anderes Format hat, unbekannt.
- **Frame-Zaehler (gebaut):** Seite zeigt TX/RX Frames pro Sekunde des Akku-CAN (Wechselrichter nicht). Messwerte: TX 272 bis 372, RX 168 bis 191 (Bench, Wachzustand).
- **DTC-Teil:** stammt aus Jonnys PR #3240 und #3242 (nicht in `main`), im Patch ohne seinen 0x376-Commit `de81065fe`. Dessen Strecke aus `$91CF` ergibt bei Pack 2 227.452 km (Rohwert abgeschnitten auf 24 Bit). Hinweis fuer die Rueckmeldung an Jonny (Text schreibt der Nutzer selbst).

### Nachtrag Merkliste 14.11.: zwei weitere Fotos der Advanced-Seite, **Pack 1** (alte Firmware, nur untere 16 Bit)
Zuordnung zu Pack 1 nur aus den Werten (Time 23332, Pack time 28277, Mileage 49616 wie in Abschnitt 5); auf den Fotos steht kein Hinweis auf den Akku. **Gemessen**, abgelesen von zwei Fotos.

- **Foto A (untere Seitenhälfte):** Balance capacity wake 0, Balance time wake 0, BMS state 4, Energy complete **320**, Energy partial **691**, Slave failures 0, Mileage **49616**, Fan speed 255, Fan period 1000, Fan control 0, Fan duty 3, Time **23332**, Pack time **28277**, SoC min **7200**, SoC max **7370**, Temporisation 0 (Activated). DTC (eine Liste, alte Firmware): **4 Codes, alle aktiv:** U214381, U214281, U214287, P1B2081 (0/4 ohne Text).
- **Foto B (obere Seitenhälfte):** SoC 7289 ppt, usable SoC 6757 ppt, SoH 8888 ppt, Pack voltage 3696 dV, Max cell 3951 mV, Min cell 3932 mV, 12 V 12009 mV, Avg/Min/Max temp 784, Max power 1684, Interlock 65533, kWh 30540, Current 32640, Current offset 32640, Max generated 2100, Max available 11533, Current voltage 11734, Charging status 0, Remaining charge 170, Balance capacity total 18480, Balance time total 45440, Balance capacity sleep 18480, Balance time sleep 45440, Balance capacity wake 0, Balance time wake 0, BMS state 4, Energy complete 320.
- **Vergleich mit dem früheren Pack-1-Foto** (SoC min 7190, nur 2 Codes U214381 und P1B2081): hier sind es 4 Codes, zusätzlich **U214281** ("CAN from Inverter", laut Abschnitt 6 an `0x0F5`) und **U214287** (Typ 87, fehlende Nachricht).
- Die Bilddateien selbst sind **nicht** in dieses Repo gelegt (auf den Fotos sind Browser-Tabs mit privaten Titeln zu sehen).
