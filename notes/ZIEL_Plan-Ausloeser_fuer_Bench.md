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
