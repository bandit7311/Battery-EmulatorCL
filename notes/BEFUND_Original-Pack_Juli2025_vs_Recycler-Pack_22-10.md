# Befund: Original-Pack des Twingo (24.07.2025) gegen Recycler-Pack (Auto, Oktober 2026) (22.10.2026)

Quelle: `2ce779a2-dumpies.zip` (7 Dateien, Unix-Zeit 24.07.2025). Nutzer (22.10.): Das ist der Pack, der frueher im Twingo verbaut war, der Original-Pack. Dekodiert mit `RBMS_MCPU_RL_06xx_V1.7` (22d72d9b); volle DID-Liste: `DID_Liste_Original-Pack_Dump_24-07-2025.md`.
Kennzeichnung: **gemessen** = steht im Dump, **Schluss** = abgeleitet.

## 1. Dateien im ZIP
| Datei | Inhalt | Nutzen |
|---|---|---|
| `1753382968_12030.txt` (18:49 UTC), `1753395419_12030.txt` (22:16 UTC) | MCPU-Dump, 205 DIDs, F191 = `293A01958R` | ja |
| `1753394411_12032.txt` | anderes Steuergeraet (`22 50 xx`, KWP `21 80/81`, Text `VF1AH000266883164`) | gering |
| `1753382388_12492.txt` | nur Anfragen, keine Antworten | nein |
| 3 x `...BMS453...txt` | nur Anfragen, keine Antworten | nein |

## 2. Dass es der Original-Pack ist (gemessen / Schluss)
- **Gemessen:** `925E` Fahrzeug-ID = `13 88 6F` (1280111) = die ID, die wir in `0x69F` senden. `925F` Fahrzeug-km = 83.214 = `91CF` Pack-km; `9261` = `91C1` = 2.344.330 min.
- **Schluss:** Am 04.11.2025 hatte der Pack im Auto `924F` = 95,12 Ah / `9250` = 163,91 h. Zaehler laufen nur aufwaerts, der Original-Pack stand am 24.07.2025 schon bei 281,42 Ah / 479,57 h. Zwischen Juli und November 2025 wurde also der Pack gewechselt (Recycler-Pack).

## 3. Vergleich
| Groesse | Original-Pack (24.07.2025) | Recycler-Pack im Auto (Okt. 2026) |
|---|---|---|
| `901B` BIN | `295108369RTY008870FFFFFFF` | `295108369RTY032271FFFFFFF` |
| `9003` SOH | 85,73 % | 94,81 % (MCPU 11.10.) |
| `91C1` Packzeit | 2.344.330 min = 4,46 Jahre | 2.187.978 min (11.10.) = 4,16 Jahre |
| `91CF` Pack-km | 83.214 km | 24.899 km |
| `9243` / `9245` / `9247` | 12.953 / 15.506 / 3.074 kWh | 3.735,7 / 4.429,1 kWh (Ladelog 1, 11.10.) |
| `9251` / `9252` (Schlaf) | 281,42 Ah / 479,57 h | 130,00 Ah / 222,11 h (11.10.) |
| `924F` / `9250` (gesamt) | nicht im Dump | gleich `9251` / `9252` |
| `9262` / `9263` (Wake Up) | 0 / 0 | 0 / 0 |
| `9259` BMS-Modus | 02 | siehe andere Notizen |
| SOC, Zellen | SOC 98,92 %, 403,4 V, Spreizung 10,7 mV (18:49) und 11,7 mV (22:16) | -- |

## 4. Folgerungen
- **Gemessen:** Zwischen den beiden Dumps (3,5 h, SOC 98,9 %, Spreizung 11 mV) blieben `9251`, `9252`, `9262`, `9263` unveraendert. **Schluss:** unter der Schwelle von rund 24 mV (Zoe-Dokument) wird nicht balanciert.
- **Schluss:** 281,42 Ah / 479,57 h = 0,587 A = rund 5,9 Zellen zu 100 mA im Mittel; stuetzt die 100-mA-Annahme (Zaehler summiert vermutlich ueber Zellen).
- **Gemessen:** Original- und Recycler-Pack haben **nur im Schlaf** gezaehlt, der Wake-Up-Zaehler steht bei beiden auf 0. Ein Pack im Auto balanciert und zaehlt also im Schlaf. Der Bench-Pack hat seit dem NVROL-Reset alle Zaehler auf 0.
- **Gemessen:** Die ID `13 88 6F` im Bench-Pack gehoert zum Auto.
