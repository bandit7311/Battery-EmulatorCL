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
  1. `912B` **vor** dem Sleep wurde nicht dokumentiert (die Notizen sagen bisher: Bench legt keinen Plan an). Ohne diesen Wert ist nicht belegt, dass die Bits erst durch den Ablauf entstanden sind.
  2. **Gegenprobe fehlt:** derselbe Ablauf (kurz, awake, ca. 6 min) **ohne** Zeitsprung. Erst dann ist der Sprung als Ursache belegt.
  3. Die MCPU zeigte um 23:18 `9261` = 71345, die SCPU 71823 (MCPU nicht gleich). **Gemessen am Auto (Airtable):** `9261` MCPU = SCPU nur bei den vier natuerlichen Aufwachern (31.10. 23:52, 01.11. 07:53, 02.11. 05:26, 02.11. 15:54); beim Wecken durch die Fahrt (16:38) Differenz 44, bei unseren Weckungen (19:07, 19:12) Differenz 2 und (20:16) Differenz 60 (= Schlafzeit). **Schluss:** Bei eigenen Weckungen zieht die MCPU-Zeit nicht nach; der Bench-Fall (71345 gegen 71823) ist damit kein Sonderfall. **Bericht (Nutzer):** Nach dem Aufwachen stand die MCPU noch auf der alten Zeit, die SCPU auf der neuen (also Lesung nach dem Wecken). **Schluss:** Die ausgelesene MCPU-`9261` ist ein Schnappschuss (Latch), kein laufender Zaehler; die Bits in `912B` entstanden, obwohl die MCPU-`9261` die +478 nicht zeigte. Das schwaecht die Lesart "der Pack uebernimmt den Zeitsprung" und macht die Gegenprobe (gleicher Ablauf ohne Sprung, `912B` vorher lesen) wichtiger. Offen: ob die MCPU beim natuerlichen Aufwachen die Zeit neu schreibt und bei unserem Wecken nicht, und ob sie den Sprung beim naechsten Einschlafen uebernimmt.
  4. Reicht ein kleinerer Sprung als 478 min?
  5. Daily Sleep (00:30) und Failsafe (20 min) koennen weitere Laeufe ausloesen; bei einem Test darueber abhaken.
- **Naechste Lesungen (nur Lesen):** `912B`, `9251`/`9252`, `9262`/`9263`, `924F`/`9250` nach einer Weile; nach dem naechsten Einschlafen und Wecken `9261`/`91C1` (MCPU und SCPU).
