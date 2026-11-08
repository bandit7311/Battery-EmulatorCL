# Alle IDs auf dem Bus zwischen LBC und EVC (Fahrlog 09.10.2026) mit Absender und Kenntnisstand - 08.11.2026

Quellen: `canmitlog_Twingofahrt.log` (PuTTY, 1613 s, 52 IDs; der Logger laesst Frames weg, Raten sind Raten IM LOG), Absender aus `teraterm.log` vom 16.10. (RX0 = vom Pack, TX1 = vom Emulator), Beschreibungen aus der Signal-Tabelle im Treiber und aus den Auswertungen vom 08.11.
Kennzeichnung: **gemessen** = im Log nachgerechnet, Rest siehe Text. Absender 'Pack' bzw. 'EVC' stammt aus dem Emulator-Log, nicht aus dem Fahrlog selbst.

| ID | Absender | Frames im Log | Rate im Log [1/s] | DLC | Was wir wissen |
|---|---|---|---|---|---|
| 0xC5 | Pack | 60692 | 37.6 | 7 | Pack. B2 = grober Strom (ca. 16 A/Zaehler, 0 A = A9/AA), folgt 0x155 um ca. 0,1 s. B0 Zaehler, B1 Pruefsumme. |
| 0xEC | EVC (wir senden es) | 57317 | 35.5 | 3 | EVC. Zaehler B1 oben, CRC-8 B2 (Poly 0x1D). Bedeutung unbekannt. |
| 0xED | EVC (wir senden es) | 60827 | 37.7 | 3 | EVC. B1 FF -> CC mit GO-Schalter. Bedeutung sonst unbekannt. |
| 0x155 | Pack | 60697 | 37.6 | 8 | Pack. Strom (B1 unten+B2, 0,25 A/Bit, raw 2000 = 0 A) und SOC (B4-5, 0,0025 %/Bit). B0 konstant 0x49, B3 konstant 0x54, B6 zwei Werte, B7 unbekannt. KEINE Spannung. Entspricht $900D auf ca. 0,1 A. |
| 0x157 | EVC (wir senden es) | 60816 | 37.7 | 8 | EVC. B1 oben Zaehler (+5). Soll-Strom: B2:B3 (Fahren, ca. 0,074 A/Zaehler, Ruhe 09 C4, |r| 0,85, laeuft ca. 0,1 s VOR dem Strom) und B5:B6 (Laden, Rampe E0 00 -> E2 24, ca. 0,25 s vor dem Strom). |
| 0x19F | EVC (wir senden es) | 60723 | 37.7 | 8 | EVC. Zaehler B3 (+5), CRC B5 (crc19f). Rampe beim Wecken. |
| 0x1A1 | EVC (wir senden es) | 60728 | 37.7 | 8 | EVC. Messwert-aehnliche Bytes, kein Zaehler, keine Pruefsumme. Bedeutung unbekannt. |
| 0x1C7 | EVC (wir senden es) | 60688 | 37.6 | 6 | EVC. Fast konstant pro Zustand. Bedeutung unbekannt. |
| 0x1C9 | Pack | 60694 | 37.6 | 2 | Pack. Zwei Bytes, Zaehler/Pruefsumme. In der Renault-CAN-Liste gleiche ID als Lenkung (EPS_R5, 10 ms). Unklar, ob dasselbe Frame. |
| 0x419 | EVC (wir senden es) | 6070 | 3.8 | 6 | Unklar (EVC lt. Emulator-Log TX1). Sechs Bytes, wechselt mit Zustand. Bedeutung unbekannt. |
| 0x423 | EVC (wir senden es) | 6087 | 3.8 | 8 | EVC. Weck-/Zustandsframe (B0 07 Ruhe/Laden, 0B Zuendung 3/Fahrt, 04 Abschaltung, 30/33 Wecken). |
| 0x424 | Pack | 6071 | 3.8 | 8 | Pack. B2 x 500 W Rekuperationsgrenze, B3 x 500 W Entladegrenze, B5 SOH, B4/B7 Min/Max-Temperatur (+40), B6 Heartbeat 55/AA. |
| 0x425 | Pack | 6073 | 3.8 | 8 | Pack. Zellmax/-min in B4-7 (10-mV-Raster, +1000 mV). |
| 0x426 | EVC (wir senden es) | 6080 | 3.8 | 8 | EVC. Zustandscodes B1/B2, B4-6 Kilometer. Bei Ladung B1 A0, B2 41. |
| 0x428 | EVC (wir senden es) | 6071 | 3.8 | 8 | EVC. Konstant 00 00 00 00 00 06 00 00 (andere Muster nach dem Wecken). Bei Ladung B5 04 / B6 8E. Bedeutung unbekannt. |
| 0x42F | EVC (wir senden es) | 6071 | 3.8 | 6 | EVC. Konstant 00 20 FF 80 0F B0. Bei Ladung B0 2F (Ruhe 00), laeuft mit dem Ladestrom mit. Bedeutung unbekannt. |
| 0x435 | EVC (wir senden es) | 6080 | 3.8 | 8 | EVC. Aendert sich mit Wecken und Tuer (B2 1D/1E, B3 EB/0B, B5 FC/00). Bedeutung unbekannt. |
| 0x436 | EVC (wir senden es) | 6080 | 3.8 | 6 | EVC. Fahrzeugalter in Minuten (B1-3), B0 80 oder AD. |
| 0x43A | Pack | 6071 | 3.8 | 6 | Pack. Zaehler/Pruefsumme-aehnlich. Bedeutung unbekannt. |
| 0x445 | Pack | 6079 | 3.8 | 7 | Pack. Laut Code Heartbeat (B2 = 55/AA). NEU: B5 = grober Strom (ca. 9-10 A/Zaehler, 0 A = 7E), folgt 0x155 um ca. 0,15 s (|r| 0,96). |
| 0x464 | Pack | 6079 | 3.8 | 3 | Pack. Zaehler/Pruefsumme-aehnlich. Bedeutung unbekannt. |
| 0x4AE | Pack | 207 | 0.1 | 8 | Pack. Konstant nach dem Wecken, alle 3 s. Bedeutung unbekannt. |
| 0x4AF | Pack | 206 | 0.1 | 5 | Pack. Konstant nach dem Wecken, alle 3 s. Bedeutung unbekannt. |
| 0x4F7 | EVC (wir senden es) | 6072 | 3.8 | 8 | EVC. B4 wechselt mit Zuendung 1 (B0 -> 90). Bei Ladung B3 5D-5F (Ruhe 00). Bedeutung unbekannt. |
| 0x500 | EVC (wir senden es) | 1928 | 1.2 | 5 | EVC. Fuenf Bytes, zufaellig wirkend, nur bei Zuendung an. Bedeutung unbekannt. |
| 0x511 | EVC (wir senden es) | 6075 | 3.8 | 7 | EVC. B0 04/00, B1-6 sechs Zaehler mit festen Schritten. Bedeutung unbekannt. |
| 0x588 | Pack | 6079 | 3.8 | 4 | Pack. Zaehler/Pruefsumme-aehnlich. Bedeutung unbekannt. (Schwacher Spannungs-Kandidat B1, nicht belegt) |
| 0x659 | Pack | 207 | 0.1 | 4 | Pack. Konstant nach dem Wecken (erstes Frame FF). Bedeutung unbekannt. Im Code 'Battery sends it on its own'. |
| 0x69F | EVC (wir senden es) | 607 | 0.4 | 4 | EVC. B1-3 Fahrzeug-ID (46 13 88 6F). |
| 0x6BE | Pack | 606 | 0.4 | 6 | Pack. Sechs Bytes, wechselt mit Zustand. Startet beim Zentralverriegeln. Bedeutung unbekannt. |
| 0x5A1...0x5F7 (20 IDs) | Pack | 4109 | 0.1 je ID | 8 (0x5DD: 2) | Zellspannungen: je 5 Zellen, 12 Bit, +2000 mV; 0x5DD eine Zelle; alle 3 s ein voller Satz. Reihenfolge der IDs siehe `CELL_FRAME_IDS` im Treiber (Zoe Ph2: 0x5D7 statt 0x5EC) |
| 0x18DADBF1 | EVC/Tester | 33 | 0.02 | 8 | UDS (ISO-TP): Anfrage mit Dienst 22 |
| 0x18DAF1DB | Pack | 33 | 0.02 | 8 | UDS (ISO-TP): Antwort 62 |

## Zusammenfassung Strom

- Messwert aus dem Sensor: nicht auf diesem Bus sichtbar. DID $900D (`Wxx_bat_i_10ms_out`, 'CAN value') = 0x155 auf ca. 0,1 A (27 Paare, teraterm.log 16.10.).
- Vom Pack abgeleitete Stromfelder: 0x155 (fein), 0x0C5 B2 (grob, ca. 0,1 s spaeter), 0x445 B5 (grob, ca. 0,15 s spaeter). Keines laeuft dem Strom voraus.
- Vom EVC gesendete Sollwerte: 0x157 B2:B3 (Fahren) und B5:B6 (Laden); laufen dem Strom voraus. Zustandsbytes bei Ladung: 0x42F B0, 0x4F7 B3.
- Spannung: nicht in 0x155. Quelle im Treiber: Summe der Zellspannungen. $9005 'Pack Voltage (CAN value)' existiert; welches Frame sie traegt, ist offen.

## Noch unerklaert (Pack): 0x1C9, 0x43A, 0x464, 0x588, 0x659, 0x6BE, 0x4AE, 0x4AF; (EVC): 0x435, 0x1A1, 0x1C7, 0x419, 0x428, 0x42F, 0x4F7, 0x500, 0x511, 0xEC, 0xED

