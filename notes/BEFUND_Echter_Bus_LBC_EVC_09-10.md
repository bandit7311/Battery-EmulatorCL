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

## 9. Wer sendet 0x423? Vergleich der beiden Busse (09.10., abends)

### 9.1 0x423 (nicht in der Renault-Datenbank; **Schluss**, nicht bewiesen: sendet das EVC)
1. **Gemessen:** Aenderungen folgen den Aktionen im Auto wie bei 0x426 (Verriegeln, Zuendung 1, Stufe 3: B0 `07->0B`, Zuendung aus: `0B->07`), siehe Abschnitt 2.
2. **Gemessen:** In den Zyklen 1-3 erscheint 0x423 0,02-0,11 s nach Zyklusstart, die Akku-Frames (0x155, 0x424, 0x425) erst ca. 0,17 s; davor nur 0x0EC (Abschnitt 6.5). In den Zyklen 4 und 5 sind die Akku-Frames zuerst da (Akku schon wach), 0x423 folgt bei +0,09 s.
3. **Gemessen:** Erstes 0x423 jedes Zyklus `30 7F FF FF FF E0 FF FF` (Startwerte "nichts berechnet"), danach `33 ...`, dann `07 1E FF FD B2 80 B2 BB`. Ein durchgereichtes Frame haette keinen Startzustand.
4. **Gemessen:** B4 und B6 wechseln zwischen `5D` und `B2`; das macht auch unser Emulator (aus dem Zoe-Gen1-Treiber, `RENAULT-TWINGO-GEN1-BATTERY.cpp:3136-3142`). Gleiche Frame-Art wie bei der Zoe.
5. **Offen:** Welches Geraet auf dem Akku-Bus tatsaechlich sendet (EVC oder anderer Teilnehmer).

### 9.2 Vergleich Akku-Bus (22aaf176) mit allen Fahrzeug-CAN-Logs (02.10., 04.10., Ladelog)
- **Gemessen:** 52 IDs auf dem Akku-Bus, davon **26 auch auf dem Fahrzeug-CAN**: `0x4AF`, `0x500`, `0x511`, `0x5A1, 0x5AC, 0x5AD, 0x5B4, 0x5B5, 0x5B7, 0x5C9, 0x5CB, 0x5CC, 0x5D6, 0x5D9, 0x5DD, 0x5EA, 0x5EC, 0x5ED, 0x5F0, 0x5F1, 0x5F2, 0x5F4, 0x5F7`, `0x69F`, `0x18DADBF1`, `0x18DAF1DB`.
- **Nur auf dem Akku-Bus (24 IDs):** `0x0C5`, `0x0EC`, `0x0ED`, `0x155`, `0x157`, `0x19F`, `0x1A1`, `0x1C7`, `0x1C9`, `0x419`, `0x423`, `0x424`, `0x425`, `0x426`, `0x428`, `0x42F`, `0x435`, `0x436`, `0x43A`, `0x445`, `0x464`, `0x4AE`, `0x4F7`, `0x588`, `0x659`, `0x6BE`.
- **Gemessen:** 0x69F ist auf beiden Bussen identisch (`46 13 88 6F`); laut Datenbank `VehicleID_CANHS_R_01` (Fahrzeug-Frame). 0x511 hat auf beiden Bussen dasselbe Format (7 Byte, B0 `04`, danach Zaehlerwerte). Richtung bei 0x511 und 0x500 unbekannt.
- **Schluss (Lesart, nicht bewiesen):** Das EVC vermittelt zwischen den Bussen: Zellframes (`BMS_`) vom Akku-Bus in den Fahrzeug-CAN, Fahrzeug-ID vom Fahrzeug-CAN zum Akku; alles Uebrige bleibt auf seinem Bus. 41 der 46 Zeilen der Simulatorliste sind reine Fahrzeug-CAN-Frames.

### 9.3 Broadcast statt UDS
- Als Broadcast verfuegbar (gemessen): 96 Zellspannungen, 0x155 (Strom, SOC), 0x424 (Limits, Temperaturen, SOH), 0x425 (Zellminimum/-maximum).
- Zeit und Zustand (9261, 91C1, 9259, 925C) kommen nach heutigem Wissen nicht als Broadcast; das Fahrzeugalter geht in Gegenrichtung (0x436, EVC -> Akku).
- **Offen:** Ob der Akku auf *irgendein* Frame hin sendet. Der Standardsatz (`0x090`, `0x242`, `0x350`, `0x53B`, `0x214`, `0x69F`) besteht fast nur aus Fahrzeug-CAN-Frames; was davon noetig ist, klaert der Test in drei Schritten (Abschnitt 7.5).

## 10. Bench-Mitschnitte des Emulators (hochgeladen 09.10.): Absender, Wecktrigger, Zellspannungen

Quellen: `cf794fb4-canlog_00-04-48.zip` (Zip-Eintraege 24.07.), `6770a7c4-canlog_06-03-23.zip` (05.08.), `406580d6-canlog_after_nvrol.zip` (25.09.). Jahr nicht in den Dateien. Format: Emulator-Log, `TX1` = wir senden, `RX0` = wir empfangen (keine Busmitschnitte). Der Chat, in dem der Zusammenhang dokumentiert war, ist fuer mich nicht lesbar.

### 10.1 Juli/August: Zoe-Gen2-Sendesatz
- Gesendet: `0x0EE` (10 ms), `0x373`, `0x375`, `0x376` (100 ms), `0x5F8`, `0x6BF` (1 s) + UDS-Abfragen. Kein `0x090/0x242/0x350/0x423/0x69F`.
- Empfangen: **nur UDS-Antworten** (`18DAF1DB`, 64 bzw. 69 Frames). **Kein** Broadcast (`0x155`, `0x424`, `0x425`, `0x5xx`).
- 24.07.: Zellwerte per UDS alle `00 00` (0 von 25), `9259 = 00`, `91C1 = 0D EE 55` (= 912.981), `9261` ohne Antwort, `925D` NRC 31, `91CF = 80 03 57 80`, `9250/9252 = 80 01 C2 29`, `9262 = 80 00 00 00`.
- 05.08.: Zellwerte per UDS 4192-4208 mV gueltig (45 von 47), `925D` NRC 31.

### 10.2 25.09. (`after_nvrol`): Zoe-Gen1-Satz, Akku sendet Broadcast (gemessen)
- Gesendet: `0x19F`, `0x423`, `0x426`, `0x436`, `0x69F`, `0x79B`, UDS-Abfragen. Daneben 29-Bit-IDs `0x4210...0x4290`, `0x7310...0x7340`, `0x4200` (**Annahme:** Wechselrichter-Schnittstelle/Pylon, nicht im Code geprueft).
- Empfangen vom **Akku**: `0x155`, `0x0C5`, `0x1C9` (10 ms), `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588` (100 ms), `0x4AE`, `0x4AF`, `0x659`, `0x6BE`, alle Zellframes `0x5A1...0x5F7`, `0x5DD`.
- **Berichtigung zu Abschnitt 3 und 9.2:** `0x0C5`, `0x1C9`, `0x43A`, `0x464`, `0x588`, `0x659`, `0x4AE`, `0x4AF`, `0x6BE` sind Akku-Frames (nicht "wohl EVC").
- Nicht vom Akku (in diesem Log nicht empfangen): `0x0EC`, `0x0ED`, `0x157`, `0x1A1`, `0x1C7`, `0x419`, `0x428`, `0x42F`, `0x435`, `0x4F7`, `0x500`, `0x511` -> passend zu EVC (**Schluss**; `0x419` koennte auch zustandsabhaengig fehlen).

### 10.3 Wecktrigger fuer den Broadcast
| Sendesatz | Akku sendet Broadcast |
|---|---|
| Zoe-Gen2-Satz (10.1) | nein |
| nur `0x350` (Code-Kommentar `RENAULT-TWINGO-GEN1-BATTERY.cpp:1964-1966`) | ja, `0x155` mit Ungueltig-Werten |
| `0x19F, 0x423, 0x426, 0x436, 0x69F` (10.2) | ja, voll |
- **Offen:** Ob `0x423` allein genuegt (im Satz 10.2 liefen `0x19F`, `0x426`, `0x436` mit). Test am Bench in vier Schritten: (1) Standard ohne `0x423`; (2) `0x090/0x242/0x350` + `0x423`; (3) nur `0x423`; (4) nur `0x350`. Je Kommunikation (Zellspannungen, `0x155`, Status) notieren.

### 10.4 Zellspannungen: Broadcast gegen UDS (25.09.)
- Broadcast-Dekodierung (Rohwert + 2000 mV) passt zu `0x425` (Zellmaximum 4190/4200 mV, Minimum 4160 mV; Broadcast-Zellen 4164-4188 mV).
- UDS-Zellwerte (DID 9021...9083, ohne 9040/9060/9080; Zelle n: n<=31 -> `0x9020+n`, 32-62 -> `0x9041+(n-32)`, 63-93 -> `0x9061+(n-63)`, 94-96 -> `0x9081+(n-94)`) liegen auf dem Bench-Akku konstant **ca. 103 mV hoeher** (4262-4295 mV; Differenz 98...109 mV bei 42 Zellen).
- Die Nummerierung der Broadcast-Zellen stimmt (Korrelation UDS/Broadcast 0,94 bei 20 Zellen; gemeinsame Ausreisser bei Zellen 71-77 und 87-96).
- **Offen:** Ursache des +100-mV-Versatzes bei UDS (Vermutung: UDS-Wert unplausibel, > 4,2 V). Die 05.08.-Werte (4192-4208 mV) liegen dagegen im Bereich der Broadcast-Werte.

## 11. Berichtigung und Zusammenhang aus dem Chat-Export `BatteryEmulator_vs_OVMS_250926` (25.-28.09.2026)

Quelle: `429c72f7-batteryemulator_vs_ovms_250926.md` (Chat vom 25.09.-28.09.2026, Export 09.10.). Kennzeichnung: **Chat** = steht im Chat, von mir nicht neu geprueft.

### 11.1 Berichtigung zu 10.4 (UDS-Zellen "+103 mV")
- **Berichtigt:** UDS-Zellwerte (DID 9021...9083) haben die Skalierung **0,976563 mV/Bit** (Chat, Auswertung 25.09.). Roh `0x10B9` = 4281 -> 4180,7 mV; Broadcast 4179 mV. Die Differenz von ca. 103 mV in 10.4 war ein Skalierungsfehler von mir; UDS und Broadcast stimmen. Die 05.08.-Rohwerte 4192-4208 entsprechen 4094-4109 mV.
- Das Pack war am 25.09. fast voll (Statusanzeige: SOC 98,88 %, 400,7 V, Zellen 4160/4187 mV); am 09.10. im Auto 3,54 V je Zelle (339,7 V).

### 11.2 Zusammenhang `canlog_after_nvrol`
- **Chat:** Aufnahme am 25.09. ca. 16:37, nach Ende der Ladung und nach NVROL-Reset (Sequenz: Session 1, Routine B009 "no response", Session 2, Write 9281=1, Read-back OK). Balancing laut `0x912B` aktiv in Zellen 85-96; Wach-Poll `0x9270/0x9281/0x9251/0x9252` nach Wake up.
- **Gemessen:** Das Log besteht aus 23 Dateien im 5-s-Raster mit je einem Burst von 0,01-0,8 s. Frame-Zaehlungen sind daher **keine Raten** (z. B. `0x155` 907 Frames in 114 s). Die Aussagen zu Absendern bleiben gueltig, die Haeufigkeiten nicht.
- **Chat:** Lifetime-Zaehler `0x9245/0x9247` antworten nach NVROL-Reset mit Nullen; am 23.09. vorher 99 Zyklen, 1026,59 kWh geladen, 1178,56 kWh entladen, 204,18 kWh regeneriert.

### 11.3 Zeit und Balancing (Chat, Nutzeraussagen kursiv zu verstehen)
- Nutzer: Pack lief ca. 10 Wochen ohne Balancing (vor dem 23.09.); nach NVROL-Reset balanciert zunaechst nur Block 85-96, Spread 40 mV -> 27 mV (25.09.) -> 20-25 mV (27.09.). **Annahme:** Die Logs vom 24.07./05.08. liegen in dieser Zeit (nicht belegt).
- Nutzer: Zoe-Ph2-Software sendet Zeit. Chat (Code gelesen): Zoe-Gen2-Treiber sendet `0x376` (Zeit, ab 24.04.2025 sekuendlich), `0x373` (Wake/Sleep), `0x375`, `0x5F8` (Vehicle ID), `0x6BF` (Boost Time), `0x0EE`. Chat (Ladelog geprueft): diese sechs IDs kommen im echten Twingo-Fahrzeug-Log **nicht** vor.
- Chat: `0x53B` im Twingo-Fahrzeug-Log ist ein hochzaehlender Sekunden-/Minutenzaehler (Zeitkandidat); ab Kontaktorschluss reale Werte, davor `F8 FC FF FF 00 07`.
- **Chat, Zitat aus dem Zoe-Gen2-Treiber (TODO-Block, Quelle ljames28):** "If the pack is in a state where it is confused about the time, you may need to reset its NVROL memory. However, if the power is later power cycled, it will revert back to its previous confused state. Therefore, after resetting the NVROL you must enable 'temporisation before sleep', and then stop streaming 373. It will then save the data and go to sleep. When the pack is confused, the state of charge may reset back to an incorrect value every time the power is reset. In this state, the voltage will still be accurate."
  - Relevanz: Hinweis, dass das Pack Daten beim Einschlafen speichert, nachdem `0x373` aufhoert. Offen, ob das auf den Twingo-LBC uebertragbar ist.
- Chat: Der Twingo-Fork hat NVROL-Reset und "temporisation before sleep" (9281) bereits umgesetzt; Zoe2-Referenztreiber hat sie als TODO. Auf der Bauliste im Chat: Wach-Balancing-Zaehler `0x9262/0x9263` (Entscheidung offen).

### 11.4 Folge fuer die offenen Fragen
- Die Logs 24.07./05.08. (kein Broadcast mit Zoe-Gen2-Satz) und 25.09. (Broadcast mit Zoe-Gen1-Satz) stimmen mit dem Chat ueberein, in dem der Zoe-Gen2-Satz nur fuer Zeit/Status gedacht war.
- **Offen:** Nutzer-Frage, ob eine gueltige Uhrzeit fuer Balancing und Zaehler noetig ist; Chat-Befund: Balancing laeuft nachweislich auch ohne gesendete Zeit (Block 85-96 aktiv).

### 11.5 Stand der Lifetime-Zaehler (Nutzer, 09.10.)
- Nutzer: `0x9245/0x9247` (und die uebrigen Zaehler aus 11.2) stehen seit dem NVROL-Reset (23./24.09.) weiterhin auf 0. Vor dem Reset am 23.09.: 99 Zyklen, 1026,59 kWh geladen, 1178,56 kWh entladen, 204,18 kWh regeneriert.
- Auch `9261` / `91C1` speichern am Bench seit Wochen keine neuen Zeitwerte (bench bleibt bei 9261 = 91C1 = `14 02 70`, siehe BEFUND_SCPU_0436_Bench_08-10.md). **Annahme (nicht belegt):** gleiche Ursache, der LBC rechnet/speichert ohne gueltige Zeitbasis nicht.

## 12. Logger-Filter (USB-CAN-Log), Presets aus dem Chat vom 09.10.

Quelle: Chat-Export `cc935ef3-batteryemulator_vs_ovms_0310262.md` (identisch mit dem Export vom 04.10. bis auf den Schluss vom 09.10., 20:28 Uhr).

- Einstellung: Settings, Feld **"CAN USB log filter (hex IDs)"**. Wirkt **erst nach Neustart** des Moduls; "sofort beim Speichern anwenden" ist als K1 auf der Bauliste und nicht gebaut. Ein `!` am Anfang bedeutet "alles ausser".
- **Lauf 1** (Mitschnitt 22aaf176 und die Logs seit 04.10.): `!90,C6,12E,17A,17E,186,18A,1B0,1F6,1F8,211,217,242,29A,29C,2B7`
- **Lauf 2** (nur die 16 schnellen IDs, `0x350`, `0x214`, alle 29-Bit-IDs, Diagnosebereich; ca. 1.360 Frames/s): `90,C6,12E,17A,17E,186,18A,1B0,1F6,1F8,211,217,242,29A,29C,2B7,350,214,ext,700-7FF`
- **Kurzliste** (wenig Last, ca. 154 Frames/s): `ext,350,214,53B,69F,42E,5D7,5DE,646,4C2,700-7FF`
- Empfehlung fuer den Bench-Mitschnitt am Akku: **kein Filter** (Feld leer), damit auch die Akku-Frames sichtbar sind, die Lauf 1 ausblendet. Logger-Verluste: im Auto-Log 91 Marker, 240.577 Frames (ca. 27 %); am Bench bei weniger Verkehr erwartet weniger (Annahme).
- Der Chat vermerkt, dass die Uebergabe-Datei fuer ein anderes Konto nicht fertig geliefert wurde.

## 13. Weitere Chat-Exporte (09.10.): Zusammenhang der Bench-Logs, Schlaf-Reihenfolge, Zeit-PIDs

Quellen: `cb57ee6f-batteryemulator_vs_ovms_280926.md` (28.-29.09., 5.862 Zeilen), `e81bfb50-batteryemulator_vs_ovms_290926.md` (29.09.), `0eb6ef37-batteryemulator_vs_ovms_011026.md` (01.-02.10., 7.008 Zeilen; nur Schluss und Stichworte gelesen). Kennzeichnung: **Chat** = steht im Chat, von mir nicht neu geprueft.

### 13.1 Herkunft der Bench-Logs (Berichtigung zu Abschnitt 10)
- **Chat (28.09., 09:32):** `canlog_00-04-48.zip` ist ein **Lauf des Zoe-Gen2-Treibers** (kein Twingo-Treiber): TX `0x0EE` (2.757), `0x373`, `0x375`, `0x376`, `0x5F8`, `0x6BF`, 137 UDS-Anfragen; RX nur 64 UDS-Antworten, keine Broadcast-Frames. Das Zeitfeld `0x376` zeigte nur den Treiber-Startwert (24.04.2025), nicht die echte Zeit. Kein `0x350`, kein `0x53B`.
- **Chat:** In diesem Lauf antwortete `91C1` mit `0D EE 55` = 912.981 min (634 Tage); `9261` ohne Antwort. Nach dem NVROL-Reset stehen `9261` und `91C1` auf 0 (Chat 28.-29.09., Anzeige "More Battery Info").
- **Chat (29.09. 01:05):** `91C1` heisst im Werkstatt-Dump "Pack Time Life (since 1st power-up)", 1:1 Minuten; `9261` ist "Absolute Time of Vehicle", eine eigene dritte Zeitgroesse. Im selben Dump ist `0x9281` mit `00` "temporisation is activated" (die NVROL-Sequenz schrieb vorher `01`; am 28.09. auf `00` geaendert).
- Die Dumps (`...RBMS_MCPU...txt`) sind `DataWrite`-Makros (Service 0x2E) fuer fast jede DID; Chat: damit sind die Felder beschreibbar.

### 13.2 Schlafablauf am Bench und Reihenfolge der Akku-Frames (Chat 29.09., Anhang 14:07)
- Ablauf: "Sleep 0x9281" (Session 1 OK, B009 uebersprungen, Write 9281=0 OK, Read-back `00`), dann nichts mehr senden; Akku **still ab ca. T+138 s**; der Chat meldet "super geklappt", Aufwachen mit "Wake up" funktioniert.
- **Gemessen (Anhang):** Letzte Frames je ID: Zellframes `0x5A1...0x5F7`, `0x5DD`, `0x659`, `0x4AE`, `0x4AF` bei T+136,7-136,8 s; `0x6BE` bei T+137,7 s; `0x424`, `0x425`, `0x43A` bei T+137,9 s; `0x445`, `0x464`, `0x588`, `0x0C5`, `0x1C9`, `0x155` zuletzt bei T+138,0 s. Die langsamen Frames enden also zuerst, die schnellen zuletzt.
- Gleiche Anzeige: Zeit `9261` = 0, Pack-Zeit `91C1` = 0, `925F` (Fahrzeug-km) = 19.400 km, `91CF` (Pack-km) = 0, Zaehler/Zyklen = 0, Balancing-Zaehler `80000000`, BMS-State nur Nullen, SOC 70,14 %, Pack 374,8 V.

### 13.3 To-do/Nice-to-have aus dem Chat (29.09., nur vorgemerkt)
- Deye: kurz 100 % SOC nach dem Aufwachen (soll nicht, auch kein 0 % melden); Events "CAN NATIVE BUS ERROR" nach dem Wiederanlauf automatisch quittieren; Fix-later: `0x090/0x242` erst nach dem vollstaendigen Wake-Burst starten.
- Nice-to-have: Hardware-Taster mit Sleep inkl. `0x9281` ohne Wiederanlauf; SSD1306-Statusfeld "Shutdown requested / Sleep requested / Battery sleeping - Turn off now!!".
- Chat 01.-02.10.: `C3` als Dauerwert im Betrieb fraglich (Ladelog zeigt `C0`); Rolling-Counter in `0x18A` (Byte 7, Schritt `0x10`) ist im Emulator vorhanden.

## 14. Weitere Bench-Logs (hochgeladen 09.10.): `ae4634dd-canlog_xx`, `67b50fd6-canlog_chronologisch_alle`, `d6887ad6-canlog_newbattemusw`, `cbd10d87-canlog_0d00h04m17s`

Format wie in Abschnitt 10 (Emulator-Log, `TX1`/`RX0`, Bursts je Datei, Frame-Zahlen sind keine Raten). Datumsangaben = Zip-Eintraege bzw. Dateinamen; Jahr nicht in den Dateien.

| Log | Zeit | Gesendet (TX) | Akku-Broadcast | `9261` / `91C1` |
|---|---|---|---|---|
| `canlog_xx` (12 Dateien, 33 s) und `canlog_chronologisch_alle` (dieselbe Sitzung, zusammengefuegt) | Zip 01.08., Dateinamen 14:44-14:45, Emulator-Laufzeit ca. 86,7 h | **nur `0x423`** (`07 1D 00 02 5D 80 5D C8`) und `0x79B` (UDS-Poll 11 Bit); kein 29-Bit-UDS | **ja, voll**: `0x155`, `0x0C5`, `0x1C9`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, `0x4AE`, `0x4AF`, `0x659`, `0x6BE`, alle Zellframes | nicht gelesen |
| `canlog_newbattemusw` (52 Dateien, 94 s) | Zip 28.09. 16:25, Laufzeit 48-50 min | `0x19F`, `0x423`, `0x426`, `0x436`, `0x69F`, **`0x350`, `0x53B`**, `0x79B`, Pylon-Frames (`0x4210...`, `0x7310...`), UDS | ja, voll | `9261` = `00 00 00` (einmal gelesen); `9262/9263/9252` = `80 00 00 00`; `9210` = `00 00` |
| `canlog_0d00h04m17s` (21 Dateien, 200 s) | Zip 01.10. 14:48-15:30 | `0x090` (10 ms), `0x242`, `0x350`, `0x423`, `0x19F`, `0x426`, `0x436`, `0x53B`, `0x69F`, `0x79B`, Pylon, UDS | ja, voll | `9261` = 0, `91C1` = 0, `9245/9247` = 0, `9281` = `00`, `9270` = NRC `12`, `925F` = `00 1D 9A 20`, `91CF` = `80 00 00 00`, Balancing-Zaehler `80000000` |

### 14.1 Befunde
- **Gemessen:** Mit **nur `0x423`** (plus `0x79B`) sendet der Akku Broadcast durchgehend (Log `canlog_xx`). Das Log beginnt mitten im Betrieb (Laufzeit 86,7 h), zeigt also nicht den Wecken-Moment: **0x423 reicht zum Wachhalten des Broadcasts**; ob es auch aus der Stille weckt, ist damit nicht gezeigt.
- Zusammen mit Abschnitt 10.3 gibt es jetzt zwei einzelne Sendesaetze, nach denen der Akku Broadcast sendet: nur `0x423` (Log) und nur `0x350` (Code-Kommentar `:1964-1966`, `0x155` mit Ungueltig-Werten); der Zoe-Gen2-Satz (`0x0EE`, `0x373`, ...) loest keinen Broadcast aus (Abschnitt 10.1). Der Vier-Schritte-Test (Abschnitt 10.3) bleibt fuer "aus der Stille wecken".
- **Gemessen:** `9261` und `91C1` stehen am 28.09. und am 01.10. auf 0, obwohl `0x350` und `0x53B` (28.09., 01.10.) sowie `0x436` gesendet wurden. In keinem dieser Logs hat `9261`/`91C1` einen Wert ungleich 0.
- **Gemessen:** `925F` = `0x1D9A20` = 1.940.000 -> 19.400,00 km = `0x4BC8` (16 Bit) aus unserem Standard-`0x426` (`00 60 01 00 4B C8 00 40`). Bestaetigt, dass der Akku die km aus `0x426` uebernimmt (wie im Auto, Abschnitt 1).
- `9006` = `00 05 FA 60` (391,8 V, falls 0,001 V/Bit - Skalierung nicht geprueft), `9011` = `31 88` (12 V).

### 14.2 `be14806b-canloghaendisch.zip` und `dbd33adf-canlog_00-05-34.zip` (09.10.)
- `dbd33adf-canlog_00-05-34.zip` ist inhaltlich **identisch** mit `cf794fb4-canlog_00-04-48.zip` (24.07., Zoe-Gen2-Lauf, Abschnitt 10.1); Verzeichnisvergleich ohne Unterschied.
- `be14806b-canloghaendisch.zip` (22 Dateien): 21 Dateien eines Zoe-Gen2-Treiberlaufs (Zip-Datum 05.08. und 29./30.07.: `canlog_00-19-25`, `06-03-05...06-04-10`, `07-29-41...07-30-33`; TX `0x0EE`, `0x373`, `0x375`, `0x376`, `0x5F8`, `0x6BF` + UDS, RX nur UDS-Antworten, kein Broadcast) und `canloghaendisch.txt` (Zip-Datum 14.08.).
- **`canloghaendisch.txt`:** haendisch gespeicherter Ausschnitt von nur 0,43 s (Laufzeit 328,9-329,4 s) mit `0x423` (4 Frames, 100 ms, `07 1D 00 02 5D 80 5D C8`) und einmal `0x79B` (`02 21 42 ...`); der Akku sendet Broadcast (`0x155`, `0x0C5`, `0x1C9`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, Zellframes). Der Ausschnitt beginnt mit Broadcast-Frames, das erste `0x423` kommt 90 ms spaeter; da `0x423` alle 100 ms laeuft, liegt das vorige `0x423` vor dem Ausschnitt -> **keine Aussage zur Weck-Reihenfolge**. Bestaetigt nur: mit `0x423` laeuft der Broadcast (wie Abschnitt 14.1).
- **Zaehler vor dem NVROL-Reset (Zoe-Gen2-Laeufe Juli/August, UDS):** `9250`/`9252` = `80 01 C2 29` -> (Wert XOR `0x80000000`)/1024 = 115.241/1024 = **112,5 h**; `924F`/`9251` = `80 00 B4 00` -> 46.080/1024 = **45,0 Ah**; `91CF` = `80 03 57 80` (Pack-km: 219.008/32 = **6.844 km**, Skalierung /32 laut Chat vom 29.09.), `9001` = `23 27` (8999; bei 0,01 %/Bit = 89,99 % interner SOC, Skalierung nur aus dem Chat-Anzeigewert 70,14 % abgeleitet). Nach dem NVROL-Reset stehen alle diese Zaehler auf `80000000` (Nutzer, Abschnitt 11.5).


## 15. Berichtigung: Filter im Mitschnitt 22aaf176 (Nutzer-Hinweis, 10.10.)
- **Gemessen:** Keine der 16 IDs `0x090`, `0x0C6`, `0x12E`, `0x17A`, `0x17E`, `0x186`, `0x18A`, `0x1B0`, `0x1F6`, `0x1F8`, `0x211`, `0x217`, `0x242`, `0x29A`, `0x29C`, `0x2B7` kommt in 22aaf176 vor; der Filter `!90,C6,...` (Abschnitt 12) blendet sie aus.
- **Berichtigung:** Die Aussagen in den Abschnitten 3, 9.2 und in der Zeilenuebersicht ("41 Zeilen laufen nur auf dem Fahrzeug-CAN, nicht auf dem Akku-Bus") gelten fuer diese 16 Zeilen **nicht**: ob sie auf dem BMS<>EVC-Bus laufen, ist **unbekannt**. Nur die uebrigen 25 Fahrzeug-CAN-Zeilen (u. a. `0x350`, `0x53B`, `0x214`) sind im Mitschnitt nachweislich nicht vorhanden.
- **Gemessen (Bench, 25.09., ungefiltert, Akku allein mit unseren fuenf Frames):** Der Akku sendet keine dieser 16 IDs. Ob das EVC sie sendet, bleibt offen.
- **Gemessen (10.10.):** Der ungefilterte Mitschnitt vom 02.10. (`364b56eb`) enthaelt alle 16 IDs (18.689 bis 20.693 Frames je `0x090`, `0x0C6`, `0x12E`, `0x17A`, `0x17E`, `0x186`, `0x18A`, `0x1F6`, `0x1F8`; 9.341 bis 10.449 je `0x1B0`, `0x211`, `0x217`, `0x242`, `0x29A`, `0x29C`, `0x2B7`), aber **keine** BMS-Bus-Frames (`0x155`, `0x424`, `0x425`, `0x423`, `0x426`, `0x436` = 0). Er stammt vom Fahrzeug-CAN und sagt nichts ueber den BMS<>EVC-Bus.
- Der Mitschnitt vom 04.10. (`5c3d7daf`) enthaelt keine der 16 IDs (und keine BMS-Bus-Frames); ob er gefiltert war, ist nicht belegt.
- **Offen:** Mitschnitt im Auto am BMS<>EVC-Bus mit dem Filter "Lauf 2" (Abschnitt 12).

## 16. 0x426: warum der Akku mit unserem Frame auf Stoerung geht (10.10., Vergleich mit dem echten Bus)

Bekannt (MERKZETTEL_Zeitwerte_33BE.md Z. 126/284, BEFUND_SCPU_0436_Bench_08-10.md Z. 90-96): Mit `0x426` an zeigt der Akku jedes Mal **E14381** ("CAN from EVC/HEVC", Typ 81 = "invalid serial data received"), meist zusammen mit **E14281** (CAN from Inverter) und **1B0715**; ohne `0x426` kein E14381. E14281 verschwindet mit `0x19F` an.

### 16.1 Vergleich unser Frame gegen den echten Bus (gemessen, 6.080 Frames im Mitschnitt 22aaf176)
| Byte | Echt (alle 6.080 Frames) | Unser Frame (`RENAULT-TWINGO-GEN1-BATTERY.h:300-305`) |
|---|---|---|
| B0 | `00` | `00` |
| B1 | `00`, `08`, `60`, `70` | `60` |
| B2 | `02`, `05`, `06`, `07` (mit B1 `00`/`08`) oder `61`, `65`, `67`, `69` (mit B1 `60`/`70`) | **`01`** (kommt nie vor) |
| B3 | **`01`** (immer) | **`00`** |
| B4-B5 | km, 16 Bit (`6A 70` = 27.248) | km (unsere 19.400 = `4B C8`) |
| B6 | `00` | `00` (aus km * 256) |
| B7 | `40` | `40` (einstellbar) |

- **Gemessen:** Echte Paare B1/B2: `00/06` (2.898), `70/69` (1.701), `60/65` (796), `00/02` (432), `60/69` (120), `60/61` (48), `08/02` (19), `00/05` (19), `08/06` (12), `00/07` (12), `70/61` (12), `60/67` (10), `70/65` (1). Unser Paar `60/01` kommt im echten Bus nie vor, unser B3 = `00` ebenfalls nie.
- Auffaellig: Unsere `01` steht in B2, im echten Bus steht sie in B3 (Frame wirkt um ein Byte verschoben bzw. aus anderem Format).
- **Schluss (nicht bewiesen):** Der Fehler "invalid serial data" kann am **Inhalt** (B2/B3) haengen, nicht am Frame an sich. Test (a) aus KONZEPT_HV_Modell Z. 111 ("0x426 an mit km = 0") wurde nie gemacht; der Inhalt von B1-B3 ist in der Simulator-Seite bisher nicht einstellbar (nur km und Byte 7).
- **Vorschlag fuer den ersten Test im Modus "echter Bus":** `0x426` im Ruhezustand wie im Auto, z. B. `00 00 06 01 [km] 00 40` (wach, Tuer offen) oder `00 00 02 01 ...` (zu); dazu `0x19F` mit 10 ms. Erwartung: E14381 bleibt aus. Nicht belegt; es koennen auch fehlende Begleitframes (`0x435`, `0x0ED`, `0x1A1`) im Spiel sein.
- Welche CPU den Fehler setzt, ist offen: E14381 steht in MCPU- und SCPU-Definition; der Fehlertext deutet auf die SCPU (Funktion LBC2).

## 17. Zaehler und Pruefsummen der 17 EVC-seitigen Frames (10.10., erste Untersuchung, Mitschnitt 22aaf176)

Methode: Skript `an.py`-Stil im Scratchpad. Pruefsumme: CRC-8 (MSB zuerst, ohne Spiegelung), Polynom `0x1D` oder `0x2F`, Startwert `0x00` oder `0xFF`, ueber "alle anderen Bytes" oder "alle Bytes davor", mit konstantem Ausgangs-XOR; jedes Byte der Frames als moegliches CRC-Byte. Zaehler: je Nibble konstanter Schritt mod 16, je Byte konstanter Schritt mod 256. **Grenze:** Nicht abgedeckt sind andere CRC-Familien (z. B. E2E-Profile mit Data-ID je Zaehlerstand) und Pruefsummen ueber Teilbereiche. Bei (fast) konstanten Frames ist jeder Treffer wertlos.

| Frame | Ergebnis | Sicherheit |
|---|---|---|
| `0x0EC` (3 Byte) | **B1 hoeheres Nibble = Zaehler +1**; **B2 = CRC-8, Polynom `0x1D`, Start `0x00`, Ausgangs-XOR `0xBE` ueber B0, B1** (100 % von 57.317 Frames) | gemessen |
| `0x19F` | **B3 niedriges Nibble = Zaehler +5 mod 16** (100 %); `B7` immer `FE`; keine Pruefsumme gefunden (B0/B1 und B5 aendern sich nur in Zuendung/Fahrt) | gemessen (Zaehler); CRC offen |
| `0x157` | **B1 hoeheres Nibble = Zaehler +5** (100 %); B0, B3 mit 256 Werten (langsam veraenderlich, 88 % unveraendert zum Vorgaenger); keine Pruefsumme gefunden (beste Quote 4 %) | Zaehler gemessen; CRC offen |
| `0x511` (7 Byte) | B0 `04`/`00` = Zustand; **B1-B6 sind sechs unabhaengige Zaehler mit festem Schritt mod 256**: B1 +63, B2 +201, B3 +185, B4 +107, B5 +13, B6 +227 (je 98 % der Frames); keine Pruefsumme gefunden | gemessen (Schritte); Startwert/Sinn offen |
| `0x500` (5 Byte) | B1-B4 zufaellig wirkend (256 Werte, kein Schritt, kein CRC gefunden), erscheint ab ca. 15 s nach Zuendung 1 | offen |
| `0x1A1`, `0x419` | Messsignale (viele Werte, ueberwiegend unveraendert zum Vorgaenger), kein Zaehler, kein CRC gefunden | offen |
| `0x4F7` | weniger als 8 Werte je Byte, kein Zaehler | konstant je Zustand |
| `0x423`, `0x426`, `0x436`, `0x435`, `0x428`, `0x42F`, `0x1C7`, `0x69F`, `0x0ED` | pro Zustand konstant bzw. nur wenige Werte; **kein Zaehler**. `0x423` B7 (`D6`, `E9`, `E8`, `CC`, `BB`) aendert sich mit dem Zustand und wechselnden B4/B6 (`5D`/`B2`); mit den getesteten CRC-Familien nicht erklaerbar (beste Quote 56 %) | `0x423` B7 offen |

Folgen fuer den Nachbau:
- **Mit Formel erzeugbar:** `0x0EC` (Zaehler + CRC), `0x19F` (Zaehler +5), `0x157` (Zaehler +5, restliche Bytes unbekannt), `0x511` (sechs Zaehler mit festen Schritten, Startwerte frei).
- **Aufgezeichnete Werte je Zustand senden:** `0x423` (B7), `0x426`, `0x436`, `0x435`, `0x428`, `0x42F`, `0x1C7`, `0x69F`, `0x0ED`, `0x4F7`.
- **Unklar:** `0x500`, `0x1A1`, `0x419`, `0x157` (B0/B3).
- Bekannt aus dem Code: `0x090`, `0x242`, `0x18A` tragen Zaehler und CRC-8 J1850 (Polynom `0x1D`), `RENAULT-TWINGO-GEN1-BATTERY.h`.

### 17.1 Zweiter Durchgang: breitere Pruefsummen-Suche (10.10.)
Suche: alle 127 ungeraden 8-Bit-Polynome, mit und ohne Spiegelung (Startwert faellt in das Ausgangs-XOR), vier Bereiche (alle anderen Bytes, Bytes davor, danach+davor, rueckwaerts), dazu Summe und XOR der anderen Bytes plus Konstante; jedes Byte als Pruefbyte; Stichprobe 160 verschiedene Frames je ID; Schwelle 97 %.
- **Positivkontrolle:** `0x0EC` Byte 2 wird gefunden (Polynom `0x1D`, XOR `0xBE`, 100 %). Das Suchverfahren funktioniert.
- **Kein Treffer:** `0x423`, `0x157`, `0x1A1`, `0x419`, `0x500` (8-Bit-Pruefsummen dieser Familien).
- **Gemessen, `0x423` Byte 7:** Frames mit identischen uebrigen sieben Bytes tragen teils unterschiedliche Byte 7 (24 von 61 Gruppen mit Widerspruch). Byte 7 ist also **keine Pruefsumme der anderen Bytes**, sondern traegt eigene Information (z. B. langsamer Zaehler oder Messwert). Gleiches gilt fuer `0x423` B1 (28 von 128 Gruppen) und `0x419` (alle variablen Bytes mit Widerspruechen).
- `0x19F` B3 (Zaehler +5) widerspricht sich innerhalb gleicher Restbytes, passend zu einem Zaehler; B0/B1/B2/B5 tragen sonst keine Widersprueche (meist eindeutige Frames, daher wenig aussagekraeftig).
- **Offen / nicht getestet:** 4-Bit-Pruefsummen im selben Byte wie ein Zaehler (z. B. `0x157` B1 unteres Nibble), E2E-Profile mit Data-ID je Zaehlerstand, Bit-Ebene.
- **Folge:** `0x423` kann mit aufgezeichneten Werten je Zustand gesendet werden; ob Byte 7 dabei stoert, zeigt nur der Test am Akku.


## 18. Zweite Berichtigung: Mitschnitt 22aaf176 war ungefiltert (Nutzer, 10.10.)
- **Nutzer:** Beim Mitschnitt auf dem BMS<>EVC-Bus im echten Fahrzeug (22aaf176, 09.10.) war **kein Filter** aktiv.
- **Folge:** Abschnitt 15 ("16 IDs unbekannt, weil gefiltert") wird hinfaellig. Die 52 IDs des Mitschnitts sind dann der gesamte Inhalt des Busses in dieser Sitzung; `0x090`, `0x242`, `0x0C6`, `0x12E`, `0x17A`, `0x17E`, `0x186`, `0x18A`, `0x1B0`, `0x1F6`, `0x1F8`, `0x211`, `0x217`, `0x29A`, `0x29C`, `0x2B7` sowie `0x350`, `0x53B`, `0x214` laufen dort **nicht**. Die 17 EVC-seitigen IDs (Abschnitt 17) sind fuer diese Sitzung vollstaendig.
- **Widerspruch, offen:** Am 09.10. hatte der Nutzer geschrieben, ab 04.10. nur noch mit Filter `!90,C6,...` aufgezeichnet zu haben; die Datei laesst sich nicht pruefen (keine der 16 IDs im Log, ob gefiltert oder nicht). Plausibilitaet: Lograte ca. 830 Frames/s + 27 % Verlust, das passt zu ca. 1.100 Frames/s ohne die 16 schnellen IDs; mit ihnen waere die Bus-Last deutlich hoeher. Das spricht fuer die Nutzeraussage, beweist sie nicht.
- Bleibt: Die Sitzung zeigt nur Wecken, Zuendung, Fahrt, Abschliessen, Schlafen (kein Laden, keine Fehler); bis zu 27 % der Frames fehlen.

## 19. Welche per UDS abgefragten Werte liegen schon als Broadcast vor? (10.10.)

Methode: Die Anzeige "More Battery Info" vom 29.09. (14:07, Chat-Anhang, Export 290926) zeigt UDS-Werte und die letzten Frames je ID aus demselben Lauf (Akku am Bench, Pack bei SOC ca. 70 %). Vergleich Zahl fuer Zahl (nachgerechnet am 10.10.).

| UDS-Wert (Anzeige 29.09.) | Broadcast | Rechnung | Ergebnis |
|---|---|---|---|
| `9002` USOC 68,41 % | `0x155` B4-B5 (`6A E2`) | 27.362 x 0,0025 = 68,405 % | **gleich** (gemessen) |
| `900D` Strom (Anzeige) | `0x155` B1 (unteres Nibble) + B2 (`D0`) | Roh 2.000 x 0,25 - 500 = 0,0 A | gleiche Quelle, Treiber nutzt den Broadcast schon |
| `9018` Max Charge Power 15,37 kW | `0x155` B0 (`33`) | 51 x 300 W = 15,3 kW | **nahe** (15,30 gegen 15,37), nicht exakt |
| `900E` Max Generated Power 43 kW | `0x424` B2 (`56`) | 86 x 500 W = 43,0 kW | **gleich** (gemessen) |
| `900F` Max Available Power 72 kW | `0x424` B3 (`90`) | 144 x 500 W = 72,0 kW | **gleich** (gemessen) |
| `9003` SOH 100,0 % | `0x424` B5 (`64`) | 100 % | **gleich** (gemessen) |
| `9007` Zelle A 3,912 V / `9009` Zelle B 3,888 V | `0x425` B4-B7 (Max/Min, 10-mV-Raster) | 3.910 / 3.890 mV | **gleich** innerhalb 10 mV |
| `9021...9083` 96 Zellen | `0x5A1...0x5F7`, `0x5DD` | siehe Abschnitt 7.2 / 8.1 | alle 96, alle 3 s |
| `9006` Pack-Spannung (Summe) 374,813 V | Summe der 96 Zellen aus den Zellframes | berechenbar | kein eigenes Frame (`0x42E` kommt auf dem Bus nicht vor) |
| `9131...9138` 8 Pack-Temperaturen | `0x424` B4/B7 nur Min/Max (bei Anzeige je 20 C) | -40 C Offset | **nur Min/Max** als Broadcast |

Nicht als Broadcast gefunden (nur UDS): `9001` interner SOC, `91B9`/`91BA` SOC min/max, `9011` 12-V-Versorgung (`31 88` = 12,39 V), Energien `9243`/`9245`/`9247`, Zyklen `9210`, Balancing-Zaehler `924F...9252`, `9262`, `9263`, Balancing-Schalter `912B`, BMS-Zustand `9270`, `9281`, Zeit `9261`/`91C1`, Kilometer `91CF`/`925F`, Fahrzeug-ID `925E` (kommt dagegen als `0x69F` B1-B3 vom EVC an den Akku).
Unbekannte Akku-Frames mit evtl. weiterem Inhalt: `0x0C5`, `0x1C9`, `0x43A`, `0x464`, `0x588` (Zaehler/CRC-artig), `0x6BE` (Bytes 63 von 138 Aenderungen), `0x4AE`, `0x4AF`, `0x659` (konstant). Nicht untersucht.
Folge fuer den Treiber: Aus dem Broadcast koennten ohne UDS kommen: SOC, Strom, Ladeleistung, die Limits (`900E`, `900F`), SOH, Zellmin/-max, alle 96 Zellen (und daraus die Pack-Spannung), Temperatur-Min/Max. Die Zeit-, Zaehler- und Balancing-Werte bleiben UDS.
