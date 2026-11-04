# Quelle: Tabelle aus dem openinverter-Thread "Renault Zoe (Continental) Inverter Reverse Engineering" (04.11.)

**Herkunft (gemessen):** `https://openinverter.org/forum/viewtopic.php?t=4749`, erster Beitrag von johu (Site Admin). Der Beitrag handelt vom Zoe-**Inverter** (Continental), die Tabelle ist darin als Google-Tabelle eingebettet: `https://docs.google.com/spreadsheets/d/1p_zEgy8RqvTV79mT4HFdyKinz3l3q-WlECp_W2ymePM/preview`. Am 04.11. als Excel exportiert (Export `.../export?format=xlsx`, 308 kB, 15 Blaetter, 15.368 gefuellte Zellen). Ein Anhang des ersten Beitrags ist nur ein Foto (JPEG), nicht ausgewertet.

**Blaetter:** 2 Free Frames, 3 LBC 79B7BB, 4 EVC 7E47EC, 5 LBC2 7967B6, 6 PEB 75A77E, 7 Telematic 7CA7DA, 8 Park Prox 74E76E, 9 Air Conditioning 744764, 10 Airbag 752772, 11 UCH 745765, 12 UPC 74D76D, 13 ABS 740760, 14 PAS 742762, 15 IP 743763, Battery. Ausgewertet sind nur **Battery** und **3 - LBC**; als CSV liegen sie neben dieser Datei (`QUELLE_Zoe_Continental_Tabelle_04-11/`). Die uebrigen 13 Blaetter sind nicht gelesen.

## 1. Blatt "Battery" ("Battery connector CanBus 500kbps")
Bitnummern wie in der Tabelle (Bit n = Bit n des Bytes, Byte 0 = Bits 0 bis 7). Die Kuerzel sind in der Tabelle **nicht erklaert**.

| Frame (Periode ms) | Bits | Kuerzel / Name in der Tabelle |
|---|---|---|
| `0x423` (100) | B0.2-3 / B0.4-5 / B0.6-7 | ETS / APC / WUC |
| | B1.0 / B1.1-7 | WUT / "DC 12V Voltage regulation" |
| | B2.0 bis B3.5 | "Quick drop counter ? Always FFF" (14 Bit) |
| | B3.6-7 | DCDCA |
| | B4 | "LBC key (5D/B2)" |
| | B5.0-2 | BCBSR |
| | B6 | "LBC2 key (5D/B2)" |
| | B7 | "12V Voltage" |
| `0x424` (100) | B0.0-1 / 2-3 / 4-5 / 6-7 | CUV / HVBIR / HVBUV / EOCR |
| | B1.0-1 / 2-3 / 4-5 / 6-7 | HVBOC / HVBOT / HVBOV / COV |
| | B2 / B3 | "Max input power kW x2" / "Max output power kW x2" |
| | B4.1-7 | "Bat. Temperature -40 C" |
| | B5.1-7 | "Bat. Health %" |
| | B6 | "LBC key answer (55/AA)" |
| | B7.1-7 | "Max Bat.Temperature -40 C" |
| `0x425` (100) | B0.2-4 / B0.5-6 | HVBS / LBCRS |
| | B0.7 bis B1.7 | "Available kWh x10" |
| | B2.0-1 / B2.4-5 | IDA / SM1F |
| | B3.0 bis B4.5 | "HV Isolation Resistance Ohm /100" |
| | B4.6 bis B5.6 / B6.7 bis B7.7 | "Highest Cell V x100 -1" / "Lowest Cell V x100 -1" |
| `0x426` (100) | B1.0-2 / B1.3-4 | OT / WUA |
| | B2.6-7 | PRS |
| | B3 bis B5 | "Odometer km" (24 Bit) |
| `0x445` (100) | B0.0-1 / B1.0-1 / B1.2-3 / B1.4-5 / B1.6-7 | RTS / SM1F / HVBF1 / HVBF2 / HVBPC |
| | B2 | "LBC2 key answer (55/AA)" |
| | B3.0-6 / B3.7 bis B4.7 / B5.0 bis B6.0 | "Max Bat.Temperature" / "Highest cell voltage x0.01 +1V" / "Lowest cell voltage x0.01 +1V" |

Ohne Beschriftung (nur als Frame genannt): `0x0155`, `0x0156`, `0x0157`, `0x0196`, `0x019F`, `0x01A1`, `0x01C7` (10 ms), `0x0422`, `0x0428`, `0x0429`, `0x042C`, `0x0435`, `0x0436`, `0x04F7`, `0x0500`, `0x0511` (100 ms), `0x0659` (3000 ms), `0x069F` (1000 ms), dazu `0x075A`, `0x077E`, `0x079B`, `0x07BB`.

### Vergleich mit unserem Code und den Notizen
- **Schon so im Code (gelesen):** Die `0x424`-Flags bis COV; `LB_EOCR` ist `0x424` Byte 0, Bits 6 und 7 (wie in der Tabelle). Das Schluessel-Wechselspiel `0x5D`/`0xB2` in `0x423` Byte 4 und 6 (`RENAULT-TWINGO-GEN1-BUS.h` Zeilen 271 bis 278, `BATTERY.cpp` Zeile 3474); die Tabelle nennt es "LBC key" und "LBC2 key", die Antwort `0x55`/`0xAA` steht in `0x424` Byte 6 und `0x445` Byte 2. Kilometerstand in `0x426` Byte 3 bis 5.
- **Neu: Namen fuer bisher unbenannte Bits** (siehe Tabelle: ETS, APC, WUC, WUT, DCDCA, BCBSR, OT, WUA, PRS, `0x425`-Felder, `0x445`).
- **Hinweise aus unseren eigenen Tabellen (Schluesse, nicht belegt):**
  - `0x423` B0: `0x07` in den Zustaenden zu, wach, Zuendung 1; `0x0B` ab Zuendung 3, GO, Fahrt (`BUS.h` 271 bis 278). Das ergibt fuer **ETS** (Bits 2-3) den Wert 1 bzw. 2: passt zu einem Zustandsfeld. Bedeutung des Kuerzels unbekannt.
  - `0x423` B1 Bit 0 (**WUT**): in unseren Tabellen wechselt B1 zwischen `0x1E`/`0x1F` und `0x34`/`0x35`, nur Bit 0 aendert sich. Ob WUT die Wecken-Art ist (Frage des Nutzers zu `0x350` WakeUpType), ist **nicht belegt**; `$3302` im EVC war nach den Messungen immer `00`.
  - `0x426` B1: die Impulse `0x08` der Wecken-Folgen passen zu **WUA = 1**, das `0x10` am Ladeende (Bau 29d, nicht gebaut) zu **WUA = 2**.
  - `0x423` B5 (bei uns `0x20`, `0x80`, `0xA0`): nur Bits 0-2 sind benannt (BCBSR), die Bits 5 und 7 bleiben unbenannt.

## 2. Blatt "3 - LBC - 79B7BB" (und "5 - LBC2 - 7967B6")
- **Gemessen (Inhalt):** Anfragen `0x21 <Code>` (KWP) auf `0x079B`, Antworten auf `0x07BB` mit Flow Control (`30 01 ...`). Codes mit Beschriftung: 01 (Strom), 03 (Gesamtspannung, Strom, Batterietemperatur, "Real SOC x10000", hoechste und niedrigste Zelle, Fehler, LBC-Status), 04 (Temperatursensoren 1 bis 4, Geschwindigkeit), 05 (Referenzspannungen, 12 V, A/D-Werte), 06 ("Quick-Drop", Vehicle ID 00 bis 09), **07 "Cell balancing shunts"** (C1 bis C8 ... je Bit), 09 (Referenz beim Wecken), **30 bis 34 "Mission History Data"**, 41 und 42 (Zellspannungen), **61 "Health"** (Innenwiderstand, Ah-Degradation, Pack-Gesundheit, Kilometerstand, Gesamtenergie), **66 Zaehler** (Quick Charge, Normal Charge, Full Charge, Partial Charge, Bolt meter, HV connector), 67 Fahrhistorie, 80 bis 84 Kennungen (Teilenummern, Software, Rueckverfolgbarkeit), 90 bis 93.
- **Vergleich (Schluss, nicht geprueft):** Unsere Notizen sagen, dass dieser Pack auf den Weg `0x79B`/`0x7BB` **nicht** antwortet, sondern nur auf die erweiterte UDS-Abfrage `0x18DADBF1`. Die KWP-Codes zeigen deshalb nur, **welche Werte es gibt**. Moegliche Gegenstuecke bei uns (nicht abgeglichen): "Mission History" zu den Missionslisten `0x9275`/`0x9276`, "Cell balancing shunts" zu `0x912B`, "Health" zu `0x91CF`/`0x9243`, Zaehler zu `0x9210`.

## 3. Offen
- Die Kuerzel (ETS, APC, WUC, WUT, DCDCA, BCBSR, OT, WUA, PRS, HVBS, LBCRS, IDA, SM1F, RTS, HVBF1, HVBF2, HVBPC) sind in der Tabelle nicht ausgeschrieben; die Bedeutung ist zu klaeren (z. B. aus der Renault-Datenbank, die schon in `BEFUND_Echter_Bus_LBC_EVC_09-10.md` benutzt wurde).
- Die uebrigen 13 Blaetter (EVC 7E47EC, PEB, UCH, ...) und das Foto sind nicht ausgewertet.
- Der Thread betrifft den Inverter, nicht den Pack: der Nutzen fuer den Emulator liegt in den Bitnamen von `0x423`, `0x426`, `0x445` und in der Liste der KWP-Werte.
