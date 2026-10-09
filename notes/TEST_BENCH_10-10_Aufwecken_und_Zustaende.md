# Test am Bench-Akku, 10.10.2026: Was weckt den Broadcast, was bewegt Zeit/km/BMS-Modus?

Nur mit vorhandenen Haken auf `/simulator`, Logger und Lesen. **Nichts bauen, nichts am Akku schreiben** ausser dem Sleep-Lauf in Teil 4. Nutzer ist vor Ort und liest selbst. 12 V waehrend der Sleep-Laeufe nie trennen.

## 0. Vorbereitung
1. Logger an (Einstellung `CANLOGUSB`), **Filter leer** (Feld "CAN USB log filter" leer, Neustart). PuTTY-Mitschnitt starten, Dateiname mit Datum.
2. `/simulator`, Manual vehicle age: **1314935** eintragen, **Set** druecken, **bevor** `0x350` je eingeschaltet wird (nach jedem Neustart wiederholen). Nie den Uhrwert (2.967.669) oder 1.054.079 senden.
3. Ausgangswerte lesen und notieren (Datum/Uhrzeit): `22 92 59`, `22 92 5C`, `22 92 79`, `22 92 61`, `22 91 C1`, `22 92 5F`, `22 91 CF`, `22 92 81`.

## 1. Stille herstellen
- "All rows off" druecken. Notieren: Sekunden bis keine Akku-Frames mehr kommen (Logger/Anzeige "BMS silent"). Wenn der Akku schon still ist: Zeitpunkt notieren.

## 2. Aufwecken: nur Frames, die auf dem echten BMS-EVC-Bus vorkommen
Grundsatz (Nutzer, 09.10.): `0x350` und die anderen Fahrzeug-CAN-Zeilen kommen auf dem Bus zwischen Akku und EVC **nicht** vor (gemessen, Mitschnitt 22aaf176). Die Hauptkette nimmt deshalb nur die fuenf Zeilen, die auch dort laufen: `0x423`, `0x426`, `0x436`, `0x19F`, `0x69F`. Fahrzeug-CAN-Zeilen kommen erst danach als Zusatz (Teil 3).

Je Schritt **30 s warten**; notieren: kommen `0x155`, `0x424`, `0x425`, Zellframes? Nach wie vielen Sekunden? Erste UDS-Antwort nach? Danach (soweit der Akku antwortet) `9259`, `9279`, `9261`, `91C1` lesen. Zwischen den Schritten immer erst "All rows off" und warten, bis der Akku still ist.

| Schritt | Haken (alles andere aus) | Broadcast? | nach s | `9259` | `9279` | `9261` | `91C1` |
|---|---|---|---|---|---|---|---|
| 2a | nur `0x423` | | | | | | |
| 2b | `0x423` + `0x69F` | | | | | | |
| 2c | `0x423` + `0x426` + `0x436` | | | | | | |
| 2d | `0x423` + `0x426` + `0x436` + `0x19F` + `0x69F` (alle fuenf) | | | | | | |

Wenn 2a nicht weckt, in 2b-2d feststellen, welches dazukommende Frame den Broadcast ausloest. Weckt keiner dieser Saetze, erst dann Teil 3a (Fahrzeug-CAN-Zeilen als Wecker).

## 3. Zusatz: Fahrzeug-CAN-Zeilen (kommen im Auto nicht zum Akku)
Basis: der Satz aus Teil 2, der den Akku weckt (oder alle fuenf). Je Gruppe 30 s warten, dieselben Werte lesen, notieren was sich aendert (`9259`, `9279`, `9261`, `91C1`, `925C`, `91CF`).

| Schritt | Dazu | Bemerkung |
|---|---|---|
| 3a | alles aus (Stille), dann **nur `0x350`** | Frage: weckt es, obwohl es auf dem Akku-Bus nicht vorkommt? (Code-Kommentar `:1964-1966`: `0x155` mit Ungueltig-Werten) |
| 3b | + `0x53B`, `0x214` | Alter vorher gesetzt (Punkt 0.2) |
| 3c | + `0x090`, `0x242` | |
| 3d | + EVC-Zeilen `0x18A`, `0x1F8`, `0x42E`, `0x427`, `0x432`, `0x650`, `0x1FD` | |
| 3e | + alle uebrigen Zeilen | |

Springt `9259` auf `05` oder zaehlt `9279` / bewegt sich `9261`/`91C1`: letzte Gruppe wieder halbieren, bis die eine Zeile feststeht.

## 4. Sleep-Lauf (mit Schreibwert `0x00 activated`)
1. Vorher lesen: `9261`, `91C1`, `9275`, `9279`, `9259`.
2. "Sleep 0x9281" (Schreibwert 0x00), durchlaufen bis zum Aufwachen; 12 V dran lassen. **Hinweis:** Die Sleep-Folge im Code sendet eigene `0x350`-/`0x214`-Frames (C3, C2, C0, 00), obwohl diese auf dem echten Bus nicht vorkommen; ohne Umbau nicht abschaltbar. Rueckleswert `9281` = `00` notieren.
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
