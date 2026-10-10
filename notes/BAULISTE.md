# Bauliste (bereinigte Fassung, Stand 10.10.2026)

**Stand 10.10.:** Prioritaet 1 und 2 sind gebaut (Branch `twingo-echter-bus-modus`, Commits `14f3aaf` und `0d0d8d2`, Bericht: `BAUBERICHT_Prio1_Prio2_10-10.md`). Alles unten ab "Neu aufgenommen" und Prioritaet 3 ist **nicht gebaut**; gebaut wird erst nach ausdruecklichem "bauen". Die vorherige, chronologisch gewachsene Fassung steht unveraendert in `BAULISTE_ALT_10-10_vor_Bereinigung.md`. Kennzeichnung: **gemessen** / **Schluss** / **Annahme** wie in den Befund-Dateien.

## Prioritaet 1: Umbau "echter Bus"
Neuer Branch ab `e3988cb` (z. B. `twingo-echter-bus-modus`, ohne Modellnamen); `claude/twingo-hv-modell-und-alter` bleibt unveraendert als Rueckfall.

1. **Block 1 "ALLE SIMULATIONSWERTE wie im FAHRZEUG"**: alle **50 Adressen (11 Bit)** des BMS<>EVC-Busses (Mitschnitt 22aaf176, laut Nutzer ungefiltert), nach Takt gruppiert (10 ms / 20 ms / 100 ms / 200 ms / 500 ms / 1 s / 3 s).
   - 10 ms (9): `0x155`, `0x0C5`, `0x1C9`, `0x157`, `0x19F`, `0x1A1`, `0x1C7`, `0x0EC`, `0x0ED`
   - 100 ms (16): `0x419`, `0x423`, `0x424`, `0x425`, `0x426`, `0x428`, `0x42F`, `0x435`, `0x436`, `0x43A`, `0x445`, `0x464`, `0x4F7`, `0x500`, `0x511`, `0x588`
   - 1 s (2): `0x69F`, `0x6BE`
   - 3 s (23): `0x4AE`, `0x4AF`, `0x659` und die 20 Zellframes `0x5A1`, `0x5AC`, `0x5AD`, `0x5B4`, `0x5B5`, `0x5B7`, `0x5C9`, `0x5CB`, `0x5CC`, `0x5D6`, `0x5D9`, `0x5EA`, `0x5EC`, `0x5ED`, `0x5F0`, `0x5F1`, `0x5F2`, `0x5F4`, `0x5F7`, `0x5DD`
   - 20 ms, 200 ms, 500 ms: keine Frames auf diesem Bus.
   - **Sendbar: 30 Zeilen** (alle ausser den 20 Zellframes), auch die vermeintlichen Akku-Frames, weil wir nichts sicher wissen. **Zellframes: nur LED, keine Sendefunktion.**
   - Echtes Format und echter Takt, Inhalt je Zustand aus dem Mitschnitt; 10-ms-Frames erst nach dem Wake-Burst.
   - Zaehler/Pruefsummen: `0x0EC` (B1 Zaehler +1, B2 = CRC-8 Polynom 0x1D, XOR 0xBE), `0x19F` (B3 Zaehler +5), `0x157` (B1 Zaehler +5), `0x511` (B1-B6 sechs Zaehler mit festen Schritten); `0x423` B7, `0x500`, `0x1A1`, `0x419` unbekannt (Befund Abschnitt 17). Bei Unbekanntem aufgezeichnete Werte senden.
2. **Globaler Schalter** "Format der BMS<>EVC-Frames: Fahrzeug (Standard) / Zoe alt" fuer `0x423`, `0x426`, `0x436`, `0x19F`, `0x69F` gemeinsam.
3. **Zustandsauswahl** (zu, wach, Zuendung 1, Zuendung 3, GO, Fahrt, aus) fuer die zustandsabhaengigen Frames.
4. **Voreinstellungen** (nur Packwerte dieses Akkus; beide Felder einstellbar):
   - km (`0x426` Bytes 4-5): **6.844** (Pack-Laufleistung `91CF` vor dem Reset).
   - Alter (`0x436` Bytes 1-3, Feld "Manual vehicle age"): **1.025.301 min** (Pack-Zeit 912.981 am 24.07. + 78 Tage x 1.440 min), danach +1 pro Minute ab dem Setzen, **ohne** Sicherheitstag. Der Uhrwert (2,97 Mio.) wird nie gesendet.
   - Risiko: Der Akku hielt am 06.10. 1.311.344 min; der Vorgabewert liegt rund 286.000 min darunter, ein kleinerer Wert wird vielleicht nicht uebernommen.
5. **Block 2 "simulator alt"**: die uebrigen **41 Zeilen** im echten Format (einschaltbar), nach Takt gruppiert. Die fuenf BMS<>EVC-Zeilen (`0x423`, `0x426`, `0x436`, `0x19F`, `0x69F`) sind dort **ausgeblendet**, nie doppelt.
6. **Zeilenindizes bleiben stabil**: bestehende Zeilen behalten ihren Index, neue Zeilen kommen hinten dran (Index 46 bis 70), die Zeilenmaske wird auf **128 Bit** erweitert (bestehende NVM-Schluessel `TWINGOSIMMASK`/`TWINGOSIMHI` bleiben, zwei neue fuer Bits 64-127, Standard 0). Die Standardmaske 0x387 gilt nur fuer ein frisches Geraet.
7. **Neue Spalten und Anzeigen** (Skizze `Skizze_Simulator_Seite_v6.html`, Scratchpad):
   - **LED** je Zeile (gruen = von aussen empfangen, grau = nicht; Schwelle 3 x Takt, mindestens 2 s, 3-s-Frames 10 s; eigene Sendungen zaehlen nicht).
   - **"Erstmals gesehen"**: relative Zeit `T+12,43 s` aus `millis()` (gleiche Zeitbasis wie das Log), **T+0 nur bei "Alle Zeilen aus"** (loescht alle Werte), NTP-Uhrzeit nur als Zusatz; optional "zuletzt gesehen" und Anzahl.
   - **Spalte "BMS<>EVC-Bus"** (ja / nein / neu) und **"Bench-Messung"** (was der Akku allein sendet, Log 25.09.). Fuer alle 41 Zeilen von Block 2 steht "nein" (Mitschnitt ungefiltert laut Nutzer; aus der Datei nicht pruefbar).
   - **Abschnitt "Weitere empfangene IDs"** (alles, was ankommt und in keiner Zeile steht).
   - **Zeitleiste oben**: "Jetzt: T+…", Uhr, "Letzte Aktion" (Zeit und Art); Zeilen mit "Erstmals gesehen" juenger als **15 s** hellgelb.
8. **Abschluss**: Tests, clang-format, Lieferung zum Flashen. **Nicht Teil:** Aenderung der Sleep-/Wake-Folge (sendet weiter eigene `0x350`/`0x214`).

## Prioritaet 2: getrennt, danach
- **Schalter "Zellwerte-Quelle: UDS (heute) / Broadcast"** (getrennt vom Formatschalter). Dekodierung: je Frame 5 Zellen, 12 Bit, mV = Roh + 2000; Zellnummer nach Renault-Datenbank (`0x5F7` = Zellen 1-5 ... `0x5A1` = 91-95, `0x5DD` = 96; `0x5EC` ersetzt `0x5D7`, Annahme); Platzhalter beim Wecken ausfiltern. Die Wechselrichter-Felder (`cell_voltages_mV[]`, Min/Max, Pack-Spannung) werden dabei automatisch aus dem Broadcast gefuellt; alle 3 s statt rund 20 s.
- Weitere Werte aus dem Broadcast statt per UDS (gemessen gleich): SOC (`0x155`), Leistungsgrenzen und SOH (`0x424`), Zellminimum/-maximum (`0x425`), Temperatur-Min/Max (`0x424`). Weiter nur UDS: Zeit, Kilometer, Zaehler, Balancing, BMS-Zustand, 12-V-Versorgung, interner SOC.

## Prioritaet 3: aeltere Wuensche und Kleineres
- Testzeile `0x62B` (HEVC_WakeUpFrame der Renault-Datenbank, 2 Byte, Inhalt unbekannt) nur zum Ausprobieren.
- Alters-Faktor 1/10; Ziele DF/DA bei der freien Anfrage; Inverter-Dump; Messung im Auto; `0x436` dauerhaft manuell einstellbar.
- Abschalt-Taster (Nice-to-have): Sleep inkl. `0x9281` ohne Wiederanlauf, SSD1306-Hinweise "Shutdown requested / Sleep requested / Battery sleeping - Turn off now!!".
- Fix-later: Wake-up-CAN-Bus-Fehler (`0x090`/`0x242` erst nach dem Wake-Burst). To-do: kurz 100 % SOC an den Deye nach dem Aufwachen, `CAN NATIVE BUS ERROR` nach dem Wiederanlauf automatisch quittieren.
- Lokaler Stash (`stash@{0}`, "All rows off"-Statusanzeige, nicht gebaut/getestet): einspielen oder verwerfen, offen.

## Neu aufgenommen (10.10.): Ablauf auf dem BMS<>EVC-Bus wie im Fahrzeug
**Erst pruefen, was voneinander abhaengt, dann bauen.** Ausloeser: Die Knoepfe "Sleep", "Sleep 0x9281=1" und "NVROL reset" fahren nur die alte Folge auf dem Fahrzeug-CAN (`0x350` C3/C2/C0/00, Wake-Buendel); die Bus-Folge des Autos fehlt.
- **Wecken (gemessen, in allen 3 Sitzungen des Logs 22aaf176 gleich):** `0x0EC` zuerst (+0,05 s), `0x423` (+0,11 s), Weck-Buendel (+0,17 s); `0x0ED` ab +0,18 s mit `A3 FF 00` (10x), `A3 FF 80` (21x), dann Zustandswert. **Schon gebaut:** Weck-Form von `0x0EC` und `0x0ED` beim Einschalten der Zeile.
- **Noch offen:**
  - `0x0C5`: Zaehler im oberen Nibble von Byte 0 (`15`, `25`, `35` ...), Byte 1 vermutlich Pruefsumme (nicht untersucht), Weck-Form `15 2D 00 00 00 00 00`; heute nur feste Bytes. `0x0C5` kommt laut Bench vom Akku.
  - `0x0EC` Byte 0 beim Fahren wechselnd (`25`, `27`, `26`, `2A`, `22`, `21`, `2B`, `29`, `28`); heute fest je Zustand.
  - Automatische Zustandsfolge (zu, wach, Zuendung 1, GO, Fahrt, aus, wach, zu) statt Handwahl.
  - Einschlafen auf dem Bus: `0x426` auf "zu" (`00 00 02 ...`), `0x0EC` endet mit "zu", `0x0ED` laeuft rund 1,2 s weiter und ist das letzte Frame (`60 FF 80`, in Sitzung 2 `60 FF 00`); Zeiten Sitzung 1: zu bei T+469,8 s, Ende T+471,0 s.
  - Verbindung der Knoepfe Sleep / Sleep 0x9281=1 / NVROL reset mit dieser Bus-Folge (heute getrennt, sie laufen nebeneinander).
- **Zu pruefen vorher:** was haengt wovon ab (Reihenfolge `0x0EC` -> `0x423` -> Rest; Wake-Buendel; Gate der 10-ms-Frames nach dem Wake-Buendel; Zeilen mit Ende "bis Busende"; Zustand und Format-Schalter). **Bench-Ergebnis 10.10. (vom Nutzer berichtet, kein Log):** `0x0EC` und `0x0ED` allein (Format Fahrzeug, Weck-Form) wecken den Akku **nicht**; erst mit `0x423` kommen alle 13 Akku-Frames (`0x155`, `0x0C5`, `0x1C9`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, `0x4AE`, `0x4AF`, `0x659`, `0x6BE`) **und alle 20 Zellframes**. `0x423` ist also der Weck-Ausloeser. Offen: wofuer `0x0EC`/`0x0ED`/die uebrigen EVC-Frames sind (z. B. MCPU-Zeit und Kilometer).

## Offen beim Nutzer
- **Bench-Test** in Schritten (Plan: `TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`): erst nur `0x423`, dann die uebrigen BMS<>EVC-Frames nach und nach; `0x350` und die anderen Fahrzeug-CAN-Zeilen erst als Zusatz. Logger ohne Filter (RX und TX).
- Pin-Belegung (CAN H/L) am LBC-Stecker, damit die Aderfarben aus dem Schaltplan zugeordnet werden koennen.
