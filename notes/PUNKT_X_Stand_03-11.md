# PUNKT X: Stand 03.11.2026 ca. 02:30 CET (hierhin kehren wir spaeter zurueck)

Kennzeichnung: **gemessen** = aus Bild/Dump/Airtable, **Bericht** = Aussage des Nutzers, **Schluss** = abgeleitet, **offen**. Einzelheiten in `ZIEL_Plan-Ausloeser_fuer_Bench.md` (Abschnitte ab "Bench 02.11. abends") und `BEFUND_Auto-Pack_Dump_Balancing_24-10.md` (37 bis 37d).

## Ziel
Herausfinden, was den Balancing-Plan (`912B`, Zaehler `9251`/`9252`) aktiviert, damit der Bench-Pack ihn anlegt.

## Wichtigste Ergebnisse
1. **Bench legte erstmals einen Plan an (02.11. abends):** Rezept = Sleep "kurz" aus Zustand "awake", in der Stille "Time in 0x436" = letzter Wert **+478 min**, nach ca. 6 min wecken. Vorher `912B` alles null (Bericht), danach **84 Bits (Zellen 96 bis 13)**.
2. **Balancing laeuft im Schlaf** (`9251`/`9252`), nicht wach (`9262`/`9263` = 0).
3. **Der Pack bucht Zeit nach der Bus-Zeit-Differenz** (Schluss, 2 Laeufe): Lauf A (ca. 24 min echte Stille, Bus-Zeit 0): **56 s** gebucht; Lauf B (ca. 25 min Stille, Bus-Zeit +25): **25,6 min** (437 Schritte), 0,750 Ah, ca. 21 mA je Zelle. Plan danach noch nicht fertig (84 Bits).
4. **Ursache am Bench:** Die getippte Zeit in `0x436` steht in der Stille still (`BATTERY.cpp` 3210 bis 3217, 3302 bis 3318); im Auto laeuft die BCM-Uhr durch. **Bau vorgemerkt** (`BAULISTE.md`, "Zeit laeuft im Balancing-Sleep weiter"), wartet auf "bauen".
5. **MCPU-`9261`** ist ein Schnappschuss beim Einschlafen und zieht die Bus-Zeit dann nach (71836 = Ausgangswert, +25 folgt beim naechsten Einschlafen). Gleich der SCPU nur bei natuerlichen Aufwachern im Auto.
6. **Zellmonitor 0:55:** Zellen 1 bis 12 = 3625 mV (ohne Bit), Zellen 13 bis 96 = 3627 bis 3632 mV; Plan baut Zellen oberhalb der tiefsten Gruppe ab. Grobe Restzeit ca. 30 min (Schluss, ungenau).
7. **Auto:** Plan-Hypothesen (SOC am Fahrtende <= 34 %, Ruhe >= 8 h) weiter nur als Hypothesen; natuerliche Aufwacher nach 482 min ab Zündung aus (2 Faelle). Letzter natuerlicher Aufwacher 02.11. 15:54:24 UTC; naechster nach der 482-Regel ca. 02:02 UTC 03.11. (durch unsere Weckungen eventuell verschoben).

## Geplant (so war der Stand)
- **Test +70 min:** Sleep, in der Stille "Time in 0x436" = letzter Wert + 70 (`http://192.168.178.222/editTwingoTime436?value=...`), wecken, dann `912B`, `9251`, `9252`, Zellmonitor, `9261` (MCPU/SCPU) lesen und Betriebsart Fahren laufen lassen; Messwerte morgen. Daily Sleep 00:30 (5 min) bucht wegen der stehenden Zeit nur ca. 1 min.
- **Danach (Reihenfolge):** Plan zu Ende laufen lassen -> Rezept mit +478 wiederholen (Reproduzierbarkeit) -> Gegenprobe ohne Sprung. Ob nur Bus-Zeit oder auch echte Ruhe noetig ist, trennt z. B. kurze echte Stille mit +65 / lange Stille mit +10.
- **Fahrzeugalter statt getippter Zeit** (laeuft nach der Uhr durch die Stille): Manual vehicle age auf Pack-Stand, "Time in 0x436" Off, Faktor 1.

## Offen
- Ob ein dreistuendiger Lauf voll balanciert (Obergrenze 1080 Schritte pro Lauf?, zwei Auto-Laeufe endeten exakt bei 1080).
- Was am Auto den Plan wirklich ausloest; `925C` beim natuerlichen Aufwachen (Skript liest es nicht); `$3302`/`$338A`/`$338B`/`$33C8` im EVC beim natuerlichen Wecken.
- Gegenprobe ohne Zeitsprung; `91C1` nach dem Test.
- Das Web-Neuladen im Sleep-Kasten ueberschreibt Eingabefelder jede Sekunde (`BATTERY.cpp` 2138 bis 2160, 3487 bis 3755); Umgehung per Direktlink, Umbau nur auf "bauen".

## OVMS-Skript (Stand)
Gesamtes Skript wurde geliefert (Token nur als Platzhalter): Abfragen als Einzelereignisse, `evc3302` zuerst, SCPU zuletzt, kein eigenes Wecken, `ticker.1`, erste Lesung 3 s nach dem Aufwachen. Airtable-Spalte `evc3302` muss existieren. Am 02.11. 20:16 UTC lief es ohne Haenger (Zeile `auto`, `evc3302` = `00`). Vorschlag offen: `925C` und `338A`/`338B`/`33B7`/`33C8` ins Skript aufnehmen.

## Git-Stand (notes-Branch `claude/notes-test-9259`)
Alle Notizen bis `93bb59c` sind gepusht (03.11.). Hinweis: GitHub meldet "repository moved" -> `https://github.com/bandit7311/Battery-EmulatorCL.git`.

## Nachtrag 03.11. (nach dem Ausflug in die Zoe Gen2 / ljames28)
- **Frage fuer die naechste Bauliste:** Bleibt der Akku still, wenn `0x436` in der Stille weiterlaeuft, und laeuft die Pack-Uhr dann weiter? (`BAULISTE.md`, letzter Abschnitt.)
- **Neue Spur (Schluss):** `9259` folgt der externen BMS-Mode-Nummerierung (`05` Driving, `04` Wait/Sleep transient, `02` Non-iso Charge); `925C` = "Relay status, internal value" (`01` latched, `02` unlatched fuer "sleep transient and balance"). Balancing wahrscheinlich nur in `04`/`02`. Zu pruefen: `925C` nach dem Wecken am Bench und beim natuerlichen Aufwachen im Auto lesen (Skript liest `925C` noch nicht).
- **Zoe Gen2:** Zeit laeuft im NVROL-Ablauf durch (kein `return` in `transmit_can`), Sleep = nur `0x373` B0 = `01`; Zoe Gen1 hat keinen Sleep/NVROL. Details in `QUELLE_Zoe_Gen2_und_ljames28_ZE50_03-11.md`.
- **ZE50-README:** "will not balance" ohne korrekte Zeit; Zeit nur vorwaerts senden.

## Nachtrag 03.11. abends: Stand Punkt X (Bericht Nutzer und Messungen)
Kennzeichnung: **gemessen** = Bild/Airtable, **Bericht** = Aussage des Nutzers, **Schluss** = abgeleitet.
- **Rezept fuer einen neuen Plan am Bench (Bericht Nutzer, 03.11.):** Nur bis **awake** aufwecken, dort **kurz verbleiben** und ueber den **Fahrlog-Einschlafweg "kurz" (Fahrlog Sitzung 3)** wieder einschlafen lassen (Bericht Nutzer 03.11.: "Fahrlogweg kurz"); das schreibt mittlerweile **zuverlaessig einen Plan**. (Frueher Weg "kurz" aus awake mit +478 min; Einzelheiten in `ZIEL_Plan-Ausloeser_fuer_Bench.md`.)
- **Balancing-Lauf am Bench (gemessen, 4 Faelle):** bucht je Schlaf-Lauf hoechstens **1080 Schritte (63,3 min)**; Ah-Zaehler = Bits x 22,2 mA x Zeit (berechnet); Plan wird bei jedem Wecken neu berechnet; Bits sanken 84 -> 33 -> 84 -> 83 -> 26 (3:36 bis 15:55).
- **Auto (gemessen):** natuerlicher Aufwacher 03.11. 16:09 UTC nach genau **480 min** Pack-Schlaf (Fahrtende USOC 26 %), Plan mit ca. 66 Bits, danach Lauf 56 min, beim Wecken von Hand um 17:06 UTC 48 Bits. Bench und Auto verhalten sich gleich (Befund 37g).
- **Offen:** SOC-Schwelle fuer das natuerliche Aufwachen (zwischen 15 und 26 %); Herkunft der 1080-Grenze (Chip-Watchdog 64 min oder Firmware); Zahl der Laeufe bis der Plan leer ist; Bau "Zeit in der Stille" (`BAULISTE.md`).
- **Nicht noetig fuer den Bench:** Weckart (`evc3302`, `0x350` `WakeUpType`, BCM `212E`); der Plan entsteht ohne diese Information.
- **Entscheidung Nutzer 03.11. (abends):** `0x436` laeuft in der Stille **nicht** weiter; die Zeit wird von Hand gebucht. Das Feld "Daily sleep" wird zu "Daily balance" mit Zykluszahl (Beispiel 70 min und 3 Zyklen = drei Balancing-Zyklen ueber die Nacht mit Aufwachen dazwischen). Eintrag in `BAULISTE.md` ("Daily balance"), der fruehere Eintrag "Zeit in `0x436` soll in der Stille weiterlaufen" ist widerrufen.
