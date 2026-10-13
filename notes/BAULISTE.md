# Bauliste (bereinigte Fassung, Stand 10.10.2026)

**Stand 10.10.:** Prioritaet 1 und 2 sind gebaut (Branch `twingo-echter-bus-modus`, Commits `14f3aaf`, `0d0d8d2` Weck-Form 0x0EC/0x0ED, `85ba74a` Seite in vier Teilen, `cb190c5` 0x426 km 24 Bit; Bericht: `BAUBERICHT_Prio1_Prio2_10-10.md`; Bench-Ergebnisse: `TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`). Alles unten ab "Neu aufgenommen" und Prioritaet 3 ist **nicht gebaut**; gebaut wird erst nach ausdruecklichem "bauen". Die vorherige, chronologisch gewachsene Fassung steht unveraendert in `BAULISTE_ALT_10-10_vor_Bereinigung.md`. Kennzeichnung: **gemessen** / **Schluss** / **Annahme** wie in den Befund-Dateien.

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
   - km (`0x426` **Bytes 3-5, 24 Bit**, korrigiert 10.10., vorher irrtuemlich Bytes 4-5 mit festem Byte 3 = 01): **6.844** (Pack-Laufleistung `91CF` vor dem Reset).
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

## Naechste Schritte nach den Bench-Ergebnissen vom 10.10. (nicht gebaut)
- **Speichertest** (kein Bau): Zustand "aus" (erwartet `9259` 04, `925C` 02, `9279` +1), 3 bis 5 min warten, `91C1`/`91CF`/`9261`/`925F` lesen; danach "zu" mit Frame-Ende wie im Auto. Vorher DTC loeschen und pruefen, ob `1B0E41` (Speicherfehler, aktiv `2F`) zurueckkommt.
- **Fehlerfreier Zeilensatz** festhalten (E14281/E14381 sind verschwunden; Satz und Reihenfolge fehlen noch beim Nutzer).
- **Abschaltfolge auf dem Bus** (siehe Abschnitt "Neu aufgenommen"): wird wichtiger, weil `9259` = 05 erreicht ist und der Schnappschuss wohl erst nach Sitzungsende geschrieben wird.
- **Seite beobachten:** Absturz beim Oeffnen mit der ersten Fassung; Gegenmassnahme `85ba74a` (vier Teile), am Geraet noch nicht bestaetigt.

## Neu aufgenommen (10.10.): Ablauf auf dem BMS<>EVC-Bus wie im Fahrzeug
**Erst pruefen, was voneinander abhaengt, dann bauen.** Ausloeser: Die Knoepfe "Sleep", "Sleep 0x9281=1" und "NVROL reset" fahren nur die alte Folge auf dem Fahrzeug-CAN (`0x350` C3/C2/C0/00, Wake-Buendel); die Bus-Folge des Autos fehlt.
- **Wecken (gemessen, in allen 3 Sitzungen des Logs 22aaf176 gleich):** `0x0EC` zuerst (+0,05 s), `0x423` (+0,11 s), Weck-Buendel (+0,17 s); `0x0ED` ab +0,18 s mit `A3 FF 00` (10x), `A3 FF 80` (21x), dann Zustandswert. **Schon gebaut:** Weck-Form von `0x0EC` und `0x0ED` beim Einschalten der Zeile.
- **Noch offen:**
  - `0x0C5` **entfaellt**: kommt vom Akku selbst (Bench 25.09. und 10.10.), kein Zaehler und keine Weck-Form von uns noetig. Die Zeile bleibt nur als Sendemoeglichkeit mit Vorsicht (Kollision mit echtem Akku).
  - ~~`0x0EC` Byte 0 beim Fahren~~ **gebaut** (`4d5fd8d`): Anlauf der ersten Fahrt (`25` x90, `27` x13, `26` x4, `2A` x6, `22` x1, `21` x328, `2B` x3, `29` x212, `28` x26, dann `29`), Start beim Eintritt in den Zustand. Die zweite Fahrt im Log (T+1373 s) hat eine andere Folge (`27` x821 ...), ohne erkennbare Regel; Byte 0 haengt wohl an der Fahrdynamik.
  - Automatische Zustandsfolge (zu, wach, Zuendung 1, GO, Fahrt, aus, wach, zu) statt Handwahl.
  - ~~Einschlafen auf dem Bus~~ **gebaut** (`4d5fd8d`), in der 00-Stufe der Abschaltfolge: Zeilen in "closed", `0x0EC` endet, `0x0ED` mit `60 FF 00` bis zum Ende der Stufe (0,9 bis 1 s), dann Stille; nach dem Wecken Weck-Form. **Annahme:** Das Auto-Bus-Ende (Log 22aaf176) wird auf die 00-Stufe des Fahrzeug-CAN gelegt; beide Logs sind nicht zeitgleich. Ursprung der Anforderung: `0x426` auf "zu" (`00 00 02 ...`), `0x0EC` endet mit "zu", `0x0ED` laeuft rund 1,2 s weiter und ist das letzte Frame (`60 FF 80`, in Sitzung 2 `60 FF 00`); Zeiten Sitzung 1: zu bei T+469,8 s, Ende T+471,0 s.
  - ~~Verbindung der Knoepfe Sleep / Sleep 0x9281=1 / NVROL reset~~ **gebaut** (`4d5fd8d`): alle drei laufen durch dieselbe 00-Stufe. Nur eingeschaltete Zeilen schliessen mit. Die Zustandswahl am Bildschirm bleibt unveraendert (nach dem Wecken Weck-Form im gewaehlten Zustand).
- **Zu pruefen vorher:** was haengt wovon ab (Reihenfolge `0x0EC` -> `0x423` -> Rest; Wake-Buendel; Gate der 10-ms-Frames nach dem Wake-Buendel; Zeilen mit Ende "bis Busende"; Zustand und Format-Schalter). **Bench-Ergebnis 10.10. (vom Nutzer berichtet, kein Log):** `0x0EC` und `0x0ED` allein (Format Fahrzeug, Weck-Form) wecken den Akku **nicht**; erst mit `0x423` kommen alle 13 Akku-Frames (`0x155`, `0x0C5`, `0x1C9`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, `0x4AE`, `0x4AF`, `0x659`, `0x6BE`) **und alle 20 Zellframes**. `0x423` ist also der Weck-Ausloeser. Offen: wofuer `0x0EC`/`0x0ED`/die uebrigen EVC-Frames sind (z. B. MCPU-Zeit und Kilometer).

## Gebaut am 11.10. (Commit 6ed9c58 auf twingo-echter-bus-modus, 427 Tests gruen, Bench-Test offen)
1. **Checkbox "manuell" am Auswahlfeld "Zustand" mit automatischer Zustandsfolge:** Aus (Standard): Zustand laeuft von selbst nach dem Ablauf aus Sitzung 3 des Auto-Logs (zu `00/02` 1,3 s, `08/02` 0,7 s, `08/06` 0,4 s, wach, ab 13,7 s Zuendung 1, 15,2 s Zuendung 3, 16,9 s GO, 20,0 s Fahrt) und bleibt auf "driving"; Auswahlfeld nur anzeigend. An: Auswahlfeld bedienbar. 0x426 folgt immer dem Zustand (auch beim Einschlafen "zu"). Start beim Geraetestart und nach jedem Aufwecken. Enthaelt die Weck-Form von 0x426 (Anlauf). Beim Einschalten von "manuell" bleibt der Zustand stehen, beim Ausschalten beginnt die Folge von vorn. **Entscheidung 11.10.: Einschlafen wie im Auto.** Bei Sleep, Sleep 0x9281=1 und NVROL reset laeuft 0x426 ueber die Stufen durch die Zustaende des Auto-Logs (Sitzung 1, T+255 s): Fahrt `70/69` -> `70/65` (0,1 s) -> `60/65` (0,1 s) -> aus `00/05` (1,9 s) -> `00/07` (2,6 s) -> wach `00/06` (bis zur 00-Stufe, im Auto rund 125 s) -> zu `00/02` in der 00-Stufe bis zum Ende; stand das Auswahlfeld nicht auf Fahrt, beginnt die Folge bei `00/05`. Nach dem Aufwecken beginnt die Zustandsfolge wieder mit dem Anlauf bis driving.
2. **Antwortpuffer der freien Anfrage** von 336 auf mindestens 700 Zeichen (`uq_result`), damit Missionslisten (128 Byte) ganz erscheinen.
3. **Text auf "More Battery Info":** "71 individually toggleable cyclic signals" statt der festen 46. **Gebaut** (Commit `6ed9c58`, 11.10., im gepushten Branch enthalten; `RENAULT-TWINGO-GEN1-BATTERY.cpp:4066`, `SIM_SIGNAL_COUNT` = 71). Der frueher notierte Vermerk "3 Zeilen liegen lokal ungebaut" ist erledigt (geprueft 12.10., Arbeitsbaum sauber).
4. **Anzeige der km mit zwei Stellen** (Aufloesung 0,01 km) statt drei (`.000` irritiert); Pack-km (`91CF`, 1/32 km) mit einer oder zwei Stellen.
5. **Feld "byte 7" von 0x426 im Fahrzeugformat wirksam machen** (heute nur im alten Zoe-Format, im Fahrzeugformat fest `0x40`).

## Weitere Bauten am 11.10. (alle auf twingo-echter-bus-modus, Bench-Test offen)
- `23961a6`: 0x19F wie im Auto (Wake-Form 12 Frames `FF FF FF Fx 00 06 00 FE`, 10 Rampenframes, Zaehler ab `F`, Byte 5 = CRC-8 Poly 0x2F ueber B2:B3hi, Zuendung-1-Rampe).
- `95f4045`: keine Unterdrueckung der 10-ms-Frames im Wake-Buendel mehr; Zeit in 0x436, km/Byte 7 von 0x426, km von 0x5D7 und Checkbox "manuell" werden gespeichert; "All rows off" ueberschreibt die gespeicherten Zeilenschalter nicht mehr.
- `3752043`: Zaehler der gesendeten 0x69F-Frames auf der Simulator-Seite; `2E` darf 925E schreiben (Free request, Ziel DB).
- `c6fb094`: 0x426-Wechsel um 0,1 s korrigiert (zu 1,2 s), Zeittests gegen Auto-Log (Aufwachen Sitzung 3, Einschlafen Sitzung 1).

## Gebaut am 12.10. ("bauen 1 bis 5", alle auf twingo-echter-bus-modus, 475 Tests gruen, Bench-Test offen)
- Bau 1 und Bau 4, `29be34b`: Einschlafweg lang / kurz / Ladelog 2 (Radiobuttons), Busende von Ladelog 2 in der Reihenfolge des Autos (0x157 und 0x0ED zuletzt), Zustand "Laden" (`A0/41`, 30 Zeilen aus Ladelog 2) und Weckfolge ueber den Stecker, Radiobuttons Fahren / Laden / Schlafen.
- Bau 2, `9153317`: eigene Zeile "DTC Read SCPU" mit Knopf; das Ergebnis bleibt bis zum naechsten Lesen stehen und aktualisiert sich von selbst.
- Bau 3, `3542303`: OLED Zeile 4 und 5 beim Einschlafen: "Sleep requested" mit 00:00:00 / Ende, in der Stille "Sleep pls wait" mit dem Zaehler ab Beginn der Stille. Danach werden die Zeilen geleert, die Stoerungen stehen wieder dort.
- Bau 5, `4c7d262`: Messung der eigenen Sendeabstaende von 0x426, 0x435, 0x436 in Mikrosekunden (min, max, Mittel, Anzahl ausserhalb 100 ms +-1 ms, Pausen ueber 1 s) und der Versatz von 0x435 / 0x436 zum davor gesendeten 0x426, auf der Seite mit Reset-Knopf. Gemessen wird der Zeitpunkt des Aufrufs der Sendefunktion, nicht der Zeitpunkt auf der Leitung. Startversatz und Wake-Formen der anderen EVC-Frames sind nicht beauftragt (erst nach der Messung).
- Offen aus dem Bau: Dauer von `00/07` im kurzen und langen Weg; das Ziel "Schlafen" ist nicht definiert (verhaelt sich wie Fahren); die dynamischen Laden-Frames 0x4F7 / 0x42F / 0x419 werden als haeufigster Wert konstant gesendet.

## Zum Bauen vorgemerkt: Zustand "Laden" (Stand 11.10. abends, nur Messwerte; GEBAUT 12.10., siehe oben)
Quellen: Ladelog 1 (`598b197c-canmitlog.log`, PuTTY, Aufwachen bei 611,77 s, `A0/41` von 619,80 bis 1149,60 s = 529,8 s) und Ladelog 2 (`445a1527-teraterm.log`, Tera Term, vollstaendig ohne "not printed"; Stille von 203,26 bis 247,64 s; Aufwachen durch den Ladestecker bei 247,64 s; `A0/41` von 255,67 bis 553,27 s = 297,6 s; Ende mit der Fernbedienung). Dumps: MCPU `1791728231` / `1791731801`, SCPU `1791728260` / `1791732192`. Alles nur Gemessenes, keine Annahmen. Zum Befund siehe `BEFUND_Ladelog_Twingo_11-10.md`.

**Gemessen, Aufwachen durch den Stecker (Log 2, Versatz zu 247,64 s)**
- Erste Frames nach der Stille (Versatz, erster Inhalt): 0x423 +0,00 `30 7F FF FF FF E0 FF FF`; 0x445 +0,03 `80 FF FF FF FF FF 80`; 0x464 +0,03 `F0 1F C0`; 0x588 +0,03 `FF FC 10 36`; 0x1C7 +0,05 `AA 23 00 02 36 00`; 0x4F7 +0,05 `00 7F 00 00 09 FC 06 F0`; 0x428 +0,05 `00 00 00 00 00 07 C0 00`; 0x42F +0,05 `00 00 00 00 00 00`; 0x155 +0,06 `49 87 D0 54 3D 6A 02 C2`; 0x157 +0,06 `D5 A7 09 C4 FF E0 00 80`; 0x0ED +0,06 `A0 FF 80` (18 Frames, dann `60 FF 80`); 0x424 +0,06 `FF FF FF FF 7F 7F FF 7F`; 0x425 +0,06 `05 FF CC FF FF FF FD FF`; 0x6BE +0,06 `FF FF FF FF FF 00`; 0x43A +0,07 `3C 7F FF 5C C6 00`; 0x1C9 +0,07 `B6 14`; 0x1A1 +0,17 `FF FF FF FF FE FC 00 20`; 0x19F +0,17 `FF FF FF FF 00 06 00 FE`; 0x419 +0,18 `00 00 00 00 00 00`; 0x426 +0,13 `00 00 02 01 6A 71 00 40`; 0x435 +0,13 `FF FF 1E EB 35 FC 00 00`; 0x436 +0,13 `80 2D 5B 1F 00 00`; 0x69F +0,73 `46 13 88 6F`; **0x0EC erst +2,05** `10 01 17`, `10 11 DA`, `10 21 90`, `10 31 5D`, `10 41 04`, `10 51 C9`, danach `1C 61 F4`, `1C 71 39`, ...
- 0x426, 0x435 und 0x436 beginnen im selben Zeitstempel (247,77 s); 0x423 beginnt 0,13 s vorher.
- 0x426-Folge Log 2: `00/02` 247,77 -> `08/02` 247,97 -> `00/02` 249,07 -> `00/03` 249,67 -> `60/63` 249,77 -> `60/61` 250,27 (B7 `40`) -> `60/61` B7 `80` 250,37 -> `A0/41` 255,67 -> `A8/41` 259,57 -> `A0/41` 260,67 (bis 553,27).
- 0x426-Folge Log 1: `00/02` 611,90 -> `00/03` 613,80 -> `60/63` 613,90 -> `60/61` B7 `80` 614,50 -> `68/61` 615,00 -> `60/61` 616,10 -> `A0/41` 619,80 -> `A8/41` 623,70 -> `A0/41` 624,80 (bis 1149,60). In Log 1 kommt kein `08/02`.
- 0x426 Byte 3 bis 5 (km) in beiden Logs konstant `01 6A 71`; Byte 6 `00`.
- Byte 7 von 0x426 (alle drei Auto-Logs gezaehlt): `80` kommt nur vor in `60/61` (35- und 53-mal), `68/61`, `00/01` (kurz), `A0/41` (7.324 Frames) und `A8/41`; in allen anderen Zustaenden und in den Fahr-Zustaenden (auch `60/61` im Fahr-Log, 48 Frames) ist Byte 7 `40`.

**Gemessen, Zustand `A0/41` (Log 2, 255,67 bis 553,27 s; konstante Bytes, haeufigster Frame)**
- 0x426 `00 A0 41 01 6A 71 00 80`; 0x436 B0 `AD`, B1 `2D`, B2 `5B`, B3 zaehlt +1 pro Minute (`1F` bis `26` im Log), B4/B5 `00`; 0x0ED `60 FF 80`; 0x0EC B0 `1C`; 0x19F `00 00 7D xx 00 6A 90 FE`; 0x1C7 `AA 23 00 04 34 00` (zwei weitere Varianten, B3 `02`); 0x157 B0 `D5`, B2/B3 `09 C4`, B7 `80`; 0x1A1 B5 `0C`, B6 `80`, B7 `20`; 0x435 `FF FF 1E 6A 35 00 00 00` (4 Varianten); 0x428 `00 00 00 00 00 04 8F 00` (9 Varianten); 0x42F ohne konstantes Byte; 0x4F7 B0 `00`, B2 `0C`, B6 `06`, B7 `F0`; 0x423 B2 `FF`; 0x419 B3 `8F`, B5 `00`; 0x69F `46 13 88 6F`; kein 0x500.
- 0x436 B0: `80` bis zum Aufwachen; ab 250,47 s Rampe `83 86 8A 8C 8F 91 94 96 97 99 9B 9C 9D 9F A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC` (bis 254,77 s), `AD` ab 256,67 s; am Ende ab 553,37 s Rampe zurueck `A9 A6 A3 A0 9D 9B 99 97 95 93 92 90 8F 8E 8C 8B 8A 89 88 87 86 85 84 83 82 81` bis `80` bei 557,87 s.
- UDS-Anfragen der EVC (nur lesen): beim Eintritt (Log 2: 260,16 bis 263,86 s) `9210`, `9215`, `91D1`, `91D2`, `92C1`; (Log 1: `9215`, `91D1`, `91D2`, `92C1`, kein `9210`); beim Ende 0,48 s nach dem Ende von `A0/41` `9243`, `9245`, `91CF`, `92C1`, `91C1`, `901B` (beide Logs).
- Strom und Grenzen der BMS in `A0/41` (Formeln aus dem Treiber): 0x155 Strom +25,0 A Median (Log 1) und +24,5 A (Log 2); SOC Log 1 von 9,08 auf 14,94 %, Log 2 von 39,3 auf 43,0 %. 0x424 "Laden erlaubt" in allen Zustaenden 43.000 W, "Entladen erlaubt" 54.500 bis 72.000 W (Fahrt im Fahr-Log 55.000 bis 57.000 W), SOH 95, Flags `11`/`40` unveraendert.

**Gemessen, Ende (Log 2, Fernbedienung bei 553,27 s)**
- `A0/41` -> `00/01` B7 `80` 553,27 -> `00/01` B7 `40` 553,77 -> `00/07` 555,27 -> `00/06` 556,37 -> `08/06` 556,77 -> `00/06` 557,87 -> `00/02` 677,87 (121,5 s Wach-Zeit nach 556,37 s). Log 1: `00/01` 1149,60 -> `00/01` B7 `40` 1150,20 -> `00/03` 1151,50 -> `00/07` 1151,60 -> `08/07` 1152,00 -> `08/06` 1152,60 -> `00/06` 1153,10 -> `00/02` 1273,20.
- 0x0EC letztes Frame 677,83 s (Log 2), 0x0ED letztes Frame `60 FF 00` bei 688,91 s, also 11,0 s nach dem `00/02` (Log 1: 1273,20 s und 1284,26 s = 11,1 s).

**Gemessen, Dumps vor und nach Ladelog 1 (MCPU)**
- `9261` `2D 59 D8` -> `2D 5A D9` (+257 min), `91C1` `21 61 DA` -> `21 62 DB` (+257 min), Abstand `9261 - 91C1` vorher und nachher 784.382; der neue `9261` ist gleich dem letzten 0x436-Wert im Log (`2D 5A D9`, 1271,9 s).
- `9279` 16 -> 20; `9275` bis `9278` unveraendert; `9270` hat zwei neue Eintraege `04`; `9259` `05` -> `04`, `925C` `01` -> `02` (SCPU im dritten Dump gleich, im vierten wieder `05`/`01`); `91CF` unveraendert `80 0C 28 60`; `925E` unveraendert `13 88 6F`.

**Bauauftrag (gebaut 12.10.)**
1. Neuer Zustand "Laden" in der Zustandsfolge: Inhalt aller Zeilen wie unter "Zustand `A0/41`", 0x426 `00 A0 41 01 6A 71 00 80`, 0x436 B0 `AD`.
2. Weckfolge davor mit den 0x426-Codes und Zeiten aus Log 2, Byte 7 `80` ab `60/61` (250,37 s), 0x436 B0 Rampe auf `AD`, 0x0EC erst 2,05 s nach dem Start.
3. Ende wie im Log (Codes und Zeiten, 0x436 B0 Rampe zurueck) bis `00/02` und Stille.
4. Dauer von "Laden" im Auto: 297,6 s und 529,8 s, jeweils bis zur Fernbedienung; Vorgabe fuer den Emulator offen (vom Nutzer zu entscheiden).
5. Nicht gemessen und deshalb nicht zu raten: ob die BMS in "Laden" das Entladen zulaesst, ob die Schuetze gleich schalten, was die Sicherung von 9261/91C1 ausloest, ob E14381 in "Laden" ausbleibt (Bench-Test nach dem Bau).
6. Standardwert 0x426 km bleibt auf der Liste (0x5D7 bleibt auf der Bench aus, Fahrzeug-CAN-Frame). Verschoben am 12.10.: **Wake-Formen der uebrigen EVC-Frames und Startversatz** in den Abschnitt "Messung der eigenen Sendeabstaende" (naechster Abschnitt); der kurze Einschlafweg ist jetzt der Abschnitt "Einschlafweg lang / kurz" (Prioritaet HOCH).
7. **Vergleich mit unserer FRAMES-Tabelle (Befund Abschnitt 7, nur Gemessenes):** Die Zeilen "wach" von 0x428, 0x435, 0x1C7, 0x4F7, 0x42F und 0x419 kommen in `A0/41` nicht vor.
   - Fast konstant in der Ladephase: 0x428 `00 00 00 00 00 04 8E 00` (Log 1) / `... 04 8F 00` (Log 2); 0x435 `FF FF 1E 6A 35|36 00 00 00` mit B5-Pulsen `FC` ca. alle 10 s (ca. 0,2 s lang); 0x1C7 `AA 23 00 04 34 00`.
   - Wake-Phase (rund 4 s): 0x428 B5 `06` -> `04`, B6 `80`, dann +2 alle 0,1 s bis `8E`/`8F`; 0x435 B3 `A9` -> `AA`; 0x1C7 `02 36` -> `02 34` -> `04 34`.
   - Dynamisch in der Ladephase, Bedeutung und Pruefsumme unbekannt: 0x4F7 (B2 `0C`, B3 48 -> 5D, B4 `28`, B1/B5 veraenderlich), 0x42F (alle Bytes veraenderlich, Start `00 20 00 0C 80 00`), 0x419 (B0 `61`, B2 `2A`, B3 `8F`, B1/B4 veraenderlich).
   - Wie die dynamischen Frames nachgebildet werden (Konstante, aufgezeichnete Folge): nicht festgelegt.
   - Zusammenhang mit E14381: nicht belegt.

**Auswahl von "Laden" (Entscheidung des Nutzers, 11.10. abends: Moeglichkeit C)**
1. **Auswahlfeld "Zustand":** "Laden" als zusaetzlicher Eintrag, nur bei gesetzter Checkbox "manuell" bedienbar (wie die bisherigen sieben Eintraege).
2. **Radiobuttons fuer die Betriebsart "Fahren", "Laden" und "Schlafen":** bestimmen das Ziel der automatischen Folge ("manuell" aus).
3. **Vorerst nur manuelle Bedienung.** Die Radiobuttons werden gesetzt und im **NVM gespeichert** (wie die Format-Wahl); sie bleiben nach einem Neustart erhalten.
4. **Noch festzulegen (der Nutzer ergaenzt es spaeter):** wie die Betriebsarten im automatischen Ablauf wirken sollen (Ziel der Folge nach Start und Aufwecken, was "Schlafen" in der Folge bedeutet, Dauer von "Laden", Auswahl der Ende-Folge).
5. Nicht festgelegt und deshalb im Bau offen zu lassen: Verhalten von Sleep, Sleep 0x9281 = 1 und NVROL je nach gewaehlter Betriebsart.

## Zum Bauen vorgemerkt: Messung der eigenen Sendeabstaende (Stand 11.10., GEBAUT 12.10., siehe oben)
Anlass: Frage, ob das Timing von 0x426 zu E14381 beitragen kann. Nur Gemessenes und Codestellen, keine Annahme ueber die Ursache.

**Gemessen im echten Auto (Fahrlog 22aaf176, Ladelog 445a1527; Zeitaufloesung der Logs 10 ms)**
- 0x426, 0x423, 0x435, 0x436: 96 bis 99 % der Abstaende sind 0,10 s, der Rest 0,09 oder 0,11 s.
- 0x419, 0x428, 0x42F, 0x4F7: 96 bis 98 % bei 0,10 s. 0x1A1 und 0x19F: 98 bis 99 % bei 0,01 s.
- 0x435 und 0x436 kommen im selben Zeitstempel wie 0x426 (Versatz 0,00 s; in 41 bzw. 104 von je ca. 1060 Zyklen im Ladelog 0,01 s).
- 0x423 kommt nach 0x426: +0,01 s (Fahrlog, 889 von 1215 Zyklen; -0,01 s in 326), +0,03 s (Ladelog, 888 von 1060; +0,04 s in 172).

**Stand im Code**
- Kernschleife 1 ms, `transmit()` bei jedem Durchlauf (`Software.cpp`, `core_loop`, Zeilen 605-676).
- `send_bus_vehicle_frames` (`RENAULT-TWINGO-GEN1-BATTERY.cpp:3611`): 0x19F, 0x423, 0x426, 0x436 je mit eigenem Timer (`now - bus_last_ms[slot] < interval`, dann `bus_last_ms = now`), kein gemeinsamer Burst.
- 0x435, 0x419, 0x428, 0x42F, 0x4F7, 0x1A1: ueber `send_simulator_signals`, je Zeile eigener Timer.
- `transmit_can_frame_to_interface` ruft `print_can_frame` im Sendepfad auf (`comm_can.cpp:317`).
- In den vorhandenen Logs stehen keine TX-Zeilen des Emulators; die eigenen Sendeabstaende sind nicht gemessen.

**Bauauftrag (gebaut 12.10.)**
1. Zaehler in der Firmware fuer 0x426, 0x435 und 0x436: Zeit zwischen zwei Sendungen und Versatz der drei Frames zueinander, in Mikrosekunden.
2. Anzeige der Werte auf der Seite (Minimum, Maximum, Mittel, Anzahl der Abstaende ausserhalb 100 ms +-1 ms ist noch festzulegen).
3. Alternative ohne Bau: zweites Geraet als CAN-Logger mit Hardware-Zeitstempeln am Bus (vom Nutzer zu entscheiden).
4. Nicht gemessen und deshalb nicht zu raten: ob die MCPU den Takt von 0x426 prueft und ob E14381 davon abhaengt.
5. **Startversatz der Frames** (verschoben aus "Zustand Laden", 12.10.): Im Auto beginnen 0x426, 0x435 und 0x436 im selben Zeitstempel, 0x423 kommt 0,13 s frueher. Bei uns hat jede Zeile einen eigenen Timer. Einstellen erst, wenn die Messung (Punkt 1 bis 3) den eigenen Versatz zeigt.
6. **Wake-Formen der uebrigen EVC-Frames** (verschoben aus "Zustand Laden", 12.10.): Gebaut sind die Wake-Formen von 0x0EC, 0x0ED und 0x19F. Fuer 0x157, 0x1A1, 0x1C7, 0x419, 0x428, 0x42F, 0x435, 0x4F7, 0x500 und 0x511 gibt es keine. Gemessen beim Wecken im Ladelog 2: 0x1C7 `AA 23 00 02 36 00` (spaeter `02 34`, dann `04 34`), 0x435 `FF FF 1E EB 35 FC 00 00`, 0x428 `00 00 00 00 00 07 C0 00`, 0x42F `00 00 00 00 00 00`, 0x4F7 `00 7F 00 00 09 FC 06 F0`, 0x419 `00 00 00 00 00 00` (Werte aus dem Abschnitt "Zustand Laden", Wecken durch den Stecker). Umsetzung nicht beauftragt.

## Zum Bauen vorgemerkt (WICHTIG): eigene Zeile "DTC SCPU" auf der Seite (Stand 12.10., GEBAUT 12.10., siehe oben)
Anlass: Nach "Read DTC SCPU" erscheint unten nichts (Nutzer, 12.10.). Nur Codestellen und Nutzerangaben, keine Annahmen.

**Stand im Code**
- Knopf "Read DTC SCPU" (`advanced_battery_html.cpp:61`, `reload_after = false`) ruft `read_DTC_scpu()` auf (`RENAULT-TWINGO-GEN1-BATTERY.cpp:4250`): freie Anfrage `19 02 <Maske>` an Ziel 1 (SCPU), die Antwort geht in das Feld "Answer" (`twingoQueryResult`, Zeile 4107). Dieses Feld wird bei jeder weiteren freien Anfrage ueberschrieben.
- Die Zeile unten "DTC Read (ext. protocol, Read DTC button)" (Zeile 4170, `dtc_ext_log_read`) gehoert zum MCPU-Knopf "Read DTC" (`reload_after = true`, Seite laedt nach 1,5 s neu).
- Die Maske kommt aus dem Haken "Read ALL DTC statuses (mask 0xFF)" (`dtc_ext_read_mask`, Standard `0x09`); mit Haken sendet der Knopf `19 02 FF`, das der Nutzer als "no response" meldet.

**Bauauftrag (gebaut 12.10.)**
1. Eigene Zeile "DTC Read SCPU" unten auf der Seite, getrennt vom Feld "Answer", mit dem letzten Ergebnis der SCPU-Abfrage (Rohantwort und Liste der Codes mit Status).
2. Das Ergebnis bleibt stehen, bis die naechste SCPU-Abfrage kommt (wird nicht von anderen freien Anfragen ueberschrieben).
3. Die Zeile aktualisiert sich nach dem Klick von selbst (kein manuelles Neuladen).
4. Gleiches fuer "Erase DTC" der SCPU ist nicht beauftragt.
5. Noch festzulegen: ob die Zeile Namen der Codes aus der SCPU-Definition zeigt (z. B. 1BB587 = "CAN current sensor (SCPU)1", Typ 135) und ob die Maske weiter vom Haken kommt.

## Zum Bauen vorgemerkt (Prioritaet HOCH): Radiobuttons "Einschlafweg lang / kurz" (Stand 12.10., korrigiert; GEBAUT 12.10., siehe oben)
Anlass: Wunsch des Nutzers vom 12.10.; Einordnung "hohe Bedeutung" ebenfalls vom Nutzer. Quelle: 0x426-Zustandswechsel in den Auto-Logs.

**Wichtige Einschraenkung der Datenlage (geprueft am 12.10.)**
- Das Fahrlog 22aaf176 (PuTTY) hat 91 Markerzeilen "N CAN frames not printed" (zusammen 240.577 verworfene Frames), das Ladelog 1 (PuTTY) 86. Das Ladelog 2 (Tera Term) hat **keine** solche Zeile und ist vollstaendig.
- Beide Enden im Fahrlog liegen in einer Logluecke: Sitzung 1 `[2152 CAN frames not printed]` zwischen 342,70 s und 344,74 s (nach `00/07`, vor `00/06`); Sitzung 3 `[4394 CAN frames not printed]` zwischen 1478,58 s und 1482,73 s (zwischen dem letzten `70/69` und dem ersten `00/07`). In der Sitzung-3-Luecke kommt von **keiner** ID ein Frame.

**Gemessen im Fahrlog (soweit gedruckt)**
- Sitzung 1 (Ende bei 340 s), "lang": `70/69` (bis 339,97 s) -> `70/65` 0,1 s -> `60/65` 0,1 s -> `00/05` 1,9 s (340,27 bis 342,07 s) -> `00/07` 0,6 s gedruckt (342,17 bis 342,67 s) -> Luecke (2152 Frames) -> `00/06` ab 344,77 s (125 s).
- Sitzung 3 (Ende bei 1483 s), "kurz": letztes `70/69` bei 1478,50 s -> Luecke (4394 Frames, 4,3 s) -> `00/07` 0,6 s (1482,80 bis 1483,30 s) -> `00/06` ab 1483,40 s (129 s bis zum Logende bei 1612,7 s).
- **Folge:** Ob Sitzung 3 wirklich einen kuerzeren Weg nimmt, ist **nicht belegt**. Die Zustaende `70/65`, `60/65`, `00/05` (zusammen 2,1 s) passen in die Luecke von 4,3 s und koennen dort gelaufen sein. Auch die Dauer von `00/07` im langen Weg (heute 2600 ms im Code) setzt die Luecke mit an.

**Gemessen im vollstaendigen Tera-Term-Log (Ladelog 2, Ende nach A0/41), mit Dauern**
- `00/01` B7=`80` 553,27 bis 553,67 s (0,5 s) -> `00/01` B7=`40` 553,77 bis 555,17 s (1,5 s) -> `00/07` 555,27 bis 556,27 s (1,1 s) -> `00/06` 556,37 bis 556,67 s (0,4 s) -> `08/06` 556,77 bis 557,77 s (1,1 s) -> `00/06` 557,87 bis 677,77 s (120,0 s) -> `00/02` 677,87 bis 688,87 s (11,1 s). Letztes Frame im Log: `0x0ED 60 FF 00` bei 688,91 s.
- Vom Ende von `A0/41` bis `00/02` 124,6 s; bis zum letzten Frame 135,6 s. Keine Luecke ueber 0,25 s im Ende.

**Stand im Code**
- Nur der lange Weg des Fahrlogs ist gebaut: `sleep_state()` in `RENAULT-TWINGO-GEN1-BUS.h` (Zeile 783): aus `BUS_FAHRT` `70/65` 100 ms, `60/65` 100 ms, `BUS_AUS` 1900 ms, `00/07` 2600 ms, dann `BUS_WACH`. Das Ende des Ladelogs 2 (`00/01`, `08/06`) ist nicht gebaut (siehe Abschnitt "Zustand Laden", Punkt 3).
- Die Folge laeuft bei Sleep, Sleep 0x9281=1 und NVROL reset (`NVROLstateMachine == 7`).

**Bauauftrag (gebaut 12.10.), drei Wege zur Auswahl**
1. Drei Radiobuttons auf `/simulator` neben der Zustandsfolge: "Einschlafweg: **lang** (Fahrlog Sitzung 1)", "**kurz** (Fahrlog Sitzung 3)" und "**Ladelog 2** (vollstaendig gemessen)". Standard: lang (wie heute).
2. Die Wahl wird im NVM gespeichert und bleibt nach einem Neustart erhalten (wie die Format-Wahl).
3. `sleep_state()` bekommt den Weg als Parameter:
   - **lang** wie heute: aus `BUS_FAHRT` `70/65` 100 ms, `60/65` 100 ms, `00/05` 1900 ms, `00/07` 2600 ms, dann wach.
   - **kurz**: aus `BUS_FAHRT` direkt `00/07` (gedruckt 0,6 s), dann wach; aus anderen Zustaenden wie lang.
   - **Ladelog 2** (Zeiten aus der Tabelle oben): `00/01` B7 `80` 0,5 s -> `00/01` B7 `40` 1,5 s -> `00/07` 1,1 s -> `00/06` 0,4 s -> `08/06` 1,1 s -> `00/06` 120,0 s -> `00/02` 11,1 s, letztes Frame `0x0ED 60 FF 00` 11,1 s nach dem Beginn von `00/02`. Wach-Zeit von 120,0 s vor `00/02` entspricht der 00-Stufe des Busses; die Abbildung auf die bestehende Stufe "closed" (BUS_ZU) und die Frames der 00-Stufe ist beim Bau zu klaeren.
4. Tests gegen die gedruckten Zeiten des Fahrlogs (lang, kurz) und gegen das Tera-Term-Log (Ladelog 2).
5. Offen, nicht zu raten: Dauer von `00/07` im kurzen Weg (gedruckt 0,6 s) und im langen Weg (heute 2600 ms, davon 2,0 s in der Logluecke); wie der Weg "Ladelog 2" bei Sleep, Sleep 0x9281=1 und NVROL reset wirken soll (Ausloeser im Auto war die Fernbedienung nach Laden mit `A0/41` als Ausgangszustand).

**Ende des Busses: Reihenfolge der letzten Frames, Emulator so stellen, dass 0x157 als letztes Frame kommt (Wunsch des Nutzers, 12.10.)**
- **Gemessen in beiden Ladelogs** (Zeit nach dem Beginn von `00/02`; Ladelog 2 vollstaendig, Ladelog 1 PuTTY):
  - 0x0EC endet beim Beginn von `00/02` (Ladelog 2: 677,83 s).
  - Die meisten Frames (0x155, 0x424, 0x1A1, 0x1C7, 0x419, 0x428, 0x42F, 0x4F7, 0x511, 0x425) enden etwa 10,0 s danach (Ladelog 2: 687,82 bis 687,91 s; Ladelog 1: 1283,15 bis 1283,24 s).
  - 0x69F endet bei +10,5 s (Ladelog 1: +10,3 s).
  - 0x426, 0x435, 0x436 enden bei +11,00 s (jeweils im Zustand `00 00 02 ...`, 0x426 `00 00 02 01 6A 71 00 40`).
  - 0x423 endet bei +11,03 s.
  - **0x0ED `60 FF 00` und 0x157 enden zuletzt, im selben Zeitstempel** (+11,04 s im Ladelog 2: 688,91 s; +11,06 s im Ladelog 1: 1284,26 s). Letzte Werte 0x157: `D5 77 09 C4 FF E0 00 80` (Ladelog 2), `D5 17 09 C4 FF E0 00 80` (Ladelog 1; B1 ist der Zaehler).
- **Stand im Code:** `send_bus_closing()` (`RENAULT-TWINGO-GEN1-BATTERY.cpp`) sendet in der 00-Stufe alle eingeschalteten `R`-Zeilen ausser 0x0EC im Zustand `BUS_ZU` (0x0ED in der Schlussform `60 FF 00`) so lange wie die 00-Stufe dauert (900 ms "wie im Auto", 1000 ms normal). Alle Zeilen enden dabei gleichzeitig, nicht in der Reihenfolge des Autos, und nach 0,9 bis 1 s statt nach etwa 11 s.
- **Bauauftrag (gebaut 12.10.):** Die Bus-Schlussfolge so legen, dass die Zeilen in der Reihenfolge des Autos enden: 0x0EC bei `00/02`, die uebrigen Zeilen nach etwa 10 s, 0x69F bei etwa 10,5 s, 0x426/0x435/0x436 bei 11,00 s, 0x423 bei 11,03 s und **0x157 zusammen mit 0x0ED zuletzt (11,04 s)**. Gehoert zum Einschlafweg "Ladelog 2" (dort steht `00/02` 11,1 s und das letzte Frame `0x0ED`). Offen, nicht zu raten: ob dieselbe Endreihenfolge fuer "lang" und "kurz" gelten soll (im Fahrlog nicht pruefbar, Luecken).

## Zum Bauen vorgemerkt: Display-Anzeige beim Einschlafen (Wunsch vom 12.10., GEBAUT 12.10., siehe oben)
**Wunsch des Nutzers:** Auf dem OLED-Display (vom Nutzer als SSD1309 bezeichnet) in der mittleren Zeile, in der sonst die Stoerungen stehen, waehrend des Einschlafens:
- Zeile 1 (mittig zentriert): `Sleep pls wait`
- Zeile 2 darunter (mittig zentriert): abgelaufene Zeit / Endzeit des Schlafens, Beispiel `00:04:34 / 00:10:00 End`

**Stand im Code (`Software/src/devboard/display/display.cpp`)**
- Der Kopf der Datei nennt den Treiber **SSD1306**, 128x64, I2C; "SSD1309" steht im Code nirgends.
- Textraster: 21 Zeichen je Zeile, 8 Zeilen (`char buf[22]`). Zeilen 0 bis 1 Batteriestatus (Seiten wechseln), Zeile 2 Trennlinie, **Zeilen 3 bis 5 die Stoerungen** (`print_events(3, 3)`), Zeile 6 Trennlinie, Zeile 7 Netzwerk. Aktualisierung alle 500 ms.
- Das Display kennt keinen Batterietyp. Der Twingo-Treiber hat die Daten: Sleep-Lauf `NVROLstateMachine == 7`, Start der Stille `nvrol_silence_start_ms`, Beginn der Abschaltfolge `powerdown_start_ms`, Endzeit `sleep_manual_failsafe_ms()` (eingestellte Minuten, 1 bis 1440, `RENAULT-TWINGO-GEN1-BATTERY.cpp:684`), Formatierung mm:ss gibt es schon (`append_mmss`, Zeile 1881).
- Der Sleep-Lauf endet vor der Endzeit, wenn ein Aufweck-Wunsch kommt (`nvrol_wake_request`, Zeile 909).

**Bauauftrag (gebaut 12.10.)**
1. Solange ein Sleep-Lauf (Sleep, Sleep 0x9281=1, NVROL reset) laeuft: statt der Stoerungen (Zeilen 3 bis 5) zwei zentrierte Zeilen: `Sleep pls wait` und die Zeitzeile.
2. Zeitzeile: abgelaufene Zeit / Endzeit, beide `HH:MM:SS` (z. B. `00:04:34 / 00:10:00`), jede Sekunde aktualisiert. Endzeit = eingestellte "Sleep failsafe minutes" (1 bis 1440 min, also bis `24:00:00`).
3. Nach dem Ende des Sleep-Laufs (Endzeit oder Aufwecken) wieder die normale Stoerungsanzeige.
4. Das Display bekommt dafuer eine Abfrage beim Batterietreiber (kein Twingo-Code im Display).

**Entschieden vom Nutzer (12.10., spaetere Fassung ersetzt die frueheren Angaben zu Startpunkt und Endzeit)**
- Zeitformat: erste Zeit = aktueller Stand, zweite Zeit = Endzeit, beide `HH:MM:SS`, Beispiel `00:04:34 / 00:10:00` (19 Zeichen, mittig, ohne den Zusatz `End`).
- Zeilen: Text in **Zeile 4**, Zeitzeile in **Zeile 5** (Zeile 3 leer).
- Es wird **nur die Zeit ab der Einschlafzeit** gezaehlt, also ab Beginn der Stille (`nvrol_silence_start_ms`, `NVROLstateMachine == 5`, Zeile 909). Die Endzeit ist **nur die eingestellte "Sleep failsafe minutes"**; die Dauer der Abschaltfolge wird **nicht** dazuaddiert.
- **Vor** der Einschlafzeit (waehrend der Abschaltfolge, `NVROLstateMachine == 7`): Zeile 4 zeigt nur `Sleep requested`, Zeile 5 zeigt den Zaehler `00:00:00 / 00:10:00` (Beispiel mit 10 min Failsafe; Stand 0, Endzeit = Failsafe).
- **Ab** der Einschlafzeit (Stille): Zeile 4 wechselt auf `Sleep pls wait` (**vom Nutzer bestaetigt, 12.10.: "sleep pls wait wechsel stimmt"**), Zeile 5 zaehlt die abgelaufene Zeit der Stille gegen die Endzeit, z. B. `00:04:34 / 00:10:00`.
- **Loeschen (Nutzer, 12.10.):** Die Texte und die Zeiten muessen im Display **geloescht (cleared)** werden, damit dort wieder die Stoerungen stehen koennen. Das gilt, sobald der Sleep-Lauf endet (Endzeit erreicht oder Aufwecken). Die Texte werden nicht bei laufendem Sleep geloescht.
  - Stand im Code (`display.cpp`): `print_events(3, 3)` schreibt die Zeilen 3 bis 5 alle 500 ms komplett neu (je 21 Zeichen, mit Leerzeichen aufgefuellt) und leert nicht benutzte Zeilen (`Clear remaining lines`, Zeilen 395-400). Beim Sleep-Lauf darf `print_events` in dieser Zeit nicht laufen (sonst ueberschreibt es die Anzeige); danach laeuft es wieder und ueberschreibt die Sleep-Texte und die Zeiten von selbst.
  - Beim Bau zu beachten: Die Sleep-Zeilen werden mit 21 Zeichen aufgefuellt geschrieben (auch Zeile 3 leer), damit beim Wechsel `Sleep requested` -> `Sleep pls wait` keine Reste stehen bleiben.

**Gemessen im Code (nur zur Info, wird nicht addiert): Dauer der Abschaltfolge vor der Stille** (`NVROLstateMachine == 7`, Zeilen 874-896; Konstanten in `.h` Zeilen 802-815)
- Normal (Schalter A aus): C3 66.000 ms + C2 60.000 ms + C0 10.000 ms + 00 1.000 ms = 137.000 ms (00:02:17).
- "Wie im Auto" (Schalter A an): 63.200 + 60.000 + 10.000 + 900 ms = 134.100 ms (00:02:14).
- Die Stille endet nach "Sleep failsafe minutes" ab dem Ende der Abschaltfolge; Standard 30 min (`SLEEP_MANUAL_FAILSAFE_DEFAULT_MIN`). Ein Aufweck-Wunsch (`nvrol_wake_request`) beendet den Lauf frueher.

**Noch festzulegen (nicht zu raten)**
- Was die Anzeige bei den geplanten Einschlafwegen "kurz" und "Ladelog 2" zeigt, und ob waehrend des Aufweck-Buendels (`NVROLstateMachine == 8`) noch etwas angezeigt wird.

## Zum Bauen vorgemerkt (Bau 6): keine Frames auf dem Bus, deren Zeile aus ist (Stand 12.10., Pruefung aller 71 Zeilen; wartet auf "bauen")

**Anlass (Nutzer):** Der Bus muss Stille erreichen; Frames, die im Simulator nicht angehakt sind, duerfen zu keiner Zeit gesendet werden.

**Gemessen (Probelauf 12.10., je Zeile nur diese an, kompletter Zyklus Aufwachen/Fahren/Sleep/Stille/Wake, Wege "Ladelog 2" und "lang"; zusaetzlich Code-Durchsicht aller Sendestellen):**
- 70 von 71 Zeilen senden ihr Frame nur bei gesetztem Haken; jede Zeile sendet ihr Frame, wenn sie an ist. `0x214` ist in beiden Modi durch den Haken gesteuert (`BATTERY.cpp:1428`, `:3937`).
- **`0x350` wird trotz ausgeschalteter Zeile gesendet:** im Sleep-Lauf (Zustand 7, alle 100 ms) und im Wake (Zustand 8). Der Haken schaltet nur den Dauerframe `send_run_350()`, nicht `send_350_frame()` (`BATTERY.cpp:888-897`, `:925-950`, `:1492-1501`, `:2391-2399`).
- **`0x79B` (11 Bit) wird gesendet, ohne dass es eine Zeile gibt:** Standard-UDS-Pfad der Basisklasse, `setup_uds(0x79B, 0x7BB)` (`BATTERY.cpp:4323`), etwa alle 200 ms im Wachzustand, in den Abschaltstufen und nach dem Wake; nicht in der Stille. Auf dem Akku-Bus kommt `0x79B` nicht vor; der Pfad antwortet an diesem Akku nie (`BATTERY.cpp:4351-4357`).
- Die Stille selbst ist still (keine 11-Bit-Frames zwischen Ende der Abschaltfolge und Wake).

**Bauauftrag (noch nicht gebaut):**
1. **Reparatur `0x350`** (Nutzer, 12.10.): `0x350` bleibt in der Simulator-Liste, muss aber auch im Sleep (Zustand 7) und Wake (Zustand 8) ausgehen, wenn die Zeile nicht angehakt ist: Senden im Sleep/Wake nur bei gesetztem Haken der Zeile 0x350 (Zeile 2). **Der Takt bleibt immer:** die Stufenzeiten (`powerdown_stage`, C3 / C2 / C0 / 00 als interne Zeitgeber, passen zu Ladelog 2: `A0/41` bis 553,27 s, `00/02` ab 677,87 s = 124,6 s gegen 126 s C3+C2) und der Aufruf `hv_observe_350(...)` fuer das HV-Modell (`BATTERY.cpp:1500`, `:2408`, `:3351`) muessen erhalten bleiben. Die Bus-Folge der EVC-Frames (0x426 "zu", 0x0EC endet, 0x0ED/0x157 zuletzt) haengt nicht am Senden von `0x350`.
2. `0x79B` (Standard-UDS-Pfad der Basisklasse) abschalten; vorher pruefen, ob etwas davon abhaengt (PID-Scan, SOH-Kandidat).
3. Regressionstest: fuer alle 71 Zeilen, nur diese an, kompletter Zyklus: keine 11-Bit-Frames ausser dem der eigenen Zeile.
4. **UDS-Rundlauf seltener (beschlossen, Nutzer 12.10.):** `EXT_POLL_INTERVAL_MS` von 200 ms auf 1000 ms (5 Anfragen pro Sekunde auf 1 pro Sekunde; `BATTERY.h:694`, einzige Verwendung `BATTERY.cpp:4038`). Folgen (gerechnet aus dem Code, nicht gemessen): Durchlauf mit Zellwerte-Quelle UDS 136 Ziele = 136 s statt 27,2 s; mit Broadcast 40 Ziele = 40 s statt 8 s; Anzeigewerte (SOC, Strom, Zaehler, Zeit `9261`/`91C1`) werden entsprechend seltener aktualisiert. Dasselbe Intervall gilt auch fuer die 4 Prioritaets-PIDs nach dem Wake (BMS-Zustand, `9281`, `9251`, `9252`; `EXT_PRIORITY_COUNT` = 4, Timeout 30 s): das Abfragen aller vier dauert dann etwa 4 s statt 0,8 s. **Entschieden (Nutzer, 12.10.): 1 s genuegt fuer alles, auch fuer die Prioritaets-PIDs nach dem Wake.** Das freie Abfragefeld, "Read DTC" und der Cellwatch sind davon nicht betroffen (eigene Zustandsmaschinen).


**29-Bit-UDS-Abfragen, geprueft 12.10. (Probelauf, alle Zeilen aus, Weg "Ladelog 2"):**
- Im Wachzustand geht alle 200 ms eine Anfrage auf `0x18DADBF1` (5 pro Sekunde; `03 22 90 xx`, 136 Abfrageziele, ein Durchlauf 27,2 s, Zellen-Rundlauf mit Prioritaets-PIDs; `BATTERY.h:667-694`, `BATTERY.cpp:4036-4075`). Korrektur 12.10.: die erste Fassung dieses Eintrags nannte 30 pro Sekunde; die 300 Frames des Probelaufs lagen in 60 s, nicht in 10 s. Es gibt dafuer keine Zeile und keinen Haken; nur "Cellwatch" und die Zellwerte-Quelle (UDS/Broadcast) greifen ein (Broadcast ueberspringt nur die Zell-PIDs 0x9021..0x9083, im Code gelesen, nicht gemessen).
- In der Abschaltfolge (Sleep, 137 s plus Schliessen) wurden keine 29-Bit-Frames gesendet, in der Stille ebenfalls keine; nach dem Wake setzen sie wieder ein (Prioritaets-PIDs, danach Rundlauf).
- Im echten Auto fragt die EVC dagegen nur gelegentlich (`9215`, `91D1`, `91D2`, `92C1`; `9243`, `9245`, `91CF`, `91C1`, `901B`; `BEFUND_Ladelog_Twingo_11-10.md:44`), keinen Zellen-Rundlauf. Auf dem Bench liegt im Wachzustand also Verkehr, der im Auto nicht vorkommt. Ob das den Akku stoert, ist nicht bekannt. Ob dafuer ein Haken (Rundlauf aus) gebaut werden soll, ist nicht entschieden.

**Offen / nicht geprueft:** Ziel "Laden", Weg "kurz" und altes Zoe-Format nur im Code durchgesehen, nicht im Probelauf; ob die Zeile `0x350` (Dauerframe) ganz entfallen soll, ist nicht entschieden. Ob die zusaetzlichen Frames das Speichern von `9261` stoeren, ist nicht bekannt.

## Zum Bauen vorgemerkt (Bau 7): Schlaf-Anforderung ueber 0x423 B0 = 04 und Wake-Frames 30/33 (Stand 12.10. abends; wartet auf "bauen")

**Behauptung (Quelle "dj2cs", Nutzer 12.10.; Adressen nicht pruefbar, Quelle unbekannt):** Die Einschlaf-Anforderung kommt per CAN, nicht ueber KL15. `0x423` Byte 0 Bit 1:0 == 0 => ECU-State 8 "Shutdown pending", Bit 5:4 waehlt die Abschaltart. Faellt `0x426` 2 s aus, erzwingt das denselben State. Danach 1 s Nachlauf mit vollstaendiger Busstille (100 Zyklen der 10-ms-Task), in dem die CAN-Timeout-Flags unterdrueckt werden (sonst Fatal-Code 0x38). In diesem Fenster schreibt das Steuergeraet sein NVM (einziger NVM-Schreiber der Firmware, kein zyklischer Writer): Fehlerspeicher (Block 0xA6, 1444 B), SOC/SOH-Historie (0xA0), Zaehler (0xAB, 0xAA, 0x19), Block 0x18 mit Fahrzeug-ID und AbsTime.

**Gemessen in den drei Auto-Logs (12.10., eigene Auswertung der `RX4`-Zeilen):**
| | Ladelog 2 (445a1527) | Fahrlog (22aaf176) | Ladelog 1 (598b197c) |
|---|---|---|---|
| `0x423` B0 `07` -> `04` | 687,90 s | 747,77 s | 1283,23 s |
| Frames mit `04` | 11 | 10 | 11 |
| letztes Akku-Frame (`0x155`, `0x1C9`, `0x0C5` ...) | 687,90 s | 747,77 s | 1283,24 s |
| letzte EVC-Frames (`0x426/0x435/0x436/0x157/0x0ED`) | 688,87-688,91 s | 748,66-748,72 s | 1284,20-1284,26 s |
- Der Akku hoert in allen drei Logs im selben Moment (hoechstens 0,01 s Abstand) auf zu senden, in dem `0x423` auf `04` geht; die EVC sendet danach noch rund 1 s weiter. In diesem Moment aendert sich bei den anderen EVC-Frames nichts ausser `0x0ED` B2 (`80` -> `00`, 0,08-0,09 s vorher) und spaeter `0x435`: B5 `00` -> `FC` (0,47 s bzw. 0,39 s nach Beginn von `04`) und B3 `0B` -> `EB` (0,57 s bzw. 0,69 s).
- Beispiel `0x423` am Ende: `04 1E FF FD B2 A0 B2 CA` (Ladelog 2), `04 1E FF FD 5D A0 B2 C8` dann `B2 A0 5D C8` (Fahrlog); B4/B6 wechseln wie sonst alle 5 Frames.
- Beim Wake zeigen alle Logs als erste `0x423`-Frames (je ein Frame, 50 ms Abstand): `30 7F FF FF FF E0 FF FF`, dann `33 7F FF FF FF E0 FF FF`, danach die normalen (`07 1E FF FD B2 80 B2 xx`). Zeiten: Ladelog 2 247,64 s, Ladelog 1 611,77 s, Fahrlog 84,61 / 565,50 / 1354,62 s.
- Sonst nur `07`, `0B`, `87` (B0 in den Phasen Laden/Wach/Fahrt).

**Bei uns (Code):** `0x423` wird nie mit B0 = `04` gesendet (nur die Zustandszeilen mit `07`/`0B`, keine Sonderform fuer `K_423`); die Schliessform aendert nur `0x0ED` B2 auf `00` (`BUS.h:684-688`), und zwar fuer die ganze Schliessphase, im Auto nur etwa in den letzten 1,2 s. `0x30`/`0x33` beim Wake werden nicht gesendet.

**Bauauftrag (noch nicht gebaut):**
1. Schliessphase (alle drei Wege): `0x423` B0 = `04` fuer die letzte Sekunde (Ladelog 2: ab etwa 10,03 s nach `00/02`, 11 Frames), Rest des Frames wie die Zustandszeile "zu".
2. `0x0ED` B2 = `00` nur kurz davor (etwa 0,1 s vor `04`), nicht fuer die ganze Schliessphase.
3. `0x435` B5 = `FC` ab etwa 0,4-0,5 s nach Beginn von `04`, B3 = `EB` ab etwa 0,6-0,7 s.
4. Wake: die ersten beiden `0x423`-Frames `30 7F FF FF FF E0 FF FF` und `33 7F FF FF FF E0 FF FF` (50 ms Abstand), danach wie bisher.
5. **Zeiten der Schliessphase genau wie im Auto (Nutzer, 12.10.)**, gemessen in den Logs (Beginn der letzten `00/02`-Phase von `0x426`, Beginn von `0x423` = `04`, letztes Frame):
   - Ladelog 2: `00/02` ab 677,87 s, `04` ab 687,90 s (10,03 s), letztes Frame 688,91 s (11,04 s).
   - Ladelog 1: `00/02` ab 1273,2 s, `04` ab 1283,23 s (10,03 s), letztes Frame 1284,26 s (11,06 s).
   - Fahrlog 22aaf176 (Sitzung 1): `00/02` ab 697,96 s, `04` ab 747,77 s (49,81 s), letztes Frame 748,72 s (rund 0,95 s nach `04`).
   - Davor in den Ladelogs: `A0/41` -> `00/01` -> `00/07` -> `00/06` -> `08/06` -> `00/06`, danach rund 120 s `00/06` bis `00/02`.
   - Zu klaeren beim Bau: Welche Zeiten die Wege "lang" und "kurz" bekommen (Ladelog: 10,03 s + rund 1 s; Fahrlog: 49,81 s + rund 0,95 s); heute ist die Schliessphase dort 1,0 s (0,9 s "like the car"), bei Ladelog 2 11,1 s.
6. **Schalter "like the car" im Flash speichern** (aus der Liste der offenen Punkte, 12.10.): heute `static bool shutdown_like_car`, Startwert `false`, nur im Speicher (`BATTERY.cpp:1009`, `webserver.cpp:868`), die Seite sagt "runtime only"; neu: eigener NVM-Schluessel, beim Start einlesen, Satz auf der Seite entfernen, Test (Wert nach dem Laden wieder gesetzt). Die Einschlafwege und die Betriebsart werden schon gespeichert (`TWSLPATH`, `TWBUSTGT`).
7. Tests gegen die drei Auto-Logs (Zeitpunkte relativ zum Beginn von `00/02`), danach Test T2 der Merkliste.

**Branch fuer Bau 7:** `twingo-bau7-schlaf-anforderung` (angelegt 12.10. von `twingo-echter-bus-modus` bei `cffc205`, gepusht, noch kein Code darauf). Bau 6 ist noch keinem Branch zugeordnet.

**Nicht belegt:** dass `0x423` B0 = `04` die Ursache ist (die Logs zeigen nur, dass es der einzige gleichzeitige Unterschied ist); die Angaben zum NVM (Bloecke, Adressen, ein Schreiber); ob unsere bisherigen Sleep-Laeufe das Schreiben ueber den Weg "`0x426` 2 s weg" ausgeloest haben.

## Offen, nicht entschieden oder abhaengig vom Bench
- **0x426 Byte 0 bis 7 bitweise einstellbar** (nicht gewuenscht in der Form "alle Bytes"; der Wunsch vom 11.10. ist Punkt 1).
- **km steigt beim Fahren von selbst** (z. B. +1 km pro Minute im Zustand "driving"): nur nach Rate aus dem Auto; heute fest.
- **Zeitquelle der MCPU-Missionen** (0x350, 0x436, 0x523 oder 0x376) per Test mit weit auseinanderliegenden Altern (1.400.000 und 1.500.000) klaeren, danach evtl. nur die richtige Quelle senden.
- **Pack-km auf 6.844 bringen:** von Hand per km-Feld (aktuell 13 km; +6.831 setzen), kein Bau noetig.
- **Pack-Zeit** (`9261`/`91C1` am MCPU) bleibt 0; Missionen tragen Zeitstempel. Test: lange Laufzeit im Fahrmodus (10, 30, 60 min). **Stand 12.10.:** MCPU-`9261` weiter 0 (auch nach +1000, `9281` und Sleep); SCPU-`9261` = 1.360.946 min (folgt 0x436); `9259` = 05, `925C` = 01; `925E` = `13 88 6F` (= 0x69F B1-B3); `925B` = `80 10` (Software Reset und Irregular Reset). Speichern am Sitzungsende (Auto) am Bench nie gesehen; `91C1` am 12.10. nicht gelesen. Naechster Test: Neustart (12 V), Sitzung mit allen 17 EVC-Zeilen, Auto-Endfolge, danach `9261`/`91C1` lesen (`9261 - 91C1 = 784.382` im Auto). Details: `TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`, Abschnitt "Ergebnis Bench 12.10.".
- **E14381** mit `0x426` an (Test B: `0x157` und weitere EVC-Zeilen einzeln zuschalten). **Stand 12.10.:** Nach dem Anheben der Zeit in 0x436 um +1000 ging E14381 von `2F` auf `2E` (nicht mehr aktiv) und liess sich quittieren; alle bisherigen Eingriffe (alle Zeilen, Pruefsummen, Zustaende, km) hatten ihn auf `2F` gelassen. Basis- und Zielwert des Tests nicht festgehalten. Schwelle (ab welchem Wert `2E`) offen.
- **`1B0E41`** (Speicherfehler MCPU, 6926 "External NV memory", Test 65 "general checksum failure") aktiv `2F`: **Stand 12.10.:** `19 02 09` an die MCPU zeigte zuerst keine Fehler, in einer spaeteren Lesung wieder `1B0E41 = 2F`. Ob er nach `14 FF FF FF` sofort wiederkommt und ob er das Speichern von `9261`/`91C1` blockiert, ist offen (Hypothese, nicht belegt). SCPU am 12.10.: `1BB587 = 2F` (Stromsensor, fehlende Frames) aktiv, `1BB108 = 2E` (Slave-Kommunikation) zeitweise.

## Offen beim Nutzer
- **Bench-Test** in Schritten (Plan: `TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`): erst nur `0x423`, dann die uebrigen BMS<>EVC-Frames nach und nach; `0x350` und die anderen Fahrzeug-CAN-Zeilen erst als Zusatz. Logger ohne Filter (RX und TX).
- Pin-Belegung (CAN H/L) am LBC-Stecker, damit die Aderfarben aus dem Schaltplan zugeordnet werden koennen.
