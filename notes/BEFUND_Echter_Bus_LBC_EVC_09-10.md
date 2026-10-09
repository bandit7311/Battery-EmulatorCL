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
| `0x500` | 1.928 | 5 Bytes, nur ab Stufe 3 bis Zündung aus |
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
