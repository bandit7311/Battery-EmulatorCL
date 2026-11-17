# Merkliste Zoe Ph1 (Gen1): DTC lesen und löschen, Stand 17.11.2026

Nur gemerkt, nichts gebaut, nichts geändert. Quelle: Code von dala `origin/main` (191de3594) und `v13.6.0`.

## Befund im Code (gelesen, nicht auf dem Gerät geprüft)

- **Lesen** ist für die Ph1 eigens programmiert: `RENAULT-ZOE-GEN1-BATTERY.cpp:263-275`.
  - Ablauf: `10 C0` (DiagnosticSessionControl, Session 0xC0), danach `19 02 09`.
  - Steuergerät `0x79B`, Antwort `0x7BB` (`:239`).
  - Auswertung für KWP2000 (Kopfbyte `0x59`, Offset 2), weil die PIDs einbytig sind (Service `0x21`).
  - Eingeführt in Commit `4e600b6b8` (08.08.2026). Eine Begründung für `0xC0` steht im Commit nicht. Die Commit-Beschreibung nennt die Maske `0xFF`, der Code sendet `09`.
- **Löschen** hat Gen1 **nicht** angepasst. Es erbt von `UdsCanBattery`:
  - `UdsCanBattery.cpp:429-439`: `10 03`, danach `14 FF FF FF`.
  - Die Antwort auf `10 03` wird nicht geprüft. Der Zustand `UDS_STATE_CLEAR_DTC_DIAG` sendet das Löschen in jedem Fall weiter.
- Es gibt **keinen Test** für Zoe Gen1 DTC im Original (`test/battery` enthält nur `UdsCanBatteryTest.cpp`).

## Offen (nicht belegt)

- Ob die Ph1-LBC `10 03` und `14 FF FF FF` annimmt. Die Ph1 spricht KWP2000.
- Hintergrundwissen, **nicht aus dem Repository belegt**: Bei Renault-KWP2000 ist `10 C0` die herstellerspezifische Session, und das Löschen ist üblicherweise `14 FF 00`.
- Ob das Löschen bei einer echten Ph1 funktioniert. Es gibt weder Test noch Bericht.

## Möglicher Weg 1 (nur Zoe Gen1, ca. 12 Zeilen in 2 Dateien, nicht eingebaut)

Nicht in `UdsCanBattery.cpp` die Session von `\x03` auf `\xC0` ändern. Die Zeile gilt für alle UDS-Akkus (Zoe Gen2, Kia, Bolt, Stellantis, MG, CMP, ECMP und die anderen).

1. `RENAULT-ZOE-GEN1-BATTERY.h`, nach `void read_DTC() override;` (Zeile 31): `void reset_DTC() override;` (+1 Zeile).
2. `RENAULT-ZOE-GEN1-BATTERY.cpp`:
   - Nach Zeile 263: `static const uint16_t ZOE_STATE_CLEAR_SESSION = 2;` (+1).
   - Nach `read_DTC()` (Zeile 267): `reset_DTC()` mit `start_sequence(ZOE_STATE_CLEAR_SESSION);` (+4).
   - In `on_uds_sequence_step()` zwei neue `else if`-Zweige (+6): erst `10 C0` mit `ZOE_STATE_CLEAR_SESSION + 10`, bei positiver Antwort `14 FF FF FF` mit `UDS_STATE_CLEAR_DTC` und Timeout 25.
3. Lehnt die Ph1 `10 C0` ab, endet die Folge ohne Löschen. Das ist anders als in der Superklasse.

## Entscheidungen

- **12 Zeilen, Weg 1: nicht eingebaut, nur gemerkt** (Nutzer, 17.11.2026).
- Alternative (nicht gewählt): einstellbare Session in der Superklasse, Standard `0x03`, Gen1 setzt `0xC0`. Berührt die gemeinsame Datei `UdsCanBattery.cpp/.h`.
- Test, sobald eine Ph1-Zoe verfügbar ist: Fehler lesen, **Erase** drücken, nochmal lesen.
