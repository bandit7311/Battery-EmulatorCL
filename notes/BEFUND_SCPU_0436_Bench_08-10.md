# Befunde vom 08.10.2026: SCPU, 0x436, Kilometer, Bench-Akku

Stand: Code auf Branch `claude/twingo-hv-modell-und-alter`, Commit `4f2fc3f`. Alle Lesungen am Bench-Akku, nur lesend, außer wo "Frame" steht.
Kennzeichnung: **gemessen** = Antwort des Akkus, **Schluss** = aus Messungen gefolgert, **Annahme** = nicht belegt.

## 1. Zeit der SCPU kommt aus 0x436 (gemessen, Schluss)

SCPU `9261` ("Abs time since first ignition", `Wxx_cm_abs_time_vhc`), Ziel DC (`0x18DADCF1` / `0x18DAF1DC`).

| Zustand | SCPU `9261` |
|---|---|
| 0x436 mit Standard `86 14 [Zähler] FF DC` | `14 00 02` ... `14 00 39`, +1 pro Minute (Zähler seit Start des Emulators) |
| 0x436 aus (5 min) | bleibt auf `14 00 39` stehen |
| 0x436 mit gesetztem Wert 1.310.770 (kleiner als der Stand) | `14 00 32`, sie folgt auch nach unten |
| 0x436 mit 1.311.344 | `14 02 70` |
| 0x436 mit 1.311.364, später | `14 02 84`, dann `14 02 88` (+1 pro Minute) |
| nach Sleep-Lauf (Aufwachen), 0x436 aus | `00 00 00` |
| 0x436 wieder an | springt sofort auf den Wert |

- **Schluss:** Die SCPU-Zeit kommt ausschließlich aus 0x436 Bytes 1-3. 0x350-Alter, 0x523 und 0x376 haben sie nicht bewegt.
- **Gemessen:** Die SCPU behält die Zeit nicht über das Schlafen. Sie steht danach auf 0, bis 0x436 wieder einen Wert liefert. `925E` (Reset-Kontext der SCPU) blieb dabei `20 80`, also kein neues Reset-Bit.
- **Annahme:** Die Zeit geht beim Schlafen verloren, nicht durch das Fehlen der Frames (5-Minuten-Test ohne Sleep: Wert blieb).

## 2. Kilometer: 0x426 wirkt auf beide CPUs, 0x5D7 nicht (gemessen)

- SCPU hat einen eigenen Kilometerzähler: `9262` ("Vehicle distance totalizer", 32 Bit, Offset 0x80000000, 0,01 km). Auto-Dump: `80 8D 6A 58` = 92.678,00 km.
- Bench: `80 00 00 00` = 0,00 km, auch mit 0x5D7 (19.400 km) an.
- Mit **0x426** an (19.400 km): SCPU `80 1D 9A 20` = 19.400,00 km.
- Nach Änderung auf 19.405 km im Feld: SCPU `80 1D 9C 14` = 19.405,00 km, MCPU `925F` ebenfalls 19.405 km.
- Beide CPUs folgen den Kilometern aus 0x426 sofort, ohne Schlaflauf.
- Auf der MCPU ist `9262` etwas anderes (Balancing im Wachzustand, `80 00 00 00`).

## 3. MCPU `9261` / `91C1` bewegt sich nicht (gemessen)

- Seit der ersten Lesung ungleich 0 am 06.10. (20:40 UTC): `9261` = `91C1` = `14 02 70` = 1.311.344.
- Nichts, was wir seither gesendet haben, hat es verändert: 0x350 mit Alter, 0x523, 0x376, 0x436 (auch größere Werte, 1.311.345 und 1.311.364, durch die ganze Abschaltfolge), HV-Frames 0x62D/0x1FD/0x57F/0x599, 0x5D7, Zoe-Frames 0x373/0x375/0x376, Sleep und Wake-up im Modus "like the car".
- `9259` bleibt auf beiden CPUs 04. `925C` MCPU 01, mit den HV-Zeilen 03; SCPU 01, nach dem Lauf 00.
- Schreiben `2E 92 61 14 02 71`: **NEGATIVE 7F 2E 33, Security Access denied.** Es wurde nichts geschrieben. Seed und Key werden nicht erraten.

## 4. Woher `14 02 70` kommt (Schluss, nicht belegt)

- Das Muster stimmt mit 0x436 überein: Byte 1 = `14` (fest im Frame), Bytes 2-3 = Minutenzähler, `0x0270` = 624 = 10 h 24 min Laufzeit des Emulators.
- 0x436 war in der alten Standardmaske 0x3FF an.
- Dagegen: Dieselbe Bedingung ließ sich nicht wiederholen (MCPU nahm 1.311.345 und 1.311.364 nicht).
- Offene Bedingungen: lange Laufzeit, leerer Speicher (0) als Voraussetzung, anderer Sleep-Knopf, andere Frames.
- `9261` und `91C1` sind gleich. Im Auto-Akku ist ihr Abstand über den Oktober 2026 konstant (784.382 min).

## 5. Bench-Akku ist ein anderer Akku als der im Auto (gemessen)

| DID | Bench | Auto (MCPU) |
|---|---|---|
| `F187` | `293A00812R` (Antwort am Ende abgeschnitten) | `293A09578R` |
| `F191` | `293A01958R` | `293A01958R` |
| `F18A` | `e2cad` | `e2cad` |
| `F196` | `HMLGT5254R` | `HMLGT5254R` |
| `9282` | `293A01958R T2028724 66PCA009AR000` | `293A01958R T2129902 01PDA009AR000` |

- Annahme: Die ersten vier Ziffern nach dem `T` sind Jahr und Woche (Bench 2020 Woche 28, Auto 2021 Woche 29). Das Buchstabenpaar `PCA`/`PDA` könnte eine Ausgabe sein.
- SCPU `925E` = `20 80` (Auto-Dump ebenfalls). Das ist **keine** Fahrzeug-ID, sondern "CPU2 reset context" (Bit-Tabelle in der SCPU-Definition). Die SCPU hat keine Fahrzeug-ID wie die MCPU `925E` (`13 88 6F`).
- Auto-Akku: `9261` 2.486.870 (04.11.2025) → 2.965.443 (06.10.2026), `91C1` 1.700.434 → 2.181.061, `925F` 85.465 km → 92.678 km.

## 6. Weitere Befunde

- SCPU `91C1` und `9279`: NRC 0x31 (nicht vorhanden). Der SCPU-Dump aus dem Auto enthält sie ebenfalls nicht.
- `10 03` auf DB: `50 03 00 32 01 F4` (P2 = 50 ms, P2* = 5 s).
- In der Stille antwortet keine CPU auf Diagnose ("no response").
- Der Filter für `0x155` hat im Lauf 295 ungültige Frames verworfen.
- MCPU-Definition: "Total boost time from HEVC to BMS saved at powerlatch" (Name erinnert an Zoe-Frame 0x6BF). Nur Namensähnlichkeit.
- Beide CPUs haben "CAN from EVC/HEVC" als Fehlerquelle in der Definition.

## 7. Korrektur zu früheren Notizen

- Byte-Positionen von 0x62D und 0x599 im Konzept waren teils 1-basiert. Im Code gilt 0-basiert: 0x62D `01 45 E0 [Phase] 06 [80/40/00] 00` (Phase in Byte 3), 0x599 `00 [04/08] [..] [..] [..] 00` (Inverter in Byte 1).

## 8. Offen

- Welcher Frame oder Vorgang lässt die MCPU ihre Zeit sichern?
- Wie kommt die MCPU zu `9259` = 05?
- Messung im Auto: Frames in der Abschaltfolge, `9259`, `9261`, `91C1`, `9262` vorher/nachher.
- 0x436 dauerhaft machen (Wert persistent, vorbelegt mit Fahrzeugalter).
