# ZIEL (Nutzer, 02.11.2026): herausfinden, was den Balancing-Plan aktiviert, damit der Bench-Pack ihn auch anlegt

**Merksatz des Nutzers:** Ohne zu wissen, was den Plan aktiviert, hilft nichts von dem, was wir hier tun. Das Ergebnis wird fuer die **Bench** gebraucht (Bench-Pack legt bisher keinen Plan an, `912B` bleibt dort 0, Zaehler `9251`/`9252` bewegen sich nicht).

Kennzeichnung: **gemessen** = aus Dump/Airtable/Mitschnitt, **Schluss** = abgeleitet, **offen**.

## Stand der Beobachtungen (aus Befund 35 bis 37b)
- **Gemessen (Plan nach Fahrt, Auto):**

| Fahrt | USOC Start -> Ende | km | Dauer | `926E` | Plan danach? |
|---|---|---|---|---|---|
| 23.10. 12:32 bis 13:04 | 46 -> **29** | 26 | 32 min | 0x0A | **ja** (24.10., 20 Bits) |
| 02.11. 07:17 bis 07:50 UTC | 54 -> **34** | 27 | 33 min | 0x0C | **ja** (15:54, 7 Bits) |
| 02.11. 16:37 bis 17:59 UTC | 33 -> **15** | 28 | 82 min | 0x0C | Plan stand schon, `912B` unveraendert |
| 29.10. 07:57 bis 08:26 | 97 -> 81 | 25 | 29 min | 0x08 | nein |
| 29.10. 17:22 bis 17:50 | 82 -> 73 | 15 | 28 min | 0x04 | nein |
| 30.10. 07:57 bis 08:18 | 72 -> 62 | 14 | 21 min | 0x04 | nein |
| 30.10. 16:10 bis 16:43 | 63 -> **47** | 26 | 33 min | 0x0A | **nein** |
| 01.11. 20:58 bis 21:23 | 43 -> 43 | 0 | 25 min (Zuendung an, Stand) | 0 | nein |

- **Gemessen (sauberes Paar):** Die Fahrten 23.10. (46 -> 29 %) und 30.10. 16:10 (63 -> 47 %) haben **gleiche km (26), fast gleiche Dauer (32/33 min) und gleiche Energieklasse `926E` = 0x0A**; einzig der **SOC-Bereich** unterscheidet sie: Plan nur bei der Fahrt, die bei 29 % endete.
- **Gemessen:** Alle drei Plan-Faelle enden bei **USOC <= 34 %**, alle Faelle ohne Plan bei **>= 47 %** (oder ohne Fahrt).
- **Gemessen (Plan bei Wecken neu berechnet?):** 15:54 UTC 7 Bits (Zellen 16, 24, 34, 68, 70, 86, 92), 16:38 UTC 8 Bits (62, 66, 77, 80, 90, 92, 95, 96), dazwischen 725 Schritte `9252`. **Schluss:** die Auswahl wird bei jedem Wecken neu berechnet; sie folgt nicht der aktuellen Zellrangfolge (Zellscan 18:2x UTC: Planzellen auf Rang 24 bis 68 von 96).
- **Gemessen (Bench):** Entladungen 97 -> 69, 75 -> 44, **44 -> 34 %** (Index 8, `926E` = 6) ohne Plan und ohne Zaehleranstieg; Bench-Strom -3,5 bis +15 A gegen bis -131 A im Auto (Befund 35).

## Kandidaten fuer den Ausloeser (Hypothesen, nicht bewiesen)
1. **SOC bzw. Zellspannung am Fahrtende** unter einer Schwelle (zwischen 34 und 47 % USOC). Spricht dagegen: Bench bei 34 % ohne Plan (aber nur Mission mit kleinem Strom/Energie; Schwelle evtl. strikt unter 34 %).
2. **Entladeleistung/-strom** der Fahrt (Auto bis -131 A, Bench nur wenige A).
3. **Zustand des EVC** in der Mission (Fahrbetrieb mit GO, Zuendung, Schlaffreigaben).
4. **Zeit/Zyklus**: Plan bei Wecken nach langem Schlaf (Latch/Uhr), nicht nach kurzem Wecken.
5. **Zellspreizung/Zellspannungsniveau** (Auto 5 bis 7 mV, Zellen ~3,58 V bei ca. 16 % SOC).

## Naechste Pruefungen (geordnet)
- Uebersicht der Zaehlersprunge vs. SOC-Ende aller Fahrten aus allen verfuegbaren Dumps (06.10. bis 02.11.), um Hypothese 1 ohne neue Messung zu pruefen.
- Wecken ohne Fahrt nach dem ungestoerten Lauf (02.11. ca. 19:08 UTC): entsteht wieder ein Plan? (trennt "bei jedem Wecken" von "nach Entladung").
- Bench-Versuch: Pack mit realistischer Mission ("Fahren", GO, hohe Last) auf < 30 % USOC entladen, danach wecken und `912B` lesen.
- Natuerliches Aufwachen heute Nacht mit Mitschnitt (Fahrzeugseite) und, wenn moeglich, Teraterm am naechsten Zuendungs-Aus (Einschlafen mit Plan).

## Nachtrag 02.11. spaet (siehe Befund 37c)
- **Gemessen:** Der Plan wird beim Wecken neu berechnet (7 -> 8 -> 0 Zellen), der Lauf beginnt 37 s nach dem Einschlafen des Packs, zaehlt in Echtzeit und ist nach insgesamt 1653 Schritten (725 + 928) beendet; kurze Weckungen (19:07, 19:12) erzeugten keinen neuen Plan.
- **Neue Prioritaet fuer die Bench:** (1) Pack-Uhr/Ruhezeit (Bench `9261` = 0, `91C1` = 0): kann der Pack ohne gueltige Zeit keinen Plan anlegen? (2) Wecken nach langer Ruhe. (3) SOC am Fahrtende <= 34 %.
- **Naechster Versuch am Auto:** 45 bis 60 min Ruhe, dann ein Wecken mit einzelnen Befehlen, `912B` lesen.


## Stand 02.11. nachts (komprimiert)
- **Gemessen:** Plan 2 endete nach **928 Schritten = 54,4 min** (nicht 63,3); abgefuehrte Ladung je Zeit sinkt (Lauf 1: 0,266 Ah in 0,708 h = 0,375 A, ca. 54 mA je Zelle bei 7 Zellen; Lauf 2: 0,25 Ah in 0,906 h = 0,276 A, im Mittel nur ca. 5 von 8 Zellen aktiv).
- **Schluss:** Der Plan endet **zellweise** (Zellen fallen nacheinander heraus), nicht nach fester Zeit; die 1080 Schritte vom 24.10./28.10. waren die Dauer der damals laengsten Zelle. Ein Wecken allein erzeugt keinen neuen Plan (19:07, 19:12 = 0).
- **Hypothese fuer die Bench (nicht bewiesen):** Plan entsteht beim Wecken nach **>= ca. 8 h Ruhe** (Ruhespannung aussagekraeftig; passt zum natuerlichen Aufwachen nach 482 min) **und** ungleichen Zellen (niedriger SOC am Fahrtende). Belegt nur durch 2 Faelle (15:54 nach 8 h, 24.10. 14:24 nach ca. 1 Tag), beide zugleich mit SOC-Ende <= 34 %.
- **Gestrichen:** Ruhe-Experiment "45 bis 60 min, dann Wecken" (kein neuer Plan ohne Entladung erwartet).
- **Tests:** (Auto) nach der naechsten Fahrt auf niedrigen SOC **nach ca. 60 min** wecken, `912B` lesen (Plan da = Ruhe nicht noetig). (Bench) Failsafe-Fenster >= 600 min (Emulator: 1 bis 1440 min), Pack vorher unter ca. 30 % SOC entladen, echter Schlaf >= 8 h, dann wecken, `912B`/`9252` lesen; Faktor und Schlafdauer im Blick behalten (Pack-Ruhe in Pack-Minuten).

## Korrektur 02.11. (Nutzer): Stand Bench-Zeit
- Der Hinweis "Bench `9261`/`91C1` = 0, keine gueltige Zeit" gilt **nicht mehr**. Seit 25.10. (Befund 21 bis 23) zaehlt der Bench-Pack Pack-Minuten nach der vom Emulator gesendeten Zeit (Faktor 3: 480 Pack-min = 1 Tag), schreibt Missionen und beendet sie beim Daily-Sleep. Ein Plan entsteht trotz Laden auf 100 %, 20 bis 25 mV Spreizung und taeglichem Daily-Sleep nicht.
- **Offener Unterschied (passt zur Hypothese 8 h Ruhe):** Bench-Schlaf nur 5 bis 6 min, Auto 8 h. Bench-Test: Faktor 1 und echter Schlaf >= 8 h (oder Faktor 3 und 24 h Schlaf = 480 Pack-min), vorher entladen, dann wecken und `912B`/`9252` lesen.

## Bench 02.11. abends (ca. 23:08 bis 23:30 Uhr Handyzeit): Sleep kurz, +478 min auf 0x436, nach ca. 6 min geweckt, `912B` gesetzt
Kennzeichnung: **gemessen** = aus den Bildern (`BILD_Bench_02-11_*`), **Bericht** = Aussage des Nutzers ohne Zeitstempel, **Schluss** = abgeleitet, **offen**.

- **Bericht (Nutzer):** Einschlafweg "kurz", nur zwischen Zustand "awake" und Schlaf (Wecken endet in "awake"); waehrend der Stille wurde die Zeit in `0x436` um **+478 min** gesetzt; nach ca. **6 min** geweckt; danach waren die Bits in `912B` gesetzt.
- **Gemessen (Bild `BILD_Bench_02-11_912B_84_Bits.jpg`, "read 13 s ago", ohne Uhrzeit):** `912B` Datenbytes 21-32 = `FF FF FF FF FF FF FF FF FF FF F0 00` = **84 Bits**, erstes Bit = Zelle 96, also **Zellen 96 bis 13** gesetzt, Zellen 1 bis 12 nicht. Das Zellbild zeigt dieselbe Aufteilung (erste 12 Balken Idle, die uebrigen Balancing). Zellen im Broadcast ueberwiegend 3632 mV, Zelle 91 = 3638 mV.
- **Gemessen:** `9251`/`9252` = 0,000 Ah / 0,000 h (raw `80000000`), `9262`/`9263` = 0 (Bilder 23:18). Der Plan lief zu diesem Zeitpunkt noch nicht (im Auto beginnt der Lauf erst nach dem Einschlafen des Packs plus ca. 37 s).
- **Gemessen (Zeit):** 23:08 SCPU `9261` = 71343 min, MCPU = 71341 min. 23:18 MCPU = **71345**, SCPU = **71823**; Feld "Time in 0x436" = 71823, ACTIVE. **71823 - 71345 = 478.** Die SCPU folgt `0x436` sofort, die MCPU zeigt 71345.
- **Gemessen (Einstellungen, Bilder 23:18):** Einschlafweg **kurz** (Fahrlog Sitzung 3), Zustand **awake**, manuell angehakt, Betriebsart Fahren, Minutenfaktor 3, Failsafe "auto wake after" **20 min**, Daily Sleep **angehakt** (00:30, 5 min, 2 km), "Shutdown and wake-up" = like the car, Zellwerte-Quelle Broadcast, "Manual vehicle age" 1380368 ACTIVE, `0x426` 6924 km.
- **Schluss (nur wenn der Bericht stimmt):** Der Plan entstand nach einem Wecken mit **+478 min Bus-Zeit bei nur ca. 6 min echter Ruhe**. Dann kaeme es auf die Zeit in `0x436` an, nicht auf echte Erholung der Zellen. Das widerspraeche der Auto-Hypothese ">= 8 h echte Ruhe" nicht, haelt sie aber fuer den Bench nicht fuer noetig.
- **Offen / nicht belegt:**
  1. `912B` **vor** dem Sleep: **Bericht (Nutzer, 02.11. spaet): "alles null vorher"** (kein Bild, kein Zeitstempel). Damit entstanden die 84 Bits nach dem Ablauf (Sleep kurz, +478 min, Wecken nach ca. 6 min). Spaetere Lesung vor dem zweiten Schlaf: Bits unveraendert (Bild 23:45, "read 543 s ago", ca. 23:36).
  2. **Gegenprobe fehlt:** derselbe Ablauf (kurz, awake, ca. 6 min) **ohne** Zeitsprung. Erst dann ist der Sprung als Ursache belegt.
  3. Die MCPU zeigte um 23:18 `9261` = 71345, die SCPU 71823 (MCPU nicht gleich). **Gemessen am Auto (Airtable):** `9261` MCPU = SCPU nur bei den vier natuerlichen Aufwachern (31.10. 23:52, 01.11. 07:53, 02.11. 05:26, 02.11. 15:54); beim Wecken durch die Fahrt (16:38) Differenz 44, bei unseren Weckungen (19:07, 19:12) Differenz 2 und (20:16) Differenz 60 (= Schlafzeit). **Schluss:** Bei eigenen Weckungen zieht die MCPU-Zeit nicht nach; der Bench-Fall (71345 gegen 71823) ist damit kein Sonderfall. **Bericht (Nutzer):** Nach dem Aufwachen stand die MCPU noch auf der alten Zeit, die SCPU auf der neuen (also Lesung nach dem Wecken). **Schluss:** Die ausgelesene MCPU-`9261` ist ein Schnappschuss (Latch), kein laufender Zaehler; die Bits in `912B` entstanden, obwohl die MCPU-`9261` die +478 nicht zeigte. [**Korrigiert, siehe Abschnitt "Messung nach dem zweiten Schlaf":** die MCPU-`9261` zieht beim naechsten Einschlafen nach und enthaelt dann die +478.] Die Gegenprobe (gleicher Ablauf ohne Sprung) bleibt wichtig. Offen: ob die MCPU beim natuerlichen Aufwachen die Zeit neu schreibt und bei unserem Wecken nicht, und ob sie den Sprung beim naechsten Einschlafen uebernimmt.
  4. Reicht ein kleinerer Sprung als 478 min?
  5. Daily Sleep (00:30) und Failsafe (20 min) koennen weitere Laeufe ausloesen; bei einem Test darueber abhaken.
- **Naechste Lesungen (nur Lesen):** `912B`, `9251`/`9252`, `9262`/`9263`, `924F`/`9250` nach einer Weile; nach dem naechsten Einschlafen und Wecken `9261`/`91C1` (MCPU und SCPU).

## Zweiter Schlaf 02.11. 23:39 bis ca. 00:04 (Failsafe 25 min), Beobachtung 23:45 (Bild `BILD_Bench_02-11_2345_Stille_5min49.jpg`)
- **Gemessen (Bild):** "SILENT for 05:49 (safety limit 25:00)", letzter BMS-Frame 0,9 s **vor** Beginn der Stille, danach Nullen. `912B` = 84 Bits (Lesung ca. 23:36, vor der Stille).
- **Bericht (Nutzer):** `9251`/`9252`/Zaehler waren vorher null. Ziel dieses Schlafs: nach ca. 25 min `912B` und `9251`/`9252` lesen (Plan laeuft in der Stille?).
- **Offen:** Ergebnis nach dem Wecken; Gegenprobe ohne Zeitsprung.

## Messung nach dem zweiten Schlaf 02.11. (Stille ab ca. 23:39, Failsafe 25 min; Antworten vom Nutzer als Text, ohne Uhrzeit des Weckens)
- **Gemessen:** `912B` = Datenbytes 1-20 null, dann `FF FF FF FF FF FF FF FF FF FF F0 00` = weiter **84 Bits** (keine Zelle herausgefallen). `9251` = `80000000` (0 Ah, zweimal gelesen), `924F` = `80000000`, `9250` = `80000010`, `9252` = `80000010` (beide **16 Schritte = 56 s**), `9262`/`9263` = `80000000`.
- **Gemessen (Vergleich Auto, `BEFUND_Auto-Pack_Dump_Balancing_24-10.md` Zeile 111):** Zeit gebucht, Ah noch 0 wie bei "Stehen 2" (2,55 min Stille: +1,93 min / +0 Ah). Aber im Auto lief der Zaehler fast ueber die ganze Stille (Stehen 3: 11,84 min bei ca. 12,4 min Stille); hier nur 56 s bei ca. 24 min Stille (wenn die Lesung nach dem Wecken um ca. 00:04 lag; Uhrzeit nicht genannt).
- **Gemessen (Zeit):** `9261` MCPU = **71830** (`01 18 96`), SCPU = **71833** (`01 18 99`).
- **Rechnung:** MCPU jetzt 71830 minus MCPU 23:18 (71345) = 485 = 478 + 7. Die SCPU stand 23:18 auf 71823; 71830 = 71823 + 7 Pack-min = 21 echte min bei Faktor 3, also 23:18 + 21 min = **23:39** = Beginn der Stille. **Schluss:** Die MCPU-`9261` ist der Schnappschuss beim Einschlafen und enthaelt die +478. Der Pack hat die Zeit also uebernommen (der Wert 71345 um 23:18 war noch der Schnappschuss vom Schlaf davor).
- **Hinweis zur Auswertung:** SCPU minus MCPU = 3 Pack-min ist **nicht** die Schlafdauer. Die getippte Zeit in `0x436` steht in der Stille still (`BATTERY.cpp`, Zeilen 3210 bis 3217), die SCPU zeigt also den Stand 71830 plus die Minuten seit dem Wecken (3 Pack-min = ca. 9 echte min bei Faktor 3). Meine Aussage, die Differenz zeige die Schlafdauer, war falsch.
- **Offen:** Warum nur 56 s Balancing in ca. 24 min Stille; Uhrzeit des Weckens; `91C1`; Gegenprobe ohne Zeitsprung.

## Hypothese 02.11. spaet (Nutzer): Bus-Zeit steht am Bench in der Stille still, im Auto laeuft sie durch
- **Gemessen/aus dem Code:** In der Stille (`NVROLstateMachine == 5`) sendet der Emulator nichts und der Zaehler der getippten Zeit in `0x436` steht (`BATTERY.cpp`, Zeilen 3210 bis 3217 und 3302 bis 3318). Fuer den Pack vergeht in der Stille keine Bus-Zeit; er sieht beim Wecken denselben Wert wie beim Einschlafen. Im Auto laeuft die BCM-Uhr durch (Differenz zwischen natuerlichen Aufwachern = echte Minuten, z. B. 761 min am 01.11.).
- **Gemessen:** Bench 02.11. zweiter Schlaf: ca. 24 min Stille, gebucht nur **56 s** (`9252` = 16 Schritte), Ah 0, Bits unveraendert 84. Auto: gebuchte Zeit entsprach fast der Stillezeit (Stehen 3: 11,84 min bei ca. 12,4 min Stille).
- **Schluss (nicht bewiesen):** Der Pack leitet die in `9252` gebuchte Zeit aus der Bus-Zeit-Differenz zwischen Einschlafen und Wecken ab. Die gebuchten 56 s entsprechen etwa **einer Pack-Minute**, also dem einen Schritt, den die Bus-Zeit nach dem Wecken weiterzaehlt.
- **Einwand (Nutzer):** Das kann ein reiner Schoenheitsfehler sein, weil das Balancing scheinbar trotzdem laeuft. Zusatz des Nutzers, Wortlaut unklar ("aber evtl nimmer beendet wird"): moeglicherweise wird der Lauf dadurch **nie bzw. nicht richtig beendet**. **Offen**, ob es nur kosmetisch ist.
- **Test ohne Umbau (vorgeschlagen, nicht ausgefuehrt):** (1) "Manual vehicle age" auf den aktuellen Pack-Stand setzen (z. B. `editTwingoAgeManual?value=71840`), damit der Pack nicht um Jahre springt; (2) "Time in 0x436" auf Off (`editTwingoTime436?value=off`), dann nimmt `0x436` das Fahrzeugalter, das nach der Uhr durch die Stille weiterlaeuft (`BATTERY.cpp`, Zeilen 2890 bis 2894 und 1475 bis 1500); (3) Minutenfaktor 1; (4) Sleep, 25 min Stille, wecken, `9252` lesen. Erwartung nach der Hypothese: um 400 Schritte (25 min / 3,516 s); bleibt es bei 16, ist die Hypothese widerlegt.

## Rezept, mit dem der Bench-Pack am 02.11. erstmals einen Plan angelegt hat (festhalten, bis die Ursache geklaert ist)
- **Einordnung (Nutzer, 02.11. spaet):** Der Plan war am Bench vorher nie gesetzt (alles null), jetzt stehen 84 Bits. Die **+478 min** koennen eine Voraussetzung sein. Bis eine Gegenprobe vorliegt, gilt der Ablauf als Rezept und wird unveraendert wiederholt.
- **Ablauf (Bericht Nutzer + Bilder):** Einschlafweg "kurz", Zustand "awake" (manuell), Sleep, in der Stille "Time in 0x436" auf letzter Wert **+478**, nach ca. 6 min wecken; danach `912B` = 84 Bits (Zellen 96 bis 13), `9251`/`9252` = 0.
- **Einstellungen laut Bildern 23:18:** Minutenfaktor 3, Failsafe 20 min (spaeter 25 min), Daily Sleep 00:30 / 5 min / 2 km angehakt, Zellwerte-Quelle Broadcast, "Shutdown and wake-up" = like the car, Betriebsart Fahren, `0x426` 6924 km.
- **Reihenfolge der naechsten Versuche:** (1) den laufenden Plan zu Ende laufen lassen und `912B` bis auf null verfolgen; (2) wenn er weg ist, das Rezept **mit** +478 wiederholen (Reproduzierbarkeit); (3) erst dann die Gegenprobe **ohne** Sprung. Ein Sprung ohne vorherige Reproduktion beweist nichts.
- **Hinweis:** Der laufende Plan verbraucht sich beim Balancing (Bits fallen zellweise heraus); danach fehlt der Ausgangszustand fuer weitere Auswertung der Bits.

## Messung nach dem dritten Schlaf 03.11. 0:47 (Handyzeit): Bus-Zeit +25 min in der Stille, Balancing 25,6 min gebucht (Bilder `BILD_Bench_03-11_0047_*`)
- **Ablauf (Bericht Nutzer):** Sleep, Stille 25 min (Failsafe 25:00), in der Stille "Time in 0x436" auf **71861** gesetzt (= letzter gesendeter Wert 71836 + 25), dann geweckt.
- **Gemessen (Bilder):** `9251` = `80000300` = 768 Schritte = **0,750 Ah**; `9252` = `800001B5` = 437 Schritte = **0,427 h = 25,6 min** (437 x 3,516 s); `9262`/`9263` = 0; `912B` weiter **84 Bits** (Lesung "8 s ago"); `9259` = `04`; `9261` MCPU = **71836** (`01 18 9C`), SCPU = **71861** (`01 18 B5`); Pack 348,7 V, 12 V = 13,1 V, SOC intern 37,5 %, USOC 30,6 %, Zellen max 3,639 V (Index 12) / 3,626 V.
- **Vergleich der beiden Laeufe (Schluss, 2 Messungen):** Lauf A (zweiter Schlaf, ca. 24 min echte Stille, **Bus-Zeit-Differenz 0**): gebucht **56 s** (16 Schritte), 0 Ah. Lauf B (ca. 25 min echte Stille, **Bus-Zeit +25**): gebucht **25,6 min** (437 Schritte), 0,75 Ah. Echte Stille war in beiden Laeufen etwa gleich lang; gebucht wurde nur dort, wo die Bus-Zeit vergangen ist. Das stuetzt die Hypothese, dass der Pack die gebuchte Zeit aus der **Bus-Zeit-Differenz** ableitet (nicht aus einer eigenen Uhr). Vorhersage vorher: um 400 Schritte; gemessen 437.
- **Gemessen (Rate):** 0,750 Ah in 0,427 h = ca. 1,76 A bei 84 Bits = ca. 21 mA je Zelle. Auto 24.10.: 0,444 A bei 20 Bits = ca. 22 mA je Zelle; Auto 02.11.: ca. 54 mA je Zelle.
- **Gemessen:** Die MCPU-`9261` = 71836 ist der Schnappschuss vom Einschlafen (= Ausgangswert), die +25 zieht sie erst beim naechsten Einschlafen nach.
- **Offen:** Der Plan ist nach 25,6 min **nicht** abgearbeitet (84 Bits stehen). Gesamtdauer fuer 84 Zellen unbekannt (24.10.: 1080 Schritte bei 20 Bits). Ob nur die Bus-Zeit zaehlt oder auch die echte Stille noetig ist: Lauf B und A sind nicht getrennt, wo beide gleich lang waren; Trennung z. B. durch kurze echte Stille mit +25 oder lange Stille mit +10.
- **Folge fuer die Bauliste:** Die Zeit in `0x436` muss in der Stille weiterlaufen (Bau vorgemerkt in `BAULISTE.md`); damit ist es **kein** reiner Schoenheitsfehler, sondern entscheidet, wie viel Balancing der Pack bucht.

## Zellmonitor 03.11. 0:55 (Handyzeit), nach 25,6 min gebuchtem Balancing (Bilder `BILD_Bench_03-11_0055_*`)
- **Gemessen (Bilder):** Max 3632 mV, Min 3625 mV, "Voltage Deviation: 7 mV (Battery is balancing now!)". **Zellen 1 bis 12 = 3625 mV** (das sind genau die Zellen ohne Bit in `912B`, im Balkenbild blau), **Zellen 13 bis 96 = 3627 bis 3632 mV** (cyan). Die Anzeige ist in Stufen von ca. 2,4 mV aufgeloest (3625 / 3627 / 3628 / 3630 / 3632).
- **Gemessen (Vergleich):** Im Zellbild von 23:2x standen Zelle 85, 87, 89, 91, 93 auf 3635 bis 3638 mV; jetzt liegen alle bei 3632 mV oder darunter. Hoechste Zelle damals: Zelle 91 mit 3638 mV.
- **Schluss (nicht bewiesen):** Der Plan baut alle Zellen oberhalb der tiefsten Gruppe (Zellen 1 bis 12, 3625 mV) ab; Zielhoehe ca. 3625 mV. Die obersten Zellen sind bereits auf das Niveau der Mehrheit (3632 mV) gesunken.
- **Grobe Abschaetzung der Restzeit (Schluss, aus einer Zelle und gleichem Strom je Zelle):** Zelle 91 sank um ca. 6 mV bei etwa 9 mAh je Zelle (0,75 Ah / 84; ca. 21 mA x 0,427 h) = ca. 0,7 mV je mAh. Die Zellen bei 3632 haben noch ca. 7 mV bis 3625 = etwa 10 mAh = rund **30 min** weiteres Balancing bei 21 mA; mit Reserve 35 bis 60 min. Die Aufloesung von 2,4 mV und die Annahme gleichen Stroms machen das ungenau.
- **Vorschlag:** naechsten Schlaf mit **+65 min** Bus-Zeit (deckt die beobachtete Lauf-Obergrenze von 1080 Schritten = 63,3 min, siehe Auto 24.10./28.10.), danach `912B`, `9252`, Zellmonitor lesen.

## Messung 03.11. 3:36 (Handyzeit): nach dem Test "+70 min" (Bilder `BILD_Bench_03-11_0336_*`)
- **Ablauf (Bericht Nutzer, Plan vom 03.11. frueh):** Sleep, in der Stille Zeit um ca. 70 min vorgestellt, geweckt, danach Betriebsart Fahren (normal) wieder an. Der genaue getippte Wert wurde nicht genannt.
- **Gemessen (Bilder):** `9252` = `800005ED` = 1517 Schritte = **1,481 h = 88,9 min**; `9251` = `80000AE0` = 2784 Schritte = **2,719 Ah**; `924F`/`9250` identisch zu `9251`/`9252`; `9262`/`9263` = 0 (alles im Schlaf gebucht). `912B` weiter **84 Bits** (Lesung "0 s ago"). `9259` = **05** (Fahren). `9261` = `0118BC` = 71868, `91C1` = `011974` = 72052. Pack 348,1 V, SOC intern 35,53 % (0:47: 37,53 %), USOC 28,46 % (30,59 %), SOC min/max 36,5 / 40,0 % (38,4 / 42,2 %), Batterie-Strom (`900D`, nur Anzeige) 0,600 A (vorher 0,375 A), 12 V = 13,1 V.
- **Gerechnet (Zuwachs gegen 0:47):** `9252` 1517 - 437 = **+1080 Schritte = 63,3 min**; `9251` 2784 - 768 = +2016 Schritte = +1,969 Ah, ca. 1,87 A bei 84 Bits = ca. 22 mA je Zelle (0:47: 21 mA).
- **Schluss (dritter Fall):** Angeboten waren ca. 70 min Bus-Zeit (= 1195 Schritte), gebucht wurden **exakt 1080 Schritte**. Das stuetzt die Obergrenze von **1080 Schritten (63,3 min) pro Lauf**: zwei Auto-Laeufe (24.10., 28.10.) und jetzt der Bench. Der Wert 1080 ist also eine Grenze je Schlaf-Lauf, nicht die Plan-Dauer. Gilt nur, wenn der Nutzer wie geplant +70 gesetzt hat.
- **Gemessen/Korrektur:** Der Plan ist nach insgesamt 88,9 min (25,6 + 63,3) **nicht** fertig (84 Bits). Meine Restzeit-Schaetzung vom 03.11. 0:55 (ca. 30 min, aus einer Zelle) war zu niedrig und ist hinfaellig.
- **Offen:** wie viele Laeufe der 84-Bit-Plan insgesamt braucht; Zellmonitor jetzt (Spreizung); `925C` nicht gelesen; Ursache des SOC-Rueckgangs um 2 Prozentpunkte in ca. 2,8 h (Strom 0,6 A; Balancing, Eigenverbrauch oder Zustand "Fahren"?).

## Hypothese des Nutzers (03.11., ca. 4 Uhr): Balancing-Lauf und Bus-Zeit gehoeren zusammen
- **Aussage (Nutzer):** Wenn ein Balancing-Lauf stattfindet, sind dabei auch Minuten vergangen, also muss `0x436` im Lauf positiv weitergelaufen sein, auch wenn nur simuliert. Erst dann stimmen die Berechnungen und die angezeigten Werte (`9250` bis `9252`, Plan in `912B`). Dass trotzdem balanciert wurde, glaubt der Nutzer schon (physisch).
- **Dazu passt (gemessen/Schluss):** Im Auto ist der Pack-Bus in der Stille ruhig; die Zeit kann der Pack nur beim Wecken aus dem Zeitwert erfahren (BCM-Uhr laeuft durch). Am Bench wurde bei etwa gleicher echter Stille nur dort gebucht, wo die Bus-Zeit gesprungen ist (56 s gegen 25,6 min; 1080 Schritte bei +70 min).
- **Schluss (nicht bewiesen):** Buchung und Abbau des Plans (Zaehler der Zellen, Bits in `912B`) folgen der Bus-Zeit; das tatsaechliche Balancing der Chips (Watchdog bis 64 min, `MAX17823B`) koennte unabhaengig davon laufen. Dann waere ohne Zeitsprung physisch etwas balanciert, aber nichts gebucht, und der Plan wuerde nicht kleiner.
- **Pruefbar:** Folgt die Zahl der Bits in `912B` nur den Laeufen mit Zeitsprung? Bei Laeufen ohne Zeitsprung duerfte sich am Plan nichts aendern.
- **Folge fuer die Bauliste:** bleibt wie vorgemerkt (Zeit muss in der Stille weiterlaufen bzw. beim Wecken stimmen); Frage `0x436` in der Stille steht in `BAULISTE.md`.

## Korrektur und Ergebnis (Nutzer, 03.11., ca. 4 Uhr): echte Stille beim +70-Lauf war 70 min; physisch zaehlt die echte Schlafzeit
- **Nutzer:** Der Zeitsprung ist nicht das Massgebende, sondern die Zeit, die wirklich geschlafen wurde (passives Balancing ueber Widerstand, fester Strom je Zelle). Die eingetragene Zeit bestimmt nur den angezeigten Wert (Balancing-Zeit und daraus der Strom).
- **Bericht (Nutzer):** Echte Stille bei dem +70-Lauf = **70 min**.
- **Tabelle der Laeufe (Zeit in `9252` angezeigt):** A: echte Stille ca. 24 min, Bus-Zeit-Differenz 0 -> 56 s. B: ca. 25 min, +25 -> 25,6 min. C: **70 min, +70 -> 63,3 min (1080 Schritte)**.
- **Schluss (nicht bewiesen):** Bei C waren echte Stille und Bus-Zeit gleich (70 min), angezeigt wurden trotzdem nur 63,3 min. Das passt zu einer **Obergrenze von rund 64 min je Schlaf-Lauf**, passend zum Balancing-Watchdog des `MAX17823B` (1 s bis 3840 s = 64 min, Schalter schalten selbst ab). Dann haetten die letzten ca. 6 min der Stille physisch nichts mehr balanciert, und Anzeige und Physik stimmen bei C ueberein. Im Auto erklaert das auch, warum ein langer Schlaf nur ca. 63 min bucht (24.10./28.10.) und der Plan ueber mehrere Wecken-Schlaf-Zyklen laeuft.
- **Folgerung fuer die Tests:** Jeder Zyklus braucht echte Stille (kein Abkuerzen ueber den Zeitsprung); ueber ca. 64 min hinaus bringt Stille nichts. Beste Einstellung: echte Stille und eingetragene Zeit gleich, ca. 64 min je Zyklus. Bei 100 mA je Zelle ca. 13 h = rund 12 Zyklen, bei 22 mA ca. 60 bis 77 h.
- **Offen:** Ob der Pack nach Ablauf des Watchdogs ohne Wecken weiter balanciert (MCU schlaeft, vermutlich nicht); ob das Wecken je Zyklus den Plan neu rechnet (Firmware-Analyse: ja).
