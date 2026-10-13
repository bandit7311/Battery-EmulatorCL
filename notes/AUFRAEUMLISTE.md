# Aufraeumliste (Stand 13.10.2026) - schlanke Twingo-Fassung fuer das Original von Dala

Nichts davon ist gebaut oder entschieden. Der Nutzer macht spaeter daran weiter (13.10.: "alles in Aufraeumliste, machen spaeter daran weiter"). Vor jedem Loeschen: Streichliste mit "bleibt / fliegt raus / unklar" gemeinsam durchgehen, keine Annahme, welche Frames erfolglos waren.

## Ziel
Der Twingo-Treiber soll im Repo von Dala (dalathegreat/Battery-Emulator) laufen, ohne die Seiten Advanced und Simulator; alle Einstellungen fest; frueher genutzte Frames und Steuerungen ohne Erfolg entfallen (Nutzer, 13.10.).

## Fakten (aus dem Code, 13.10.)
- Der Treiber besteht aus fuenf Dateien, zusammen 7.882 Zeilen: `RENAULT-TWINGO-GEN1-BATTERY.cpp` (5.324), `-BATTERY.h` (1.070), `-BUS.h` (1.106), `-LOGIC.h` (245), `-TXSTAT.h` (137). Dazu 11 Testdateien (rund 4.500 Zeilen).
- Zum Vergleich (Schnappschuss in diesem Repo): Zoe Gen2 1.163 Zeilen (`.cpp` 564, `.h` 529, `-HTML.h` 70), Zoe Gen1 342, Kangoo 310; groesste uebrige `.cpp`: Tesla 2.999, ECMP 1.908, MEB 1.751, Nissan Leaf 1.411. Unsere `.cpp` allein ist fast doppelt so gross wie Tesla.
- Dieses Repo hat keinen Verlauf von Dalas Original (erster Commit am 3.10. ist ein Schnappschuss aus 498 Dateien); ein Abgleich mit Dalas aktuellem `main` ist nicht gemacht. Ob Dala den Twingo-Treiber schon in anderer Form hat, ist nicht geprueft.
- Aenderungen ausserhalb des Twingo-Treibers seit dem 5.10. (8 Dateien): `Battery.h` (+6, fuenf virtuelle Funktionen fuer das zweite Steuergeraet: SCPU-Knoepfe), `datalayer.h` (+7, drei Display-Felder), `datalayer_extended.h` (+59, Twingo-Felder), `display.cpp` (+20) und neue `sleep_display.h` (68), `advanced_battery_html.cpp` (+12, SCPU-Knoepfe und Titel-Suffix), `webserver.cpp` (+395, Twingo-Routen), `simulator_html.cpp` (ca. 600 Zeilen, Seite `/simulator`). Dazu `BATTERIES.cpp` mit acht Verweisen auf den Twingo.

## Vier Stellen, die in Dalas Repo sonst nicht kompilieren
1. `datalayer_extended.h`: Der Treiber nutzt `twingoGen1` 74-mal -> Struktur in den Treiber verschieben.
2. `Battery.h`: fuenf `override`-Funktionen (SCPU-Knoepfe) -> weglassen oder die 6 Zeilen mit einreichen.
3. `datalayer.h`: Display-Felder fuer die Sleep-Anzeige (`sleep_display_*`) -> weglassen (dann keine OLED-Anzeige) oder mit einreichen (`display.cpp`, `sleep_display.h`).
4. `BATTERIES.cpp`: Eintrag des Twingo (pruefen, ob Dala ihn schon hat).
- Weiter: Der Treiber enthaelt HTML und JavaScript fuer Advanced-Felder (Sleep failsafe, Daily sleep, Faktor), die Webserver-Routen aufrufen -> muessten raus. Einstellungen, die der Webserver beim Start aus dem Flash laedt (`init_webserver`), muessten fest werden oder vom Treiber selbst gelesen werden. Die Tests brauchen dieselbe Anpassung.

## Kandidaten zum Entfernen (grob 1.500 bis 2.000 Zeilen der `.cpp`; Groessen aus Abstaenden der Funktionsanfaenge, nur Naeherung)
1. Aeltere Frame-Formate: altes Zoe-Format von `0x19F`, `0x426`, `0x436` (`bus_format_zoe_old`), ersetzt durch das Fahrzeugformat des Busses.
2. 0x214-Experiment: `send_214_like_car` (49 Zeilen) und der Experimentteil in `transmit_can`.
3. Bench-Diagnose: freie Anfrage und Vorab-Lesen (`start_user_query` 83, `handle_user_query_reply` 67, `handle_pre_read_reply` 52, `uq_reply_complete` 131), DTC-Erweiterung (`handle_dtc_ext` 130), Stille-Aufzeichnung und Seiten (`record_silence_frame` 113, `append_quiet_html` 246, `append_live_html` 127).
4. Simulator: Zeilentabelle mit Maske und Zeitleiste, `send_simulator_signals` (226); dazu Cellwatch und die Messung der Sendeabstaende (TXSTAT).
5. Weitere Bench-Funktionen der letzten Baus: Daily sleep, Minutenfaktor, Hardware-Taster (Bau 9, 11, 12), Einschlafwege lang/kurz/Ladelog 2 (nur einer bleibt), "like the car"-Schalter.
6. NVROL-Reset mit Routine B009 (antwortet negativ, NRC 0x7F; Teil von `transmit_reset_nvrol_frames`).

## Was bleiben muesste (Kern)
- Dekodieren der UDS-Werte (`handle_pid` 61, `handle_extended_single_frame` 276, `handle_extended_reply` 87, ...).
- Die Frames, die der Akku braucht: `send_bus_vehicle_frames` (die 17 EVC-Zeilen, BUS.h), Zeit und Alter in `0x350`, `0x436`, `0x523`, `0x376`.
- Die Abschaltfolge (Zustand 7: C3/C2/C0/00, Schliessphase mit `0x423` B0=04) und das Aufwecken (Burst, `0x423` 30/33).

## Vorsicht
- `transmit_reset_nvrol_frames` (246 Zeilen) enthaelt neben dem NVROL-Reset (nie erfolgreich) auch die Abschaltfolge (Zustand 7) und muss aufgeteilt werden, nicht gestrichen.
- Die Gesamtzeit der Sleep-Anzeige und der "safety limit"-Text haengen heute am Fenster "Sleep failsafe manual" (Luecke, in Bau 12 vorgemerkt).

## Naechster Schritt (wenn der Nutzer weitermacht)
Streichliste mit den drei Spalten "bleibt / fliegt raus / unklar" gemeinsam durchgehen; danach eigener Branch mit der schlanken Fassung; zuerst pruefen, ob und wie Dala den Twingo schon hat (dann waere Dalas Repo als Quelle hinzuzufuegen; Netzwerkaktion nur auf Ja des Nutzers).
