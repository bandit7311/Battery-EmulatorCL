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
