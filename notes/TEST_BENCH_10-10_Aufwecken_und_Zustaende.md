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
- **Messung nach dem zweiten NVROL-Reset (10.10., 21:34, Nutzer, mit Bildern):** MCPU im Fahrmodus, `925F` = 6.500 (gesendet). Dann `0x5D7` und `0x426` beide auf 6.508 (+8), danach Sleep 0x9281=1 und Aufwachen. Nach dem Aufwachen `91CF` = **8,000 km** (genau der Zuwachs +8 gegenueber der Basis 6.500), `925F` zeitweise 0,000 (noch keine km vom Frame uebernommen), `9261` = `91C1` = 0, `9270` nur wenige `06` am Ende. Danach beide km auf 6.512 (+4): `925F` = 6.512, `91CF` weiter 8,000 (Seitenwert, Abfragerunde bis 24 s alt; frische Lesung steht aus, erwartet 12). **Folgerung:** passt zur Regel "`91CF` summiert positive km-Zuwaechse von `0x426` im Fahrmodus"; der Pack-Wert ueberlebt einen Sleep-Lauf (8 km nach dem Aufwachen). Zeit (`9261`/`91C1`) bleibt 0.
- Aktuell gesendete Zeiten (Bild der Simulator-Seite): Fahrzeugalter manuell 1.311.529 min (fuer `0x350`/`0x523`/`0x376`), `0x436` manuell 1.311.568 min; "0x436 carries the vehicle age" ist aus, ein von Hand gesetzter Wert gewinnt. Schalter A "like the car" ist gewaehlt.
- **`91CF` = 13 km bei `925F` = 6.517 (10.10., Nutzer, Bild):** Reihe: 6.500 → 6.508 (+8, `91CF` = 8), Sleep 0x9281=1 und Aufwachen (`925F` kurz 0), 6.512 (km nach dem Wecken, Basis), 6.517 (+5) ergibt 8 + 5 = **13** (die +4 von 6.508 auf 6.512 wurden nicht gezaehlt). **Regel bestaetigt:** `91CF` summiert positive km-Zuwaechse von `0x426` im Fahrmodus; der Pack-Wert bleibt ueber einen Sleep erhalten; der **erste km-Wert nach dem Aufwachen ist die neue Basis** (zaehlt nicht mit). `9261`/`91C1` weiter 0, `9270` nur wenige `06`.
- Um `91CF` auf 6.844 zu bringen: ab 6.517 noch +6.831 (km-Feld direkt auf 13.348 setzen); noch nicht gemacht.
- **Missionsliste am Bench (Nutzer, `22 92 75`, nach dem zweiten NVROL-Reset; `9279` = 0x0E = 14 Missionen):** 32 Eintraege je 3 Byte, die ersten 18 sind 0, die letzten 14 sind (in Minuten, wie in der Antwort geordnet): 1.311.752, 1.311.630 (3 mal), 1.311.624, 1.311.614, 1.311.606, 1.311.601, 1.311.566, 1.311.518, 1.311.513, 1.311.505, 1.311.433, 1.311.428. Spanne 324 min (5,4 h). **Das MCPU stempelt seine Missionen mit einer Zeit in unserer Zeitskala** (unser Alter ab 1.311.384 aufwaerts, `0x436`/`0x350`/`0x523`/`0x376`); es bekommt die Zeit also, nur `9261`/`91C1` (Schnappschuss-Paar) sind weiter 0. Welcher Frame die Quelle ist (0x436, 0x350, 0x523 oder 0x376) ist offen. Fehlend: `9276` (Ende), `9277`/`9278` (Pack-km der Missionen).
- **Endzeiten der Missionen (`22 92 76`, Nutzer, 11.10.):** 14 Eintraege, neueste zuerst: 1.311.758, 1.311.630 (4 mal), 1.311.624, 1.311.613, 1.311.605, 1.311.566, 1.311.518, 1.311.515, 1.311.508, 1.311.504, 1.311.430. Zusammen mit den Anfangszeiten (`9275`) ergeben sich Dauern von **0 bis 10 min**, eine Mission mit **71 min** (1.311.433 bis 1.311.504, vermutlich die lange Fahrmodus-Sitzung nach dem Reset), die neueste 6 min (1.311.752 bis 1.311.758). Mehrere Missionen mit Dauer 0 (gleicher Stempel an Anfang und Ende) entstehen bei schnellen Wechseln. Die Zeitquelle (0x350, 0x436, 0x523 oder 0x376) ist noch offen. Entscheidungstest: `0x350`-Alter (Feld "Manual vehicle age") und `0x436` auf weit auseinanderliegende Werte setzen (z. B. 1.400.000 und 1.500.000), eine Mission erzeugen (Sleep-Lauf und Aufwecken), dann `9275`/`9276` lesen.
- **Pack-km der Missionen (`22 92 77`, Anfang, Nutzer, Antwort am Ende abgeschnitten):** 32 Eintraege je 4 Byte, `(raw ^ 0x80000000) / 32`. Die ersten 18 sind `80 00 00 00` = 0 km, danach 6 mal `80 00 01 A0` = **13 km**, dann `80 00 01 00` = **8 km**, danach weitere (nicht uebermittelt). Das passt zur Messreihe: `91CF` = 8 nach +8, 13 nach +5 (Missionen speichern den Pack-Zaehler zum Zeitpunkt des Missionsanfangs). Die neuesten sechs Missionen begannen bei 13 km, die siebte bei 8 km. Offen: Rest der Antwort, `9278` (Ende).

## Ergebnis Bench 12.10. (vom Nutzer berichtet, kein Log)
- **Zeit in 0x436 um +1000 min erhoeht:** E14381 (MCPU, "CAN from EVC/HEVC") ging von Status `2F` auf `2E` (Bit `0x01` "testFailed" nicht mehr gesetzt) und liess sich quittieren (loeschen). Das ist der erste Eingriff in diesem Aufbau, der E14381 veraendert hat; alle bisherigen (alle Zeilen zugeschaltet, Pruefsummen, `0x426`-Zustaende, km) liessen den Fehler auf `2F`.
- **`9261` (MCPU) steht weiterhin auf 0**, auch nach `9281` Temporisation (Schreiben) und Sleep.
- **Nicht genannt, deshalb offen:** Basiswert und Zielwert von 0x436 vor/nach +1000; ob E14381 nach dem Quittieren ausblieb oder wieder kam; welche Zeilen (0x426 und uebrige) an waren; Werte von `91C1`, `9279`, `9259` vor/nach.
- **Bekannt aus den Notizen zum Vergleich:** Missions-Zeiten im Pack (`9275`/`9276`) liegen bei 1.311.430 bis 1.311.758 min; `91C1` stand am 06.10. auf 1.311.344; die Voreinstellung des Emulators ist 1.025.301 min (+1 je Minute).
- **`925E` (MCPU, "Vehicle ID", 24 Bit) gelesen am 12.10. (Nutzer, Ziel DB):** `22 92 5E` -> `62 92 5E 13 88 6F`. Das ist gleich den Bytes 1-3 von 0x69F (`46 13 88 6F`, so sendet der Emulator, `RENAULT-TWINGO-GEN1-BATTERY.h:345`) und gleich dem Auto-Wert (`13 88 6F`). Frueher am Bench: `00 00 00` (vor dem Schreiben der ID, Commit `3752043` vom 11.10.).
- **Einordnung (Schluss):** Die behauptete Bedingung "0x69F muss mit 925E uebereinstimmen, sonst uebernimmt die LBC die Zeit nicht" (Quelle der Behauptung unbekannt; in der MCPU- und SCPU-Definition steht dazu nichts) ist am 12.10. erfuellt, und `9261` (MCPU) steht trotzdem auf 0. Die Uebereinstimmung ist damit hoechstens eine notwendige, nicht die ausreichende Bedingung. Unbelegt bleibt, ob sie notwendig ist.
- **Format der Bus-Frames am 12.10. (Nutzer, /simulator):** "Fahrzeug (Standard)" gewaehlt, nicht "Zoe alt". 0x436 geht damit im Auto-Layout raus: 6 Byte, B0 `80`/`AD`, B1-B3 Alter, B4/B5 `00 00` (`RENAULT-TWINGO-GEN1-BUS.h:738`). Im Auto sind B4/B5 in allen 16.968 Frames der drei Logs konstant `00 00`, eine Pruefsumme wurde nicht gefunden. Die Vermutung "andere Startposition oder zusaetzliches Pruefsummen-Byte" ist damit fuer 0x436 nicht gestuetzt.
- **Fehlerspeicher am 12.10. (Nutzer, Maske `09` = testFailed oder confirmed):**
  - MCPU (Ziel DB): `19 02 09` -> **keine Fehler**. E14381 ist nach dem Quittieren nicht zurueckgekommen; auch `1B0E41` (6926 "External NV memory (EEPROM, FLASH)", Test 65 "general checksum failure", frueher aktiv `2F`) steht nicht in der Liste.
  - SCPU (Ziel DC): `19 02 09` -> `59 02 FF`, ein Eintrag **`1BB587 = 2F`** (7093 "CAN current sensor (SCPU)1", Typ 135 "Missing frame(s) from CAN Current sensor"; testFailed gesetzt, aktiv). Unveraendert gegenueber frueheren Lesungen (`2C`/`2F`). Im Auto-Dump war dieser Code nicht aktiv (dort `1BA249` Status `0x68`).
  - `19 02 FF` (Freier Request) lieferte immer "no response"; Ursache nicht geklaert (moegliche Gruende: lange Mehrfachantwort bei 2-s-Wartezeit, oder nur in Extended Session). Mit Maske `09` kommen Antworten.
- **Folgerung:** Ein aktiver Speicherfehler (`1B0E41`) blockiert das Speichern von `9261`/`91C1` am 12.10. nicht, denn er ist nicht aktiv. `9261` (MCPU) steht trotzdem auf 0.
- **Zustand und Zeit am 12.10. (Nutzer):** MCPU `9259` = `05`, `925C` = `01` (Kombination der Auto-Dumps im Fahrbetrieb). MCPU `9261` = `00 00 00` (0 min). SCPU `9261` = `14 C4 32` = **1.360.946 min** (folgt dem gesendeten 0x436-Wert, vgl. Abschnitt 1 in BEFUND_SCPU_0436_Bench_08-10.md). `91C1` wurde nicht gelesen.
- **Folgerung:** Die MCPU ist im Fahrmodus (`9259` = `05`), und ihre `9261` steht trotzdem auf 0, waehrend die SCPU die Zeit uebernimmt. Der Fahrmodus allein ist also nicht die fehlende Bedingung. Im Auto aendert sich die gespeicherte MCPU-`9261` nur am Sitzungsende (Ladelog 1: `2D 59 D8` -> `2D 5A D9`); ein Wert 0 waehrend der laufenden Sitzung ist daher noch kein Fehlerbefund.
- **`925B` (MCPU reset status, `Wxx_BMS_rst_U_16`) am 12.10. (Nutzer):** `62 92 5B 00 10` = `0x0010`. In allen 13 MCPU-Dumps (Auto und Bench) steht `00 00`. Bitliste der Definition: 0 RAM check error, 1 ROM check error, 2 Stack check error, 3 Trap error, **4 Software Reset**, 5 Watchdog Reset 0, 6 Watchdog Reset 1, 7 External Reset, 14 Flash security violation Reset, 15 Irregular Reset. Bei Zaehlung ab Bit 0 (LSB) ist Bit 4 = Software Reset. Die Bitreihenfolge ist aus der Definition nicht belegt. Passend: Der NVROL-Reset (B009) startet die MCPU neu (siehe 10.10., "no response" = Neustart).
- **Reset-Status der SCPU am 12.10. (Nutzer: "auch 0010"; gemeint ist nach Rueckfrage-Annahme `[DC] 22 92 5E`, Wortlaut der Antwort nicht uebermittelt):** `00 10`. In allen SCPU-Dumps stand `925E` = `20 80` (Bit 7 External Reset, Bit 13 Power-Up Reset). `00 10` = Bit 4 = Software Reset (Bitliste `DID_$925E.00` bis `.15` der SCPU-Definition). Die MCPU zeigt in `925B` ebenfalls `00 10`. SCPU `925B` ist dagegen der Index des kaeltesten Thermistors (kein Reset).
- **Korrektur zum Fehlerspeicher der MCPU am 12.10. (spaetere Lesung, Nutzer):** `19 02 09` (Ziel DB) -> `59 02 FF 1B 0E 41 2F`, **`1B0E41 = 2F` aktiv**. Das ersetzt die frueher am selben Tag gemeldete Lesung "keine Fehler in der MCPU" (Zeitpunkt beider Lesungen und Zwischenschritte nicht festgehalten). `1B0E41` = 6926 "External NV memory (EEPROM, FLASH)", Test 65 "general checksum failure" (MCPU-Definition Zeilen 27957-27960). Der Code war schon am 10.10. mit `2F` aktiv (fruehere Staende `28`/`2C`) und kommt offenbar zurueck. Die Hypothese "aktiver Speicherfehler blockiert das Speichern von `9261`/`91C1`" bleibt damit offen und ist zu testen (DTC loeschen, sofort und nach Sitzungsende neu lesen).
- **SCPU-Fehlerspeicher, spaetere Lesung am 12.10. (Nutzer):** `[DC] 19 02 09` -> `59 02 FF 1B B5 87 2F 1B B1 08 2E`, also `1BB587 = 2F` (aktiv, wie zuvor) und neu in der Liste **`1BB108 = 2E`** (7089 "Safety CPU/Slave Communication", `slv_com_2_bus`, Typ 8 "Bus Signal / Message Failures"; Bit `0x01` nicht gesetzt = nicht aktuell, bestaetigt). `1BB108` stand schon am 08.10. in der SCPU (Status `28`, siehe BEFUND_SCPU_0436_Bench_08-10.md, Zeilen 90-96). E14381 steht in dieser SCPU-Liste nicht.
- **`925B` (MCPU) spaeter am 12.10. (Nutzer):** `62 92 5B 80 10` (vorher `00 10`). Bits ab LSB: Bit 4 Software Reset (wie zuvor) und neu **Bit 15 Irregular Reset** (Bitliste der MCPU-Definition). Dazwischen lief ein weiterer Reset der MCPU; was dazwischen passiert ist (Schlaf, 12-V-Unterbrechung, NVROL, Neustart) ist nicht festgehalten. SCPU-`925B` in derselben Zeit: `00` (Index kaeltester Thermistor, kein Reset). Bestaetigung von `[DC] 22 92 5E` steht noch aus.
- **Bestaetigung (Nutzer, 12.10.):** `[DC] 22 92 5E` -> `62 92 5E 00 10`. Damit ist die Annahme im Eintrag "Reset-Status der SCPU" bestaetigt: SCPU `925E` = `00 10` = Bit 4 Software Reset (frueher in allen Dumps `20 80`). Die SCPU hat das zweite Bit der MCPU (Irregular Reset, Bit 15) nicht.
