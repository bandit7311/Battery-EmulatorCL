# Auszug aus der DDT2000-Datei "BMS453_00.02.02_00472" (LBC), vom Nutzer, 18.11.2026

Datei: `BMS453_00.02.02_00472_00.02.02.ddt_20200218T120557.xml` (DDT2000-Export, 18.02.2020, 600 KB, vom Nutzer hochgeladen, liegt nicht in den Notizen). Gruppe **LBC**, Adresse **0x93** (147), Projekte **x07** und **x07Ph2**. Der Name BMS453 deutet auf das smart 453 / Twingo (nach dem Namen, nicht belegt). **Nicht** die Zoe-Ph1-Datei. Gleiche Familie wie der Zoe-Ph1-Treiber vermutet (gleiche `21 41`, `21 42`, `21 61`), nicht bewiesen. Nur gelesen, was in der Datei steht; nichts auf einem Gerat geprueft.

## Sitzung, Lesen, Loeschen (Zeilen im XML)
| Name in der Datei | Anfrage | Antwort |
|---|---|---|
| StartDiagnosticSession (Standard) | `10 C0` | `50 C0` |
| StartDiagnosticSession.Default | `10 81` | `50 81` |
| StartDiagnosticSession.Programming | `10 85` | `50 85` |
| Sonderstarts (EOL, RSA) | `10 F0`, `10 F1`, `10 F2` | `50 Fx` |
| **ClearDiagnosticInformation.All** | **`14 FF FF FF`** | **`54`** |
| ReadDTCInformation.ReportDTC | `19 02 FF` | `59 02 <Maske> <hi> <lo> <Fehlerart> <Status>` |
| Hard Reset Ecu Reset | `11 01` | `51 01` |

**Aufbau der DTC-Antwort** (Bytenummern der Datei, 1 = `59`): Byte 3 = **DTCStatusAvailabilityMask**, Byte 4 und 5 = Code (DeviceIdentifier und Rest), Byte 6 = **DTCFailureType**, Byte 7 = **StatusOfDTC** (Bit 4 = confirmedDTC, Bit 7 = testFailed).

## Routinen (`31 ...`) in der Datei
`31 21 00 00` Interlock Generator Start, `31 25 00` SOC Recalibration Start, `31 28 00 00` Check Programming Preconditions Start, `31 29 00 00` Isolation Measurement Device Start. **`31 05 00` steht nicht drin**, auch kein Eintrag zu geschuetzten DTCs oder Balancing-Sperre.

## Weitere Lese- und Schreibanfragen (Auswahl)
- `21 01` Actual Limitation Values (Strom, Spannung, Power, User SOC), `21 02` Contactor Aging Counter, `21 04` Temperatures, `21 05` OCV Characteristics.
- `21 07` **Actual Data**: Zellspannung min/max, Link Voltage, Summe Zellspannungen, Batteriespannung, Strom, Zustand Schuetz und Interlock, FUSI Mode, Fail flag und weitere (bis Byte 25). **Kein Balancing-Bitfeld** in dieser Datei, obwohl der Zoe-Ph1-Treiber `0x07` als Balancing-Register nennt (`RENAULT-ZOE-GEN1-BATTERY.h:57`). Abweichung, Ursache offen (andere Softwarestand?).
- `21 08` Actual kernel data, `21 41` und `21 42` Single Cell Voltages Teil 1 und 2, `21 61` **Battery data** (Present Full Capacity, SOH, **Battery Milage** ab Byte 12, **Sum of Energy from the beginning of battery life** ab Byte 16), `21 62` Kennung (Lieferant, Werk, Seriennummer), `21 66` Charge Counter.
- **`21 09` lesen / `3B 09` schreiben: "Mileage HV Battery At Installation" (Bytes 3 bis 5) und "At Removal" (Bytes 6 bis 8).** Schreiben ohne Sicherheitsfreigabe in der Datei nicht vermerkt (nicht geprueft).
- Aus den Ueberschriften der Datei (Beschreibung 03.02.2020): "Sum of Energy" in J statt kWh (Wert mal 3 600 000), Kapazitaeten in Ah statt As.
