# Befund: Gesamtauswertung der drei Auto-Logs (20.10.2026)

Quellen: `22aaf176-canmitlog.log` (Fahrlog 09.10., 1528 s), `598b197c-canmitlog.log` (Ladelog 1, 1284 s), `ab5f2c05-Ladelog_LBC_EVC.log` (Ladelog 2, 574 s; `445a1527-teraterm.log` ist byte-gleich, kein zweiter Mitschnitt).
Kennzeichnung: **gemessen** = aus den Logs gerechnet (Skripte in `notes/tools/log_ana_*.py`), **Schluss** = abgeleitet.

## 1. SOC-Bereich der Logs (gemessen, 0x155)
- Fahrlog: SOC 9,27 bis 10,04 %, 336 bis 342 V. Ladelog 1: 9,01 bis 14,98 %, Strom ca. 25 A. Ladelog 2: 39,30 bis 43,02 %, 353 bis 360 V.
- **Schluss:** Keines der Logs kommt in die Naehe von 97 % SOC. Ob der Auto-Akku dort balanciert, kann aus ihnen nicht gesehen werden. Balancing-Meldungen oder -Abfragen (912B, 924F bis 9252) kommen in keinem der drei Logs vor.

## 2. Zellspreizung (gemessen, 95 Zellen aus den Broadcast-Frames, Zelle 96 = 0x5DD hat nur 2 Byte und ist nicht eingerechnet)
- In Ruhe und beim Laden mit 25 A: Max minus Min **0 bis 8 mV** (Rasterschritt 2 mV), im Fahrlog Werte von bis zu 31 mV beim Beschleunigen und Rekuperieren (Strom -131 bis +87 A).
- Spitzenwert 229 mV im Fahrlog und 37 mV im Ladelog 1 sind Einzelwerte beim Aufwachen und werden hier nicht als Spreizung gewertet (**Schluss**).
- Zelle 96 und die Zuordnung 0x5EC = Zelle 41 bis 45 bleiben Annahme.

## 3. UDS-Anfragen der EVC (gemessen, schon in `BAULISTE.md:95` und `BEFUND_Echter_Bus_LBC_EVC_09-10.md:65` erfasst)
- Beim Wecken lesen: `9243`, `9245`, `91CF`, `92C1` (NRC 0x31), `91C1`, `901B`; im Fahrlog zusaetzlich `92C2`, `92C3` (NRC 0x31).
- Beim Eintritt in den Ladezustand: `9210`, `9215`, `91D1`, `91D2` (je 53 Byte), `92C1`.
- Im Code der Twingo-Treiberdatei kommen `9243`, `9245`, `9210` vor, `9215`, `91D1`, `91D2`, `901B`, `92C1` bis `92C3` nicht.

## 4. Unbekannte Frames (gemessen, 22aaf176 und ab5f2c05)
- `0x1C9` (2 Byte): B1 zaehlt je Frame +0x10 (hohes Halbbyte = Zaehler), B0 hat 48 bis 64 verschiedene Werte, kein CRC-8 gefunden.
- `0x0C5` (7 Byte): B0 hohes Halbbyte zaehlt +1 je Frame (Delta 0x10), B1 aendert sich fast bei jedem Frame (Pruefwert), B2 bis B6 aendern sich selten (B2 +-1, B3 +-0xC0/0x40, B5 +-8, B6 +-4).
- `0x157` (8 Byte): B1 hohes Halbbyte +5 je Frame, im Fahrlog haben B0 und B3 je 256 verschiedene Werte, im Ladelog 2 hat B6 117 (B0 und B3 konstant).
- CRC-8-Suche ueber Polynome 0x1D, 0x2F, 0x07, 0x31, 0x9B, 0x39, 0x1C, alle Anfangswerte, Ausgangs-XOR aus dem ersten Frame (`log_ana_crc8.py`): Das Verfahren findet `0x0EC` B2 mit 3000 von 3000 Frames (Kontrolle). `0x1C9`, `0x0C5`, `0x157`, `0x423` B7, `0x1A1`, `0x155` B7 ergeben **keinen Treffer** (best 4 bis 235 von 3000).
- `0x423` B7 liegt im Betrieb zwischen 0xBB und 0xEA und steigt langsam; B1 steht bei 07 zwischen 0x1E und 0x3C (Fahrlog), im Ladelog 2 zwischen 0x1E und 0x20.

## 5. Offen
- Keine Daten zu Balancing im Auto aus diesen Logs; dafuer braucht es eine Aufnahme bei hohem SOC mit Abfrage von 912B und 924F bis 9252.
- Bedeutung von B0/B1 bei 0x1C9, B1 bis B6 bei 0x0C5 und der Zaehler bei 0x157 bleibt unbekannt.

## 6. Inhalt der UDS-Antworten, die die EVC liest (gemessen; Namen aus der MCPU-XML `22d72d9b`)
Beim Wecken (Fahrlog 09.10., Ladelog 1 11.10., Ladelog 2 15.10.):

| DID | Name laut XML | Rohwert | Wert |
|---|---|---|---|
| 9243 | Cumulated energy in charge (Zxx_kwh_chg), 32 Bit, x0,001 | `00 38 FB 8F`, `00 39 00 76`, `00 39 16 35` | 3734,4 / 3735,7 / 3741,2 kWh |
| 9245 | Cumulated energy in discharge CD mode (Zxx_kwh_dch_cd), 32 Bit, x0,001 | `00 43 93 71` bis `00 43 95 2A` | 4428,7 bis 4429,1 kWh |
| 91CF | Battery mileage (Zxx_dist_pack_mem), (Roh - 0x80000000) x 0,03125 | `80 0C 28 40` / `...60` | 24.898 / 24.899 km |
| 91C1 | Pack Time Life since 1st power-up, 24 Bit, min | `21 58 08` bis `21 63 20` | 2.185.224 bis 2.188.064 min = 1517 bis 1519 Tage |
| 901B | Battery Identification Number (BIN), 25 ASCII | `32 39 35 31 ...` | `295108369RTY032271FFFFFFF` |
| 92C1, 92C2, 92C3 | XML kennt nur $92C0 (DIDs supported 92C1-92E0) | `7F 22 31` | NRC 0x31 request out of range |

Beim Eintritt in den Ladezustand:

| DID | Name laut XML | Rohwert | Wert |
|---|---|---|---|
| 9210 | Overall number of complete charge (Zxx_full_chg_ctr), 16 Bit | `00 CC` | 204 (nur Ladelog 2) |
| 9215 | Overall number of partial charge (Zxx_partial_chg_ctr), 16 Bit | `01 33`, `01 37` | 307 (11.10.), 311 (15.10.) |
| 91D1 | Charge Deplating Driving history (T°Max/BSOC), 25 x 16 Bit, min | 53 Byte (3 Byte Kopf, 50 Byte Daten) | Summe 8755 min (11.10.), 8759 min (15.10.) |
| 91D2 | Parking history (T°Max/BSOC), 25 x 16 Bit, h | 53 Byte | Summe 4202 h |

91D1 (11.10.) als 5 Zeilen: `0 0 0 0 0 / 59 109 103 2 0 / 610 1381 1610 158 0 / 611 1583 2249 229 1 / 5 18 27 0 0`. Aenderung bis 15.10.: 1610 auf 1611, 2249 auf 2250, 229 auf 231.
91D2: `0 0 0 0 0 / 7 31 38 1 0 / 287 658 825 70 0 / 378 687 1027 113 0 / 9 47 24 0 0`.
**Schluss:** 5 x 5 aus der Anzahl 25 abgeleitet; die Achsen (Temperatur- und BSOC-Stufen) stehen in den Antworten nicht.

Im Treiber vorhanden: 9243, 9245, 9210 (`BATTERY.cpp:388-399`, `BATTERY.h:574-576`), 91CF und 91C1 auf der Advanced-Seite. Es fehlen 9215, 91D1, 91D2, 901B, 92C1 bis 92C3. Alle sind Lesewerte zu Alter/Energie/Historie, keiner betrifft das Balancing.
