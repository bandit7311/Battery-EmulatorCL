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

## Nachtrag 18.11.2026: geschuetzte DTCs und Balancing (Quelle vom Nutzer, **nicht geprueft**)
Quelle: `QUELLE_Zoe_Ph1_geschuetzte_DTCs_18-11.md` (Herkunft unbekannt; Nutzer: soll Ph1 sein). Nur gemerkt, nichts gebaut, nichts geaendert.

**Was die Quelle behauptet:**
1. Ein gespeicherter DTC der **Klasse 1** (Beispiel P3090-11) setzt ein Flag, das die Schwere (2,1) auf 3 haelt, **auch wenn der Fehler nicht mehr aktiv ist**.
2. Das Balancing-Tor erlaubt Entladen nur bei Schwere 0. Sonst wird die Balancing-Maske geloescht: `21 07` liefert nur Nullen.
3. **`14 FF FF FF` ueberspringt Slots der Klassen 1, 2 und 4** und setzt das Flag nie zurueck. Nur die Routine **`31 05 00`** loescht die geschuetzten DTCs und das Flag.
4. Offen laut Quelle: ob gespeicherte Klasse-2- und Klasse-4-DTCs das Balancing auch beeinflussen.

**Was das fuer unseren Code bedeuten wuerde (Schluss, nicht belegt):**
- "Erase DTC" bei der Ph1 sendet `10 03` und `14 FF FF FF` (`UdsCanBattery.cpp:429-439`, geerbt). Er koennte genau die Fehler **nicht** loeschen, die das Balancing sperren. `31 05 00` kennen weder dala noch wir.
- Ein Loeschversuch mit `31 05 00` waere ein Test an einer echten Ph1. Ob die Sitzung `10 C0` (Lesen) oder `10 03` noetig ist, ist offen.

**Dazu passende Beobachtungen vom 18.11. (Screenshot Ph1, nicht unser Pack):**
- Angezeigt wurde `4F33D2`, Status "Confirmed", Beschreibung "Unknown", "0/1 DTCs matched". In `renault_zoe_gen1_dtc.json` steht `P33D2` = "Temperature Sensor 11 out of range" (110 Eintraege, nur Textschluessel `dtc`, kein `code`).
- Vermutung: das fuehrende `4F` gehoert nicht zum Code (vermutlich die Statusmaske, Lesen ab Byte 2: `UdsCanBattery.cpp:458-466`). Dann waere auch der Status aus dem falschen Byte. Pruefung nur mit den Rohbytes der Antwort.
- "Unknown" auch ohne das `4F`: der Gen1-Treiber sucht ueber die Dezimalzahl (`get_dtc_standard_code_string()` = false), die Datei hat nur Textschluessel.
- In der Quelle hat ein gespeicherter DTC 3 Bytes (`30 90 11` = P3090 mit Fehlerart 11). Das passt zur Vermutung mit dem Versatz, beweist sie nicht.

**Offen:** Herkunft der Quelle; welcher Klasse `P33D2` angehoert; ob `31 05 00` bei der Ph1 loescht; ob "Time/Balancing" der Ph1 am Bench dadurch wieder laeuft.

## Nachtrag 18.11.2026: Suche im Internet nach Referenzen (nichts gebaut)
**Gefunden:**
- **PyRen** (Spiegelkopie `emtee40/pyren-OBD2`, `pyren/mod_ecu.py`, Auszug eines Abrufs, die Datei wurde nicht vollstaendig gelesen): Standard fuer den Start der Diagnosesitzung ist **`startDiagReq = '10C0'`** (`find_real_ecuid`). Das stuetzt, dass Renault-Steuergeraete mit KWP2000 `10 C0` nehmen, also das Lesen bei der Ph1 (`RENAULT-ZOE-GEN1-BATTERY.cpp:271`).
- Die **Bytes zum Loeschen stehen nicht im PyRen-Programm**, sie kommen aus den Renault-Steuergeraetedateien (XML) der Datenbank. Dasselbe bei **DDT4ALL**: Die ECU-Datenbank ist ein getrennter Download (`ecu.zip`), nicht im Repository.
- Allgemeines UDS: `0x14` = ClearDiagnosticInformation (`uds.readthedocs.io`). Nicht Ph1-spezifisch.

**Nicht gefunden (vier Suchen):** `14 FF 00` fuer die Ph1-LBC, `31 05 00`, geschuetzte DTC-Klassen, `P33D2`. Alles dazu bleibt **ungeprueft**.

**Naechster Schritt (offen, beim Nutzer):** die Steuergeraetedatei des Zoe-Ph1-LBC aus der Datenbank von PyRen oder DDT4ALL oeffnen und nachlesen, was unter "Clear DTC" steht. Die Datenbank muss auf dem PC des Nutzers vorhanden sein.

**Quellen:** https://raw.githubusercontent.com/emtee40/pyren-OBD2/master/pyren/mod_ecu.py, https://github.com/cedricp/ddt4all, https://github.com/cedricp/ddt4all/discussions/1683, https://uds.readthedocs.io/en/latest/pages/knowledge_base/service.html
