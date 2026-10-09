# Mitschnitt des echten Busses zwischen LBC und EVC (09.10.2026)

Datei: `22aaf176-canmitlog.log` (PuTTY-Log, Kopfzeile 2026.10.09 19:04:06, Kanal RX4, 1.613 s, 642.256 Frames, 52 IDs).
Aufgezeichnet im Auto zwischen Akku (LBC) und EVC, ohne Eingriff des Emulators. Der Logger lässt Frames weg: 10-ms-Frames erscheinen im Abstand von etwa 25 ms, 100-ms-Frames im Abstand von etwa 251 ms.
Kennzeichnung: **gemessen** = steht im Log, **Schluss** = gefolgert, **Annahme** = nicht belegt.

## 1. Das Wichtigste

- **Gemessen:** Auf diesem Bus gibt es **kein `0x350`**. Das 0x350-Alter erreicht den LBC auf diesem Bus nicht. Unsere früheren Logs (02.10., 04.10., Ladelog) stammen vom Fahrzeug-CAN.
- **Gemessen:** Das echte `0x436` trägt das **Fahrzeugalter in den Bytes 1-3**: `80 2D 50 0E 00 00`, Bytes 1-3 = `2D 500E` = 2.969.614 min, steigt im Log auf `…26` (+1 pro Minute, Abstand 58 bis 66 s). Epoche 15.02.2021 11:13:08 UTC: 2.969.614 min = 09.10.2026 16:47 UTC.
  - Byte 0 = `80`, bei Fahrt `AD`. Bytes 4 und 5 = `00 00`.
  - Unser Emulator sendet `86 14 [Zähler] FF DC`: Byte 0 und Bytes 4-5 weichen ab. Die SCPU nimmt Bytes 1-3 trotzdem (gemessen am Bench, siehe `BEFUND_SCPU_0436_Bench_08-10.md`). Ob die MCPU Byte 0 oder die Bytes 4-5 prüft, ist **Annahme**.
- **Gemessen:** `0x426` Bytes 4-6 = Kilometer als **16 Bit** (`6A 70 00` = 27.248 km; bei 1.483 s `6A 71`). 92.678 km + 106 km = 92.784, modulo 65.536 = 27.248.
- **Gemessen:** Das echte `0x155` ist am Anfang jedes Zyklus ungültig (`FF`, Strom 4095, SOC 163,8 %), danach gültig. Der Filter für `0x155` im Emulator ist damit richtig.

## 2. Zuordnung der Aktionen (Protokoll des Nutzers) zu Frame-Wechseln

Null der Protokollzeit = etwa **69,5 s im Log** (aus Zündung Stufe 1 bei 99 s und Zündung aus bei 340 s bestimmt).

| Protokollzeit | Aktion | Wechsel |
|---|---|---|
| 0:15 | Zentralverriegelung | `0x426` B1 `00→08`, B2 `02→06`; `0x423` B0 `30→33→07`, B3 `FF→FD`, B5 `E0→80`; `0x435` B3 `EB→0B`, B5 `FC→00`; `0x19F` B0/B1 `FF→00`; `0x6BE` startet; `0x42F`, `0x4F7`, `0x1C7`, `0x0C5`, `0x428` stellen sich um |
| 0:18-0:20 | Tür auf | `0x426` B1 `08→00`; `0x423` B5 `80→A0`; `0x6BE` B2 `04→01` |
| 0:30 | Zündung Stufe 1 | `0x426` B1 `00→60`, B2 `06→67→65`; `0x423` B3 `FD→FE`, B5 `A0→80`; `0x4F7` B4 `B0→90` |
| 2:10 | Stufe 3 | `0x426` B2 `65→69`; `0x423` B0 `07→0B`; `0x19F` B0/B1 `00→A0`; `0x500` startet; `0x511` B0 `04→05→00` |
| 2:20 | GO | `0x426` B2 `69→61`; `0x0ED` B1 `FF→CC` |
| 2:23 | Bremse | `0x426` B2 `61→69` |
| 2:28 | Drive | `0x426` B1 `60→70` |
| 3:34 | Rekuperieren | `0x0C5` B4 `07→08→07` |
| 4:30 | Zündung aus | `0x426` B1 `70→60→00`, B2 `69→65→05`; `0x423` B0 `0B→07`, B3 `FE→FD`, B5 `80→A0`; `0x0ED` B1 `CC→FF`; `0x500` B0 `02→00` |
| 4:32-4:35 | Tür auf | `0x426` B2 `05→07→06`; `0x435` B2 `1E→1D→1E`; `0x19F` B0/B1 `A0→00` |
| 4:40 | Abschließen | etwa 2 min später (470 s): `0x426` B2 `06→02` |

Die zwei weiteren Zyklen im Log (566 s, 1.355 s) sind laut Nutzer dieselbe Fahrt zurück, ohne Aufzeichnung der Aktionen. Der Zyklus bei 1.355 s zeigt dieselbe Folge wie der erste. Der Zyklus bei 566 s hat keine Zündung (kein `60` in `0x426` B1): **Annahme** Tür oder Verriegelung.

Daraus (**Schluss**): `0x426` B2 ist der Zustand (`02` zu, `06` offen/geweckt, `67/65` Zündung 1, `69` Zündung 3, `61` GO, `05/07` beim Ausschalten), B1 = `08` Wecken, `60` Zündung, `70` Fahrt. `0x0ED` B1 ist der GO-Schalter.

## 3. Frames dieses Busses (Byte-Statistik über das ganze Log)

`=xx` = konstant, Zahl = verschiedene Werte.

| ID | Frames | Byte-Statistik |
|---|---|---|
| `0x436` | 6.080 | 42 / `=2D` / `=50` / 16 / `=00` / `=00` |
| `0x426` | 6.080 | `=00` / 4 / 8 / `=01` / `=6A` / 2 / `=00` / `=40` |
| `0x423` | 6.087 | 5 / 24 / `=FF` / 3 / 3 / 3 / 3 / 48 |
| `0x435` | 6.080 | `=FF` / `=FF` / 2 / 2 / `=33` / 2 / `=00` / `=00` |
| `0x19F` | 60.723 | 73 / 73 / 41 / 256 / 2 / 256 / 4 / `=FE` |
| `0x1A1` | 60.728 | 103 / 17 / 124 / 13 / 33 / 10 / 64 / `=20` |
| `0x0ED` | 60.827 | 12 / 2 / 2 |
| `0x157` | 60.816 | 256 / 32 / 7 / 256 / `=FF` / `=E0` / 2 / 2 |
| `0x0EC` | 57.317 | 13 / 32 / 162 |
| `0x0C5` | 60.692 | 16 / 256 / 16 / 4 / 4 / 29 / 27 |
| `0x1C7` | 60.688 | 2 / 2 / `=00` / 2 / 2 / `=00` |
| `0x1C9` | 60.694 | 64 / 64 |
| `0x69F` | 607 | `46 13 88 6F` (alle 2,5 s) |
| `0x500` | 1.928 | 5 Bytes, ab 114 s (ca. 15 s nach Zündung Stufe 1) bis Zündung aus |
| `0x6BE`, `0x511`, `0x4F7`, `0x42F`, `0x419`, `0x428` | | siehe Log |

Akku-seitig (**Annahme**, nach Formaten): `0x155`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, `0x5A1` bis `0x5F7`, `0x659`, `0x4AE`, `0x4AF`.
Die Frames `0x1A1`, `0x0ED`, `0x157`, `0x0EC`, `0x0C5`, `0x1C7`, `0x1C9` hat der Emulator nicht.

## 4. UDS des EVC beim Start (gemessen)

Bei jedem Zündzyklus, 4 bis 8 s nach dem Aufwachen, in dieser Reihenfolge: `9243`, `9245`, `91CF`, `91C1`, `901B` (Mehrfach-Antwort mit der Teilenummer), `92C1` (NRC 0x31), `92C2` (NRC 0x31); `92C3` bei 1.483 s (NRC 0x31).
`91C1` in den drei Lesungen: `21 58 08`, `21 58 16`, `21 58 1A`. Das Fahrzeugalter in `0x436` stieg im gleichen Zeitraum um +8 und +11. Pack-Zeit und Fahrzeugalter laufen nicht 1:1.

## 5. Folgerungen und offene Punkte

- **Schluss:** Die Quelle der Akku-Zeit ist am Bench `0x436` Bytes 1-3 auf diesem Bus, nicht `0x350`. Das erklärt, warum 0x350 mit Alter bei der MCPU nie etwas bewegt hat und warum die SCPU `0x436` nimmt.
- **Offen:** Warum nimmt die MCPU unser `0x436` nicht? Kandidaten: Byte 0 (`86` statt `80`/`AD`), Bytes 4-5 (`FF DC` statt `00 00`), fehlende Begleitframes (`0x426` im echten Format, `0x423`, `0x435`, `0x0ED`, `0x19F` mit A0), fehlender Zustand wie Wecken/Zündung/GO.
- **Vorschlag (noch nicht gebaut):** Ein Emulator-Modus "EVC wie im echten Bus" mit den Frames aus Abschnitt 3 und den Zuständen aus Abschnitt 2.

## 6. Nachtrag 09.10. (spaeter): Korrektur, Zustaende, Schlaf, neue Frames

Zeiten = Zeitstempel im Log (Sekunden). Kennzeichnung: **gemessen** / **Schluss** / **Annahme**.

### 6.1 Korrektur zu Abschnitt 0 (Logger)
- **Gemessen:** Die Aussage "Logger zeigt 10-ms-Frames im Abstand von ca. 25 ms" ist falsch. `0x155`: 60.697 geloggt, ca. 77.500 erwartet in aktiver Zeit, Abstand meist 10-19 ms.
- Es fehlen ca. 16.800 Frames (22 %) in 88 kurzen Loechern (Mittel 1,9 s, zusammen 169 s). Lange Pausen (753 s: 471-566, 748-1355, 1531-1553, 1557-1586) sind echte Busstille.
- Zykluszeiten im Log: Start 84,5 / 565,4 / 1354,6 / 1553,2 / 1585,7 s.

### 6.2 Zustand 05 (9259)
- **Gemessen:** Das Log enthaelt keine 9259-Abfrage (EVC liest nur 9243, 9245, 91CF, 91C1, 901B, 92C1-92C3). Welcher Zustand 9259=05 bringt, ist aus dem Log **nicht ableitbar**.
- Zustandsfolge des EVC (gemessen, 1. Zyklus): zu `0x426` B1/B2 `00/02` -> Wecken `08/06` (0x423 B0 `30->33->07`) -> Zuendung 1 `60/67->65` (99,4 s) -> Zuendung 3 `60/69` (201,5 s; 0x423 B0 `0B`, 0x19F A0, 0x500 startet) -> GO `60/61` (209,2 s; 0x0ED B1 `CC`) -> Fahrt `70/69` (217,4 s) -> aus `00/05->07->06` (340 s).
- Der Bench sendet diese Zustaende nie. Test: Zustaende nacheinander senden, nach jedem Schritt 9259 lesen (Bauliste B).

### 6.3 Schlafimpuls
- **Gemessen:** Bei jeder Stille (471,0 / 748,7 / 1530,6 s) enden alle Frames von Akku und EVC innerhalb ca. 30 ms. Kein besonderes Akku-Frame in den letzten 6 s davor gefunden.
- **Gemessen:** Zyklus 1: `0x426` B2 `06->02` bei 469,8 s, Stille 471,0 s. Zyklus 2: B2 `02` bei 698,0 s, Stille 748,7 s; am Ende 0x423 B0 `07->04` (747,8 s), 0x435 B3/B5 `EB/FC` (748,2-748,5 s). Zyklus 3: Stille bei B2=`06` (1530,6 s und 1556,7 s), also ohne `02`.
- **Schluss (nicht belegt):** Der Schlafimpuls ist auf dem Bus kein Frame, eher Wegfall von Versorgung/Weckleitung. B2=`02` ist Merkmal "zu", nicht Ursache.

### 6.4 Balancieren
- **Gemessen:** Keine Balancing-Meldung und keine UDS-Abfrage dazu im Log. Der Zoe-Code liest den Balancing-Status per UDS; `0x4AE`, `0x4AF`, `0x5A1...0x5F7` stehen dort nur in der Liste ignorierter Frames.
- 0x4AE/0x4AF/0x659 erscheinen 3 s nach dem Wecken mit konstanten Werten (`D2 BE 32 A0 A0 50 96 7C` / `95 45 4B 35 C0` / `67 71 A1 02`) und verschwinden beim Abschalten; die uebrigen 0x5xx kommen alle ca. 7 s.

### 6.5 Frames, die im Twingo-Code und in den Notizen bisher fehlten (Suche im Repo)
- Wohl vom EVC (**Annahme**, nur nach Format): 0x157, 0x1A1, 0x0C5, 0x0EC, 0x0ED, 0x1C7, 0x1C9, 0x428, 0x42F, 0x435, 0x4F7, 0x500, 0x511, 0x6BE.
- Wohl vom Akku (**Annahme**): 0x419, 0x43A, 0x464, 0x588, 0x659.
- Nur im Zoe-Gen2-Code, nicht im Twingo-Code: 0x4AE, 0x4AF, 0x5A1, 0x5AC, 0x5AD, 0x5B4, 0x5B5, 0x5B7, 0x5C9, 0x5CB, 0x5CC, 0x5D6, 0x5D9, 0x5DD, 0x5EA, 0x5EC, 0x5ED, 0x5F0, 0x5F1, 0x5F2, 0x5F4, 0x5F7.
- **Gemessen:** Beim Wecken kommen zuerst 0x0EC (+0,05 s) und 0x423 (+0,11 s), ca. 0,17 s nach Zyklusstart alle uebrigen Frames.

## 7. Nachtrag 09.10. (Abend): Zellspannungen, Ladelog, Logger-Marker, Gemini

### 7.1 Berichtigungen zu frueheren Abschnitten
- `0x500` beginnt bei 114 s, ca. 15 s nach Zuendung Stufe 1 (99 s), nicht erst bei Stufe 3 (Abschnitt 2 und 3 oben, Tabelle korrigiert).
- Die langsamen Frames (`0x4AE`, `0x4AF`, `0x659`, `0x5A1...0x5F7`, `0x5DD`) kommen im Median alle ca. 3 s, nicht alle ~7 s.
- **Fehlende Frames, exakt:** Der Logger schreibt Marker `[N CAN frames not printed]` (`comm_can.cpp:505-520`, Puffer voll). Log 22aaf176: 91 Marker, zusammen **240.577** nicht gedruckte Frames, gedruckt 642.256 -> ca. 27 % fehlen. Logs 02.10./04.10.: 21 bzw. 36 Marker. Die Schaetzung "16.800" betraf nur `0x155`.
- Die Logs vom 02.10./04.10. und das Ladelog (Fahrzeug-CAN) enthalten die langsamen Akku-Frames sehr wohl (`0x5A1`, `0x5DD`, `0x4AF`, `0x599`, `0x658`), aber nicht die schnellen (`0x155`, `0x424`, `0x425`, `0x445`).

### 7.2 Zellspannungen aus den Broadcast-Frames (gemessen)
- 19 Frames `0x5A1, 0x5AC, 0x5AD, 0x5B4, 0x5B5, 0x5B7, 0x5C9, 0x5CB, 0x5CC, 0x5D6, 0x5D9, 0x5EA, 0x5EC, 0x5ED, 0x5F0, 0x5F1, 0x5F2, 0x5F4, 0x5F7` je 5 Zellen + `0x5DD` 1 Zelle = **96 Zellen**.
- Je Frame: Z1 = `(B0<<4)|(B1>>4)`, Z2 = `((B1&0F)<<8)|B2`, Z3 = `(B3<<4)|(B4>>4)`, Z4 = `((B4&0F)<<8)|B5`, Z5 = `(B6<<4)|(B7>>4)`; B7 Low-Nibble immer `F`. `0x5DD`: `(B0<<4)|(B1>>4)`.
- **Spannung in mV = Rohwert + 2000** (1 mV/Bit).
- Pruefung: 196 Zyklen, Max/Min der 96 Zellen gegen `0x425` (10-mV-Raster): Max -1 mV (122x), Min -9 mV (97x) / +1 mV (33x), passt zu abgerundeten 10 mV. Faktor 1 mV/Bit nur schwach unabhaengig geprueft (Zellspreizung im Log nur wenige mV).
- **Annahme:** Zuordnung Reihenfolge der IDs -> Zellnummer nicht geprueft.
- Beispiele: 100 s (Zuendung 1) min 3537 / max 3540 / Mittel 3539 mV, Summe 339,7 V; 250 s (Fahrt) 3527 / 3531 / 3531 mV, 339,0 V.
- Die unteren Nibbles in B1/B2 sind Zellwerte, keine Statusbits (fruehere Vermutung "Dreiergruppen" widerrufen).
- Folge: Zellspannungen brauchen keine UDS-Abfrage, sie kommen als Broadcast.

### 7.3 Ladelog (Fahrzeug-CAN, BusMaster, Start 20.11.2025, Ladevorgang 10:43-10:49)
- 126 IDs, kein `0x155/0x424/0x425/0x426/0x436/0x423`.
- Mit ca. 4 s Takt (92-95 Frames): `0x4AF`, `0x504`, `0x599`, `0x5A1...0x5F7`, `0x5DD`, `0x5BD`, `0x5C6`, `0x5CE`, `0x632`, `0x658`, `0x665`, `0x66F`, `0x6A4`, `0x6A6`, `0x6A7`, `0x6A9`, `0x6AA`, `0x6B5`, `0x6F3...0x6F7`, `0x6FB`. **Schluss (nicht belegt):** Gleichtakt mit den Zellframes -> wohl ebenfalls vom Akku.

### 7.4 Gemini-Antwort geprueft (09.10.)
Widersprueche zum Code/Log:
1. `0x155` nennt Gemini EVC; im Code vom Akku (`RENAULT-TWINGO-GEN1-BATTERY.cpp:1963`).
2. `0x423`/`0x426` nennt Gemini LBC; der Emulator (EVC-Rolle) sendet sie an den Akku (`RENAULT-TWINGO-GEN1-BATTERY.h:290-318`).
3. `0x424` = Charge limits/Temperaturen/SOH und `0x425` = Zellspannungen/kWh (Code `:1984`, `:2028`), nicht SOC/HV-Spannung bzw. Power Limits.
4. `0x436` nennt Gemini LBC; gemessen Fahrzeugalter vom EVC.
Rest (Bedeutung `0x157`, `0x1A1`, ... ) ohne Fundstelle -> Annahme. Fazit: nicht hilfreich fuer die offenen IDs.

### 7.5 Absender der Frames am Bench bestimmen
- Mit "alle Zeilen aus" sendet der Akku nichts (Nutzer). Standardmaske `0x387` = `0x090`, `0x242`, `0x350`, `0x69F`, `0x53B`, `0x214` (Kommentar `datalayer_extended.h:1130`: `0x423/0x19F/0x426/0x436` aus).
- `0x423` als Weckframe: Code-Kommentar `RENAULT-TWINGO-GEN1-BATTERY.cpp:3133` (aus Zoe-Code geerbt, nicht gemessen); im Echt-Log kommt `0x423` 0,11 s nach Zyklusstart als erstes EVC-Frame. MERKZETTEL_Zeitwerte_33BE.md:275-276: Satz mit `0x423` funktionierte, Notwendigkeit nicht bewiesen. **Offen:** Kommuniziert der Akku mit nur `0x423` (ohne `0x090/0x242/0x350`)?
- Test vor Ort: (1) Standard ohne `0x423`; (2) `0x090/0x242/0x350` + `0x423`; (3) nur `0x423`; je Kommunikation (Zellspannungen, Status) notieren.
- **Logger:** `print_can_frame()` protokolliert RX und TX (`comm_can.cpp:317`, `:544`); Kanal RX = Schnittstelle*2, TX = Schnittstelle*2+1 (`RX4` = `CAN_ADDON_MCP2515`). Am Bench mit USB-Logging (`CANLOGUSB`) sind alle RX-Zeilen Akku-Frames, alle TX-Zeilen unsere. Damit ist der Absender messbar.

## 8. Renault-CAN-Datenbank (09.10., Datei `ca9a909f-CAN_MESSAGE_SET_C1A_Q4_2017_ALL_MESSAGE_LIST_OFFICIAL_01_12_2017…xml`)

Die Datei ist die offizielle Renault-Nachrichtenliste (DDT2000, Plattform C1A, Projekte x10Ph2/xFBPh2/..., 500 kbit/s). Pro Frame: Name mit Absender-Praefix (`BMS_`, `HEVC_`, `BCM_`, ...), ID (`SentBytes`), Zykluszeit (`Comment`), Signale mit Startbyte/Bit. Es ist die Datenbank des **Fahrzeug-CAN**; die schnellen Frames des Akku-Busses (0x155, 0x423-0x426, 0x436, 0x445, ...) stehen nicht darin.

### 8.1 Zellspannungen: Dekodierung durch die Datenbank bestaetigt
- `HVB_CellNNVoltage`: 12 Bit, Schritt 0,001 V, Offset 2 V = Rohwert + 2000 mV. Layout z. B. `BMS_A10` (0x5F7): Zelle 1 = Startbyte 1, Zelle 2 = Byte 2 ab Bit 4, Zelle 3 = Byte 4, Zelle 4 = Byte 5 ab Bit 4, Zelle 5 = Byte 7. Deckt sich mit der gemessenen Dekodierung (Abschnitt 7.2).
- **Berichtigung der Zellnummern:** nicht aufsteigend mit der ID. Es gilt `BMS_A10` = Zellen 1-5, `A11` = 6-10, ... `A28` = 91-95, `A29` = Zelle 96, also:
  0x5F7=A10 (Z1-5), 0x5F4=A11, 0x5F2=A12, 0x5F1=A13, 0x5F0=A14, 0x5ED=A15, 0x5EA=A16, 0x5D9=A17, (0x5D7=A18), 0x5D6=A19, 0x5CC=A20, 0x5CB=A21, 0x5C9=A22, 0x5B7=A23, 0x5B5=A24, 0x5B4=A25, 0x5AC=A26, 0x5AD=A27, 0x5A1=A28 (Z91-95), 0x5DD=A29 (Z96).
- Auf dem Akku-Bus kommt statt `0x5D7` (A18) der Frame `0x5EC`. **Annahme (durch Ausschluss):** `0x5EC` ersetzt A18 (Zellen 36-40); das gleiche 0x5D7 belegt der Twingo-Emulator als Kilometerframe.
- `BMS_A30` (0x5DC) = 8 Temperaturfuehler (`HVB_ProbeTemp01-08`); auf dem Akku-Bus nicht vorhanden.

### 8.2 Absender der Frames (Datenbank, 3000-ms-Gruppe)
- Akku (`BMS_`): 0x5A1..0x5F7, 0x5DD, 0x5DC.
- **Nicht** Akku, obwohl im selben 3-s-Takt (Berichtigung zu 7.3): 0x599 `HEVC_R32` und 0x632 `HEVC_A36` (EVC), 0x6A4 `BCM_R12`, 0x6F3 `IVI_R8`, 0x5BD `IVI_UserSetPref_A4`, 0x5C6 `UserSetPref_CANHS_R_02`.
- 0x69F `VehicleID_CANHS_R_01`, 1000 ms, Signal `VehicleID` (bestaetigt die Fahrzeug-ID-Funktion).
- `HEVC_WakeUpFrame` = **0x62B** (2 Byte, Signal `HEVC_WakeUp_Signal`). In den drei Logs (22aaf176, 364b56eb, 5c3d7daf) kommt 0x62B nicht vor.
- Nicht in der Datenbank: 0x0C5, 0x0EC, 0x0ED, 0x155, 0x157, 0x19F, 0x1A1, 0x1C7, 0x419, 0x423-0x426, 0x428, 0x42F, 0x435, 0x436, 0x43A, 0x445, 0x464, 0x4AE, 0x4AF, 0x4F7, 0x500, 0x511, 0x588, 0x658, 0x659, 0x6BE, 0x504, 0x5CE, 0x665, 0x66F, 0x6B5.
- 0x1C9 steht als `EPS_R5` (10 ms, 4 Signale, EPS = Lenkung). Auf dem Akku-Bus hat 0x1C9 DLC 2; ob es dasselbe Frame ist, ist **nicht** geklaert.

### 8.3 Weitere Quellen geprueft
- OVMS Zoe Ph2 (`vehicle_renaultzoe_ph2.zip`): liest nur Fahrzeug-CAN (`HEVC_*`, `BCM_*`, `METER_*`, ...) und UDS-Kennungen vom LBC (u. a. 9005, 9002, 9003, 9243, 9245, 9247, 9210, 9015, 9018, 91C8, 9131-913C, Zellen 9021-9083). Keine unserer offenen Akku-Bus-IDs; auch keine 9259, 925C, 9261, 91C1.
- Battery Emulator: Zoe Gen1 dekodiert 0x155/0x424/0x425/0x445; Zoe Gen2 listet 0x4AE, 0x4AF, 0x5A1-0x5F7 nur als ignorierte Frames. Die Namen "PEB Inverter" (0x19F), "EVC Power Mux" (0x426), "EVC Status" (0x436) stammen aus dem Upstream-Code (PR #2907), nicht aus Messung.
