# Plan (nur gemerkt, noch nichts entschieden oder gebaut) - 06.11.2026

Ziel: Twingo-Kern fuer dalathegreat/battery-emulator einreichbar, alles Analyse/Zusatz nur in unserem Repo.

## Zielstruktur (Vorschlag)
1. RENAULT-TWINGO-GEN1-BATTERY.h/.cpp - Klasse und Treiberlogik
2. RENAULT-TWINGO-GEN1-LOGIC.h (322 Zeilen) - bleibt oder wandert in BATTERY.h
3. Header fuer Weck-/Schlaf-Ablauf (Rest von BUS.h, nur was der Kern braucht)
4. RENAULT-TWINGO-GEN1-HTML.h - Renderer fuer die Statusseite (Muster: RENAULT-ZOE-GEN2-HTML.h, get_status_renderer())
5. RENAULT-TWINGO-GEN1-SPECIAL.h/.cpp - TXSTAT, SLEEPBTN, Simulator-Teil von BUS.h, Cellwatch, freie Anfrage, Stille-Diagnose;
   Kern bindet ihn nur ein, wenn vorhanden (__has_include oder Compile-Flag)

## Arbeit
1. BUS.h durchgehen: Ablauf (Kern) vs. Simulator-Tabellen (Special) mit Zeilenbereichen markieren (nur lesen)
2. Simulator-Teile nach SPECIAL verschieben, Namespaces zunaechst unveraendert
3. Nach jedem Schritt 522 Tests + Golden-Lauf, Hashes muessen identisch bleiben
4. Erst danach auf upstream/main aufsetzen (neuer Zweig; kangoo-parallel bleibt unberuehrt, Produktivsystem bleibt auf 7ab39c0)

## Offene Fragen
- Hat dala einen Mechanismus fuer batterie-eigene Webserver-Routen/Einstellungen? (nicht gefunden; sonst eigener Hook-Vorschlag)
- Welche Funktionen zaehlen zu den Grundfunktionen? (Liste vom Nutzer steht aus)
- CANLOGFLT (comm_can.*, comm_nvm.cpp, settings_html.cpp) ist ein eigenes Feature, nicht Teil des Twingo-Beitrags
- Quelle der Befunde: Upstream-Stand 2621aaf (05.11.2026)

## Ergaenzung 07.11.2026: Eingriffe in dalas Dateien (Stand upstream 2621aaf)
Basisdateien von dala sollen unveraendert bleiben. Erlaubte Eingriffe (Minimum):
1. Battery.h: Enum-Eintrag RenaultTwingo1 + Name
2. BATTERIES.cpp: Registrierung
3. datalayer_extended.h: Datenblock (nur Kern-Felder), Muster wie bydAtto3 (Zeilen 85, 916-917)
4. webserver.cpp: EINE Zeile register_twingo_routes(server); (Muster: register_dump_can_route, webserver.cpp:1133)
   Routen selbst in RENAULT-TWINGO-GEN1-ROUTES.cpp. Dala hat Atto3-Routen (8) und /TeslaBalAct direkt in webserver.cpp -> Inline waere dort ebenfalls akzeptiert.
5. test/CMakeLists.txt: Testdateien
Seite: Renderer in RENAULT-TWINGO-GEN1-HTML.h (Muster Zoe Ph2, get_status_renderer()).
Weg B (Hook Battery::register_routes) spaeter als Vorschlag an dala: init_webserver() laeuft bei Software.cpp:104, setup_battery() erst bei :734 -> Hook braeuchte spaete Registrierung.
Nicht Teil des Beitrags: CANLOGFLT (comm_can.*, comm_nvm.cpp, settings_html.cpp).

## Sortierung der 39 Twingo-Routen in webserver.cpp (VORSCHLAG, Nutzerliste der Grundfunktionen steht aus)
Kern (vorgeschlagen): editTwingoNvrolWriteValue, editTwingoB009Session, editTwingoSleepMinutes, editTwingoSleepButton,
  editTwingoDailyBalance, twingoDailyBalanceAbort, editTwingoDtcAllStatus, twingoBalBits
Special (Analyse/Zusatz): editTwingoVehicleId, editTwingoAgeFactor, editTwingoAgeManual, editTwingoTime436Follow, editTwingoTime436,
  editTwingoOdo, editTwingoCarMode, twingoSimAllOff, twingoSimRestore, editTwingoSimSignal, simulator, cellwatch, cellwatchStatus,
  editTwingoCellwatchEnable, editTwingoCellwatchCell, twingoBusState, twingoRxStatus, twingoBusManual, twingoBusFormat, twingoSleepPath,
  twingoBusTarget, twingoCellSource, twingoQuery, twingoQueryTarget, twingoQueryResult, twingoSetTimeNow, editTwingoSilenceDiag,
  twingoTxStat, twingoRx155
Unklar (Entscheidung noetig): twingoSleepPath, twingoBusTarget, twingoBusManual (Bus-Ablauf: Kern oder Analyse?), editTwingoSleepButton (Hardware IO15)
