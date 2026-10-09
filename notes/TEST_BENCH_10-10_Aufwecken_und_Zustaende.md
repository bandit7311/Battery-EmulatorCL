# Test am Bench-Akku, 10.10.2026: Was weckt den Broadcast, was bewegt Zeit/km/BMS-Modus?

Nur mit vorhandenen Haken auf `/simulator`, Logger und Lesen. **Nichts bauen, nichts am Akku schreiben** ausser dem Sleep-Lauf in Teil 4. Nutzer ist vor Ort und liest selbst. 12 V waehrend der Sleep-Laeufe nie trennen.

## 0. Vorbereitung
1. Logger an (Einstellung `CANLOGUSB`), **Filter leer** (Feld "CAN USB log filter" leer, Neustart). PuTTY-Mitschnitt starten, Dateiname mit Datum.
2. `/simulator`, Manual vehicle age: **1314935** eintragen, **Set** druecken, **bevor** `0x350` je eingeschaltet wird (nach jedem Neustart wiederholen). Nie den Uhrwert (2.967.669) oder 1.054.079 senden.
3. Ausgangswerte lesen und notieren (Datum/Uhrzeit): `22 92 59`, `22 92 5C`, `22 92 79`, `22 92 61`, `22 91 C1`, `22 92 5F`, `22 91 CF`, `22 92 81`.

## 1. Stille herstellen
- "All rows off" druecken. Notieren: Sekunden bis keine Akku-Frames mehr kommen (Logger/Anzeige "BMS silent"). Wenn der Akku schon still ist: Zeitpunkt notieren.

## 2. Aufwecken: welches Frame reicht?
Je Schritt **30 s warten**; notieren: kommen `0x155`, `0x424`, `0x425`, Zellframes? Nach wie vielen Sekunden? Erste UDS-Antwort nach? Danach (soweit der Akku antwortet) `9259`, `9279`, `9261`, `91C1` lesen.

| Schritt | Haken (alles andere aus) | Broadcast? | nach s | `9259` | `9279` | `9261` | `91C1` |
|---|---|---|---|---|---|---|---|
| 2a | nur `0x423` | | | | | | |
| 2b | alles aus (Stille), dann nur `0x350` | | | | | | |
| 2c | alles aus (Stille), dann `0x423` + `0x350` | | | | | | |
| 2d | alles aus (Stille), dann `0x090` + `0x242` + `0x350` + `0x53B` + `0x214` + `0x69F` (Standardsatz) | | | | | | |

Zwischen den Schritten immer erst "All rows off" und warten, bis der Akku still ist.

## 3. Zeilen nach und nach dazuschalten (auf Basis des Satzes, der den Akku weckt)
Je Gruppe 30 s warten, dann dieselben Werte lesen und notieren, was sich aendert (`9259`, `9279`, `9261`, `91C1`, `925C`, `91CF`).

| Schritt | Dazu | Erwartung/Bemerkung |
|---|---|---|
| 3a | `0x426`, `0x436` (aktuelle Zoe-Form) | `925F` soll 19.400 km zeigen |
| 3b | `0x19F` | |
| 3c | `0x69F` | Fahrzeug-ID `13 88 6F` |
| 3d | `0x350`, `0x53B`, `0x214` | Alter erst gesetzt (Punkt 0.2) |
| 3e | `0x090`, `0x242` | |
| 3f | EVC-Zeilen `0x18A`, `0x1F8`, `0x42E`, `0x427`, `0x432`, `0x650`, `0x1FD` | |
| 3g | alle uebrigen Zeilen | |

Springt `9259` auf `05` oder zaehlt `9279` / bewegt sich `9261`/`91C1`: letzte Gruppe wieder halbieren, bis die eine Zeile feststeht.

## 4. Sleep-Lauf (mit Schreibwert `0x00 activated`)
1. Vorher lesen: `9261`, `91C1`, `9275`, `9279`, `9259`.
2. "Sleep 0x9281" (Schreibwert 0x00), durchlaufen bis zum Aufwachen; 12 V dran lassen. Rueckleswert `9281` = `00` notieren.
3. Dieselben Werte nachher lesen, `9275`/`9276` und die Missionsliste komplett.

## 5. Mitschnitt auswerten (macht Claude)
- Welche Frames kommen bei 2a-2d vom Akku, in welcher Reihenfolge, nach wie vielen ms; Zuordnung der RX-Frames zu Akku und der TX-Frames zu uns.
- Vergleich der Akku-Frames mit Abschnitt 10.2 in BEFUND_Echter_Bus_LBC_EVC_09-10.md.

## Grenzen dieses Tests
- Die Zeilen `0x423`, `0x426`, `0x436`, `0x19F` senden noch die **Zoe-Form**, nicht das Format des echten Busses (Notizen Abschnitt 1 und 14). Bewegt sich nichts, ist der naechste Schritt der Modus "EVC wie im echten Bus" (Bauliste B) und braucht ein ausdrueckliches "bauen".
- Der Test beantwortet, ob `0x423` allein aus der Stille weckt (bisher gemessen nur: haelt den Broadcast, Abschnitt 14.1).

## Ergebnisse eintragen
- Teil 2:
- Teil 3:
- Teil 4:
- Auffaelligkeiten:
