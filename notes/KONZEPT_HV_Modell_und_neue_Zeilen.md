# Konzept: HV-Zustandsmodell und neue Simulator-Zeilen (Twingo-Emulator)

Stand 08.10.2026. **Nur Konzept. Es wird nichts gebaut, bis der Nutzer "bauen" sagt.**
Basis: `main` auf `7fc0136`. Belege: Fahrlog 02.10. (`364b56eb`, ungefiltert), Log 04.10. (`5c3d7daf`, gefiltert), Lade-Log 20.11.2025, CAN-Liste C1A_Q4_2017, Zoe-Gen2-Treiber, OVMS-Twingo-Modul.
Kennzeichnung: **gemessen** = steht in den Logs, **Schluss** = aus Messungen gefolgert, **Annahme** = nicht belegt.

## 0. Nummerierung (neu, nach Vorgabe des Nutzers)

| Punkt | Inhalt |
|---|---|
| 12 | Zielauswahl in der freien Anfrage |
| 13 | `0x5D7` (Fahrzeug-Kilometer) senden |
| 14 | `0x426` editierbar machen |
| 15 | Neue Simulator-Zeilen: `0x57F`, `0x599`, `0x62D`, `0x523` (dazu Änderung an `0x1FD`) |
| 16 | Zoe-Gen2-Frames `0x373`, `0x375`, `0x376` |

(Die alten Punkte 12, 13, 14 der früheren Bauliste sind gebaut, Commit `2001e6e`. Hier sind die neuen gemeint.)

## 1. Grundidee: `0x350` ist der Taktgeber

Alle Zustandswechsel, die wir im Twingo beobachten (HV zu, Inverter an, HV auf), hängen an den Stufen von `0x350` (Byte 0: `C0`, `C4`, `C5`, `C7`, `C3`, `C2`, `C0`, `00`). Der Emulator sendet diese Stufen schon selbst (Aufwachen, Dauerframe, Abschaltfolge). Das neue **HV-Modell** liest nur, welche Stufe der Emulator gerade sendet, und leitet daraus die Inhalte der neuen Frames ab. Es entsteht keine zweite Zeitsteuerung.

Hook-Stellen im Code (alle vorhanden):
- `send_350_frame()` (Wake-/Abschaltstufen) und `send_run_350()` (Dauerframe): hier wird die Stufe beobachtet.
- `powerdown_stage`, `powerdown_stage_start_ms` (Abschaltfolge, Zustand 7) und `wake_burst_index` (Aufwachen, Zustand 8).
- `send_simulator_signals()`: Zeilen mit besonderem Inhalt haben dort eigene Zweige (wie `0x42E`, `0x29A`, `0x0C6`).
- `SimEnd` (Ende einer Zeile): `AT_C0`, `AT_00`, `BUS`.

## 2. Ablauf im Twingo (gemessen)

Zeiten relativ zum Stufenwechsel; L1 = Fahrlog 02.10., L2a/L2b = Log 04.10. (erster/zweiter Start bzw. Stopp). Spannungen sind nur im 1-s-Raster bekannt.

### Aufwachen (T0 = `C0`)
| Zeit | Ereignis |
|---|---|
| +0,1 bis +0,3 s | `0x599`: erster Frame `00 07 FF FF FF E0` (ungültig), danach nach 3 s normale Frames |
| +0,2 s | `0x1FD`: erster Frame `FF FF ...` (ungültig), danach `FE 40 7F FF 7F 50 50 00` |
| +0,6 / +1,1 / +3,6 s | `0x62D`: `01 45 E0 04 7F CC 00`, dann `01 45 E0 04 00 00 00`, dann `01 45 E0 04 06 80 00` |
| +1,1 s | `0x57F`: erster Frame `64 00 05 7F 80 00 00` (0,5 V, 0 A) |

### Zuschalten (T4 = `C4`; im Log 04.10. erster Start ohne `C4`, dort `C7`)
| Zeit | Ereignis | L1 | L2b |
|---|---|---|---|
| +0,0 bis +0,1 s | `0x5D7` startet: erster Frame `FF FF .. 08` (ungültig), dann gültig | +0,1 | – |
| +0,06 bis +0,37 s | `0x62D` Byte 4: `04` → `06` | +0,37 | +0,06 |
| +0,33 bis +0,62 s | `0x1FD` Byte 6: `50` → `A0` (HV zu) | +0,62 | +0,33 |
| +0,0 bis +1,0 s | `0x57F` Spannung: 11,5 V → Packspannung (±0,5 V) | ≤ +0,9 | +0,04 → +1,04 |
| +1,06 bis +1,37 s | `0x62D` Byte 4: `06` → `02` | +1,37 | +1,06 |
| ab `C5` +0,3 s | `0x57F` Bytes 3-4: `7F 80` → `CF A8` | +0,3 | im Raster nicht trennbar |
| `C7` +0,2 bis +14 s | `0x599` Byte 2: `04` → `08` (Inverter On) | +14,3 | +0,2 / +2,0 |

### Abtrennen (T3 = `C3` der Abschaltfolge)
| Zeit | Ereignis |
|---|---|
| +0,02 bis +0,13 s | `0x1FD` Byte 1 → `FE` |
| +0,7 bis +1,7 s | `0x599` Byte 2: `08` → `04` (Inverter Off) |
| +0,8 bis +2,9 s | `0x57F` Bytes 3-4: `CF A8` → `7F 80` |
| +1,3 bis +1,95 s | `0x62D` Byte 4: `02` → `06` |
| +2,07 / +2,26 / +3,44 s | **HV auf**: `0x62D` Byte 4 `06` → `04` und `0x1FD` Byte 6 `A0` → `50` |
| ab HV auf | `0x57F` Spannung fällt: 69 V (+0,55 s), 36 V (+1,5 s), 28 V (+2,5 s), dann exponentiell mit τ ≈ 22 s bis 0,5 V (L1: 8,5 V nach 27 s, 2,5 V nach 56 s; L2: 20 V nach +9,5 s, 15 V nach +15,5 s) |

### Ende der Frames (L1, `C0` bei 525,7 s, `00` ab 535,8 s)
`0x5D7` endet bei `C0` (525,7 s). `0x57F`, `0x62D`, `0x1FD`, `0x42E` laufen bis zum Busende (535,7 bis 535,8 s). `0x599` letzter Frame 533,9 s (3-s-Takt, nächster läge nach dem Busende). Also: `0x5D7` = `SIM_END_AT_C0`, die anderen = `SIM_END_BUS`.

## 3. Das HV-Modell (Emulation)

**Eingaben** (alle schon im Code): gesendete `0x350`-Stufe, `NVROLstateMachine` (5 Stille, 7 Abschaltung, 8 Aufwachen), `powerdown_stage`, Zeitpunkt des Stufenwechsels.

**Zustand:** `hv` (aus / zu), `hv_since_ms`, `hv_open_ms`, `inverter_on`, `b34_active`, `phase_62d`.

**Ereignisse (Konstanten, Mittelwerte aus Abschnitt 2; Streuung ±1 s, für den Akku vermutlich unkritisch):**
| Auslöser | Folge |
|---|---|
| Aufwachen (`C0` gesendet) | alles aus, ungültige Erstframes |
| Stufe `C4` zum ersten Mal | `T4`: Spannungsrampe 0 → Packspannung in 0,9 s; `T4+0,2 s` `0x62D` → `06`; `T4+0,4 s` `0x1FD` → HV zu; `T4+1,2 s` `0x62D` → `02` |
| Stufe `C5` (oder `C7`, falls kein `C5`) | `b34_active` = wahr (`CF A8`) |
| Stufe `C7` | Inverter On nach **2,0 s, fest** (entschieden 08.10.; wie L2; L1 hatte 14 s, vermutlich Fahrerhandlung) |
| `C3` aus der Abschaltfolge (`powerdown_stage` 0) | `T3`: `0x1FD` Byte 1 → `FE` sofort; Inverter Off `T3+1,0 s`; `0x62D` → `06` `T3+1,5 s`; HV auf `T3+2,2 s` (Bits, Spannungsabfall); `b34` aus `T3+2,0 s` |
| Dauerframe `C7` ohne Aufwachen (Start des Emulators) | wie "HV zu seit Beginn" (damit sich beim Start nichts Seltsames abspielt) |
| Dauerframe `C3` (Testmodus "steady C3") | HV bleibt zu (kein Abschalten, nur die Abschaltfolge löst es aus) |

## 4. Punkt 15: neue Zeilen (alle standardmäßig **aus**, eigener Haken je Zeile)

Neue Indizes ab 35 (die Maske ist 64 Bit, passt; insgesamt höchstens bis 45). Alle als `A` (Annahme) gekennzeichnet; Info-Text nennt Beleg und Herkunft.

| Zeile | ID | DLC | Takt | Inhalt | Ende | Beleg |
|---|---|---|---|---|---|---|
| 35 | `0x57F` | 7 | 1000 ms | B0 = Strom Byte (Strom×2+800)>>3; B1 oben 3 Bit = Rest des Strom-Werts, B1 unten 5 Bit + B2 = Spannung ×10 (13 Bit); B3-B4 = `7F 80` oder `CF A8` (`b34`); B5-B6 = `00 00` | `BUS` | gemessen (Aufbau, Zeiten, Korrelation r = +0,95 mit `0x1FD`) |
| 36 | `0x599` | 6 | 3000 ms | B1 = `00`; B2 = `04` (Off) / `08` (On); B3 = `26`, B4 = `62`, B5 = `2D`, B6 = `00` (Ruhewerte aus L2); erster Frame nach Aufwachen `00 07 FF FF FF E0` | `BUS` | gemessen (Länge 6, Zeitpunkte); B3-B5 Bedeutung **unbekannt** |
| 37 | `0x62D` | 7 | 500 ms | `01 45 E0 [B4] 06 [B6] 00`, B4 = `04` (HV aus) / `06` (Übergang) / `02` (HV zu); B6 = `80` (HV zu, Inverter Off) / `40` (Inverter On) / `00` (HV aus); erste zwei Frames nach Aufwachen `.. 04 7F CC 00`, `.. 04 00 00 00` | `BUS` | gemessen; B5/B6-Bedeutung **unbekannt**, B5 in L2 = `27` statt `06` |
| 38 | `0x523` | 3 | 1000 ms | 24 Bit Minuten = `vehicle_age_minutes()` (gleiche Quelle wie `0x350` Bytes 1-3, also auch "manuelles Alter" und "glatter Zähler") | `BUS` | **nicht** im Twingo-Log; nur in der CAN-Liste (BCM_A8, "AbsoluteTimeSince1rstIgnition"). Reiner Test, Annahme |

**Änderung an der bestehenden Zeile `0x1FD`:** Byte 1 = `FE` bei HV aus, `45` bei HV zu (Leerlaufwert aus L1; bei Fahrt folgt es der Motorleistung); Byte 6 = `50` bei HV aus, `A0` bei HV zu. Rest wie bisher. Erster Frame nach Aufwachen `FF ...`.

**Hinweis (Schluss, nicht belegt):** `0x57F`, `0x599` (und `0x6C9`, `0x652`, `0x654`, `0x632`) tragen in der CAN-Liste das Suffix `_BLMS`. Das spricht für Telemetrie des EVC an die TCU (Batterie-Überwachung). Es gibt **keinen Hinweis**, dass der Akku diese Frames auswertet. Wahrscheinlicher sind `0x62D`, `0x1FD` und `0x350`. Die neuen Zeilen sind daher vor allem Realismus und Testmaterial.

## 5. Punkt 13: `0x5D7` mit Kilometern

- **Gemessen:** Frame kommt im Twingo alle 100 ms (DLC 8). Bytes 2-5 = Kilometerstand: `(Bytes 2-5 als 32 Bit) >> 4`, Auflösung 0,01 km (OVMS-Formel). Beispiel `08 D4 86 80` = 92.591,12 km. Der Akku-Kilometerstand `925F` im Auto (92.677 km am 06.10.) passt dazu (**Schluss**, nicht belegt, dass der Akku ihn von dort nimmt).
- **Byte 6** ist ein Zähler: `C0, C2, ... FE` (32 Werte, Schritt 2, gemessen lückenlos), kein CRC.
- **Byte 7** = `00`; nur der erste Frame nach dem Start hat `08` (mit Geschwindigkeit `FF FF`).
- **Bytes 0-1** Geschwindigkeit (0,01 km/h), im Stand `00 00`.
- **Zeile:** neue Zeile Index 39, `0x5D7`, 100 ms, Ende `SIM_END_AT_C0` (im Log letzter Frame bei `C0`), Standard aus.
- **Kilometerwert:** frei eingebbar wie "Manual vehicle age" (nur zur Laufzeit), Voreinstellung **19.400 km** (entschieden 08.10.; wie bisher bei `0x426`, damit der Vergleich sauber ist).
- **Warum nicht über `0x426`:** `0x426` hat nur 24 Bit mit 1/256 km, also höchstens **65.535,99 km**. Ein echter Twingo-Stand (92.678 km) passt dort nicht hinein. `0x5D7` schafft bis 2.684.354 km.

## 6. Punkt 14: `0x426` editierbar

- Unser Frame `00 60 01 00 4B C8 00 40`: Bytes 4-6 = `0x4BC800 / 256 = 19.400,0 km` (**gemessen**: `925F` zeigte genau diesen Wert).
- **Editierbar:** Kilometerwert (0 bis 65.535 km), Byte 7 (Voreinstellung `40`), je als Feld auf `/simulator`, nur Laufzeit.
- **Testfolge (nur lesen, nach dem Bau):** (a) `0x426` an mit km = 0: bleibt E14381? (b) `0x426` aus, `0x5D7` an: bekommt der Akku die km, und bleibt E14381 aus? (c) beide an.
- Hinweis: `0x426` löst E14381 aus (jede Beobachtung mit `0x426` an). Ob der Fehler am Frame an sich oder an seinem Inhalt (Byte 7, Kilometer) hängt, klärt (a).

## 7. Punkt 12: Zielauswahl

- **Heute:** Die freie Anfrage sendet immer auf `0x18DADBF1` (MCPU) und liest `0x18DAF1DB`. Die Anfrage-ID steht im Frame `ZOE_POLL_18DADBF1`, die Antwort-ID wird in `handle_incoming_can_frame()` fest als `0x18DAF1DB` behandelt.
- **Entschieden (08.10.):** Auswahl `DB` (MCPU, Standard) und `DC` (Safety-CPU: Anfrage `0x18DADCF1`, Antwort `0x18DAF1DC`). Nur zur Laufzeit, nicht im NVM. Beim Wechsel wird der Antwortfilter mitgeschaltet; das Flow-Control-Frame bekommt dieselbe Ziel-ID.
- **`79B` gestrichen (entschieden 08.10.):** Der Code-Kommentar sagt, `0x79B/0x7BB` "hat auf diesem Akku nie eine Anfrage beantwortet (über viele Mitschnitte bestätigt)". Eine Auswahl `79B` brächte nichts.
- **Optional (nur im Auto sinnvoll, da am Bench nicht vorhanden):** `DF` (Inverter `0x18DADFF1` / `0x18DAF1DF`, für `$2004`, `$70D7`, `$7083`) und `DA` (EVC `0x18DADAF1` / `0x18DAF1DA`).
- **Einschränkung:** Der Treiber hat eine einzige geteilte Anfrage in der Schwebe (`ext_isotp_in_progress`); die Auswahl gilt für die freie Anfrage, nicht für das zyklische Polling (das bleibt auf MCPU).

## 8. Punkt 16: Zoe-Gen2-Frames

Alle drei kommen im Twingo-Log **nicht** vor und stehen auch nicht in der CAN-Liste. Sie stammen aus dem Zoe-Gen2-Treiber (ljames28) und sind Experimentiermaterial.

| ID | DLC | Takt (Zoe-Treiber) | Inhalt |
|---|---|---|---|
| `0x373` | 8 | 100 ms | `C1 40 5D B2 00 01 FF E3`, Bytes 2-3 wechseln alle 5 Frames zwischen `B2 5D` und `5D B2` ("HEVC Wakeup/Sleep") |
| `0x375` | 8 | 100 ms | `02 29 00 BF FE 64 00 FF` ("HEVC Status") |
| `0x376` | 8 | im Zoe-Treiber alle 100 ms | Bytes 0-2 und 3-5 = dieselbe Zeit als Minuten seit Produktion in Basis-255-Stellen (Jahr, Stunde, Minute); Bytes 6-7 = `0A 00` |

- **Zweck des Tests:** In der Zoe speichert der LBC offenbar die Zeit über diese HEVC-Frames. Ob der Twingo-Akku sie annimmt, ist **Annahme**; am Bench stehen `9261` und `91C1` auf demselben Wert.
- **Optional dazu** (nicht beauftragt): `0x5F8` (Fahrzeug-ID, 1 s, DLC 4 `16 44 90 8F`), `0x6BF` (Total Boost Time, 1 s, DLC 3), `0x0EE` (10 ms mit CRC). **Achtung:** `0x5F8` und unser `0x69F` tragen beide eine Fahrzeug-ID, aber verschiedene; nicht zusammen einschalten.
- **Zeilen:** Indizes 40 bis 42 (`0x373`, `0x375`, `0x376`), Standard aus, Ende `SIM_END_BUS`. Mit den optionalen Zusatzframes bis Index 45, weit unter der Grenze von 64.

## 9. Schalter und Standard

- Jede neue Zeile hat ihren eigenen Haken (Regel des Nutzers), Standard aus. Die Standardmaske `0x3FF` ändert dieses Konzept **nicht** (das ist der getrennte Punkt 5 der alten Liste, Vorschlag `0x387`).
- Das HV-Modell läuft immer mit (rechnet nur), sendet aber nichts, solange keine neue Zeile an ist. Ohne Haken bleibt der Bus bitgleich wie heute.
- Kein neuer Schalter A/B nötig: Die neuen Zeilen folgen den Stufen, die in beiden Modi gesendet werden. Im Modus "wie bisher" fehlt `C4` bis `C6`; dort gilt `C7` bzw. der erste Dauerframe als Zuschalten.

## 10. Tests (GoogleTest, vor dem Bau schreiben)

1. HV-Modell als reine Funktion: Zeitreihe der Stufen hinein, Zustand heraus (Zuschalten, Abschalten, Zerfallskurve, ungültige Erstframes).
2. Frame-Kodierung `0x57F`: Spannung und Strom, Rundlauf (kodieren/dekodieren) gegen Messwerte aus L1 (z. B. `64 0D 3E 7F 80 00 00` = 339,0 V, 0,0 A).
3. `0x5D7`: Zähler `C0..FE` mit Umbruch, Kilometer-Kodierung (92.591,12 km = `08 D4 86 80`).
4. Maske: neue Indizes 35 bis 42 liegen in der 64-Bit-Maske und werden gespeichert.
5. `clang-format 22.1.8`, alle 318 bestehenden Tests grün.

## 11. Test am Bench (nach dem Bau; nur Haken und Lesen)

Mit dem funktionierenden Satz (`0x090`, `0x242`, `0x350`, `0x69F`, `0x53B`, ohne Zoe-Frames) jeweils eine Zeile dazu, 30 s warten, dann `9259`, `925C`, `9261`, `91C1`, `9279`, Zellspannungen, DTCs:
1. `0x62D`, `0x1FD` (HV-Bits), 2. `0x57F`, `0x599`, 3. `0x5D7` (mit `0x426` aus), 4. `0x523`, 5. `0x373`, `0x375`, `0x376`.
Erfolg wäre: `9259` springt auf `05`, `925C` auf `01`, `9279` zählt, oder `9261`/`91C1` ändern sich.

## 12. Risiken

- Die Zeiten sind Mittelwerte aus zwei Logs; die Streuung beträgt ±1 s. Das reicht für Tests, ist aber keine Replik.
- `0x599` B3-B5 und `0x62D` B5-B6 sind in ihrer Bedeutung unbekannt; sie werden mit Ruhewerten gefüllt.
- Möglich, dass **keiner** der neuen Frames `9259` beeinflusst. Dann ist das Ergebnis trotzdem verwertbar: Dieser Satz ist es nicht.
- Mehr gesendete Frames erhöhen die Buslast auf dem Bench (1 + 0,33 + 2 + 1 Hz, vernachlässigbar) und das Risiko neuer DTCs (E14xxx); darum alle aus.

## 12a. Punkt 17 (Vorschlag, noch nicht freigegeben): Fahrzeugalter ohne Uhrwert

Ziel: Der Emulator sendet nie mehr den Uhrwert; das Alter läuft stetig (+1 pro Minute) von einem Startwert, den der Akku schon kennt.
- **Quelle ersetzen:** Die Uhr-Quelle (`vehicle_age_clock_minutes()`) und der glatte Zähler, der sich an der Uhr orientiert, entfallen als Standard.
- **Startwert, Variante A (einfach):** Das manuelle Alter wird im NVM gespeichert (Wert plus Unix-Zeit des Setzens). Nach einem Neustart gilt `Alter = gespeicherter Wert + (jetzt - Setzzeitpunkt) / 60`. Das ist stetig und braucht keine Akku-Abfrage. Ohne NTP: gespeicherter Wert + Laufzeit seit Start.
- **Startwert, Variante B:** Beim Start/Aufwachen einmal `22 92 61` vom Akku lesen und von dort zählen. Bis die Antwort da ist, kein `0x350` mit Alter senden (offen, was dann stattdessen).
- **Solange nichts gesetzt ist:** `0x350` darf nicht mit dem Uhrwert laufen. Entweder gar kein Alter senden (offen: welche Bytes) oder die Zeile `0x350` bleibt aus, bis ein Alter gesetzt ist.
- **Ausgelöst durch:** Beobachtung am Bench 06.10., der Akku hält `9261` = `91C1` = 1.311.344 min, der Uhrwert liegt 3,15 Jahre darüber.

## 13. Entscheidungen (Stand 08.10.2026)

**Entschieden:**
1. `79B` wird aus Punkt 12 gestrichen. Zielauswahl nur `DB` (MCPU, Standard) und `DC` (Safety-CPU). `DF`/`DA` sind noch nicht entschieden und bleiben draußen, bis der Nutzer sie nennt.
2. Strom in `0x57F` = **gemessener Packstrom**. Umrechnung: `I_0x57F = -current_dA / 10` (Emulator: negativ = Entladen; `0x57F`: positiv = Entladen). Ist der Wert ungültig (Betrag über 400 A, solange der `0x155`-Filter fehlt) oder HV aus: 0 A. Rohwert = `(I + 400) * 2`, 11 Bit.
3. Inverter On = **fest `C7` + 2,0 s** (keine Einstellung).
4. `0x5D7` Anfangskilometer = **19.400**.

**REGEL (Nutzer, 08.10.): Der Uhrwert als Fahrzeugalter (am 08.10. 2.967.669 min) darf nie wieder gesendet werden.** Bis heute sendet der Emulator ihn aber im Standard: ohne "Manual vehicle age" nimmt `vehicle_age_minutes()` die Uhr (UTC ab 15.02.2021) oder den glatten Zähler, der ebenfalls beim Uhrwert startet. "Manual vehicle age" ist nur zur Laufzeit gültig und nach jedem Neustart weg; die Maske (mit `0x350` an) bleibt aber gespeichert. **Folge: Nach jedem Neustart sendet der Emulator mit `0x350` an wieder den Uhrwert**, bis man das manuelle Alter neu setzt. Das gilt auch für `0x523` (Punkt 15) und Aufwachen/Abschaltfolge.

5. Punkt 17 = **Variante A** (manuelles Alter wird mit Setzzeitpunkt gespeichert und läuft nach dem Neustart stetig weiter). Der Faktor "Alterung nur 1/10" kommt später (Todo, Abschnitt 14).
6. `0x376`-Zeit **aus unserem gesetzten Alter** (`vehicle_age_minutes()`), nicht fest April 2025.
7. Zoe-Zusatzframes `0x5F8`, `0x6BF`, `0x0EE`: **nein**.
8. `DF`/`DA` als Ziele: **nicht nötig**, kommen auf die Nice-to-have-Liste fürs Auto (Abschnitt 14).

**Noch offen:**
9. Was sendet `0x350` mit Alter, solange nichts gesetzt ist? Empfehlung: Zeile `0x350` sendet erst, wenn ein Alter gesetzt ist; beim allerersten Start also einmal manuell setzen.
10. Freigabe "bauen" und Reihenfolge.

## 13a. Ursprüngliche Fragen (zur Dokumentation)

1. `79B` aus Punkt 12 streichen? (Empfehlung ja.) `DF`/`DA` aufnehmen?
2. Inverter-Strom in `0x57F`: gemessenen Packstrom verwenden oder immer 0 A?
3. Inverter On: `C7` + 2,0 s (Voreinstellung) oder konfigurierbar?
4. `0x5D7`: Anfangs-Kilometer 19.400 oder 92.678?
5. `0x376`-Zeit: wie der Zoe-Treiber (fester Zeitpunkt, hier April 2025) oder aus der Echtzeit/`vehicle_age_minutes()`?
6. Zoe-Zusatzframes `0x5F8`, `0x6BF`, `0x0EE` mit aufnehmen? (Voreinstellung nein)


## 14. Todo (später) und Nice-to-have am Auto

**Todo (später, nicht jetzt):**
- Faktor für das Fahrzeugalter: Das Alter wächst nur mit 1/10 der Echtzeit (Wunsch des Nutzers 08.10.). Vor dem Bau klären: Der Akku-Zähler läuft real +1 pro Minute. Mit 1/10 fällt unser Alter pro Minute um 0,9 min hinter die Packzeit zurück; ein Sicherheitsvorsprung von 1.440 min wäre nach etwa 1.600 min (27 h) aufgebraucht. Faktor deshalb einstellbar machen (Standard 1) oder den Vorsprung mitwachsen lassen.

**Nice-to-have am Auto (nur dort sinnvoll):**
- Zielauswahl `DF` (Inverter `0x18DADFF1`/`0x18DAF1DF`: `$2004`, `$70D7`, `$7083`) und `DA` (EVC `0x18DADAF1`/`0x18DAF1DA`) in der freien Anfrage.
- Inverter-Dump bei Zündung aus (`9259` = 04) zum Beleg, dass der Wechsel 04 → 05 das Schließen der Schütze ist.
- Messung mit Zeitauflösung: `22 92 59` und `22 90 0D` im Sekundentakt während der Zündung, dazu CAN-Mitschnitt.
- Ungefilterter Mitschnitt (30 s Zündung an) auf der Frage, ob `0x437`/`0x676` existieren.
