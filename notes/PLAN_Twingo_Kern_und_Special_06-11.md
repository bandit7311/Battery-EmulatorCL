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
