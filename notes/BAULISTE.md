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
- Abschalt-Taster (Nice-to-have): Sleep inkl. `0x9281` ohne Wiederanlauf, SSD1306-Hinweise "Shutdown requested / Sleep requested / Battery sleeping - Turn off now!!". **Jetzt konkretisiert als Bau 12 (GPIO15), siehe unten.**
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

## Zum Bauen vorgemerkt (Bau 6): keine Frames auf dem Bus, deren Zeile aus ist (Stand 12.10., Pruefung aller 71 Zeilen; GEBAUT 13.10., siehe unten)

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

## Zum Bauen vorgemerkt (Bau 7): Schlaf-Anforderung ueber 0x423 B0 = 04 und Wake-Frames 30/33 (Stand 12.10. abends; GEBAUT 13.10., siehe unten)

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

## Gebaut am 13.10. (Bau 6 und Bau 7, Branch twingo-bau7-schlaf-anforderung, Commit f8b9acd, 483 Tests gruen, Bench-Test offen)
- Bau 6: 0x350 (Sleep/Wake) nur mit Haken in Zeile 0x350, Takt bleibt; 0x79B-Scan aus; UDS-Rundlauf 1000 ms statt 200 ms; Test ueber alle 71 Zeilen (nur die angehakte Zeile sendet, Fahren/Sleep/Wake) - Ergebnis: gruen.
- Bau 7: Schliessphase aller drei Wege nach den Autologs (Ladelog 2: 04 nach 10,03 s, Ende 11,04 s; Fahrlog lang/kurz: 04 nach 49,81 s, Ende 50,76 s). 0x0ED B2=00 etwa 0,1 s vor 04, 0x435 B5=FC und B3=EB danach, 0x423 B0=04 am Schluss. Wake: 0x423 `30 7F FF FF FF E0 FF FF`, nach 50 ms `33 ...`. Schalter "like the car" im Flash (Schluessel TWLIKECAR).
- Folge: Weg lang/kurz dauert jetzt ca. 136 s Stufen + 50,8 s Schliessphase = ca. 188 s (vorher ca. 137 s).
- NICHT bewiesen: dass 0x423=04 das Schreiben der Zeit in den MCPU-Speicher (9261) ausloest. Bench-Test T2 klaert das.
- Ein Test-Hilfsmittel: die Simulator-Maske wird vor jedem Test zurueckgesetzt (Tests hatten sich gegenseitig die Maske verstellt).

## Zum Bauen vorgemerkt (Bau 8, klein): Anzeige der Stille-Seite: Zeilenumbruch und Zeile "BMS SILENT since" (Stand 13.10.; GEBAUT 13.10., siehe unten)
- Problem (Screenshot des Nutzers, 13.10.): Die Zeile "Frames per second (first 60 s): 360,360,384,..." hat 60 Zahlen ohne Leerzeichen und bricht nicht um; sie laeuft ueber den rechten Rand der Seite hinaus. Die Zeile "Frames per 10 s" ist kurz genug.
- Stelle im Code: `BATTERY.cpp:1951-1962` (Komma ohne Leerzeichen, `s += ','`). **Entschieden (Nutzer, 13.10.): nach je 10 Werten ein `<br>`** (eine Zehnergruppe pro Zeile, Komma ohne Leerzeichen bleibt). Reine Anzeige, kein Einfluss auf das Senden.
- **Zweiter Punkt in Bau 8 (Nutzer, 13.10.): Zeile "BMS frames received ... last one X s ago - BMS SILENT since mm:ss" ist falsch.** Ursache (im Code geprueft): `BATTERY.cpp:909` setzt `nvrol_silence_last_rx_ms = 0` beim Beginn der Stille. Danach zeigt `:1938` "jetzt minus 0" = Betriebszeit des ESP (Screenshot: 6233,6 s statt ca. 140 s), und `:1945` rechnet "0 minus Beginn der Stille" ohne Vorzeichen -> Ueberlauf (Screenshot: 71481:14). Die Zaehler und die ID-Liste sind richtig. Vorschlag: In der Stille, wenn kein Frame mehr kam (letzter Zeitstempel <= Beginn der Stille), den Text wie die "Result"-Zeile (`:1986-1992`) bilden: "BMS war schon vor der Stille still, letzter Frame X s davor". Reine Anzeige. Fehler ist aelter als Bau 6/7.

## Zum Bauen vorgemerkt (Bau 9): vier neue Eingabefelder auf der Advanced-Seite, direkt unter "Sleep failsafe (auto wake after)" (Wunsch Nutzer, 13.10.; GEBAUT 13.10., siehe unten)
Alle vier Felder stehen auf der Advanced-Seite (Seite mit "Sleep failsafe") unter den Schlaf-Minuten, jeweils mit "Set" und im Flash gespeichert.
1. **Minutenfaktor fuer das Fahrzeugalter** (Voreinstellung 1 = Echtzeit): Das Alter in `0x350`, `0x436`, `0x523`, `0x376` geht nur alle N Minuten um 1 weiter (N = 10: alle 10 Minuten +1; zum Testen 3). Dazwischen bleibt der Wert stehen. Rechnung: Startwert + abgerundet(vergangene Minuten / N) statt Startwert + vergangene Minuten (`twingo::age_plain`, `LOGIC.h`; `vehicle_age_available`, `BATTERY.cpp:1285`). Sauberes Raster bei Teilern von 60 (2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60). Zweck: Alterung des Bench-Akkus verlangsamen (Entscheidung Nutzer 13.10.: Alter absichtlich auf ca. 43 Tage reduziert).
   - Offen/zu testen: dj2cs behauptet, die Zeit in `0x436` muss streng steigen (nicht von uns bestaetigt); ein konstanter Wert zwischen den Schritten koennte anders behandelt werden. Test mit 3, dann MCPU-Zeit nach einem Sleep lesen.
2. **Uhrzeit fuer das naechtliche Schlafen** (z. B. 00:00): zu dieser Uhrzeit startet der Emulator selbstaendig einen Sleep-Lauf, damit die Werte vom Tag in den Akku geschrieben werden (MCPU-Schnappschuss, siehe MERKLISTE_Test, Messwerte 13.10.). Braucht eine gueltige Uhr (Unix-Zeit, `unix_now_valid`).
3. **Dauer des Schlafens:** wie lange dieser naechtliche Sleep dauert, bis der Emulator wieder aufweckt.
4. **Kilometer-Inkrement:** Wert in km, um den der Kilometerstand zu dieser Uhrzeit erhoeht wird (gesendeter Stand, `0x426`/`0x5D7`/`0x925F`-Quelle; genaue Zuordnung beim Bauen aus dem Code klaeren).
- **Entschieden (Nutzer, 13.10.):**
  - Der naechtliche Lauf nutzt den Einschlafweg **lang** mit **Sleep 9281**.
  - Das Feld "Dauer" kommt **zusaetzlich** zum Sicherheitsfenster dazu; das bestehende Feld "Sleep failsafe" bleibt und bekommt in der Beschriftung das Wort **"manual"** (gilt dann fuer den manuell gestarteten Sleep; Beschriftung bisher "Sleep failsafe (auto wake after)", `BATTERY.cpp:4186`).
  - Erhoeht wird der Kilometerstand in **`0x426`** (`odo_426_km`); das Kilometerfeld hat den Startwert **0** (kein Inkrement), der Minutenfaktor den Startwert **1** (Echtzeit).
- **Layout (Nutzer, 13.10.):** Die drei Felder Uhrzeit / Dauer / Kilometer-Inkrement stehen in **einer Zeile** unter der Ueberschrift **"Daily sleep"** (ein gemeinsames "Set"). Der Minutenfaktor bleibt ein eigenes Feld.
- Klarstellung (Nutzer, 13.10.): `0x5D7` wird an der Bench nicht emuliert (bleibt aus); `925F` wird nicht von uns gesendet, sondern vom Akku per UDS gelesen (Anzeige). Es wird also nur `0x426` erhoeht. Offene Punkte: keine.

## Gebaut am 13.10. (Bau 8 und Bau 9, Branch twingo-bau7-schlaf-anforderung, Commit 15fc5dd, 491 Tests gruen, Bench-Test offen)
- Bau 8: "Frames per second" bricht nach je 10 Werten um. Zeile "BMS SILENT": `nvrol_silence_last_rx_ms` wird beim Beginn der Stille nicht mehr auf 0 gesetzt; war der letzte Frame vor der Stille, steht dort "BMS SILENT: last frame X s before the quiet phase began", "last one X s ago" zeigt das echte Alter.
- Bau 9 (Advanced-Seite unter den Schlaf-Minuten): "Sleep failsafe manual (auto wake after)"; Ueberschrift "Daily sleep" mit einer Zeile Uhrzeit / Duration (min, 0 = aus) / Km und einem "Set"; darunter "Age minute factor" (Voreinstellung 1). Alle Werte im Flash (`TWAGEFAC`, `TWDSTIME`, `TWDSDUR`, `TWDSKM`).
- Verhalten Daily sleep (Ortszeit der Uhr, braucht eine gesetzte Uhr): zur eingestellten Minute wird `0x426` (odo_426_km) um das Km-Feld erhoeht und ein "Sleep 0x9281=1" auf dem Weg lang gestartet; die Stille dauert die eingestellte Dauer statt des Sicherheitsfensters; danach ist der vorher gewaehlte Einschlafweg wieder eingestellt. Einmal pro Minute des Tages; laeuft zu der Zeit schon ein Sleep, faellt der Tag aus. Dauer 0 = aus (Voreinstellung), dann wird auch kein km erhoeht.
- Minutenfaktor: Alter = Startwert + abgerundet(Minuten / Faktor). Ein neuer Faktor senkt das gesendete Alter nicht (der aktuelle Wert wird der neue Start). Beim Wechsel im Modus mit Unix-Anker wird die Systemzeit benutzt; getestet ist der Modus mit Laufzeit-Zaehlung.
- Nicht getestet am Bench: Daily sleep, Faktor 3/10 (dj2cs-Frage "streng steigend" offen).

## Zum Bauen vorgemerkt (Bau 10, klein): Stille-Seite einklappbar (Read DTC / Erase DTC sichtbar) und Checkbox fuer den Daily sleep (Wunsch Nutzer, 13.10.; Vorschlag angenommen; GEBAUT 13.10., siehe unten)
- Problem (Nutzer, 13.10.): Der Block der Stille-Seite (Frames pro Sekunde / pro 10 s, "Frames by CAN ID" mit gut 30 Zeilen, Ergebnis) bleibt nach dem Lauf bis zum naechsten Sleep stehen. Er wird vor dem Bereich "Diagnostic Trouble Codes" eingefuegt (`BATTERY.cpp:4459`, `append_quiet_html` ab `:1965`), dadurch rutschen die Knoepfe Read DTC und Erase DTC weit nach unten.
- Hinweis aus dem Code: Die Zaehler, die ID-Liste und die Frame-Zahl werden bei jedem neuen Sleep schon geloescht (Start des Laufs, `BATTERY.cpp` etwa `:735-760`); das alte Ergebnis wird also nicht addiert, es steht nur bis zum naechsten Lauf auf der Seite.
- **Beschlossen:** Den ganzen Block in einen einklappbaren Bereich "Sleep run details" (HTML `<details>`) setzen: **offen, solange ein Lauf laeuft** (Zustand 7, 5, 8), **nach dem Lauf eingeklappt**; ein Klick klappt ihn wieder auf, die Ergebnisse bleiben erhalten. Reine Anzeige, kein Einfluss auf das Senden.
- Verworfene Alternativen: Block nach dem Wake ganz ausblenden (Ergebnisse waeren weg); Bereich "Diagnostic Trouble Codes" ueber den Block verschieben.
- **Zweiter Punkt in Bau 10 (Nutzer, 13.10.): Checkbox "enable" in der Zeile des Daily sleep** (Uhrzeit / Dauer / Km), die den Daily sleep **aktiviert oder deaktiviert** und im **NVM gespeichert** wird (Voreinstellung: aus). Ohne Haken tut der Daily sleep nichts (auch kein Km-Inkrement). Heute (Bau 9) ist "Dauer 0" der Ausschalter.
  - Vorschlag, noch nicht entschieden: Mit der Checkbox wird "Dauer 0 = aus" ueberfluessig; bei gesetztem Haken soll die Dauer mindestens 1 min sein (0 wird beim Setzen abgelehnt oder auf 1 gesetzt), damit der Lauf nicht ohne Stille sofort aufweckt.
- Zu klaeren beim Bauen: Die Seite wird per Neuladen/Aktualisierung neu aufgebaut; ob ein vom Nutzer aufgeklappter Bereich beim Aktualisieren offen bleibt, aus dem Seitencode pruefen.

## Gebaut am 13.10. (Bau 10 und Bau 11, Branch twingo-bau7-schlaf-anforderung, Commit 40d41ac, 494 Tests gruen, Bench-Test offen)
- Bau 10: Der Block der Stille-Seite steht in einem einklappbaren Bereich "Sleep run details" (`<details>`): offen, solange ein Lauf laeuft (Zustand 5, 7, 8), danach eingeklappt; vor dem ersten Lauf bleibt nur die eine Zeile "not run yet". Waehrend eines Laufs baut die Seite den Bereich jede Sekunde neu auf (Aktualisierung von `nvrolBox`), er bleibt dort also offen.
- Bau 10: Checkbox "enable" in der Zeile des Daily sleep (im NVM, Voreinstellung aus). Dauer mindestens 1 min, Voreinstellung 5 min (Nutzer, 13.10.: "default 5 Minuten bei automatisiertem Sleep, danach eingebbar"); "Dauer 0 = aus" entfaellt, der Set-Knopf lehnt eine Dauer unter 1 ab.
- Bau 11: "Time in 0x436" (ACTIVE) zaehlt nur noch alle N Minuten +1 (Minutenfaktor); vorher zaehlte er jede Minute und ignorierte den Faktor (Ursache fuer "SCPU laeuft trotz Faktor 3 im Minutentakt weiter", Befund 13.10.: Screenshot Time in 0x436 ACTIVE 62549). Nicht angefasst: der Standardzaehler "14 00 xx" (`zoe_436_counter`), wenn der Eigenwert aus ist.
- Auffaellig (13.10., nicht geaendert): "Manual vehicle age" steht auf 1.372.407 min, "Time in 0x436" auf ca. 62.5k; 0x350 und 0x436 senden also verschiedene Alter.

## Zum Bauen vorgemerkt (Bau 12): Hardware-Taster "Sleep anfordern" an GPIO15 (Wunsch Nutzer, 13.10.; Funktion entschieden; wartet auf "bauen", Bau auf Zuruf)
Bisher nur "Nice-to-have" (Prioritaet 3: "Abschalt-Taster, Sleep inkl. 0x9281 ohne Wiederanlauf, SSD1306-Hinweise"); jetzt konkretisiert.
- **Pin (Nutzer, 13.10.): GPIO15** auf der Stiftleiste des T-2CAN (linke Spalte, 8. Pin von oben, `hw_lilygo2can.h`). Gewaehlt, weil er im Code nirgends belegt ist (nur CHAdeMO-Zusatzfunktion `CHADEMO_PIN_10`) und kein Strapping-Pin ist. Verworfen: 46 (Strapping, SMA-Schuetz), 17 (Minus-Schuetz), 3 (BMS_POWER), 0 (AP-Taster, Werksreset auf derselben Taste), 43/44 (RS485, Bootloader-Text auf 43).
- **Schaltung:** Taster zwischen GPIO15 und GND, **nur GND schalten, keine 12 V** (der GPIO vertraegt nur 3,3 V). Interner Pull-up (`INPUT_PULLUP`), Software-Entprellung; Leitung kurz halten, bei langen Kabeln 1 kOhm in Reihe und 100 nF nach GND.
- **Code (Vorschlag, nach dem Muster des Equipment-Stop-Tasters, `comm_equipmentstopbutton.cpp`):** neue Hardware-Funktion `SLEEP_BUTTON_PIN()` (Voreinstellung nicht verbunden; T-2CAN: GPIO15), Pin ueber `alloc_pins` anfordern, `DebouncedButton`. Schalter "Sleep button enable" (Checkbox, im NVM, Voreinstellung aus), damit der Pin ohne Taster nicht ausloest.
- **Funktion:** Ein langer Druck startet **"Sleep 0x9281=1"** (`request_sleep_temporisation()`, derselbe Lauf wie der Knopf auf der Seite): 9281 wird geschrieben, die Abschaltfolge laeuft, danach ist der Bus still; die Display-Hinweise (Bau 3: "Sleep requested", "Sleep pls wait" mit Zeit) erscheinen wie beim Knopf. Waehrend ein Lauf schon laeuft (auch in der Stille), wird ein weiterer Druck ignoriert.
- **Entschieden (Nutzer, 13.10.):**
  1. **Langer Druck** startet den Sleep (Vorschlag der Haltezeit: 2 s, gegen versehentliches Beruehren; ein kurzer Druck tut nichts).
  2. **Einschlafweg: lang**, fest (wie beim Daily sleep); der Radiobutton-Wert wird fuer den Lauf ueberschrieben und danach wiederhergestellt.
  3. **Dauer der Stille: 15 Minuten**; der Akku ist dabei ohnehin komplett abgeschaltet (der Bus ist still). Eigene feste Dauer, unabhaengig vom Fenster "Sleep failsafe manual" und von der Dauer des Daily sleep.
  4. **Kein Wecken per Taster**: ein Druck in der Stille tut nichts (Wecken wie bisher ueber die Seite oder nach Ablauf der 15 Minuten).
- Noch offen: ob die 15 Minuten spaeter ein einstellbares Feld werden sollen (heute fest).
- **Anzeige (Nutzer, 13.10.: "die Anzeige im SSD1306 haben wir ja schon vorbereitet fuer den Taster"):** Die Texte "Sleep requested" / "Sleep pls wait" mit Zeit (Bau 3, `sleep_display.h`, `display.cpp:467`) haengen am laufenden Lauf und erscheinen beim Taster-Sleep ohne neuen Anzeigecode. Die Texte "Shutdown requested" und "Battery sleeping - Turn off now!!" aus der Wunschliste sind nicht gebaut.
- **Stille-Seite, in Bau 12 integriert (Nutzer, 14.10.: "in Bau 12 integrieren, erst bauen auf Zuruf"):**
  1. Die Zeile "Frames per 10 s (from 01:00)" bricht nach je 10 Werten um (Schleife unter "Frames per 10 s" in `append_quiet_html`). In Bau 8 war nur "Frames per second" umgebrochen; die 10-s-Zeile hat nach einem langen Lauf (ueber 8 Minuten, ca. 50 Werte) den Rand ueberschritten (Screenshot 14.10.).
  2. Das "Result" ("BMS kept sending until the end - it did not go to sleep") ist falsch, wenn der Akku in der Stille still war: `record_silence_frame` wird auch im Wake-Buendel (Zustand 8, `BATTERY.cpp:2252`) aufgerufen; dadurch liegt der "letzte Frame" nach dem Ende der Stille und das Urteil faellt auf "hat bis zum Ende gesendet". Korrektur: fuer das Urteil nur Frames bis zum Ende der Stille beruecksichtigen (die Zaehler und die Zeile pro 10 s zaehlen die Wake-Frames weiter). Der Fehler war schon vor Bau 6 bis 11 vorhanden.
- **Lueckenschluss, in Bau 12 aufgenommen (Nutzer, 13.10.: "ja"):** Die Gesamtzeit in Zeile 5 des Displays (`BATTERY.cpp:3935` und `:3941`) und der Text "safety limit" auf der Stille-Seite (`:1983`) nehmen heute immer das Fenster "Sleep failsafe manual" (`sleep_manual_failsafe_ms()`). Sie sollen die tatsaechliche Dauer des laufenden Laufs zeigen (`silence_limit_ms()`): Fenster beim Knopf, Dauer beim Daily sleep (Bau 9, dort heute falsch), 15 Minuten beim Taster. Dazu braucht `silence_limit_ms()` einen Merker fuer den Taster-Lauf (15 min).

## GEBAUT (Bau 13, 14.10., Commits eff6a85 + 2c6f550, 436 Tests gruen, Golden-Lauf byte-identisch): Block 2 "simulator alt" samt Logik entfernt
Wortlaut des Nutzers: "alles was an Frameadressen unterhalb 'Simulator alt' steht, entfernen wir samt Logik aus dem Source Code und der Simulator-Seite; die weitere Ausgabe fuer empfangene IDs (nicht in der Liste) belassen wir; unterhalb dessen 'EXPERIMENTAL - unverified' entfernen wir auch; alles Punkt 13".
Aufbau der Seite `/simulator` (`simulator_html.cpp`, Stand 14.10.):
- Block 1 (Zeile 370): "ALLE SIMULATIONSWERTE wie im FAHRZEUG - BMS<>EVC-Bus (50 Adressen)" = Zeilen mit Tag R und die Indizes 3 bis 7 (0x19F, 0x426, 0x436, 0x423, 0x69F) - **bleibt**.
- **Block 2 (Zeile 406): "simulator alt (Fahrzeug-CAN, 41 Zeilen)" = alle uebrigen Zeilen: Indizes 0, 1, 2 und 8 bis 45** - Kandidat zum Entfernen, samt der Sender-Logik im Treiber (`send_simulator_signals`, `send_fast_frames` u. a.), den Eintraegen in `sim_signals` und den Tests.
- "Weitere empfangene IDs (nicht in der Liste)" (Zeile 427) - **bleibt** (Text "weder in Block 1 noch in Block 2" muss angepasst werden).
- "EXPERIMENTAL - unverified" (Zeile 466, Schalter `sim_55d_drive_mode_enabled`, Route `/editTwingoSim55dDriveMode`) - **soll weg** (Nutzer). Betrifft nur die Zeile 0x55D aus Block 2.
- "EXPERIMENTAL - rest state & staged relay closing" (Zeile 485, Schalter `sim_55d_rest_active_enabled`, Route `/editTwingoSim55dRestActive`) steht direkt darunter, betrifft ebenfalls nur 0x55D. Nicht ausdruecklich genannt; faellt mit der Zeile 0x55D weg - **Nutzer bestaetigen lassen**.
**Entscheidung des Nutzers zur Rueckfrage (14.10.):**
- 0x090 und 0x242: werden vom Nutzer ohnehin nicht gesendet; **entfernen**, sofern sie nicht mit Frames aus Block 1 zusammenwirken (nach den IDs gibt es keine Ueberschneidung; ob Code gemeinsam genutzt wird, beim Bauen pruefen und mit den Tests absichern).
- 0x53B (Uhrzeit): **bleibt**, wird aber **nicht aktiv gesendet**.
- 0x350: **bleibt**, wird aber **nicht aktiv gesendet** auf dem CAN.
- Das entspricht NICHT meiner Variante 2 ("bleiben als feste Sender"), sondern einer dritten: Block 2 entfaellt vollstaendig aus Seite und Senden; **intern bleiben** die Uhr (fuer die Uhrzeit), das Fahrzeugalter und die 0x350-Stufenuhr, aber ohne Frame auf dem Bus.
- **KORREKTUR (Code, 14.10.):** Die frueher hier stehende Aussage, das HV-Modell speise Frames aus Block 1 (0x57F, 0x599, 0x62D), war falsch. 0x57F, 0x599, 0x62D und 0x523 sind Zeilen 35 bis 38 = **Block 2**. Das HV-Modell (`hv_observe_350` `BATTERY.cpp:3463`, `hv_times`, `hv_woke`, `hv_frames_sent`, `twingo::hv_compute` in `LOGIC.h`) wird nur von diesen Block-2-Zeilen benutzt (`BATTERY.cpp:3504`, `3638`, `3688`) und faellt mit ihnen weg. Block 1 braucht es nicht.
- **Was fuer Block 1 und die Abschaltfolge erhalten bleiben muss (Code, 14.10.):** die Stufenuhr der Abschaltfolge (`powerdown_stage`, Zustand 7, Schliessphase, Wake-Buendel), `vehicle_age_available` (Alter fuer `0x436` im Fahrzeugformat, `BATTERY.cpp:3771`, und fuer "Set time now", `:4794`), die Kilometer `odo_426_km`, "Time in 0x436", `send_bus_vehicle_frames` und `send_bus_closing`. Nur das Erzeugen und Senden der 0x350- und 0x53B-Frames faellt weg; `fill_vehicle_age_350` wird nur von den 0x350-Sendern und einer Zeile von Block 2 benutzt.
- **Sicherungsplan fuer Bau 13 (damit kein noetiger Code verloren geht):**
  1. **Vorher** wird ein Referenzlauf aufgezeichnet: alle Frames (ID, Daten, Zeit) eines vollen Zyklus nur mit den Zeilen von Block 1 (wach, Sleep Weg lang, Stille, Wake; auch Ladelog 2 und kurz), aus dem unveraenderten Stand `40d41ac`.
  2. **Nachher** muss derselbe Lauf **Frame fuer Frame identisch** sein (Abweichung = Fehler, Bau wird nicht committet).
  3. Geloescht wird nur, was per Suche (`grep`) **keinen Verwender ausserhalb der entfernten Teile** hat; jede geloeschte Funktion und Variable wird mit ihren Verwendern in der Commit-Beschreibung aufgelistet.
  4. Mehrere kleine Commits (Seite, Zeilen, Logik, Tests), damit einzelne Schritte zurueckgenommen werden koennen; Compiler-Warnungen zu ungenutzten Funktionen helfen, Reste zu finden.
  5. Tests: die Tests fuer Block 1, die Einschlafwege, den Bus und die Abschaltfolge bleiben **unveraendert** und muessen gruen sein; nur Tests der entfernten Zeilen entfallen (mit Liste).
- **0x214** (Zeile 9, Sleep-Anforderungs-Experiment, `send_214_like_car` und Experimentteil in `transmit_can`): **auch entfernen** (Nutzer, 14.10.: "0x214 auch weg").
- **Bestaetigt (Nutzer, 14.10.): "komplett weg, beide Adressen nur intern"**: Block 2 entfaellt vollstaendig aus Seite und Senden; 0x350 und 0x53B bleiben nur intern (kein Frame auf dem CAN). 0x090 und 0x242 sind am Geraet nicht angehakt und koennen ebenfalls weg. Bau 13 ist damit entschieden, wartet nur noch auf "bauen".

## Zum Bauen vorgemerkt (Bau 14): SCPU-Fehlerfeld neben dem MCPU-Fehlerfeld, Reihenfolge der DTC-Knoepfe (Wunsch Nutzer, 14.10.; wartet auf "bauen")
Ausgangslage (Screenshot 14.10., Befund aus dem Code):
- Das Feld "Diagnostic Trouble Codes" (Tabelle mit Code, Status, Beschreibung; "Not read yet - use the Read DTC button below to scan") ist der Abschnitt von Dala (`render_dtc_section_html`, `BatteryHtmlRenderer.h:26`, aufgerufen aus `UdsCanBattery.cpp:516`) und zeigt nur die Fehler der **MCPU**.
- Der untere Knopf "Read DTC SCPU" (`advanced_battery_html.cpp:61`, ruft `read_DTC_scpu()` auf, `BATTERY.cpp:4551`, Anfrage `19 02 <Maske>` an die SCPU) zeigt sein Ergebnis nicht in einem Feld im DTC-Abschnitt; die Antwort geht in das freie Antwortfeld und in die Textzeile "DTC Read SCPU (last result, stays until the next read)" (`dtc_scpu_result`, `BATTERY.cpp:4460`) weiter oben auf der Seite (Nutzer: der Knopf hat nicht funktioniert).
- Deshalb gibt es einen zweiten, eigenen Knopf "Read DTC SCPU" in der Info-Zeile (`twingoScpuDtc`, Route `/triggerTwingoDtcScpu`, `webserver.cpp:1279`); er funktioniert, ist laut Nutzer aber ueberfluessig.
Wunsch:
1. **Zweites Ausgabefeld fuer die SCPU-Fehler** neben dem Feld fuer die MCPU-Fehler (zwei Spalten, Ueberschriften "MCPU" und "SCPU"), in derselben Tabellenform (Code, Status, Beschreibung) wie beim MCPU-Feld; "Not read yet" bis zum ersten Lesen. Dazu muss die Antwort der SCPU (`59 02 FF ...`) in eine zweite Fehlerliste dekodiert und mit demselben Darstellungscode gezeigt werden.
2. **Der untere Knopf "Read DTC SCPU" muss das neue Feld fuellen** (er ist der einzige, der bleiben soll); der zweite Knopf in der Info-Zeile entfaellt, sobald das nachgewiesen ist.
3. **Reihenfolge der Knoepfe in der unteren Reihe:** links zuerst **Read DTC MCPU**, dann **Erase DTC MCPU**; rechts zuerst **Read DTC SCPU**, dann **Erase DTC SCPU** (heute: obere Reihe "... Wake up, Erase DTC MCPU", untere Reihe "Erase DTC SCPU, Read DTC MCPU, Read DTC SCPU"). Die Reihenfolge ergibt sich heute aus der Befehlstabelle (`battery_commands`, `advanced_battery_html.cpp:30-90`); die Knoepfe fuer DTC werden als eigene Gruppe in der unteren Reihe angeordnet.
- **Entfernen, in Bau 14 aufgenommen (Nutzer, 14.10.: "kann beides entfernt werden, inklusive dem Source dahinter"; zwei Screenshots):**
  1. **Info-Zeile "Read DTC SCPU" mit "DTC Read SCPU (last result, stays until the next read): ..."**: Knopf `twingoScpuDtc()` (JavaScript `BATTERY.cpp:4408-4411`), Route `/triggerTwingoDtcScpu` (`webserver.cpp:1279`), Textzeile mit `dtc_scpu_result` (`BATTERY.cpp:4460-4461`). Das Ergebnis des unteren Knopfes geht dann in das neue SCPU-Feld; die Hilfsvariablen `dtc_scpu_result`, `uq_to_scpu_row`, `uq_scpu_row_pending`, `dtc_scpu_query_result()` (`BATTERY.cpp:4555-4561`, `4684-4685`, `4935-4938`, `5238-5241`, `BATTERY.h:232, 968-970`) werden fuer das neue Feld umgebaut oder entfernt. Reihenfolge: **erst** das neue Feld, dann diese Zeile (sonst hat der untere Knopf keine sichtbare Ausgabe).
  2. **"Read DTC details (EXPERIMENTAL, UNTESTED, subfunction 0x06)" mit den Zeilen "DTC details - e14381 / 1b0715: ..."**: Knopf und Zeilen (`BATTERY.cpp:4464-4467`), Route `/triggerTwingoDtcDetails` (`webserver.cpp:1223-1228`), `read_DTC_details()` (`BATTERY.cpp:4569`, `BATTERY.h:111`), Zustaende `DTC_EXT_DETAILS_*` und der Teil in `handle_dtc_ext` (`BATTERY.cpp:5193-5260`), `DTC_DETAILS_CODES`, `dtc_ext_log_details`, `dtc_ext_details_index` (`BATTERY.h:910-933`, `BATTERY.cpp:4549`, `4840-4841`, `4951-4952`), die zugehoerigen Tests.
  3. **Das Feld "0x155 (B7..B0): ..." (Anzeige der letzten 0x155-Bytes, Byte 7 bis 0 dezimal)** steht im Screenshot in derselben Zeile wie der DTC-details-Knopf (`rx_155_text()`, `BATTERY.cpp:3914`, Route `/twingoRx155`, JavaScript `:4418-4419`, Span `twingoRx155` `:4465`, `rx_155_last`, `rx_155_seen`). **Entscheidung des Nutzers (14.10.): "wenn wir aus den Daten keine weiteren Erkenntnisse gewinnen konnten oder koennen, kann es weg; ich weiss nicht, ob da wirklich ein Status ueber 0x9281 kommt".** Stand (Nutzer, 14.10.: "wir belassen das 0x155 mal, koennen es spaeter entfernen"): **das Feld bleibt**; Entfernen spaeter, nach dem Test (MERKLISTE_Test, "0x155 und 0x9281"). In Bau 14 verschwindet nur der Knopf "Read DTC details" daneben; die Zeile "0x155 (B7..B0): ..." bleibt allein stehen. Die Zeile "0x155 frames dropped as invalid" ist ein anderes Feld und bleibt.
  - Tests, die diese Teile pruefen (`TwingoBuildList0810Test.cpp`, `TwingoSimulatorTest.cpp`, `TwingoTxStatTest.cpp`), werden angepasst oder entfernt.
- **Skizze des unteren Seitenteils nach Bau 14 (Entwurf, 14.10.):**
```
 ... (Freie Anfrage, Antwort-Feld, Read DTC fault counters 19 14 - wie heute) ...
 NVROL Log ... (wie heute)
 [ ] Read ALL DTC statuses (mask 0xFF) ...
 DTC Read (ext. protocol, Read DTC button): ...        <- bleibt
 DTC Erase (ext. protocol, Erase DTC button): ...      <- bleibt
 [Reset] Own send intervals 0x426 / 0x435 / 0x436 ...
   (Messwerte)
 0x155 (B7..B0): 3 131 194 126 84 208 135 42           <- bleibt, ohne den DTC-details-Knopf
 > Sleep run details                                    (eingeklappt nach dem Lauf)
 ---------------------------------------------------------------
 Diagnostic Trouble Codes
 +--------------------------+  +--------------------------+
 | MCPU                     |  | SCPU                     |
 | Not read yet - use       |  | Not read yet - use       |
 | Read DTC MCPU below      |  | Read DTC SCPU below      |
 | (Tabelle: DTC / Status / |  | (Tabelle: DTC / Status / |
 |  Description)            |  |  Description)            |
 +--------------------------+  +--------------------------+
 [Perform NVROL reset] [Sleep] [Sleep 0x9281=1] [Wake up]
 [Read DTC MCPU] [Erase DTC MCPU]    [Read DTC SCPU] [Erase DTC SCPU]
```
- **Wegfall gegenueber heute:** Info-Zeile "Read DTC SCPU / DTC Read SCPU (last result)"; "Read DTC details" samt "DTC details - e14381 / 1b0715"; die Knoepfe "Erase DTC MCPU" in der oberen und "Erase DTC SCPU / Read DTC MCPU / Read DTC SCPU" in der unteren Knopfreihe wandern in die neue Reihe.
- **Rueckfrage (Nutzer, 14.10.: "den Read DTC hast du mir auch mehrfach gebaut ohne Grund"):** Heute gibt es mehrere Wege, die Fehler zu lesen: (a) Knopf "Read DTC MCPU" unten (fuellt das MCPU-Feld, Log-Zeile "DTC Read (ext. protocol, ...)"), (b) Knopf "Read DTC SCPU" unten (ohne Feld), (c) Info-Zeile "Read DTC SCPU" (entfaellt), (d) Schnellknoepfe "DTC 19 02 09 MCPU" und "DTC 19 02 09 SCPU" in der Freien Anfrage (Rohantwort im Feld "Answer"), (e) "Read DTC fault counters (19 14)", (f) "Read DTC details" (entfaellt). Nach Bau 14 bleiben (a) und (b) als die Knoepfe mit Feld; **Entschieden (Nutzer, 14.10.: "alles andere auch die freie Anfrage soll verbleiben, die war schon sehr nuetzlich"):** (d) die Schnellknoepfe und die ganze Freie Anfrage, (e) "Read DTC fault counters (19 14)" und die zwei Log-Zeilen "DTC Read / DTC Erase" **bleiben unveraendert**. Entfernt werden in Bau 14 nur (c) die Info-Zeile "Read DTC SCPU" und (f) "Read DTC details"; das 0x155-Feld bleibt (Entfernen spaeter). Die Skizze oben gilt mit diesen Zeilen "<- Kandidat" als **bleibt**.
- Offen beim Bauen: Beschreibungstexte der SCPU-Codes (Dala-JSON hat sie evtl. nicht, dann "Unknown"); ob die Textzeile "DTC Read SCPU (last result)" bleibt oder durch das neue Feld ersetzt wird.

## Offen, nicht entschieden oder abhaengig vom Bench
- **0x426 Byte 0 bis 7 bitweise einstellbar** (nicht gewuenscht in der Form "alle Bytes"; der Wunsch vom 11.10. ist Punkt 1).
- **km steigt beim Fahren von selbst** (z. B. +1 km pro Minute im Zustand "driving"): nur nach Rate aus dem Auto; heute fest.
- **Zeitquelle der MCPU-Missionen** (0x350, 0x436, 0x523 oder 0x376) per Test mit weit auseinanderliegenden Altern (1.400.000 und 1.500.000) klaeren, danach evtl. nur die richtige Quelle senden.
- **Pack-km auf 6.844 bringen:** von Hand per km-Feld (aktuell 13 km; +6.831 setzen), kein Bau noetig.
- **Pack-Zeit** (`9261`/`91C1` am MCPU) bleibt 0; Missionen tragen Zeitstempel. Test: lange Laufzeit im Fahrmodus (10, 30, 60 min). **Stand 12.10.:** MCPU-`9261` weiter 0 (auch nach +1000, `9281` und Sleep); SCPU-`9261` = 1.360.946 min (folgt 0x436); `9259` = 05, `925C` = 01; `925E` = `13 88 6F` (= 0x69F B1-B3); `925B` = `80 10` (Software Reset und Irregular Reset). Speichern am Sitzungsende (Auto) am Bench nie gesehen; `91C1` am 12.10. nicht gelesen. Naechster Test: Neustart (12 V), Sitzung mit allen 17 EVC-Zeilen, Auto-Endfolge, danach `9261`/`91C1` lesen (`9261 - 91C1 = 784.382` im Auto). Details: `TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`, Abschnitt "Ergebnis Bench 12.10.".
- **E14381** mit `0x426` an (Test B: `0x157` und weitere EVC-Zeilen einzeln zuschalten). **Stand 12.10.:** Nach dem Anheben der Zeit in 0x436 um +1000 ging E14381 von `2F` auf `2E` (nicht mehr aktiv) und liess sich quittieren; alle bisherigen Eingriffe (alle Zeilen, Pruefsummen, Zustaende, km) hatten ihn auf `2F` gelassen. Basis- und Zielwert des Tests nicht festgehalten. Schwelle (ab welchem Wert `2E`) offen.
- **`1B0E41`** (Speicherfehler MCPU, 6926 "External NV memory", Test 65 "general checksum failure") aktiv `2F`: **Stand 12.10.:** `19 02 09` an die MCPU zeigte zuerst keine Fehler, in einer spaeteren Lesung wieder `1B0E41 = 2F`. Ob er nach `14 FF FF FF` sofort wiederkommt und ob er das Speichern von `9261`/`91C1` blockiert, ist offen (Hypothese, nicht belegt). SCPU am 12.10.: `1BB587 = 2F` (Stromsensor, fehlende Frames) aktiv, `1BB108 = 2E` (Slave-Kommunikation) zeitweise.

## Aufraeumen (spaeter)
- Schlanke Fassung fuer Dalas Repo, Streichliste und Abhaengigkeiten: siehe `notes/AUFRAEUMLISTE.md` (nichts gebaut, Nutzer macht spaeter weiter).

## Offen beim Nutzer
- **Bench-Test** in Schritten (Plan: `TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`): erst nur `0x423`, dann die uebrigen BMS<>EVC-Frames nach und nach; `0x350` und die anderen Fahrzeug-CAN-Zeilen erst als Zusatz. Logger ohne Filter (RX und TX).
- Pin-Belegung (CAN H/L) am LBC-Stecker, damit die Aderfarben aus dem Schaltplan zugeordnet werden koennen.


### Bau 13 - Stand nach dem Bauen (14.10.)
- Tabelle 30 Zeilen (SIM_ROW_*), Standardmaske 0x10 (nur 0x69F). NVM-Schluessel TWINGOSIMB1 migriert die alten 71-Zeilen-Schalter (twingo::migrate_old_row_mask).
- Rest-Aufraeumen (AUFRAEUMLISTE): end_stage/SIM_END, 128-Bit-Maskenmaschinerie (sim_row_*_hi), 0x350-Wake-Byte-Spalten.
- Als naechstes (nur auf "bauen"): Bau 14, dann Bau 12.


### Aenderungen vor dem Bauen (Nutzer, 14.10.; noch nichts gebaut)
- **Bau 14 zusaetzlich: "Read DTC fault counters (19 14)" mit "not run yet" entfaellt** (Zeile `BATTERY.cpp:3305`, JavaScript `twingoFdc` `:3368`, `read_DTC_fdc()` `:3732` / `.h:235`, `fdc_query_result()` `.h:227`, `fdc_result`, Modus `UQ_FDC` in Sammler, `which=fdc` in der Route `webserver.cpp:1249-1256`, Route `:1281-1283`, zugehoerige Tests). Der Eintrag (e) der Rueckfrage oben ("bleibt") ist damit ueberholt. Freie Anfrage und die zwei Log-Zeilen DTC Read / DTC Erase bleiben.
- **Bau 12 geaendert: eine gemeinsame Zeit fuer Knopf und Taster.** Das Feld heisst kuenftig **"Sleep failsafe manual / push button hardware (IO15) --> auto wake after: [ ] min [Set]"** (heute "Sleep failsafe manual (auto wake after)", `BATTERY.cpp:3220`, NVM `sleep_failsafe_minutes`). Der Taster an IO15 nutzt diese Zeit fuer die Stille. Damit entfaellt die feste Dauer von 15 Minuten (Entscheidung 3) und der geplante Merker fuer den Taster-Lauf in `silence_limit_ms()`: ohne Daily-sleep-Merker liefert `silence_limit_ms()` schon `sleep_manual_failsafe_ms()`. Der Rest (langer Druck 2 s, Einschlafweg lang, 0x9281=1, kein Wecken per Taster) bleibt. Offene Frage entfaellt ebenfalls (Feld ist ja einstellbar).

## Zum Bauen vorgemerkt (Bau 15, ohne funktionelle Aenderung): Steuerfelder der /simulator-Seite auf die Advanced-Seite (Wunsch Nutzer, 14.10.; Screenshots mit Pfeil und Kasten; Umfang noch zu bestaetigen)
- **Ziel:** auf der Advanced-Seite (Twingo-Teil, `BATTERY.cpp` um `:3220-3260`) **unter "Age minute factor" und ueber "Cellwatch"** (roter Kasten im Screenshot).
- **Gemeint nach Screenshot 1 (Vorschlag):** der Steuerblock am Anfang der /simulator-Seite (`simulator_html.cpp:136-310`): Format der BMS<->EVC-Frames (busfmt0/1), Betriebsart, Einschlafweg, Zustand + manuell (busstate, busmanual), "0x69F gesendet" (tx69f), Zellwerte-Quelle (cellsrc0/1), Shutdown and wake-up (carmode0/1), Vehicle age, Manual vehicle age (ageManual, Set/Off), Time in 0x436 (t436, Set/Off), "0x436 carries the vehicle age" (t436follow), Kilometer 0x426 + Byte 7 (odo426, odo426b7) samt den JavaScript-Funktionen und Routen. Gleiche Elemente, gleiche Routen, keine neue Funktion.
- **Zu klaeren:** bleibt die Tabelle "Block 1" (30 Zeilen) mit "Weitere empfangene IDs" auf der /simulator-Seite (Vorschlag, der Knopf "Open CAN Signal Simulator page" bleibt), oder zieht die ganze Seite um?
- Beim Bau: Element-IDs und Routen unveraendert lassen; Seiten-Tests (`TwingoBusModeTest.cpp` Page-Tests) auf den neuen Ort anpassen; Golden-Lauf (gesendete Frames) muss identisch bleiben.

### Bau 15 - Umfang bestaetigt (Nutzer, 14.10., zwei Screenshots mit rotem Rahmen)
- **Umzug auf die Advanced-Seite** (unter "Age minute factor", ueber "Cellwatch"): der gesamte rot umrahmte Steuerblock (Format, Betriebsart, Einschlafweg, Zustand/manuell, "0x69F gesendet", Zellwerte-Quelle, Shutdown and wake-up, Vehicle age, Manual vehicle age, Time in 0x436, "0x436 carries the vehicle age") **und das Feld "0x426: km, byte 7 [Set]"** (odo426, odo426b7, odoState).
- **Bleibt auf /simulator (Rueckkehr zur ersten Auslegung, "STOPP"): Block 1 mit den 30 Zeilen, "Weitere empfangene IDs", Zeitleiste (Jetzt / Uhr / Letzte Aktion) und die Erklaertexte.**
- **Doppelte Knoepfe:** "Alle Zeilen aus" / "Zeilen wie vorher" (oben, `simulator_html.cpp:141-145`) und "All rows off at once" / "Rows back as before" (unten, `:284-285`) rufen dieselben Routen `/twingoSimAllOff` und `/twingoSimRestore` (`webserver.cpp:975-981`) und damit dieselben Funktionen `sim_all_off()` / `sim_restore()`: identisch. **Es bleiben nur die englischen**; sie ziehen mit dem Block auf die Advanced-Seite, die deutschen entfallen. Die Zeitleiste "T+0 = letzter Druck" haengt serverseitig an `sim_all_off()`, nicht am Knopf; der Hinweistext zu "Alle Zeilen aus" wird entsprechend umformuliert.
- Beim Bau: Element-IDs und Routen unveraendert, Golden-Lauf muss identisch bleiben, Seiten-Tests anpassen. Der Hinweis "Reload the page to see the checkboxes" bleibt sinngemaess (die Haken stehen auf /simulator).
- **Korrektur (Nutzer, 14.10.): die englischen Knoepfe "All rows off at once" / "Rows back as before" bleiben auf der /simulator-Seite** (zusammen mit ihrem Hinweistext "Reload the page to see the checkboxes"), sie ziehen NICHT mit. Die deutschen "Alle Zeilen aus" / "Zeilen wie vorher" entfallen. Auf die Advanced-Seite zieht nur der Steuerblock und das 0x426-Feld.
- **Bestaetigt (Nutzer, 14.10.): Ueberschrift "CAN Signal Simulator" und die beiden Erklaertexte (Block 1 sendet ... / LED: ... Typ: ...) bleiben auf /simulator** (`simulator_html.cpp:125-135`), ebenso die Zeitleiste.


## GEBAUT am 14.10. (Bau 14, 15, 12; Branch twingo-bau7-schlaf-anforderung, Commits 86e3a29, 9d9d7d0, 8ec26e3; 445 Tests gruen; Golden-Lauf byte-identisch; Bench-Test offen; ESP-Build nicht auf diesem Rechner geprueft)
- **Bau 14 (86e3a29):** DTC-Abschnitt mit Spalten MCPU / SCPU (`render_dtc_columns_html`, `get_dtc_second`); "Read DTC SCPU" fuellt die SCPU-Liste (laedt die Seite danach neu); untere Knopfreihe Read/Erase MCPU | Read/Erase SCPU. Entfernt: Info-Zeile "Read DTC SCPU", "Read DTC details" samt Zeilen, "Read DTC fault counters (19 14)" (mit Routen, Funktionen, Zustaenden, Tests). 0x155-Feld, Freie Anfrage, Log-Zeilen DTC Read/Erase bleiben.
- **Bau 15 (9d9d7d0):** Steuerblock und 0x426-Feld nach `simulator_controls_html()` im Twingo-Teil der Advanced-Seite (zwischen "Age minute factor" und "Cellwatch"). /simulator behaelt Ueberschrift, Erklaertexte, Zeitleiste, Block 1 mit 30 Zeilen, Weitere IDs und die englischen Knoepfe. Deutsche Knoepfe entfallen.
- **Bau 12 (8ec26e3):** Taster GPIO15 (langer Druck 2 s, Sleep 0x9281=1, langer Weg, Stille = Zeit des Feldes "Sleep failsafe manual / push button hardware (IO15) --> auto wake after"), Checkbox "push button Sleep" (NVM TWSLPBTN, aus), Display-/Seiten-Gesamtzeit = `silence_limit_ms()`, Umbruch der 10-s-Zeile, "Result" ignoriert das Wake-Buendel.
- **Bench-Test fuer den Nutzer:** (1) Advanced-Seite: SCPU-Feld nach "Read DTC SCPU", Knopfreihenfolge; (2) /simulator ohne Steuerblock, Advanced mit Steuerblock, Haken/Felder wirken; (3) Taster: Checkbox an, Taster zwischen GPIO15 und GND, 2 s halten -> Display "Sleep requested", Stille = Failsafe-Minuten; kurzer Druck tut nichts; Druck in der Stille tut nichts; (4) Daily sleep: Display zeigt die Dauer des Daily sleep; (5) Stille-Seite nach langem Lauf: 10-s-Zeile umgebrochen, Result sinnvoll.

## Zum Bauen vorgemerkt (Bau 16, spaeter; Wunsch Nutzer, 14.10.; wartet auf "bauen"): Klartext fuer die Thermistoren T1 und T2 in der Freien Anfrage
- **Ausgangslage:** Die Freie Anfrage zeigt nur die Zeiten (9261, 91C1), die Kilometer (925F, 9262) und die Fehlerlisten (19 02) im Klartext (`uq_reply_complete`, `BATTERY.cpp`, Block "Known identifiers shown in plain words"). `22 91 CD` (T1 "Cooling temperature", Roh `22 91 73`) und `22 92 1C` (T2 "Skin temperature", Roh `22 91 74`) kommen nur als Hex.
- **Gewuenscht:** beide Werte im Klartext anhaengen, z.B. `| 18.75 C`, an dieselbe Stelle wie die Zeiten und Kilometer. Gesendete Frames bleiben unberuehrt.
- **Skala (unbestaetigt):** Rohwert / 16 - 40 = Grad C (wie die Zellsensoren 9131..9138, `BATTERY.cpp` Abschnitt "Pack temperature sensor 1-8"). Bench 14.10.: 3,3 kOhm am Stecker B Pin 8 - 12, `22 91 CD` = `03 AC` = 940 -> 18,75 C; aus den Messpunkten (15k=-12,1 / 12k=-8,0 / 6,67k=+3,0 / 4,29k=+13,0 C) hochgerechnet ca. 19 C, passt. `22 91 73` = `04 BB` = 1211, Einheit unklar (nicht mV: im Spannungsteiler waeren ca. 2,3 V zu erwarten).
- **Test zur Bestaetigung (Nutzer):** 4,29 kOhm einsetzen: `22 91 CD` muss 848 liefern (= +13,0 C). Dann ist die Skala belegt und der Hinweis "unbestaetigt" im Text entfaellt. Bis dahin Text mit Zusatz "(Skala wie die Zellsensoren, unbestaetigt)".
- Test: Antwort `62 91 CD 03 AC` -> Text enthaelt `18.75`; Randwerte (0, 0xFFFF) wie bei den Zellsensoren (kein gueltiger Wert) ohne Grad.

## Zum Bauen vorgemerkt (Bau 17, Wunsch Nutzer, 14.10.; wartet auf "bauen"): Zeilen-Standard als Bools im Parameterblock, Bauschalter TWINGO_SIM_PAGE, Restschalter TWINGO_FAST_VEHICLE_FRAMES entfernen
**Ziel:** Welche Frames der Emulator sendet, laesst sich auch ohne /simulator-Seite im Code vorgeben; ist die Seite da, haben ihre (im Flash gespeicherten) Einstellungen Vorrang.
**Parameterblock** ganz oben in `RENAULT-TWINGO-GEN1-BATTERY.h` (nach den Includes, vor der Klasse), in dieser Reihenfolge:
1. `#define TWINGO_SIM_PAGE` (**Voreinstellung: AUS, also auskommentiert**). Muss ein `#define` sein wie die anderen Bauschalter, damit Seite und Routen per `#ifdef` ganz aus dem Programm fallen.
2. die vorhandenen Bauschalter `TWINGO_EXTENDED_CELL_POLLING`, `TWINGO_TIME_FRAMES`.
3. je Zeile eine `constexpr bool SIM_ON_<ID>` (Reihenfolge wie die Tabelle). **Voreinstellung true (Nutzer, 14.10.):** 0x157, 0x0EC, 0x0ED, 0x19F, 0x426, 0x436, 0x423, 0x419, 0x428, 0x42F, 0x435, 0x69F; **alle anderen Zeilen false** (0x155, 0x0C5, 0x1C9, 0x1A1, 0x1C7, 0x424, 0x425, 0x43A, 0x445, 0x464, 0x4F7, 0x500, 0x511, 0x588, 0x6BE, 0x4AE, 0x4AF, 0x659).
4. `SIM_DEFAULT_MASK` aus den Bools zusammengesetzt. Zeilenindizes (Tabelle `sim_signals`): 0x19F=0, 0x426=1, 0x436=2, 0x423=3, 0x69F=4, 0x157=8, 0x0EC=11, 0x0ED=12, 0x419=13, 0x428=16, 0x42F=17, 0x435=18 -> Maske **0x0007391F** (heute 0x00000010, `datalayer_extended.h:1152`).
**Regeln:**
- `TWINGO_SIM_PAGE` an: die Bools sind nur der Standard (erster Start, kein Schluessel `TWINGOSIMB1` im Flash). Die Seite schreibt die Maske ins Flash; beim Start gewinnt der Flash-Wert. Dafuer setzt der Lader ein Merkflag `sim_mask_from_nvm` (Datalayer), der Treiber setzt im `setup()` die Maske auf `SIM_DEFAULT_MASK`, wenn das Flag fehlt. `datalayer_extended.h` bindet den Twingo-Header nicht ein.
- `TWINGO_SIM_PAGE` aus (Voreinstellung): Seite `/simulator`, Routen `/editTwingoSimSignal`, `/twingoSimAllOff`, `/twingoSimRestore`, Empfangsverfolgung (`rx_*`), Knopf "Open CAN Signal Simulator page" werden nicht gebaut; die Maske kommt IMMER aus `SIM_DEFAULT_MASK` (ein alter Flash-Wert wird ignoriert, aber nicht geloescht; mit Schalter an ist er wieder da).
- Der Steuerblock auf der Advanced-Seite (`simulator_controls_html()`, Bau 15) und seine Routen bleiben unabhaengig von `TWINGO_SIM_PAGE` (Teil der Advanced-Seite); dazu den Block in `simulator_html.cpp` ausserhalb des `#ifdef` lassen (oder in den Treiber verschieben). `/twingoRxStatus` bleibt (Steuerblock fragt sie ab) oder die Abfrage wird ersetzt - beim Bauen entscheiden.
- Die uebrigen Seiten-Einstellungen (Format, Zustand, Weg, Betriebsart, Zellwerte-Quelle) behalten ihre Standards im Datalayer und ihren Flash-Schluessel.
**Entfernen im selben Bau (Nutzer, 14.10.):** `#define TWINGO_FAST_VEHICLE_FRAMES` (`BATTERY.h:57` samt Kommentarblock davor) und die eine Verwendung in `BATTERY.cpp`; 0x090/0x242 gibt es seit Bau 13 nicht mehr.
**Tests:** Standardmaske nur aus den Bools (0x0007391F); Flash-Wert schlaegt den Standard; Build ohne `TWINGO_SIM_PAGE` ignoriert den Flash; gesendete Frames bei Standardmaske = die zwoelf IDs; Golden-Lauf mit der alten Maske identisch (gleiche Maske gesetzt). Beide Zustaende des Schalters muessen uebersetzen (Testbuild mit und ohne).
**Folge fuer das Geraet des Nutzers:** Mit dem Standard AUS verschwindet die Seite /simulator nach dem Flashen und seine gespeicherte Maske wird ignoriert (die zwoelf Frames sind dann an, alle anderen aus); zum Weiterarbeiten mit der Seite `#define TWINGO_SIM_PAGE` einschalten.

## Zum Bauen vorgemerkt (Bau 18, Wunsch Nutzer, 14.10.; wartet auf "bauen"): Eingabefeld "Fahrzeug-ID" (0x69F) auf der Advanced-Seite
**Hintergrund (aus dem Code):** Die Fahrzeug-ID `46 13 88 6F` steht fest in `ZOE_69F_BCM_GATEWAY` (`BATTERY.h:350-354`, 4 Datenbytes, Bytes 1-3 = die ID, die der LBC als $925E speichert). Gesendet wird sie an genau zwei Stellen: `BATTERY.cpp:3043` (Abschaltfolge) und `:3130` (Sekundentakt) ueber `transmit_can_frame(&ZOE_69F_BCM_GATEWAY)`. Die Tabelle in `BUS.h:526-541` (0x69F, 4 Bytes) wird fuer 0x69F nicht gesendet (Zeile "I"), nur der Wert dort ist gleich; sie bleibt unveraendert oder wird beim Bau auf den Parameter umgestellt.
**Wunsch:**
- Neue Zeile auf der Advanced-Seite **zwischen "B009 diagnostic session" und "Sleep failsafe manual / push button hardware (IO15) ..."** (Position laut Screenshot).
- **Eingabefeld** fuer die 4 Bytes (Schreibweise `46 13 88 6F`, Leerzeichen tolerant, 8 Hexziffern), **Checkbox davor**, **Set-Knopf dahinter**, Wert **im NVM**.
- **Standard = die eigene ID des Nutzers (`46 13 88 6F`)**, als benannte Konstante im Parameterblock von `BATTERY.h`, damit sie bei einem anderen Fahrzeug an einer Stelle zu aendern ist.
- Checkbox als Sicherung: die ID wird nicht versehentlich geaendert; das Feld ist nur bei gesetzter Checkbox bearbeitbar (**Auslegung noch vom Nutzer zu bestaetigen**), Set speichert den eingegebenen Wert im NVM und setzt ihn sofort in `ZOE_69F_BCM_GATEWAY.data[0..3]`. Checkbox aus = die Standard-ID wird gesendet (der gespeicherte Wert bleibt im NVM, bei erneutem Einschalten ist er wieder da).
**Technik:** Datalayer `uint8_t vehicle_id[4]`, `bool vehicle_id_custom`; NVM-Schluessel (z.B. `TWVEHID` als uint32, `TWVEHIDEN` als bool); Route `/editTwingoVehicleId?en=..&id=XXXXXXXX` mit Pruefung (genau 8 Hexziffern, 0x00000000 und 0xFFFFFFFF abweisen); Treiber uebernimmt die ID in `setup()` und nach jedem Set in `ZOE_69F_BCM_GATEWAY.data`. Anzeige "0x69F gesendet ... Fahrzeug-ID" im Steuerblock (Bau 15) soll die aktuelle ID zeigen statt der festen Zeichenkette (`simulator_html.cpp:153`).
**Tests:** Standard sendet `46 13 88 6F` (Test `TwingoCarMode.VehicleIdIsTheOneOfTheUsersCarInBothModes` bleibt gruen); mit Checkbox und eigener ID sendet 0x69F die eigene; Checkbox aus -> Standard; ungueltige Eingaben (zu kurz, kein Hex, 00000000) werden abgewiesen; NVM-Round-trip (Laden setzt Feld und Frame); Seite zeigt die Zeile an der richtigen Stelle (nach B009, vor Failsafe); Golden-Lauf mit Standard-ID byte-identisch.
- **Bau 18 - Auslegung der Checkbox bestaetigt (Nutzer, 14.10.): Schalter "eigene ID benutzen", Set-Knopf dabei.** Das Feld ist immer bearbeitbar (kein Sperren). Ablauf: ID eintippen, **Set** speichert sie im NVM, die **Checkbox** schaltet um: an = die eingetippte/gespeicherte ID wird gesendet, aus = die Standard-ID aus dem Parameterblock. Die gespeicherte ID bleibt bei "aus" im NVM. Die Auslegung "Feld nur bei gesetzter Checkbox bearbeitbar" oben entfaellt.

## GEBAUT am 14.10. (Bau 16, 18, 17; Branch twingo-bau7-schlaf-anforderung; 457 Tests mit Seite / 320 ohne Seite gruen; Golden-Lauf in beiden Varianten byte-identisch; ESP-Build und Bench-Test offen)
- **Bau 16:** `22 91 CD` / `22 92 1C` zeigen `| 18.75 C (scale like the cell sensors, unconfirmed)`; Test mit 4,29 kOhm: `91 CD` muss 848 liefern.
- **Bau 18:** Feld "use own vehicle ID (0x69F)" zwischen B009 und Failsafe, Set, Checkbox, NVM `TWVEHID` / `TWVEHIDEN`, Standard `TWINGO_VEHICLE_ID_DEFAULT` (46 13 88 6F); ungueltig: kein Hex, nicht 8 Ziffern, 00000000, FFFFFFFF.
- **Bau 17:** Parameterblock am Kopf von `BATTERY.h`: `// #define TWINGO_SIM_PAGE` (aus), `SIM_ON_*` (zwoelf true), `SIM_DEFAULT_MASK` = 0x0007391F. Ohne Seite: Maske immer aus dem Parameterblock, Flash ignoriert (nicht geloescht). `TWINGO_FAST_VEHICLE_FRAMES` entfernt - Berichtigung: der Schalter umschloss auch `send_simulator_signals()` (die 25 Busspalten), der Aufruf bleibt jetzt immer aktiv. Die Empfangsverfolgung (`rx_*`, `/twingoRxStatus`) bleibt auch ohne Seite (der Steuerblock fragt sie ab); entgegen der frueheren Aussage faellt sie bei ausgeschalteter Seite NICHT weg.
- **Zum Weiterarbeiten mit der Seite:** `#define TWINGO_SIM_PAGE` einschalten (oder `-DTWINGO_SIM_PAGE` in den Build-Flags).
- **Test mit/ohne Seite lokal:** `cmake -DTWINGO_SIM_PAGE=ON|OFF` (Option in `test/CMakeLists.txt`).

## Zum Bauen vorgemerkt (Bau 19, klein, Wunsch Nutzer, 14.10.; wartet auf "bauen"): Beschriftung der Antwort auf "Erase DTC" (positive Antwort 54)
- **Befund (Bench 14.10.):** "DTC Erase (ext. protocol, Erase DTC button): short reply (1 bytes)" obwohl das Loeschen funktioniert hat ("DTC Read: OK, 0 DTC(s)", der Fehler 1B03FD / Status 2E war danach weg).
- **Ursache (aus dem Code):** `handle_dtc_ext_reply` (`BATTERY.cpp:4279-4296`) beschriftet jede Antwort mit PCI < 3 pauschal als "short reply (%u bytes)". Die positive Antwort auf `14 FF FF FF` ist das einzelne Byte `54` (0x14 + 0x40, steht auch im Kommentar des Codes); das Byte wird aber nicht gelesen. In der Freien Anfrage ist es schon erkannt (`:3992`, `p[0] == 0x54`).
- **Aenderung:** bei PCI == 1 und Byte 1 == 0x54 den Text "OK (positive response 54)" setzen; PCI == 1 mit anderem Byte: "short reply (1 byte: XX)" mit dem Byte; PCI == 2: beide Bytes zeigen. Negative Antwort `7F 14 NRC` wie bisher ("NEGATIVE SID=.. NRC=..", PCI 3). Nur Text, gesendete Frames unveraendert.
- **Tests:** Antwort `01 54` -> "OK (positive response 54)"; `01 AA` -> zeigt AA; `03 7F 14 22` -> NEGATIVE; Zustand geht danach auf IDLE und die Poll-Vorlage wird zurueckgesetzt (bestehendes Verhalten).

## Zum Bauen vorgemerkt (Bau 20, Wunsch Nutzer, 14.10.; wartet auf "bauen"): Cellwatch (schnelle Einzelzellabfrage) wie die Simulator-Seite abschaltbar, Voreinstellung AUS
**Ziel:** Cellwatch bekommt einen Bauschalter `#define TWINGO_CELLWATCH` im Parameterblock von `RENAULT-TWINGO-GEN1-BATTERY.h`, **Voreinstellung AUS (auskommentiert)**, gleiche Mechanik wie `TWINGO_SIM_PAGE` (Bau 17). Ohne den Schalter verschwindet alles davon aus Seite, Routen und Treiber; mit dem Schalter (oder `-DTWINGO_CELLWATCH`) ist es wie heute.
**Was bei AUS nicht gebaut wird (Stand des Codes, 14.10.):**
- **Advanced-Seite:** die ganze Zeile "Cellwatch (fast single-cell poll): [ ] enable, cell [1] [Set] [Open Cellwatch page] - pauses the normal cell round-robin while enabled" (`BATTERY.cpp:3390-3398`) und die JavaScript-Funktionen `twingoSetCellwatchEnable` (`:3484`) und `twingoSetCellwatchCell` (`:3511`).
- **Seite und Routen in `webserver.cpp`:** `/cellwatch` (Seite, `cellwatch_processor`, `:960-962`), `/cellwatchStatus` (`:951`), `/editTwingoCellwatchEnable` (`:919`), `/editTwingoCellwatchCell` (`:934`) und `#include "cellwatch_html.h"` (`:51`).
- **Seitendateien:** `cellwatch_html.cpp` (71 Zeilen) und `cellwatch_html.h` (17 Zeilen), Inhalt per `#ifdef` (Dateien selbst bleiben stehen, beide Zustaende uebersetzen).
- **Treiber:** der Cellwatch-Zweig der Abfrageschleife (`:3186-3211`, eigener Takt `previousMillisCellwatch`, `CELLWATCH_MIN_GAP_MS` = 50 ms, Log "EXT UDS TX (cellwatch)") und das Mitschreiben des Messwerts in `handle_pid` (`:259-263`: `cellwatch_last_mV`, `cellwatch_sample_count`, `cellwatch_last_sample_ms`); Member in `BATTERY.h:654-659`.
**Was bleibt (nicht anfassen):** die normale Rundabfrage der Zellen, das Anhalten der Abfrage waehrend eines DTC-Austauschs (`dtc_ext_state != IDLE`, `:3177-3183`) - der Cellwatch-Zweig ist ein `else if` davor/dahinter und muss beim Entfernen die Struktur der Schleife erhalten; die Datalayer-Felder `cellwatch_*` in `datalayer_extended.h:248-256` und `:1154` (Dalas Datei, Struktur nicht aendern) bleiben stehen, auch bei AUS.
**Vorsicht beim Bau:** der Cellwatch-Zweig steht in einer `if / else if / else`-Kette der Abfrage (`:3177-3241`); bei AUS muss die Kette ohne den Zweig dasselbe tun wie heute bei `cellwatch_enabled == false` (Golden-Lauf!).
**Tests:** beide Zustaende uebersetzen (CMake-Option wie bei der Seite, z.B. `TWINGO_CELLWATCH` ON/OFF); AUS: Advanced-Seite enthaelt weder "Cellwatch" noch `/cellwatch`; EIN: Zeile und Knopf wie vorher; Abfrage der Zellen unveraendert (Golden-Lauf in beiden Zustaenden byte-identisch bei `cellwatch_enabled = false`); vorhandener Test in `TwingoBusModeTest.cpp` (Erwaehnung von Cellwatch) anpassen. `cellwatch_html.cpp` wird im Testbuild nicht uebersetzt (steht nicht in der Quellenliste der Tests).
**Zu beachten (fuer den Nutzer):** Wer Cellwatch am Geraet benutzt, muss danach `#define TWINGO_CELLWATCH` einschalten.

## GEBAUT am 14.10. (Bau 19 und 20; Branch twingo-bau7-schlaf-anforderung; Tests 464 / 327 / 464 gruen; Golden-Lauf in allen drei Konfigurationen byte-identisch; ESP-Build und Bench-Test offen)
- **Bau 19:** Antwort auf "Erase DTC": `01 54` -> "OK (positive response 54)"; anderes Einzelbyte -> "short reply (1 byte: XX)"; zwei Bytes -> beide; negativ wie bisher.
- **Bau 20:** `// #define TWINGO_CELLWATCH` (aus) im Parameterblock: Zeile auf der Advanced-Seite, Seite `/cellwatch`, drei Routen, Treiber-Zweig und Messwert-Mitschrift nur mit dem Schalter. Zum Weiterarbeiten mit Cellwatch einschalten (oder `-DTWINGO_CELLWATCH`).
- **Tests lokal:** `cmake -DTWINGO_SIM_PAGE=ON|OFF -DTWINGO_CELLWATCH=ON|OFF` (Optionen in `test/CMakeLists.txt`).

## GEBAUT am 14.10. (Bau 21, nur Kommentare): TX / RX am Parameterblock
- Hinter den zwoelf auf true gesetzten `SIM_ON_*` (19F, 426, 436, 423, 69F, 157, 0EC, 0ED, 419, 428, 42F, 435): `// TX Emulator`.
- Hinter 0x424, 0x425, 0x43A, 0x445, 0x464, 0x588, 0x6BE, 0x4AE, 0x4AF, 0x659, 0x155, 0x0C5, 0x1C9: `// RX from Battery`.
- Nachtrag (Nutzer, 14.10.): 0x1A1, 0x1C7, 0x4F7, 0x500, 0x511 haben `// not needed for emulator but traffic on EVC<>LBC!`.

## Zum Bauen vorgemerkt (Bau 22, klein, gefunden am 15.10. am Bench; GEBAUT 15.10., siehe unten): "Result"-Zeile der Stille-Seite wird abgeschnitten
- **Befund (Bench 15.10., Lauf mit 0x00, Failsafe 5 min):** Die Zeile endet mitten im Wort: "... it was already silent during the shutdown announcemen" und die naechste Zeile ("Wake up: ...") klebt direkt dran.
- **Ursache (aus dem Code):** `append_quiet_html` (`BATTERY.cpp:1866`) schreibt mit `snprintf(b, sizeof(b), ...)` in `char b[120]`. Zwei Texte sind laenger als 119 Zeichen und werden abgeschnitten, das abschliessende `<br>` geht verloren: **`:1995` "Result: BMS's last frame was ... BEFORE the quiet phase began - it was already silent during the shutdown announcement (that alone does not prove the pack slept)."** (172 Zeichen) und **`:2004` "Result: BMS went silent after ... (measured from when the quiet phase began) - it stopped sending (that alone does not prove the pack slept)."** (151 Zeichen). Alle anderen Texte der Funktion sind kuerzer als 120 (laengster: `:1935` etwa 85). Der Fehler war schon vor den Baus 12 bis 20 da.
- **Aenderung:** Puffer fuer diese beiden Texte auf mindestens 200 Zeichen (eigener Puffer oder `char b[200]`; Stack-Verbrauch beachten) und danach pruefen, dass keine Zeile der Funktion mehr als die Puffergroesse braucht (Test mit dem laengsten Text und grossen Zahlen).
- **Tests:** Lauf bis zum Ende mit Frames nur in der Abschaltfolge -> Seite enthaelt den vollstaendigen Text "... (that alone does not prove the pack slept)." und danach "<br>"; derselbe fuer "BMS went silent after ...". Nur Text, gesendete Frames unveraendert.

## Zum Bauen vorgemerkt (Bau 23, klein, gefunden am 15.10. am Bench; GEBAUT 15.10., siehe unten): Zykluszeiten-Block springt waehrend des Sleep-Laufs
- **Befund (Bench 15.10.):** Der Block "Own send intervals 0x426 / 0x435 / 0x436" und die Zeile "0x155 (B7..B0)" erscheinen und verschwinden abwechselnd, solange der Sleep-Lauf laeuft.
- **Ursache (aus dem Code):** Die Box `nvrolBox` laedt sich im Lauf einmal pro Sekunde neu (`append_refresh_script_html`, `BATTERY.cpp:2114-2128`, `cur.innerHTML = nb.innerHTML`). Im neu geladenen HTML steht in `<pre id='twingoTxStat'>` nur der Platzhalter `-` (`BATTERY.cpp:3543-3544`). Das eigene 1-s-Skript (`BATTERY.cpp:3498-3500`) fuellt ihn erst beim naechsten Abruf wieder; die zwei Takte laufen nicht gleich.
- **Aenderung (Weg 1, vom Nutzer gewaehlt):** Im Refresh-Skript den Text von `twingoTxStat` und `twingoRx155` vor dem Ersetzen merken und danach sofort wieder einsetzen. Nur Skript der Seite, gesendete Frames unveraendert. (Weg 2, Text beim Seitenaufbau gleich mitliefern, nicht gewaehlt.)
- **Tests:** Seiten-Text enthaelt weiter `twingoTxStat`/`twingoRx155` und das Refresh-Skript enthaelt das Merken und Wiedereinsetzen; Golden-Test (gesendete Frames) unveraendert.

## Zum Bauen vorgemerkt (Bau 24, mittel, geplant am 15.10. mit dem Nutzer; GEBAUT 15.10., siehe unten): Klick auf die Betriebsart startet die Bus-Folge neu
- **Ziel (Nutzer):** Klick auf "Laden" spielt die Weckfolge wie beim Einstecken des Steckers (`LADEN_WAKE_STEPS`, `BUS.h:826-835`, etwa 12,9 s), danach bleibt der Ladezustand `A0/41` mit der Kommunikation des Autos stehen, bis eine andere Betriebsart angeklickt wird. Klick auf "Fahren" spielt entsprechend die Fahren-Weckfolge (`WAKE_STEPS`) und der Fahrzustand bleibt stehen.
- **Heute (aus dem Code):** `/twingoBusTarget` (`webserver.cpp:1222-1240`) speichert nur `bus_wake_target` (NVM `TWBUSTGT`); `wake_state()` rechnet mit der Zeit seit dem Sitzungsstart (`BATTERY.cpp:2899`), im laufenden Betrieb ist sie laenger als die Folge, der Bus springt sofort auf `BUS_LADEN` (kein Ablauf der Schritte).
- **Aenderung:** Im Handler von `/twingoBusTarget` die Sitzung neu starten (vorhandener Ausloeser `bus_session_restart`, `webserver.cpp:1185-1187`, `BATTERY.cpp:2883-2886`; setzt `bus_session_started = false`). Keine neuen Radiobuttons; das Feld "Zustand: manuell" bleibt unveraendert und unabhaengig.
- **Entschieden (Nutzer, 15.10.):** (a) "Schlafen" bleibt wie jetzt (verhaelt sich wie "Fahren", kein Neustart der Folge beim Klick). (b) Die Zusaetze unten werden **mit eingebaut**.
- **Zusaetze, mit eingebaut (aus dem vollstaendigen Ladelog 2 `ab5f2c05-Ladelog_LBC_EVC.log`, 114,93 bis 688,91 s):**
  1. 0x423 Byte 0 = `87` statt `07` fuer 5,5 s: beim Wecken ab +2,96 s nach dem ersten 0x423 (250,60 bis 256,10 s, dazu Byte 1 Rampe `1E` -> `2A` -> `1F`, siehe Log), am Ladeende ab +0,33 s nach dem Ende von `A0/41` (553,60 bis 559,10 s). Unsere Tabelle hat nur `07` (`BUS.h:263-282`).
  2. Wach-Zeit nach dem Ladeende auf die des Autos: 121,5 s (556,37 bis 677,87 s); heute 133,2 s (like the car) bzw. 136 s (`BATTERY.h:784-796`). Gilt fuer den Weg "Ladelog 2".
  3. Wake-Formen der uebrigen EVC-Frames beim Wecken ueber den Stecker (0x157, 0x1A1, 0x1C7, 0x419, 0x428, 0x42F, 0x435, 0x4F7, 0x500, 0x511); Werte aus dem Abschnitt "Zustand Laden" und dem Log.
  4. Im Zustand "Laden" alle EVC-Zeilen senden (heute nur die Zeilen der Maske, Standard 12 Frames, `BATTERY.h:28`); die Frames, die der echte Akku selbst sendet (`from_pack`), nicht, sie kollidieren mit dem echten Akku (`BUS.h:12-14`).
- **Klarstellung (Nutzer, 15.10.):** Wir senden bereits in der Wach-Phase nach dem Ladeende weiter, wie im Ladelog (Auto 557,87 bis 677,77 s mit allen Frames; bei uns die Stufen C3/C2/C0 mit 133,2 bzw. 136 s, "own frames still run"). Das bleibt so.
- **Entfaellt (Nutzer, 15.10.):** "Weitersenden in der echten Stille bei steckendem Stecker" gibt es im Auto nicht: Der Nutzer hat das Auto mit dem Stecker geweckt, es hat geladen, dann aufgeschlossen, Stecker gezogen und sofort wieder abgeschlossen; erst danach schlaeft das Auto ein (Wach-Phase 124,6 s, Schliessphase, Stille). Es gibt also keinen Schlaf bei steckendem Stecker und keine Aufnahme, die dazu fehlt.
- **Tests:** Klick auf "Laden" aus dem Zustand "Fahren": `bus_state_now` laeuft durch alle Schritte von `LADEN_WAKE_STEPS` (Zeiten, 0x426 B1/B2) und bleibt auf `BUS_LADEN`; Klick auf "Fahren" aus "Laden": Fahren-Folge; Golden-Test (gesendete Frames) bei unveraendertem Ziel unveraendert.

## GEBAUT am 15.10. (Bau 22, 23, 24; Branch twingo-bau7-schlaf-anforderung, Commits b2f2a78 und 38d8b88; 484 Tests mit Seite und Cellwatch / 484 ohne Cellwatch / 347 ohne beides gruen; ESP-Build und Bench-Test offen)
**Bau 22:** `append_quiet_html` schreibt in `char b[240]` (vorher 120); die zwei langen Result-Texte sind vollstaendig und enden mit `<br>`. Test (zwei Faelle) schlug mit dem alten Puffer fehl (Text endete in "announcem" wie am Bench) und laeuft jetzt.
**Bau 23:** Refresh-Skript der Box `nvrolBox` (`append_refresh_script_html`) merkt den Text von `twingoTxStat` und `twingoRx155` vor dem Ersetzen und setzt ihn danach wieder ein, wo die frische Kopie nur den Platzhalter `-` hat. Nur Skript der Seite. Golden-Lauf (alle gesendeten Frames) byte-identisch.
**Bau 24:**
- Klick auf die Betriebsart Fahren oder Laden (nicht Schlafen, nur bei Aenderung des Ziels) setzt `bus_target_restart` (`webserver.cpp`, Route `/twingoBusTarget`); `bus_state_now` startet daraufhin die Bus-Sitzung neu (`bus_restart_sessions()`): die Weckfolge des neuen Ziels laeuft ab und der Zustand bleibt stehen. Nicht im Zustand "manuell", nicht waehrend eines Sleep-Laufs. Keine neuen Radiobuttons; Seitentext ergaenzt.
- Wake-Formen des Ziels Laden aus dem Ladelog 2 (`ab5f2c05`, 247,6 s ff., Daten in `BUS.h`): 0x419, 0x428, 0x42F, 0x435, 0x4F7 die ersten 130 Frames vollstaendig; 0x1C7 (B3/B4 02/36 fuer 8,04 s, 02/34 acht Frames), 0x157 (B5/B6 E0/00 fuer 11 s, E0 bis 12,16 s, E1 bis 13,39 s; die Rampe von B6 wird nicht kopiert), 0x1A1 (sieben Frames FF FF FF FF FE FC 00 20, danach B0/B1 00/47, Rampe von B0 ab Frame 186). 0x511 hat Rauschwerte und bekommt keine Wake-Form.
- 0x423 Byte 0 = `87` beim Wecken ueber den Stecker (regulaere Frames 29 bis 84) mit der Rampe von Byte 1 (1E -> 2A -> 20 -> 1F), und am Ladeende (Weg "Ladelog 2": 330 bis 5830 ms nach dem Druck auf Sleep).
- Im Zustand "Laden" (und waehrend dessen Wake-Folge) werden auch die EVC-Frames gesendet, deren Zeile aus ist: 0x1A1, 0x1C7, 0x4F7, 0x511 (Schalter `LADEN_SENDS_ALL_EVC_FRAMES` im Parameterblock von `BATTERY.h`, Standard true). 0x500 nie (`active_in_state`: nur Zuendung 1 bis aus), die Frames des Akkus nie.
- Weg "Ladelog 2": Wach-Zeit bis zur Schliessphase 124,6 s (Log 553,27 -> 677,87 s), C3 ist der Rest nach C2 und C0; gilt mit und ohne "like the car". Die anderen Wege unveraendert (136 s / 133,2 s).
- **Golden-Lauf:** 24 Szenarien; 16 byte-identisch (alle Wege lang und kurz), 8 verschieden, und zwar nur der Weg "Ladelog 2" (gewollt: Wach-Zeit und 0x423-87).
- **Entfaellt:** Weitersenden in der echten Stille bei steckendem Stecker (das Auto schlaeft nur nach Abziehen des Steckers und Abschliessen, siehe oben).
- **Grenzen:** Rampe von B6 (0x157) und die Rauschbytes von 0x1A1/0x511 nicht kopiert. Nach dem Sleep-Druck aus dem Ladezustand enden die vier zugeschalteten Zeilen (0x1A1, 0x1C7, 0x4F7, 0x511), das Auto sendet sie in der Wach-Phase weiter.

## Zum Bauen vorgemerkt (Bau 25, mittel, geplant am 15.10. mit dem Nutzer; Empfehlung angenommen; GEBAUT 15.10., siehe unten): Ende des Lademodus beim Umschalten auf Fahren, 0x423 Byte 3/5 beim Wecken
- **Grundlage:** Vergleich des Ladelogs 2 (`ab5f2c05`, 114,93 bis 688,91 s) mit dem Nachspielen im Emulator (Betriebsart Laden, Wecken ueber den Stecker, 305,6 s Laden, Sleep Weg "Ladelog 2"), Stand nach Bau 24. Die Zahlen pro Phase stimmen fuer alle gesendeten EVC-Frames bis auf die unten genannten Punkte.
- **Wichtig (Nutzer, 15.10.):** Im Bench-Modus "Laden" gibt es kein Ladeende und keinen Sleep; das Auto bleibt wach, bis auf "Fahren" umgeschaltet wird. Das Ende des Lademodus ist also der Klick auf "Fahren". Der Punkt "0x1A1, 0x1C7, 0x4F7, 0x511 laufen im Sleep-Lauf weiter" entfaellt; im Modus Laden laufen sie dauerhaft (Bau 24).
- **Punkt 1: Laden -> Fahren (Empfehlung des Assistenten, vom Nutzer angenommen):** Der Klick auf "Fahren" im Zustand "Laden" spielt zuerst die Ende-Folge des Autos (4,6 s: 0x426 `A0/41` -> `00/01` mit Byte 7 = 80 (0,5 s) -> `00/01` (1,5 s) -> `00/07` (1,1 s) -> `00/06` (0,4 s) -> `08/06` (1,1 s), Schritte wie `SLEEP_LADELOG2` in `BUS.h`, Log 553,27 bis 557,87 s) mit den Ende-Formen der Frames, danach laeuft die Fahren-Weckfolge ab dem Zustand "wach" (nicht von "zu"; `WAKE_STEPS` ab dem Schritt "wach 10,8 s", beim Bau gegen das Fahr-Log 22aaf176 pruefen) und der Fahrzustand bleibt stehen. Heute (Bau 24) startet der Klick von "zu" mit der ganzen Fahren-Weckfolge (etwa 20 s), das Auto ist nach dem Ladeende aber wach, nicht geschlossen.
- **Ende-Formen aus dem Log (relativ zum Ende von `A0/41` = 553,27 s):** 0x436 Byte 0 faellt von AD zurueck (`laden_end_b0`, schon vorhanden); 0x423 Byte 0 = 87 von +0,33 bis +5,83 s (schon in Bau 24 fuer den Weg Ladelog 2); 0x423 Byte 5: 80 ab +0 s, A0 ab +0,73 s (554,00); 0x419, 0x42F, 0x4F7 (Byte 3 faellt von 5E auf 00), 0x428 (Byte 5 `04` -> `06`, Byte 6 `8F` -> `00`), 0x435 (Byte 3 `6A` -> `0B`), 0x1C7 (Byte 3/4: 08 und FE kommen vor), 0x157 (B5 E0, B6 00/01): als Wertefolgen aus dem Log zu uebernehmen, wie bei den Wake-Formen in Bau 24 (`laden_wake_form`). Fuer den Ablauf Laden -> Fahren dieselben Folgen verwenden.
- **Punkt 2: 0x423 Byte 3 und Byte 5 beim Wecken ueber den Stecker (Teil von Bau 24, dort nicht umgesetzt):** Stufen aus dem Log: `07 FD 80` ab +0,06 s bis 250,30 s, `07 FE 80` bis 250,60 s, `87 FE 80` bis 255,70 s, `87 FE 20` bis 256,20 s, danach `07 FE 20`; Byte 0/1 wie in Bau 24.
- **Punkt 3 (klein): 0x0EC:** im Log fehlen etwa 5,4 % der Frames (94,6/s statt 100/s), das letzte Frame kommt 677,83 s (vor dem Beginn der Schliessphase bei 677,87 s); bei uns laufen vier Frames in die Schliessphase. Nur falls gewuenscht.
- **Nicht noetig:** Kilometer (0x426) und Fahrzeugalter (0x436) sind absichtlich eigene Werte; Zaehlerbytes ohne bekannte Pruefsumme (0x423 Byte 7, Rauschwerte 0x1A1/0x511) werden nicht nachgebildet.
- **Tests:** Klick auf "Fahren" aus "Laden": 0x426 geht durch die Schritte der Ende-Folge, dann Fahren-Folge ab "wach" (kein 0x426 `00/02` am Anfang), Zustand bleibt Fahren; der Weg Sleep und die anderen Klicks unveraendert; Wake-Stufen von 0x423 Byte 3/5; Golden-Lauf fuer alle Szenarien mit Ziel Fahren ohne Laden davor byte-identisch.

## GEBAUT am 15.10. (Bau 25; Branch twingo-bau7-schlaf-anforderung, Commit 72ec48c; 492 Tests mit Seite und Cellwatch / 355 ohne beides gruen; Golden-Lauf: Wege lang und kurz byte-identisch, nur der Weg "Ladelog 2" anders (gewollt); ESP-Build und Bench-Test offen)
- **Klick auf "Fahren" im Zustand "Laden"** (`bus_state_now`, `BATTERY.cpp`): zuerst die Ende-Folge des Autos (`LADEN_END_WALK_MS` = 4,6 s, die Schritte von `sleep_state` Weg Ladelog 2: A0/41 -> 00/01 (B7 80) -> 00/01 -> 00/07 -> 00/06 -> 08/06; der Bus ist wach, nie "zu"), danach das Aufwecken zum Fahren ab dem Schritt "wach 10,8 s" (`FAHREN_AFTER_CHARGE_OFFSET_MS` = 2300: die ersten drei Schritte entfallen); der Fahrzustand bleibt stehen. 0x436 Byte 0 faellt mit der Rampe zurueck; 0x1A1, 0x1C7, 0x4F7, 0x511 laufen waehrend der Ende-Folge mit. Aus jedem anderen Zustand (auch aus "Laden", solange die Weckfolge noch laeuft) bleibt es beim Neustart von "zu" wie in Bau 24.
- **Ende-Formen aus dem Ladelog 2** (Zeit 0 = Ende von A0/41, 553,27 s; 16 s lang; `BUS.h`, `laden_end_form`): 0x419, 0x428, 0x42F, 0x435, 0x4F7 als Tabellen der ersten 160 Slots zu 100 ms; 0x1C7 B3/B4 (04/34 bis 0,42 s, 02/34 bis 0,99 s, 02/36 bis 2,03 s, 08/00 bis 3,01 s, dann 08/FE); 0x157 B6/B7 = 01/00 von 3,02 bis 3,35 s. Gilt auch fuer den Sleep-Lauf auf dem Weg "Ladelog 2".
- **0x423:** Byte 5 = 80 in den ersten 0,73 s am Ladeende (danach A0), Byte 0 = 87 von 0,33 s bis 5,83 s (Bau 24); beim Wecken ueber den Stecker Byte 3 / Byte 5 in den Stufen des Logs (Frames 0 bis 25 FD/80, 26 bis 79 FE/80, 80 bis 84 FE/20).
- **Nachgespielt und mit dem Log verglichen (Sleep Weg Ladelog 2):** 0x423 Byte 5, 0x428, 0x42F, 0x435 stimmen in der Wach-Phase jetzt mit dem Log ueberein; Rest sind Rauschbytes und die langsame Drift von 0x419 / 0x4F7 nach den 16 s.
- **Nicht gebaut:** 0x0EC (im Log 94,6 statt 100 Frames pro Sekunde, letztes Frame 677,83 s) und die Rampe von 0x157 B6; Rauschbytes von 0x1A1 / 0x511.

## Zum Bauen vorgemerkt (Bau 26, klein, Wunsch Nutzer, 15.10.; GEBAUT 15.10., siehe unten): Balancing-Bits des BMS unter der Zeile "0x155 (B7..B0)" anzeigen
- **Wunsch (Nutzer, Screenshot der Zeile `0x155 (B7..B0): 34 131 154 152 84 211 135 30`):** direkt unter dieser Zeile (Advanced-Seite, `<span id='twingoRx155'>` in der Box `nvrolBox`, `BATTERY.cpp` Abschnitt Zykluszeiten) die Balancing-Bits des BMS anzeigen.
- **Grundlage (aus dem Code):** die Bits stammen aus `0x912B` (96 Bit, mehrteilige Antwort, `handle_extended_multiframe_complete()`, `BATTERY.cpp`), werden in `datalayer_battery->status.cell_balancing_status[]` (Index 0 = Zelle 1) geschrieben; der Rundlauf fragt 0x912B einmal pro Durchlauf (136 s) ab, die Freie Anfrage `22 91 2B` jederzeit.
- **Entschieden (Nutzer, 15.10.):** Anzeige als Rohwerte, so wie sie in den Datenbytes der Antwort stehen (Hex, ohne Umrechnung auf Zellen). Abfrage von 0x912B alle 20 s, "oder in dem Bereich, was gut ins Timing passt".
- **Umsetzung (Vorschlag):** neue Zeile `<span id='twingoBalBits'>` direkt unter `twingoRx155`, 1-s-Abfrage ueber eine kleine Route (wie `/twingoRx155`). Text zum Beispiel `0x912B raw (data bytes 21-32): 00 00 00 FF F0 00 ...` (die 12 Datenbytes mit den 96 Bits in der Reihenfolge der Antwort, erstes Bit = Zelle 96, wie `handle_extended_multiframe_complete`) plus das Alter der letzten Antwort ("read 12 s ago" / "not yet read"). Die ganze Antwort (32 Datenbytes) bleibt ueber die Freie Anfrage `22 91 2B` lesbar. Dazu ein Zeitstempel und die 12 Rohbytes der letzten 0x912B-Antwort im Treiber (neue Felder, `handle_extended_multiframe_complete`).
- **Takt 20 s (Vorschlag, passt ins Timing):** der Rundlauf bleibt bei einer Abfrage pro Sekunde (`EXT_POLL_INTERVAL_MS` 1000, 136 Eintraege). Zusaetzlich wird 0x912B alle 20 s als Abfrage eingeschoben (anstelle des naechsten Listeneintrags, der dann einen Takt spaeter kommt); der Eintrag 0x912B in der Liste kann bleiben, der Rundlauf wird dadurch pro Durchgang etwa 7 s laenger (136 s -> etwa 143 s). Nicht waehrend einer DTC-Abfrage, der NVROL-Folge oder der Stille (dort gelten die bestehenden Regeln). Offen fuer die Umsetzung: ob die 20 s als Konstante im Parameterblock stehen sollen (Vorschlag: ja, `BAL_POLL_INTERVAL_MS`).
- **Tests:** Seite enthaelt die Zeile und die Route; Text zu bekannten Bits (z. B. `FF F0` in Byte 21/22 der Antwort = Zellen 85 bis 96); "not yet read" ohne Antwort; Golden-Lauf unveraendert.

## GEBAUT am 15.10. (Bau 26; Branch twingo-bau7-schlaf-anforderung; 496 Tests mit Seite (mit und ohne Cellwatch) / 359 ohne beides gruen; Golden-Lauf: ohne die UDS-Abfragen alle 24 Szenarien byte-identisch; ESP-Build und Bench-Test offen)
- Neue Zeile `<span id='twingoBalBits'>` direkt unter `twingoRx155` auf der Advanced-Seite: `0x912B raw (data bytes 21-32, first bit = cell 96): 00 00 00 FF F0 ... (read N s ago)`, vor der ersten Antwort `not yet read`; Route `/twingoBalBits`, 1-s-Abfrage der Seite (`bal_bits_text()`, `BATTERY.cpp`). Rohwerte = die 12 Datenbytes (21 bis 32 = `buffer[23..34]`) der letzten Antwort, in der Reihenfolge der Antwort; die ganze Antwort bleibt ueber die Freie Anfrage `22 91 2B` lesbar.
- Abfrage von 0x912B alle `BAL_POLL_INTERVAL_MS` = 20 s (Parameterblock von `BATTERY.h`, Abschnitt 5) zusaetzlich zum Rundlauf, anstelle des naechsten Listeneintrags (eine Abfrage pro Sekunde bleibt, der Rundlauf wird etwa 7 s laenger); der Listeneintrag 0x912B wird uebersprungen, wenn die Extra-Abfrage weniger als 10 s her ist, sonst startet er die 20 s neu. Nicht in der NVROL-Folge, in der Stille oder waehrend einer DTC-Abfrage (dort laeuft der Abfragezweig nicht).
- Gesendete Frames der Bus-Zeilen unveraendert; nur die UDS-Abfragen enthalten die zusaetzlichen 0x912B.
