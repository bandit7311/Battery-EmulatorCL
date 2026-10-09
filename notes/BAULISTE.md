# Bauliste (nur Vormerkung, NICHTS davon ist gebaut)

Stand 09.10.2026. Gebaut wird erst nach ausdruecklichem "bauen".
Basis-Branch fuer Code: claude/twingo-hv-modell-und-alter (HEAD e3988cb).

## BAUUMFANG "echter Bus" (Zusammenfassung, Stand 10.10., wartet auf ausdrueckliches "bauen")
Neuer Branch ab `e3988cb` (z. B. `twingo-echter-bus-modus`); `claude/twingo-hv-modell-und-alter` bleibt unveraendert als Rueckfall.
1. **Block 1 "ALLE SIMULATIONSWERTE wie im FAHRZEUG"** (nur BMS<>EVC-Bus, nach Takt gruppiert 10 ms / 20 ms / 100 ms / 200 ms / 500 ms / 1 s / 3 s): `0x19F`, `0x0EC`, `0x0ED`, `0x157`, `0x1A1`, `0x1C7` (10 ms), `0x423`, `0x426`, `0x436`, `0x419`, `0x428`, `0x42F`, `0x435`, `0x4F7`, `0x500`, `0x511` (100 ms), `0x69F` (1 s). Echtes Format und echter Takt, Inhalt je Zustand aus dem Mitschnitt 22aaf176; 10-ms-Frames erst nach dem Wake-Burst.
2. **Globaler Schalter** "Format der BMS<>EVC-Frames: Fahrzeug (Standard) / Zoe alt" fuer die fuenf Zoe-Form-Frames (`0x423`, `0x426`, `0x436`, `0x19F`, `0x69F`) gemeinsam (Nutzer 10.10.).
3. **Zustandsauswahl** (zu, wach, Zuendung 1, Zuendung 3, GO, Fahrt, aus), Alter-Feld fuer `0x436` (B1-B3), km-Feld fuer `0x426` (B4-B5).
4. **Block 2 "simulator alt"**: die uebrigen 41 Zeilen mit echtem Format, nach Takt gruppiert; die fuenf BMS<>EVC-Zeilen dort **ausgeblendet** (Index unveraendert, kein Verrutschen der NVM-Maske); neue Zeilen hinten (Index 46-57).
5. **Spalte BMS<>EVC-Bus** (ja / nein / unbekannt (gefiltert) / neu) und Absender.
6. **Empfangs-LED** je Zeile (gruen = von aussen empfangen, grau = nicht), Abschnitt "Weitere empfangene IDs", Aktualisierung ohne Neuladen.
7. Tests, clang-format, Lieferung zum Flashen. Nicht Teil: Aenderung der Sleep-/Wake-Folge.

## PRIORITAET 1c (Nutzer, 10.10.): Empfangs-LED in der Simulator-Liste
- Neue Spalte **zwischen Haken und ID**: LED **gruen** = Frame mit dieser ID wird gerade empfangen (also von aussen, vom Akku), **grau** = nichts empfangen. Zweck: ausschliessen, dass wir ein Frame emulieren muessen, das der Akku ohnehin selbst sendet.
- Gilt fuer alle Zeilen beider Bloecke (inkl. der zwoelf neuen EVC-Frames). Eigene Sendungen zaehlen nicht (die Empfangsliste der Logs enthaelt nie unsere TX-IDs, z. B. kein RX `0x423`/`0x19F` im Log 25.09.).
- Zusatz: Abschnitt "Weitere empfangene IDs" (alle IDs, die gehoert werden und in keiner Zeile stehen, mit Anzahl und letzter Zeit).
- Schwelle gruen/grau: z. B. 3 x Takt, mindestens 2 s (3-s-Frames: 10 s). Aktualisierung per kleinem Abruf alle 1-2 s ohne Seiten-Neuladen.
- Technik (im Code nachgeschaut): Zaehler pro ID gibt es heute nur fuer das Schlaf-Log (`nvrol_silence_ids`, `SILENCE_ID_MAX`), nicht allgemein; neu waere ein Feld "zuletzt gesehen" je Zeile im Empfangsteil des Treibers plus ein JSON-Abruf. Nicht bauen ohne ausdrueckliches "bauen".

## PRIORITAET 1b (Nutzer, 10.10.): Modus "wie im Fahrzeug" und zweigeteilte Simulator-Liste
Vorgabe: Auch die 12 weiteren EVC-Frames des echten BMS<>EVC-Busses aufnehmen. Neuer Code auf **neuem Branch ab e3988cb** (z. B. `twingo-echter-bus-modus`); `claude/twingo-hv-modell-und-alter` bleibt als Rueckfall unveraendert.

**Block 1 "ALLE SIMULATIONSWERTE wie im FAHRZEUG"** (echtes Format und echter Takt, nach Takt gruppiert; Takt gemessen im Mitschnitt 22aaf176):
- 10 ms: `0x19F`, `0x0EC`, `0x0ED`, `0x157`, `0x1A1`, `0x1C7`
- 100 ms: `0x423`, `0x426`, `0x436`, `0x419`, `0x428`, `0x42F`, `0x435`, `0x4F7`, `0x500`, `0x511`
- 1 s: `0x69F`
- 20 ms, 200 ms, 500 ms, 3 s: auf der EVC-Seite dieses Busses keine Frames (3-s-Frames sind Akku-Frames, die senden wir nicht). Gruppen bleiben als leere Ueberschriften bzw. entfallen (zu klaeren).
- Zustandsauswahl (zu, wach, Zuendung 1, Zuendung 3, GO, Fahrt, aus), Alter-Feld (`0x436` B1-B3), km-Feld (`0x426` B4-B5), Inhalt je Zustand aus dem Auto-Mitschnitt.

**Block 2 "simulator alt"**: die heutigen Zeilen **ohne** die fuenf BMS<>EVC-Frames (`0x423`, `0x426`, `0x436`, `0x19F`, `0x69F`), also 41 Zeilen, nach Takt gruppiert (10 / 20 / 100 / 200 / 500 / 1 s / 3 s). Entscheidung Nutzer 10.10.: Alle Frames, die auf dem BMS<>EVC-Bus laufen, stehen **nur noch in Block 1**, nie doppelt. Das alte Zoe-Format der fuenf bleibt hoechstens als Umschalter innerhalb von Block 1 (Vorschlag, noch zu bestaetigen). Beim Bau beachten (im Code nachgeschaut: `webserver.cpp:211-217` laedt beim Start die gespeicherte Maske `TWINGOSIMMASK`/`TWINGOSIMHI` und wertet sie bitweise nach Zeilenindex aus; `webserver.cpp:968-984` schreibt bei jedem Klick die ganze Maske zurueck): Zeilenindizes **nicht verschieben**. Die fuenf BMS<>EVC-Zeilen bleiben mit unveraendertem Index in der Tabelle und werden in Block 2 nur ausgeblendet; neue Zeilen kommen hinten dran (Index 46 bis 57, passt in die 64 Bit; gespeicherte Bits dort sind 0 = aus). Die Standardmaske 0x387 gilt nur fuer ein frisches Geraet, eine gespeicherte Maske hat Vorrang.

**Entschieden (Nutzer, 10.10.):** Block 1 enthaelt nur den BMS<>EVC-Bus. Die Fahrzeug-CAN-Zeilen gehoeren in Block 2 "simulator alt", dort **mit echtem Format**, so dass man sie wirklich einschalten kann. 10-ms-Frames erst nach dem Wake-Burst ist in Ordnung.

**Markierung der Fahrzeug-CAN-Zeilen (Stand 10.10., zweite Berichtigung):** Nutzer: Der Mitschnitt auf dem BMS<>EVC-Bus im Auto (22aaf176) wurde **ohne Filter** aufgenommen. Dann sind die 16 Frame-IDs `0x090`, `0x0C6`, `0x12E`, `0x17A`, `0x17E`, `0x186`, `0x18A`, `0x1B0`, `0x1F6`, `0x1F8`, `0x211`, `0x217`, `0x242`, `0x29A`, `0x29C`, `0x2B7` dort nachweislich **nicht** vorhanden: Markierung "BMS<>EVC-Bus" = **nein** fuer alle 41 Zeilen von Block 2 (nicht "unbekannt (gefiltert)"). Hinweis: Aus der Datei selbst laesst sich der Filterzustand nicht ablesen; der Nutzer hatte am 09.10. zuvor von einem Filter gesprochen (siehe BEFUND_Echter_Bus_LBC_EVC_09-10.md Abschnitt 18).

**Offene Punkte vor dem Bau:**
- Last: sechs 10-ms-Frames = ca. 600 Frames/s. Fruehere Wake-Fehler (CAN NATIVE BUS ERROR) kamen bei 0x090/0x242 gleichzeitig mit dem Wake-Burst; die schnellen Frames erst nach dem Wake-Burst starten (Fix-later-Punkt aus dem Chat vom 29.09.).
- Nicht bauen ohne ausdrueckliches "bauen".

## NEU (Nutzer, 10.10.): Spalte "Erstmals gesehen" in der Simulator-Tabelle
- Spalte neben der LED, Wert = Zeitpunkt des ersten Empfangs einer Nachricht mit dieser ID. Vorschlag: **Hauptzeit relativ in Sekunden (`T+12,43 s`)** aus `millis()` (monoton, ohne WLAN/NTP, gleiche Zeitbasis wie das Log `(84.55) RX4 ...`), Bezug **T+0 = letzter Druck auf "Alle Zeilen aus"** (Entscheidung Nutzer 10.10.: Zeit nur dort auf 0, kein eigener Knopf; loescht dabei alle "Erstmals gesehen"). **Zusatz** als Tooltip/zweite Zeile: Uhrzeit per NTP (`hh:mm:ss`), nur wenn die Uhr gestellt ist (der Treiber hat dafuer schon `get_unix_time()`), und die Roh-`millis()` fuer den Abgleich mit dem Log. Optional billig dazu: "zuletzt gesehen" und Anzahl.
- **Zeitleiste oben (Nutzer 10.10.):** feststehende Leiste mit der **aktuellen Zeit** `Jetzt: T+142,7 s` (zusaetzlich Uhr per NTP, falls gestellt) und **"Letzte Aktion"** (Zeit und Art, z. B. `T+98,2 s - 0x423 an`); aktualisiert sich per kleinem Abruf alle 1 s. Damit laesst sich beim Zuschalten oder Knopfdruck erkennen, was neu dazukam: Zeilen, deren "Erstmals gesehen" juenger als 15 s ist (Entscheidung Nutzer 10.10.), werden hellgelb hervorgehoben. Skizze: `Skizze_Simulator_Seite_v6.html`.
- Genauigkeit: Zeitstempel im Empfangsteil des Treibers genommen; 10-ms-Frames sind damit auf wenige ms genau (Task-Latenz).
- Skizze: `Skizze_Simulator_Seite_v4.html` (Scratchpad). Nicht bauen ohne ausdrueckliches "bauen".

## NEU (Nutzer, 10.10.): Zellspannungen aus den Broadcast-Frames auswerten
- Der Akku sendet am Bench alle 96 Zellen im Broadcast (`0x5A1...0x5F7`, `0x5DD`, alle 3 s, ein Burst dauert 100 ms; gemessen in den Logs 25.09., 28.09., 01.10.). Der Treiber wertet sie bisher nicht aus (kein `case` in `RENAULT-TWINGO-GEN1-BATTERY.cpp`); die Zellwerte kommen aus der UDS-Abfrage (`TWINGO_EXTENDED_CELL_POLLING`).
- Dekodierung: je Frame 5 Zellen, 12 Bit, mV = Roh + 2000; Zellnummer nach Renault-Datenbank (`0x5F7` = Zelle 1-5 ... `0x5A1` = 91-95, `0x5DD` = 96; `0x5EC` ersetzt `0x5D7`, Annahme); Platzhalter beim Wecken (`FF...`, `00 ... 0F`) ausfiltern.
- **Eigener Schalter "Zellwerte-Quelle: UDS (heute) / Broadcast"**, getrennt vom Schalter "Format Fahrzeug / Zoe alt" (der betrifft nur gesendete BMS<>EVC-Frames). Vorschlag, noch zu bestaetigen.
- Wirkung auf den Wechselrichter (im Code nachgeschaut, `RENAULT-TWINGO-GEN1-BATTERY.cpp:36-105`): Wechselrichter lesen `datalayer.battery.status` (`cell_voltages_mV[]`, `cell_min/max_voltage_mV`, Pack-Spannung als Summe, Temperaturen, Limits). Mit Broadcast als Quelle wuerden dieselben Felder gefuellt, die Werte kaemen also ohne weitere Aenderung beim Wechselrichter an (alle 3 s statt rund 20 s fuer ein volles UDS-Bild, 1-mV-Aufloesung, ohne UDS-Abfragelast).
- Nicht bauen ohne ausdrueckliches "bauen".

## ENTSCHEIDUNG Block 1 (Nutzer, 10.10., endgueltig): Sendbar sind 30 Zeilen, die 20 Zellframes nur mit LED
- **Nur LED, keine Sendemoeglichkeit:** die 20 Zellframes `0x5A1`, `0x5AC`, `0x5AD`, `0x5B4`, `0x5B5`, `0x5B7`, `0x5C9`, `0x5CB`, `0x5CC`, `0x5D6`, `0x5D9`, `0x5EA`, `0x5EC`, `0x5ED`, `0x5F0`, `0x5F1`, `0x5F2`, `0x5F4`, `0x5F7`, `0x5DD` (kommen sicher vom BMS; LED zeigt, wann sie anfangen).
- **Sendbar mit LED (30 Zeilen):** alle uebrigen 30 Adressen von Block 1, **auch die vermeintlichen Akku-Frames** `0x155`, `0x0C5`, `0x1C9`, `0x424`, `0x425`, `0x43A`, `0x445`, `0x464`, `0x588`, `0x4AE`, `0x4AF`, `0x659`, `0x6BE` ("da wir ja gar nix wissen"), plus die 17 anderen.
- **Folge fuer die Maske (im Code gerechnet):** 46 bestehende Zeilen + 25 neue sendbare Zeilen (30 minus die 5 bestehenden `0x423`, `0x426`, `0x436`, `0x19F`, `0x69F`) = 71 > 64 Bit. Die Zeilenmaske muss auf **128 Bit** erweitert werden: bestehende NVM-Schluessel `TWINGOSIMMASK` (Bits 0-31) und `TWINGOSIMHI` (Bits 32-63) bleiben, zwei neue Schluessel fuer Bits 64-127 (Standard 0 = aus). Keine Migration der gespeicherten Haken noetig. Cell-Zeilen brauchen kein Bit.
- Skizze: `Skizze_Simulator_Seite_v3.html` (Scratchpad).

## KORREKTUR Block 1 (Nutzer, 10.10.): ALLE Adressen des BMS<>EVC-Busses, keine Vorauswahl nach Absender
Block 1 enthaelt **alle 50 Adressen (11 Bit)** des Mitschnitts 22aaf176 (ungefiltert laut Nutzer), nicht nur die 17 vom Claude vermuteten EVC-Frames. Wer sendet, wird nicht vorab angenommen: Spalte "Bench-Messung" zeigt nur, was der Akku allein gesendet hat (Log 25.09.), die LED zeigt am Bench, was von aussen ankommt. Keine technische Sperre; Hinweis wie bisher bei Zeilen mit BMS-Ursprung. Takt-Verteilung: 10 ms (9): `0x155`, `0x0C5`, `0x1C9`, `0x157`, `0x19F`, `0x1A1`, `0x1C7`, `0x0EC`, `0x0ED`; 100 ms (16): `0x419`, `0x423`, `0x424`, `0x425`, `0x426`, `0x428`, `0x42F`, `0x435`, `0x436`, `0x43A`, `0x445`, `0x464`, `0x4F7`, `0x500`, `0x511`, `0x588`; 1 s (2): `0x69F`, `0x6BE`; 3 s (23): `0x4AE`, `0x4AF`, `0x659` und die 20 Zellframes. Inhalte je Zustand aus dem Mitschnitt (aufgezeichnete Werte; Zaehler/CRC wo bekannt, siehe Abschnitt 17). Skizze: `Skizze_Simulator_Seite_v2.html` (Scratchpad). Die Aussage "17 EVC-Frames" in den Punkten 1b/Bauumfang ist damit ueberholt: 17 = Teilmenge, die nicht vom Akku allein gesendet wurde.

## PRIORITAET 1 (Nutzer, 10.10.): Markierung "echter BMS<>EVC-Bus" in der Simulator-Liste
Vorgabe: Der Traffic auf dem BMS<>EVC-CAN reicht. In der Simulator-Liste muss pro Zeile sichtbar sein, ob das Frame dort wirklich vorkommt (Quelle: Mitschnitt 22aaf176, Abschnitt 3/9.2 in BEFUND_Echter_Bus_LBC_EVC_09-10.md).
- **ja (5 Zeilen):** `0x423`, `0x426`, `0x436`, `0x19F`, `0x69F` (bei `0x423/0x426/0x436/0x19F` Zusatz "Format weicht ab").
- **nein (41 Zeilen):** alle uebrigen (Fahrzeug-CAN), darunter `0x350`, `0x090`, `0x242`, `0x53B`, `0x214`.
- Zusatz: Spalte "Absender" (Akku/EVC/Fahrzeug) und "auch auf Fahrzeug-CAN".
- Nicht bauen ohne ausdrueckliches "bauen".

## A. Anzeige / Simulator-Liste
1. Neue Spalte "Echter LBC/EVC-Bus" in /simulator (ja / nein / neu).
   - Grundlage: Mitschnitt 22aaf176 (Bus zwischen Akku und EVC, ohne unseren Eingriff).
   - "ja": Frames, die dort wirklich laufen (u.a. 0x155, 0x424, 0x425, 0x436, 0x426, 0x423, 0x435, 0x19F, 0x1A1, 0x0ED, 0x500).
   - "nein": Frames, die nur auf dem Fahrzeug-CAN laufen (z.B. 0x350, 0x214, 0x53B, 0x55D, 0x5DE, 0x634) - der Akku hoert sie im Auto nicht.
   - "neu": im echten Bus vorhanden, in unserer Liste noch nicht.
2. Stash stash@{0} (nur lokal): Statusanzeige "ALLE ZEILEN AUS seit ..." / "Zeilen wie in der Tabelle" + Neuladen nach Klick + Test. Nur auf "bauen" einspielen, sonst verwerfen.

## B. Modus "EVC wie im echten Bus"
3. 0x436 im echten Format: [80/AD][Alter 24 Bit][00 00].
4. 0x426 wie im echten Bus (Kilometer 16 Bit mod 65536, echte Bytes).
5. Fehlende Frames ergaenzen: 0x423, 0x435, 0x19F (A0), 0x0ED, 0x500, 0x1A1 u.a.
6. Waehlbare Zustaende: zu / wach / Zuendung 1 / Zuendung 3 / GO / Fahrt / aus (Folge wie im Fahrprotokoll vom 09.10.).
   Ziel: herausfinden, was das MCPU dazu bringt, 9261/91C1 zu speichern und 9259 auf 05 zu bringen.

## C. Weitere Wuensche (aus frueheren Listen)
7. Alters-Faktor 1/10.
8. Ziele DF/DA bei freier Anfrage.
9. Inverter-Dump.
10. Messung im Auto.
11. Abschalt-Knopf.
12. 0x436 dauerhaft manuell einstellbar.

## D. Neu am 09.10. (abends)
13. Testzeile `0x62B` (HEVC_WakeUpFrame, 2 Byte, Signal HEVC_WakeUp_Signal laut Renault-Datenbank, Inhalt unbekannt) im Simulator, nur zum Ausprobieren, ob der Akku darauf reagiert. In keinem unserer Logs vorhanden.
14. Zellspannungen aus den Broadcast-Frames anzeigen (96 Zellen, mV = Rohwert + 2000) mit der korrigierten Nummerierung: 0x5F7 = Zellen 1-5, 0x5F4 = 6-10, 0x5F2, 0x5F1, 0x5F0, 0x5ED, 0x5EA, 0x5D9, (0x5EC statt 0x5D7 = 36-40, Annahme), 0x5D6, 0x5CC, 0x5CB, 0x5C9, 0x5B7, 0x5B5, 0x5B4, 0x5AC, 0x5AD, 0x5A1 = 91-95, 0x5DD = 96. Siehe BEFUND_Echter_Bus_LBC_EVC_09-10.md Abschnitt 8.1.
15. Marker "Absender" (Akku / EVC / Fahrzeug) in der Simulator-Liste, gespeist aus dem Bench-Mitschnitt mit RX/TX (Abschnitt 7.5) und der Renault-Datenbank (Abschnitt 8.2).

16. Spalte "auf beiden Bussen" (26 IDs) in der Simulator-Liste; Datenbasis BEFUND_Echter_Bus_LBC_EVC_09-10.md Abschnitt 9.2.

17. Absender-Marker auf Basis der Bench-Mitschnitte (BEFUND_Echter_Bus_LBC_EVC_09-10.md Abschnitt 10.2): Akku sendet 0x155, 0x0C5, 0x1C9, 0x424, 0x425, 0x43A, 0x445, 0x464, 0x588, 0x4AE, 0x4AF, 0x659, 0x6BE, 0x5A1...0x5F7, 0x5DD.

## Offen beim Nutzer
- **Mitschnitt im Auto am BMS<>EVC-Bus mit Filter "Lauf 2"** (`90,C6,12E,17A,17E,186,18A,1B0,1F6,1F8,211,217,242,29A,29C,2B7,350,214,ext,700-7FF`): klaert, ob die 16 gefilterten IDs (0x090, 0x242, ...) dort laufen. Der einzige ungefilterte Mitschnitt (02.10., 364b56eb) ist Fahrzeug-CAN (enthaelt alle 16 IDs, aber 0x155/0x424/0x425/0x423/0x426/0x436 = 0) und beantwortet es nicht.
- Bench-Mitschnitt mit Logger (RX und TX) in vier Schritten (0x423 ohne/mit/allein, nur 0x350; Abschnitt 10.3)
- Pin-Belegung am LBC-Stecker (CAN H/L), damit die Aderfarben aus dem Schaltplan zugeordnet werden koennen.

## Korrektur zum Mitschnitt 22aaf176
- 0x155 (10 ms): 60.697 geloggt, ca. 77.500 erwartet in aktiver Zeit -> ca. 16.800 Frames (22 %) fehlen.
- Ursache: 88 kurze Loecher (Mittel 1,9 s, zusammen 169 s) im Mitschnitt, keine Dezimierung.
- Lange Pausen (753 s: 471-566, 748-1355, 1531-1553, 1557-1586 s) sind echte Busstille.
- Filter ab 04.10.: !90,C6,12E,17A,17E,186,18A,1B0,1F6,1F8,211,217,242,29A,29C,2B7.
- 02.10./04.10. waren Fahrzeug-CAN (0x350 etc.), nicht der Akku-Bus.
