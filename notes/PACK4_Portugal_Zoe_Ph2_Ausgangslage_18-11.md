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
