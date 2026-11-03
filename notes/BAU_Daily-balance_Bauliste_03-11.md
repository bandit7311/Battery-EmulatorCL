# Bauliste "Daily balance" (Stand 03.11. abends) - NICHTS GEBAUT, wartet auf das Wort "bauen"

Kennzeichnung: **entschieden** = Aussage des Nutzers, **gelesen** = aus dem Code (Branch `kangoo-parallel`, `90af77b`), **Vorschlag** = noch nicht entschieden.

## 1. Zweck
"Daily balance" ersetzt "Daily sleep". Zur eingestellten Uhrzeit schlaeft der Pack N-mal D Minuten (Fahrlog "kurz"), damit der Bench-Pack seinen Balancing-Plan anlegt und abarbeitet. Vorher laeuft ein Testzyklus ("Zyklus 0") mit Plan-Pruefung.

## 2. Felder (Seite, Kasten "Daily balance")
```
Daily balance
[x] enable  Time [00:30]  Duration [65] min  Cycles [3]
Test [10] min  Check [120] s   [Set]
(recommended 65 min)
Status: Zyklus 2 von 3
```
Vorher (heute, `BATTERY.cpp` 3562 bis 3575): `enable`, `Time`, `Duration`, `Km`, `Set`.
- **bleibt:** enable, Time, Duration (jetzt Balancezeit D), Set.
- **entfaellt:** Km (entschieden, keine Kilometer mehr).
- **neu:** Cycles N (1 bis 5, entschieden), Test (Zyklus 0, einstellbar, Standard 10 min, entschieden), Check (Wartezeit "kein Plan", Standard 120 s, einstellbar, entschieden), Hinweis "recommended 65 min" (entschieden), Statuszeile "Zyklus x von N" (entschieden).
- **Vorschlag:** Test = 0 schaltet den Testzyklus aus; Statuszeile zeigt auch "Abbruch: kein Plan", "wartet auf Uhrzeit", "nicht gestartet: nicht in Drive / Zeit in 0x436 nicht aktiv".

## 3. Ablauf
1. Start zur Uhrzeit, nur wenn: enable, Uhr gueltig, Zustand Drive (`05`/`01`), getippte Zeit in `0x436` aktiv, kein anderer Sleep-Lauf aktiv. Sonst kein Start und nichts inkrementieren.
2. Zustand merken (Betriebsart, Zustand manuell, Einschlafweg, Zeitmodus).
3. Freigabe zum Wechselrichter auf null (Halte-Flag), **5 s** warten.
4. Zustand awake, Schlaf ueber Fahrlog "kurz".
5. **30 s nach dem letzten BMS-Frame:** Zeit in `0x436` = aktuell gesendeter Wert + Zeit des Zyklus (Zyklus 0: Test-min, sonst D); `0x436` laeuft in der Stille nicht weiter.
6. Stille, Wecken, alle Zustaende bis `04`/`02` awake.
7. **Plan-Pruefung** (nach jedem Wecken): sofort "Plan da", sobald eine `912B`-Antwort **nach dem Wecken** mindestens 1 Bit hat; "kein Plan" erst nach Check-Zeit (Standard 120 s) und mindestens 3 Antworten. Zeit bis zum ersten Bit mitschreiben (Anzeige/Log).
8. Kein Plan: Abbruch, alten Zustand herstellen, Freigabe nach Bedingung, Anzeige "Abbruch: kein Plan", naechster Versuch zur Uhrzeit der naechsten Nacht; erhoehte Zeit bleibt.
9. Plan da: bei weiteren Zyklen nach **10 s** wieder einschlafen (Fahrlog kurz, Zeit += D); Zyklen 1 bis N.
10. Letzter Zyklus: alten Zustand wiederherstellen; Freigabe, wenn alter Zustand erreicht, Pack meldet ihn und die vier Prioritaets-Abfragen beantwortet sind, **spaetestens 10 s nach dem Fahrlog**.
11. Fehlerfall: normalen Fahrlog ablaufen lassen, in Betrieb gehen.

## 4. Bauschritte
- **B1 Felder:** `daily_sleep_*` in `datalayer_extended.twingoGen1` (Datei beim Bau nachlesen): `km` raus, `cycles`, `test_min`, `check_s` neu.
- **B2 Speicher laden** (`webserver.cpp` 229 bis 240): `TWDSKM` raus, `TWDSCYC`, `TWDSTST`, `TWDSCHK` neu; `TWDSTIME`, `TWDSDUR`, `TWDSEN` bleiben.
- **B3 Route** (`webserver.cpp` 913 bis 938): `/editTwingoDailyBalance?time=&dur=&cyc=&tst=&chk=&en=` mit Pruefung (cyc 1 bis 5, dur 1 bis 1440).
- **B4 Seite und Skript** (`BATTERY.cpp` 3562 bis 3575 und 3662 bis 3669): Felder wie in Abschnitt 2, Statuszeile.
- **B5 Zustandsmaschine** `daily_balance_service()` ersetzt `daily_sleep_service()` (`BATTERY.cpp` 745 bis 770, Aufruf Zeile 2399) und die Reste in 790 bis 792 und 838 bis 843.
- **B6 Halte-Flag** im `update_values()` (`BATTERY.cpp` 130 bis 134): nullt `current_dA`, `max_discharge_power_W`, `max_charge_power_W`; die beiden Maximalstroeme folgen ueber `safety.cpp` 496 bis 501 (Reihenfolge `Software.cpp` 648, 660, 666).
- **B7 Zeitbuchung** ueber `time_436_set()` (`BATTERY.cpp` 1400 bis 1407), einmal je Einschlafen.
- **B8 Plan-Pruefung** (`bal_bits_valid`/`bal_bits_ms`, `bal_bits_raw[12]`, `BATTERY.cpp` 525 bis 528; 912B alle 20 s, Abfrage `BATTERY.cpp` 3412).
- **B9 Wiederherstellung und Freigabe** (Punkt 3.10).
- **B10 Tests** (`LOGIC.h`, reine Logik): Faelligkeit, Zykluszaehler, Wartezeiten 5 s / 10 s / 30 s, Zeitbuchung genau einmal, Plan-Pruefung (sofort / nach Check-Zeit), Abbruch, Freigabebedingung; Golden-Lauf Wege lang, kurz, Ladelog 2 byte-identisch.

## 5. Hinweise
- Der Sleep-Kasten (`nvrolBox`) laedt waehrend eines Laufs jede Sekunde neu und ueberschreibt die Eingabefelder; Einstellen waehrend eines Laufs per Direktlink.
- Nicht gemessen: welchen Frame der Deye auswertet; Dauer bis der Plan nach dem Wecken sichtbar wird (Testzyklus misst sie); `reported_current_dA` nicht bis zur Zuweisung verfolgt.
- Offen vor dem Bau: Branch (`kangoo-parallel` Stand `90af77b` oder anderer); Vorschlaege aus Abschnitt 2 bestaetigen.
