# Befund Balancing-Zaehler und Adressen fuer die Advanced-Seite (17.10.2026)

Nur eine Notiz, nichts gebaut. Die Advanced-Seite uebernimmt davon nur nach "bauen".

## 1. Balancing-Zaehler im Pack (MCPU, Anfrage 22 xx xx an 0x18DADBF1)
Quelle: RBMS_MCPU_RL_06xx_V1.7 (22d72d9b). Alle sechs DIDs: 4 Byte, vorzeichenbehaftet, Wert = Rohwert * 1/1024 + 2097152.
| DID | Name | Einheit | 04.11.2025 13:17 (Auto) | 06.10.2026 (AUTO) | 11.10.2026 (AUTO) |
|---|---|---|---|---|---|
| $924F | Total cumulated capacity balanced (Zxx_bal_chg_tot) | Ah | 95,12 | 127,97 | 130,00 |
| $9250 | Total balancing time (Zxx_bal_time_tot) | h | 163,91 | 219,28 | 222,11 |
| $9251 | ... in sleep mode (Zxx_bal_chg_tot_slp) | Ah | 95,12 | 127,97 | 130,00 |
| $9252 | balancing time in sleep mode | h | 163,91 | 219,28 | 222,11 |
| $9262 | ... Wake Up mode (Zxx_bal_chg_tot_wk) | - | 0 | 0 | 0 |
| $9263 | balancing time, Wake Up mode (Zxx_bal_time_tot_wk) | h | 0 | 0 | 0 |
Rohbytes 11.10.: 924F/9251 = 80 02 08 00, 9250/9252 = 80 03 78 74, 9262/9263 = 80 00 00 00.
- Befund: In allen 13 MCPU-Dumps sind Total = Sleep-mode, Wake-Up-Zaehler = 0. Zwischen 06.10. und 11.10.2026 +2,03 Ah und +2,83 h: der AUTO-Pack hat im Schlaf balanciert (nicht der Bench-Pack, siehe Korrektur unten).
- Schlussfolgerung (nicht geprueft): Balancing laeuft im Schlafmodus und ist dann fuer uns nicht lesbar; wach gelesene 0x912B-Bits sind deshalb meist 0. Offen, ob die Zaehler nur bei Schlaf-/Aufwachuebergaengen fortgeschrieben werden (wie $9261) - das erklaert dann auch die wach gesehenen Bits (25.09., ESP-Log SOC 97 %).
- Test: vor und nach mehreren Stunden Stille die sechs DIDs lesen, dazu Min/Max der 96 Zellen (22 90 08/0A bzw. Zellabfragen). Waechst $9252, schrumpft die Spreizung?

## 2. Weitere Balancing-DIDs
- $912B Balancing switches status (Wtx_bal_chg_act): 35 Byte Antwort, Bytes 21-32 = 96 Bit, Bit 1 = Balancing, erstes Bit (Byte 21 MSB) = Zelle 96 (bestaetigt an Zellspannungen).
- BMS453 (LBC EV, 11 Bit 0x79B/0x7BB, in x07 gelistet, antwortet bei unserem Pack nie): Anfrage 21 16 -> 61 16 + 96 Byte (1 pro Zelle, 1 = "balancing needed") + Byte 99 BBALU_BAL_ACTIVE + Byte 100 BBALU_U_Target (Zielspannung). Anderes Steuergeraet; nur als Hinweis, dass aeltere BMS mit Zielspannung balancieren.

## 3. Adressen (Anfrage -> Antwort)
| Steuergeraet | Anfrage-ID | Antwort-ID | Quelle |
|---|---|---|---|
| MCPU (LBC HEV 29b, DB) | 0x18DADBF1 | 0x18DAF1DB | Code BATTERY.cpp:187 |
| SCPU (LBC2 29b, DC) | 0x18DADCF1 | 0x18DAF1DC | Code BATTERY.cpp (case 0x18DAF1DC) |
| EVC 29b | 0x18DADAF1 | 0x18DAF1DA | Merkzettel Zeitwerte 33BE |
| EVC 11b (H)EVC | 0x7E4 | 0x7EC | mod_ddt-Liste (Screenshot 17.10.) |
| LBC (EV) 11b BMS453 | 0x79B | 0x7BB | antwortet nie |
| PEB (Inverter) | 0x75A | 0x77E | mod_ddt-Liste |
| UCH (BCM) | 0x745 | 0x765 | mod_ddt-Liste |
| BCB (Bordlader, JB2-XML) | 0x792 | 0x793 | mod_ddt-Liste |
| VCCU (Sacha_6300_531) | 0x719 | 0x739 | mod_ddt-Liste, XML fehlt |
| Klimabox | 0x744 | 0x764 | mod_ddt-Liste |

## 4. Vorschlag Schnellknoepfe fuer die Advanced-Seite (Ziel 0 = MCPU, 1 = SCPU; Format twingoQuick('22 92 4F',0))
Bisher vorhanden: DTC 19 02 09, Time 22 92 61, State 22 92 59, Relay 22 92 5C, Km 22 92 5F/62 (BATTERY.cpp:3605 ff.).
Vorschlag neu (Klartext in der Antwort: Wert = Rohwert/1024 + 2097152, Einheit je DID):
- Balanced Ah 22 92 4F | Balancing h 22 92 50 | Balanced sleep Ah 22 92 51 | Balancing sleep h 22 92 52 | Wake Ah 22 92 62 | Wake h 22 92 63 (MCPU)
- Balancing 22 91 2B (MCPU), Klartext: Anzahl gesetzter Bits, Zellnummern
- Mode 22 92 70 (BMS-Statusverlauf, 32 Eintraege: 4 Non Isolated Charge, 6 Driving), 22 92 6D (Schreib-Freigabe-Bit, nur lesen), 22 92 5E (Fahrzeug-ID), 22 91 C1 (Pack-Zeit)
- EVC als drittes Ziel (0x18DADAF1): $33C2, $3567, $33BE, $3201, $3202, $3444 (Auto-Wecken-Test, am Bench nicht sinnvoll, da wir das EVC sind)
Die komplette DID-Liste mit Namen, Skalierung und letztem Dumpwert: DID_Liste_MCPU_Dump_11-10.md, DID_Liste_SCPU_Dump_11-10.md (und EVC_Dump_a80d130b_dekodiert.md, INV_Dump_5a971c55_dekodiert.md).

## KORREKTUR 17.10.2026 (Fehler in dieser Notiz)
- Alle 13 MCPU-Dumps (04.11.2025, 06.10., 11.10.2026) und die SCPU-Dumps sind vom AUTO-Pack, nicht vom Bench-Pack. Belege: $925F = 85465 km (04.11.2025, = EVC $2006), 92677 km (06.10.), 92785 km (11.10.); $9261 folgt der BCM-Zeit; Bench zeigt $925F = 19400 km (aus unserem 0x426). SCPU $9262 = 92677/92785 km.
- Die Tabelle in Abschnitt 1 gilt deshalb fuer das AUTO. Dort laeuft der Zaehler: +2,03 Ah und +2,83 h zwischen 06.10. und 11.10.2026, alles im Schlafmodus (Wake-Zaehler 0).
- Bench-Pack: Seit dem NVROL-Reset (23./24.09.) sind alle Lebensdaten und die Balancing-Zaehler 0 bzw. 80000000 (gemessen 28.09., 29.09., 01.10.; Nutzer 09.10. und 17.10.), siehe WERTE_Bench-Akku_vor_NVROL-Reset.md. Vor dem Reset (05.08.): 45,0 Ah / 112,5 h, alles im Schlaf (laut Notiz, nicht neu geprueft).
- Folge: Am Bench wurde Balancing in 0x912B gesehen (25.09. Zellen 85-96; ESP-Log mit 76 Bits, SOC 97 %), die Zaehler $924F/$9250 blieben aber 0. Die Zaehler laufen am Bench-Pack also nicht, wie auch $91C1/$9261/$91CF seit dem Reset stehen. Sie taugen am Bench nicht als Nachweis fuer Balancing.
