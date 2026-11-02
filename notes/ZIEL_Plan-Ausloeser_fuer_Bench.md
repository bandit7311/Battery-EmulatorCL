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
- **Tests:** (Auto) nach der naechsten Fahrt auf niedrigen SOC **nach ca. 60 min** wecken, `912B` lesen (Plan da = Ruhe nicht noetig). (Bench) Failsafe-Fenster >= 600 min (Emulator: 1 bis 1440 min), Pack vorher unter ca. 30 % SOC entladen, echter Schlaf >= 8 h, dann wecken, `912B`/`9252` lesen; Bench-`9261`/`91C1` = 0 im Blick behalten.
