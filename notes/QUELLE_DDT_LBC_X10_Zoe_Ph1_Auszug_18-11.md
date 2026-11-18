# Auszug aus der DDT2000-Datei "X10_X61_X62_L38_EV_BMS_v6.3.1_LR" (LBC der Zoe Ph1), vom Nutzer, 18.11.2026

Datei: `X10_X61_X62_L38_EV_BMS_v6.3.1_LR_20180301T144850.xml` (DDT2000-Export, 01.03.2018, 880 KB, vom Nutzer hochgeladen, liegt nicht in den Notizen). Gruppe **BMS**, Funktion **LBC**, Adresse **147 (0x93)**. Projekte im Kopf: **x10, x38, x61, x61Ph2, x62**. Das ist die Datei mit **`<x10/>`**, nach der wir gesucht hatten. Nur gelesen, was in der Datei steht, nichts am Geraet geprueft. Der Dateikopf nennt Aenderungen von 2012 bis 2016 (Vdiag $35 fuer "LR"-Projekte, LG-Chem-Session).

## Sitzung, Lesen, Loeschen
| Name in der Datei | Anfrage | Antwort |
|---|---|---|
| StartDiagnosticSession.DefaultSession | `10 81` | `50 81` |
| StartDiagnosticSession.ProgrammingSession | `10 85` | `50 85` |
| **StartDiagnosticSession.ExtendedDiagSession** | **`10 C0`** | `50 C0` |
| StartDiagnosticSession.RenautSession | `10 F2` | `50 F2` |
| StartDiagnosticSession.LGchemSession | `10 F3` | `50 F3` |
| ReadDTCInformation.ReportDTC | **`19 02 0F`** | `59 02 <Maske> <hi> <lo> <Fehlerart> <Status>` |
| **ClearDiagnosticInformation.All** | **`14 FF FF FF`** | **`54`** |
| ReadDTC Snapshot / ExtendedData / Mileage / OccurrenceCounter | `19 04 ...`, `19 06 00 00 00 FF`, `...80`, `...82`, `...83` | |
| TesterPresent | `3E 01` | `7E` |

**DTC-Antwort** (Bytenummern der Datei, 1 = `59`): Byte 3 = **DTCStatusAvailabilityMask**, Byte 4 = DeviceIdentifier (hi), Byte 6 = **DTCFailureType**, Byte 7 = **StatusOfDTC** (Bit 4 = confirmed, Bit 7 = testFailed). Gleicher Aufbau wie in der BMS453-Datei. Beispiel aus dem Dateikopf: "DTC [L38] **33_B2_1C** TS (Temperature Sensing) Circuit Failure" = Code `33B2`, Fehlerart `1C`.
**Der Treiber sendet `19 02 09`**, die Datei `19 02 0F` (nur die Maske der angefragten Status unterscheidet sich).

## Routinen (`31 xx 00`) und ihre Tasten
| Anfrage | Antwort | Name in der Datei | Taste (Bildschirm) |
|---|---|---|---|
| **`31 05 00`** | `71 05` | **"Routines 310500 eraze SF1" / "erase safety mode"** | **"Eraze safety mode"**: sendet **`10 C0`, dann `31 05 00`** (5 ms Pause) |
| `31 02 00` | `71 02 01` | **stop cell balancing** | keine Taste gefunden |
| `31 06 00` | `71 06 01` | increment QD latch of the battery +1 | mit `10 C0` davor |
| `31 07 00` | `71 07 01` | RAZ (zuruecksetzen) QD latches | mit `10 C0` davor |
| `31 08 00` | `71 08 01` | increment QD connector +2 | |
| `31 09 00` | `71 09 01` | RAZ HV connector of the battery | mit `10 C0` davor |
| `31 FA 00` | `71 FA 01` | write EEPROM | |
| `31 10 00`, `31 11 00`, `31 12 00`, `31 13 00` | `71 xx 01` | SOHE module update, BMS update, Long time storage, SOHE diag | |

**Zu "SF1":** `21 03` Byte 17 heisst **"21_03_#17_Safety Mode 1 Flags"**, Wert 1 = **"SAFETY MODE FLAG 1 PRESENT"** (Dateikopf 27.06.2013: "Add LID 03-17 Safety mode flag 1 present"). `31 05 00` loescht dieses Safety-Mode-Flag.

## Bezug zu den Behauptungen in `QUELLE_Zoe_Ph1_geschuetzte_DTCs_18-11.md`
- **Bestaetigt (in der Datei):** `31 05 00` gibt es und sie loescht das "safety mode"-Flag. Die Taste sendet vorher `10 C0`. `14 FF FF FF` heisst "Clear All" und antwortet `54`.
- **Nicht in der Datei:** die Klassen 1, 2, 4 der DTCs, dass `14 FF FF FF` diese Klassen ueberspringt, und dass das Flag das Balancing sperrt. Das steht nur in der Quelle des Nutzers.
- **`31 02 00` "stop cell balancing"** gibt es als Routine (Wirkung nicht gelesen, nur der Name).
- Der Zoe-Ph1-Treiber liest `21 03` nicht (nur `0x41`, `0x42`, `0x61`, `0x07`, `RENAULT-ZOE-GEN1-BATTERY.h:55-58`).

## Dazu: aeltere DDT-Datei "R_L38_X61_EV_LBC_v4.4" (vom Nutzer, 18.11.2026)
Datei: `R_L38_X61_EV_LBC_v4.4_20140407T155120.xml` (DDT2000-Export 07.04.2014, 720 KB). Gruppe LBC, Adresse 147, Projekte **x38 und x61** (nicht x10; im Kopf "NISSAN XX4"). Nach den Projektnamen Fluence ZE und Kangoo ZE (Hintergrundwissen, nicht aus der Datei). Nur gelesen, nichts am Geraet geprueft.
- Gleiche Grundbefehle wie bei der Zoe Ph1: **`10 C0`** (ExtendedDiagSession), **`19 02 0F`** (ReadDTC), **`14 FF FF FF` -> `54`** (ClearDiagnosticInformation.All), `10 81`, `10 85`, `10 F2`.
- Routinen: nur allgemein **`31 00 00`** (RoutineStart.SRBLID 01-04), `31 00 01` (Status). **Kein `31 05 00`, kein "safety mode", keine Taste zum Loeschen des Flags.**
- Schluss: `31 05 00` und das "Safety Mode 1 Flag" (`21 03` Byte 17) kamen laut Dateikopf der Zoe-Datei erst mit spaeteren Softwareaenderungen (Eintrag vom 27.06.2013, Vdiag-Wechsel). Ob ein Kangoo- oder Fluence-Pack mit neuerer Software `31 05 00` trotzdem kennt, ist nicht belegt.
