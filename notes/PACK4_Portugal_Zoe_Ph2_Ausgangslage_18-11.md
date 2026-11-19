# Pack 4 (Portugal, Zoe Ph2): Ausgangslage vom 18.11.2026

Quelle: fuenf Screenshots der Advanced-Seite (192.168.1.231/advanced) ueber TeamViewer, Uhrzeit auf dem Fremdrechner 20:27 bis 20:29 (Portugal), Firmware **Patch v9** (neue Seite mit Haken, Safety-Zeile und grauem Minutenfeld, siehe unten). Abgelesen von Claude, nicht vom Nutzer nachgeprueft. Das ist der Zustand **vor** jedem Eingriff (alle Zusatzframes aus). Eine Person vor Ort (Raul) arbeitet mit dem Nutzer per Notizblock. Ihr Plan laut Notizblock: zuerst die Fehler ansehen und pruefen, ob die fehlenden Frames die DTCs beheben.

## Werte
| Wert | Pack 4 |
|---|---|
| usable SoC | 5643 ppt |
| SoH | **10000 ppt (100 %)** |
| Packspannung | 3598 dV (359,8 V) |
| Zelle max / min | 3850 / 3826 mV (Abstand 24 mV) |
| 12 V | 13391 mV |
| Temperatur avg / min / max | 962 / 960 / 976 (Rohwerte) |
| kWh | 29,45 (raw 29450) |
| Strom / Offset | 32638 / 32640 |
| Max generated / available | 4300 / 11550 |
| Charging status / Remaining charge | 0 / 142 |
| **Balancing (alle sechs Zaehler)** | **0,00 Ah bzw. h (raw 2147483648)** |
| BMS state | 4 |
| Energy complete / partial / Slave failures | **0 / 0 / 0** |
| SoC min / max | 6007 / 6277 |
| Temporisation | 0 Activated! |

## Zeit- und Streckenblock (v9)
| Zeile | Haken | Anzeige |
|---|---|---|
| LBC1 time `$9261` | ok | 2203012 min (raw 21 9D 84), full days total 1530, 9 days 2 month 4 years |
| LBC2 time `$9261` | ok | gleich |
| **Pack time `$91C1`** | **rotes X** | **2222387 min (raw 21 E9 33)**, 23 days 2 month 4 years (groesser als die LBC1-Zeit, Regel) |
| LBC1 distance `$925F` | ok | 220301,20 mi (raw 35248192) |
| LBC2 distance `$9262` | ok | 352481,92 km (raw 2182731840 = 0x80000000 + 35248192) |
| **Pack distance `$91CF`** | ok | **0 km (raw 2147483648)** |
| Safety mode 1 root cause `$9216` | ohne Haken | **0 (raw 00)** |

Boxen: Tage leer, Minuten 0 (grau), Strecke leer, Einheit `mi`. Schalter: `0x376`, `0x4CE`, `0x4FB` aus (Standard), Sleep mode "Sleep", Auto wake 10 s, TX 232 / RX 168, Status awake.

## DTCs
**LBC (7 Codes, 7/7 beschrieben):** P1B0D47 Confirmed (Internal memory RAM/ROM), **P1B0E41 Confirmed (External NV memory, General checksum failure)**, **U214381 Active** (CAN from EVC/HEVC, Invalid serial data received), P1B0D41 Confirmed (Internal memory RAM/ROM), U214281 Confirmed (CAN from Inverter), U214287 Confirmed (CAN from Inverter, Missing message), **P1B2081 Active** (Relay 4 control output, Invalid serial data received).
**LBC2 (9 Codes, Beschreibung "Unknown"):** U214481 Confirmed, **U214381 Active**, **U214387 Active**, P1BA481, P1BA487, P1BAD41, P1BA249, P1BA016, P1BB108 (alle Confirmed).

## Auffaelligkeiten (Schluesse, nicht belegt)
1. **LBC1 distance raw 35248192 = 2203012 x 16**, also genau die Zeit mal 16. Das ist dasselbe Muster wie bei Pack 3 (Strecke = Kopie der Zeit durch den alten dala-Treiber). Die Strecke ist kein echter Kilometerstand.
2. Die Zeit 2203012 (etwa 10.11.2025) ist wie bei Pack 2 und 3 ein Rest des alten Treibers.
3. **Pack time ist groesser als die LBC1-Zeit** (wie bei Pack 2) und bekommt das rote X.
4. **Zaehler alle null** (Balancing, Energie, Pack-Kilometer 0, SoH 100 %) und **"External NV memory checksum failure" bestaetigt**: passt zu einem frisch geloeschten Speicher (zum Beispiel nach einem NVROL-Reset). Ob das hier so war, ist nicht bekannt.
5. **`$9216` = 0**, in den beiden Dumps (Original-Pack 24.07.2025, Auto-Pack 11.10.) steht 10. Die Bedeutung bleibt offen.
6. Die neue Seite (v9) laeuft auf einem fremden Geraet wie im Mock-up: Haken und Kreuz, Tage/Monat/Jahre (2203012 gleich 9 days 2 month 4 years, 2222387 gleich 23 days 2 month 4 years), fette Einheit bei LBC1 und LBC2 (LBC2 gleich 1,6 mal LBC1).

## Schritt 2 (Screenshot 20:42 Portugal): Tage 1530 und Strecke 5 mi gesetzt, `0x376` an, die vier unteren Frames schon vorher an
Abgelesen von Claude. Die Fehlerliste ist auf diesem Bild nicht zu sehen. **Laut Notizblock (Raul, nicht von Claude gesehen): "as soon we have set days and kilometer both left over faults went away".**
- Tage **1530**, Strecke **5 mi**, Minuten 0 (grau), `0x376` **on**, `0x4CE` und `0x4FB` on.
- **LBC1 time jetzt 2203200 min** (raw 21 9E 40) = 1530 x 1440 genau, also **hat der Pack unsere Zeit uebernommen** (vorher 2203012). Haken ok.
- **LBC2 time noch 2203012** (188 min dahinter), **rotes X** durch die 60-min-Regel (Regel greift wie geplant, vermutlich nur zeitweise).
- **Pack time 2222387 weiter rotes X** (groesser als die Fahrzeugzeit). Um es gruen zu bekommen, muessten die Tage mindestens 1544 sein (ceil(2222387 / 1440)).
- **LBC1 distance weiter 220301,20 mi (raw 35248192)**, LBC2 distance 352481,92 km, **Pack distance 0 km**, `$9216` = 0. Die Strecke 5 mi ist (noch) **nicht** im Pack angekommen. Moegliche Ursache (nicht belegt): der Pack uebernimmt keinen kleineren Wert als den gespeicherten. Oder die Strecke wurde nur noch nicht neu gelesen (Minutenfeld 0, die Seite ist weniger als eine Minute alt).
- Notizblock: Der NVROL-Reset war vermutlich **9 Tage vorher** (Raul: "most likely nvroled at 9 days ago"). Pack aus einem verunglueckten franzoesischen Auto, gekauft in Portugal. Raul rechnete eine Idee "ein Jahr dazu" ab 14.09.2025, Entscheidung: bei **1530 Tagen** bleiben ("should be the age of the pack"; Claude: das ist nicht das Alter des Packs, die Packzeit entspricht 1543 Tagen).

## Schritt 3 (Screenshot 20:42 Portugal, gleiche Minute): Fehlerlisten nach den vier unteren Frames und `0x376`
Abgelesen von Claude.
- **LBC: 1 Code** (vorher 7): **U214381 Confirmed** (CAN from EVC/HEVC, Invalid serial data received), **nicht mehr Active**. "1/1 DTCs matched".
- **LBC2: 1 Code** (vorher 9): **U214387 Confirmed** (Beschreibung "Unknown" in der LBC2-Tabelle, in der LBC-Tabelle heisst derselbe Code "CAN from Inverter - Missing message").
- Alle aktiven Fehler (U214381 LBC, P1B2081, U214387 LBC2, U214381 LBC2) sind weg bzw. nur noch gespeichert. Die Speicherfehler (P1B0D47, P1B0D41, P1B0E41) sind nicht mehr in der Liste.
- TX-Frames pro Sekunde **372** (vorher 232 mit allen Zusatzframes aus), RX **168** (unveraendert). Status awake, Sleep mode "Sleep", Auto wake 10 s.
- Offen: ob die zwei letzten Fehler mit "Erase DTC" verschwinden und nach einem Sleep nicht zurueckkommen.

## Schritt 4 (v10/v11, Raul 21:41 und 21:58): Knopf "Erase Safety Mode 1"
Abgelesen von Claude.
- **v10, 21:41:** `31 01 B0 04 00` (Laenge 5) -> Antwort `03 7F 31 31` = NRC 0x31 (requestOutOfRange nach UDS-Norm).
- **v11, 21:58:** erster Versuch `31 01 B0 04 00` wieder `03 7F 31 31`; zweiter Versuch `31 01 B0 04` (Laenge 4) ebenfalls `03 7F 31 31`. **Beide Formen abgelehnt mit 0x31.** Sitzung `10 03` und Adresse funktionieren (das Pack antwortet), die Routine B004 geht in diesem Zustand nicht.
- Moegliche Gruende, mit dieser Messung nicht zu trennen: (a) B004 gibt es in der Software von Pack 4 nicht (die DDT ist "SW5XX V1.5", Stand 25.05.2022; B009/NVROL steht dort auch nicht und geht trotzdem), (b) die Routine laeuft nur, wenn ein Safety Mode 1 aktiv ist (`$9216` ist hier 0), (c) es fehlt eine andere Sitzung oder ein Freischalten.
- **Boot-Zaehler:** nach v11-Flash Tage 1552 (+1), Strecke 15 mi (+5), Minuten 5. Zeit LBC1/LBC2 2234885 min, gleich. Pack time weiter 2232002, Pack distance weiter 0 km (kein Sleep seit 21:26). **Gelbe Warnung weg** (v11-Korrektur wirkt).
- Boot 21:54: LBC 1 Code P1B2081 Confirmed (Relay 4, `0x4FB`), LBC2 keine Codes.

## Schritt 5 (Sleep 6 min mit v11, Raul 22:10 vorher, 22:20 nachher)
Abgelesen von Claude. BMS state laut Nutzer: 5 (Pack 4 steht mit unseren Frames in 5; die Bench-Notiz "05 nie erreicht" galt fuer einen anderen Aufbau).
| | vorher 22:10 | nachher 22:20 |
|---|---|---|
| Tage / Minuten | 1552 / 17 | 1552 / 27 |
| LBC1 time | 2234896 | 2234906 |
| LBC2 time | 2234896 | 2234906 (gleich, kein Nachhinken) |
| **Pack time** | 2232002 | **2234899** (+2897 min, 7 min unter LBC1, gueltig) |
| LBC1 distance | 15,02 mi (2404) | 15,02 mi |
| LBC2 distance | 24,04 km | 24,04 km |
| **Pack distance** | 0 km | **16 km** (raw 2147484160 = 0x80000000 + 512, 512/32 = 16) |
| `$9216` | 0 | 0 |
| Fan duty | 3 | **213** (Bedeutung offen) |
- **Pack distance 16 km:** rechnerisch 10 mi x 1,6 = 16 km. Das passt zur gesendeten Aenderung 5 mi (erster Wert) bis 15 mi (aktuell), bleibt aber eine Rechnung, das Pack nennt seinen Ausgangswert nicht. Das Pack hat Zeit und Strecke beim Sleep uebernommen wie bei den anderen Packs.
- Beide Erase-Versuche (v11) weiter `refused (7F), code 31`.
- SoC min/max 6000/6263, Temporisation 0 Activated. Die DTC-Listen waren im Foto nicht zu sehen.
- **Nachtrag Nutzer (22:20):** nach dem Sleep **keine DTC** mehr (P1B2081 weg), BMS state weiter **5**, alle Balancing-Zeiten leer (0). Dazu nur die ungeprueften Hypothesen aus `BEFUND_Fremdanalyse_Zoe_Balancing_22-10.md` (Totband 2 % SOC, Neuberechnung nur beim Aufwachen und Fahrtbeginn nach vollem Laden); Pack 4 steht bei SoC 60,00 bis 62,63 %.

## Nachtrag 19.11. (Bench-Pack des Nutzers, freie Anfrage `22 9216`)
Abgelesen von Claude, **Korrektur einer eigenen falschen Aussage** (ein erstes Bild zeigte im Auswahlfeld DC, die Antwort stammte aber noch von der MCPU-Abfrage):
- **DB (MCPU):** `22 9216` -> `62 92 16 00` (1 Byte, Wert 00).
- **DC (SCPU):** `22 9216` -> `7F 22 31` (NRC 0x31, requestOutOfRange). `$9216` gibt es im SCPU **nicht**. Die Zoe-Seite liest sie richtig nur auf LBC1.
- Dieses Pack nutzt NRC 0x31 fuer einen Identifier, den das Steuergeraet nicht kennt. Das stuetzt (nicht beweisend) die Annahme, dass die Routine B004 bei Pack 4 fehlt (Code 31 bei v1 und v2).
- MCPU `$9262`/`$9263`: `0.000 Ah / 0.000 h` (raw 80000000), also ein echter Wert 0, kein Lesefehler.
