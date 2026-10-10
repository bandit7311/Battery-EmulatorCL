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

## 2e. Danach die bisher unbekannten EVC-Frames nach und nach (erst nach dem Umbau, Bauliste 1b)
Nutzer 10.10.: frisch anfangen und die Frames nach und nach zuschalten; `0x428` und `0x435` kommen auf dem echten Bus vom EVC-Teil, wir hatten bisher nichts damit zu tun (nicht im Code, nicht in der Simulator-Liste). Reihenfolge (jeweils zusaetzlich zu den fuenf, je 30 s warten, `9259`, `9279`, `9261`, `91C1`, `925C`, `91CF` lesen). Inhalt je Zustand aus dem Auto-Mitschnitt 22aaf176:

| Schritt | Frame | Takt | Verhalten im Auto (gemessen) |
|---|---|---|---|
| 2e | `0x435` | 100 ms | `FF FF 1D EB 33 FC 00 00` beim Wecken, danach B2 `1D/1E`, B3 `EB`->`0B`, B5 `FC`->`00` (aendert sich beim Wecken und bei Tuer auf) |
| 2f | `0x428` | 100 ms | `00 00 00 00 00 06 00 00`, beim Start kurz `... 07 C0 00` |
| 2g | `0x42F` | 100 ms | `00 20 FF 80 0F B0`, beim Start kurz alles `00` |
| 2h | `0x4F7` | 100 ms | `00 1C/1D 08 00 [B4] 40 06 F0`, B4 aendert sich bei Zuendung 1 (`B0`->`90`) |
| 2i | `0x419` | 100 ms | 6 Byte, Werte schwanken je Zustand (unklar, ob vom EVC) |
| 2j | `0x0ED` | 10 ms | 3 Byte, B1 `FF`->`CC` bei GO |
| 2k | `0x500` | 100 ms | 5 Byte, ab ca. 15 s nach Zuendung 1 |
| 2l | `0x511` | 100 ms | 7 Byte, B0 `04` (Ruhe) / `00` (Zuendung 3, Fahrt) |
| 2m | `0x1C7`, `0x157`, `0x1A1`, `0x0EC` | 10 ms | schnelle Frames, Inhalt aus dem Mitschnitt |

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


## Ergebnis Bench 10.10. (Firmware 0d0d8d2, vom Nutzer berichtet, kein Log)
- Format "Fahrzeug" (Standard). Zuerst nur `0x0EC` und `0x0ED` angehakt: **keine** Frames vom Akku.
- Danach `0x423` dazu: es kommen sofort alle 13 Akku-Frames (`0x155`, `0x0C5`, `0x1C9`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, `0x4AE`, `0x4AF`, `0x659`, `0x6BE`) und alle 20 Zellframes.
- Schluss: `0x423` weckt den Akku; `0x0EC`/`0x0ED` allein nicht. Ob `0x0EC`/`0x0ED` bei laufendem `0x423` etwas aendern (Zustand, MCPU), ist ungetestet.
- Weiter beobachtet: DTC `1B0E41` mit Status `2F` (aktiv, erstmals mit gesetztem Bit 0; fruehere Staende `28`/`2C`); Ausloeser unklar.
- **Fehler gefunden (10.10.):** `9261` = `0F A8 C4` = 1.026.244 min = unser gesendetes Alter (1.025.301 + 943 min seit Unix 1791590400). Der Pack uebernimmt unser Alter. `925F` = `00 6E 71 70` = 72.380 km statt 6.844: `0x426` Bytes 3-5 = `01 1A BC` = 0x011ABC = 72.380; Byte 3 war fest `01`. Im Auto: `01 6A 70` = 92.784 km. Korrektur: km 24 Bit in Bytes 3-5 (Commit cb190c5, Fahrzeugformat). `9262` (SCPU) zeigt denselben Wert (`80 6E 71 70`). Der Pack-Zaehler `91CF` wurde nicht gelesen.
- **`9259` = 05 und `925C` = 01 am Bench erreicht (10.10., vom Nutzer berichtet, Firmware mit 0x426 24 Bit, DC-Antworten `62 92 59 05` und `62 92 5C 01`).** Das ist die Kombination der Auto-Dumps im Fahrbetrieb (MCPU und SCPU), erstmals am Bench. Offen: welche Zeilen und welcher Zustand angehakt waren und in welcher Reihenfolge; ob es bei einzeln ausgeschalteten Zeilen bleibt; `91CA`/`91CB` (Haupt-/Vorladerelais), `9279`, `91C1`, `91CF`. Sicherheit: `925C` = 01 heisst im Auto vermutlich geschlossene Schuetze, also Hochvolt an den Polen pruefen (Annahme, nicht belegt).
- **Weitere Lesungen am Bench (10.10., vom Nutzer berichtet):** `9279` = `01`; `91C1` = `14 02 70` = **1.311.344** (unveraendert der alte Wert vom 06.10.), waehrend `9261` = `0F A8 C4` = 1.026.244 (unser gesendetes Alter, folgt live). `9261` und `91C1` sind damit **nicht mehr gleich**: `9261` folgt unserem Frame, `91C1` ist der gespeicherte Schnappschuss und wurde **nicht** erneuert. Naechster Test (Hypothese "gespeichert nach Sitzungsende, ca. 3 min"): Zustand auf "aus" (erwartet `9259` 04, `925C` 02, `9279` +1), Frames weiter senden, nach ca. 3 min `91C1` und `9261` lesen; danach Zustand "zu" und Frames enden wie im Auto (`0x0EC` zuerst, `0x0ED` zuletzt).
- `91CF` = `80 00 00 00` = 0 km (Pack-km, Offset 0x80000000): unveraendert 0 seit dem NVROL-Reset (vorher 6.844), folgt `925F` (live, 72.380 km vor der Korrektur) nicht. Wie `91C1` ein gespeicherter Wert, der noch nicht geschrieben wurde.
- **`925C` folgt dem Zustand (10.10., vom Nutzer berichtet):** Mit Zustand nur "awake" zeigen MCPU und SCPU (`[DC]`) `925C` = `02` (`62 92 5C 02`); vorher mit dem anderen Satz/Zustand `01` (mit `9259` = 05). `02` ist der Wert des Autos bei "Zuendung aus" (dort `9259` = 04). Offen: `9259` in diesem Zustand, und bei welchem Zustand genau `01`/`05` auftrat.
- **Angehakter Satz (10.10., vom Nutzer berichtet), Zustand "awake", Format Fahrzeug:** `0x0EC`, `0x0ED`, `0x19F`, `0x426`, `0x436`, `0x419`, `0x423` (7 Zeilen aus Block 1). Mit diesem Satz `925C` = `02` (MCPU und SCPU). Unklar, ob das auch der Satz ist, bei dem E14281/E14381 verschwanden und welche Zeile zuletzt dazukam; `0x419` (100 ms, im Auto `56 00 28 00 3A 00` o. ae., Inhalt je Zustand aufgezeichnet) ist die einzige Zeile dieses Satzes, die nicht zu den bisher getesteten gehoert.
- **0x426 an/aus, Zustand "awake" (10.10., vom Nutzer berichtet; MCPU = ohne Kennung, SCPU = `[DC]`).** Angehakt waren `0x0EC`, `0x0ED`, `0x19F`, `0x436`, `0x419`, `0x423`.

| | `9259` MCPU | `9259` SCPU | `925C` MCPU | `925C` SCPU | DTC |
|---|---|---|---|---|---|
| **ohne** `0x426` | **04** | 05 | **03** | 01 | keine |
| **mit** `0x426` | **05** | 05 | **01** | 01 | **E14381 aktiv** |

- Schluss (gemessen am Wert, Ursache offen): `0x426` ist der Frame, der das MCPU von 04/03 auf 05/01 (Fahrmodus) bringt; das SCPU steht auch ohne `0x426` auf 05. Preis: E14381 ("CAN from EVC/HEVC", ungueltige Daten) ist mit `0x426` aktiv, ohne `0x426` ist er weg (wie in `MERKZETTEL_Zeitwerte_33BE.md`, Z. 126/285). Die Erwartung in `BEFUND_Echter_Bus_LBC_EVC_09-10.md`, Abschnitt 17.2, "E14381 bleibt aus" mit dem echten Format ist **nicht eingetreten** (Firmwarestand beim Test unklar).
- Offene Hypothese (nicht belegt): Mit `0x426` ueberwacht das MCPU weitere EVC-Frames (`0x157`, `0x1A1`, `0x1C7`, `0x428`, `0x42F`, `0x435`, `0x4F7`, `0x511`, `0x500`), die wir nicht senden. Naechster Test: mit `0x426` an die uebrigen EVC-Zeilen aus Block 1 nacheinander dazu, nach jeder DTC lesen.
- **Test A (10.10., vom Nutzer berichtet): 0x426 mit km = 92784 (Auto-Frame exakt im Zustand "awake"):** `925F` = 92.784 km (Anzeige zeigt `.000`, Aufloesung 0,01 km), **E14381 bleibt (keine Aenderung)**. Damit ist der Inhalt von `0x426` im Zustand "awake" nicht die Ursache.
- **0x426 im Auto-Log (6.080 Frames, 16 verschiedene):** B0 = `00`; B1 = `00`/`08`/`60`/`70`; B2 = `02` zu, `06` wach, `65` Zuendung 1, `69` Zuendung 3 und Fahrt, `61` GO, `05` aus, dazu Uebergaenge `67`, `07`; B3-B5 = km 24 Bit (`01 6A 70` = 92.784, einmal `01 6A 71` bei T+1412,1 s im Fahren); B6 = `00`; B7 = `40`; Takt 100 ms. Jede Sitzung beginnt mit `00 00 02` (1,1-1,2 s), `00 08 02` (0,6-0,7 s), `00 08 06` (0,4-0,5 s), dann `00 00 06`. Uns fehlt dieser Anlauf und die Uebergangswerte.
- **`0x157` als Kandidat fuer E14381 ("Invalid data received on CAN Current sensor frames"):** B1 = Zaehler (+5), B4 `FF`, B5 `E0` fest, B2/B3 = 16-Bit-Wert, ueberwiegend `09 C4` (2.500), zeitweise `0A 3C`, `08 xx`, `0C xx`; B0 nimmt 256 Werte an, meist `D5`. **Annahme:** ein gemessener Wert (Strom?) mit Pruefbyte. Test: mit `0x426` an zuerst `0x157` einschalten.
- **`91CF` im Auto-Log:** Antwort `80 0C 28 40` = 24.898 km, gleich bei T+90,6 s, 341,2 s, 570,9 s und 1360,2 s (auch nach rund 2 min Fahrt in Sitzung 1). Der Pack-Zaehler laeuft nicht mit jeder Fahrt; `9245` stieg in Sitzung 1 um `0x38` (56 Schritte = 0,056 kWh). Bench: `91CF` = 0 seit dem NVROL-Reset (vorher 6.844 = `80 03 57 80`). Schreiben waere `2E 91 CF 80 03 57 80`; nicht empfohlen, solange `1B0E41` (Fehler im externen Dauerspeicher) aktiv ist.
- **`91CF` (Pack-km) zaehlt Zuwaechse der Fahrzeug-km von `0x426` (10.10., Messreihe vom Nutzer, MCPU im Fahrmodus `9259` = 05):**

| Lesung | `925F` (Fahrzeug-km) | `91CF` (Pack-km) |
|---|---|---|
| 1 | 6.482 | 85.942 |
| 2 | 6.487 | 86.304 (+362) |
| 3 | 6.500 | 86.317 (+13) |

  Erklaerung, die alle Werte trifft (**Schluss, nicht belegt**): `91CF` summiert nur positive Aenderungen der km aus `0x426`, abnehmende Werte werden nicht abgezogen. +362 = 6.844 − 6.482 (die km standen zwischen Lesung 1 und 2 vermutlich kurz auf 6.844, dem Standard nach einem Neustart, und wurden danach auf 6.487 gesetzt); +13 = 6.500 − 6.487. Auch der Startwert 85.942 passt (Testwert 92.784 minus rund 6.842 Ausgangswert), bei zwei km Abweichung. Es braucht dazu den Fahrmodus (`9259` = 05). Gegenprobe: km um 100 senken (`91CF` bleibt), dann um 50 anheben (`91CF` +50).
- `9270` (Missionsliste, 32 Eintraege): Zahl der `06` ("Driving") am Ende waechst mit der Zeit: 8, 12, 17, 18 (Lesung 1 bis 4); das MCPU haengt im Fahrmodus laufend Eintraege an. Uhrzeiten der Lesungen fehlen, die Rate ist offen.
- `0x155` verworfen (ungueltiger Strom oder SOC): 295, dann 590; steigt seit dem Sleep-Lauf an.
- **NVROL-Reset am 10.10. nach der Messreihe: der Reset hat gewirkt (Berichtigung).** Lauf-Log: `Routine B009: no response`, `Routine B009 results: NEGATIVE SID=0x31 NRC=0x7F` (Ergebnisabfrage nach dem Neustart des MCPU), Session2 und Schreiben `9281` = 01 OK. Frische Werte danach (Seite von vor dem naechsten Lauf): `9261` = `91C1` = 0, `91CF` = 0 (war 86.317), `9245` = 0,00 kWh (war 0,46), `9270` nur Nullen (vorher 18 mal `06`), SOH 100,000 %, `925F` = 6.500 (unser Frame). Die fruehere Aussage in der Sitzung, der Reset sei wegen des Fahrmodus abgelehnt worden, war falsch: "no response" ist der Neustart des MCPU; `NRC 0x7F` ist die Folgeabfrage.
- **Plan, Pack-km sauber aufzubauen (Annahme aus der km-Zuwachs-Beobachtung, noch zu pruefen):** Fahrmodus `9259` = 05 abwarten, km-Feld auf 1000 (Basis), 15 s warten, dann auf 7844 (+6844); erwartet `91CF` = 6.844.
