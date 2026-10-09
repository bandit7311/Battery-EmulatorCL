# Bauliste (nur Vormerkung, NICHTS davon ist gebaut)

Stand 09.10.2026. Gebaut wird erst nach ausdruecklichem "bauen".
Basis-Branch fuer Code: claude/twingo-hv-modell-und-alter (HEAD e3988cb).

## PRIORITAET 1b (Nutzer, 10.10.): Modus "wie im Fahrzeug" und zweigeteilte Simulator-Liste
Vorgabe: Auch die 12 weiteren EVC-Frames des echten BMS<>EVC-Busses aufnehmen. Neuer Code auf **neuem Branch ab e3988cb** (z. B. `twingo-echter-bus-modus`); `claude/twingo-hv-modell-und-alter` bleibt als Rueckfall unveraendert.

**Block 1 "ALLE SIMULATIONSWERTE wie im FAHRZEUG"** (echtes Format und echter Takt, nach Takt gruppiert; Takt gemessen im Mitschnitt 22aaf176):
- 10 ms: `0x19F`, `0x0EC`, `0x0ED`, `0x157`, `0x1A1`, `0x1C7`
- 100 ms: `0x423`, `0x426`, `0x436`, `0x419`, `0x428`, `0x42F`, `0x435`, `0x4F7`, `0x500`, `0x511`
- 1 s: `0x69F`
- 20 ms, 200 ms, 500 ms, 3 s: auf der EVC-Seite dieses Busses keine Frames (3-s-Frames sind Akku-Frames, die senden wir nicht). Gruppen bleiben als leere Ueberschriften bzw. entfallen (zu klaeren).
- Zustandsauswahl (zu, wach, Zuendung 1, Zuendung 3, GO, Fahrt, aus), Alter-Feld (`0x436` B1-B3), km-Feld (`0x426` B4-B5), Inhalt je Zustand aus dem Auto-Mitschnitt.

**Block 2 "simulator alt"** (die heutigen 46 Zeilen unveraendert, nach Takt gruppiert: 10 / 20 / 100 / 200 / 500 / 1 s / 3 s).

**Entschieden (Nutzer, 10.10.):** Block 1 enthaelt nur den BMS<>EVC-Bus. Die Fahrzeug-CAN-Zeilen gehoeren in Block 2 "simulator alt", dort **mit echtem Format**, so dass man sie wirklich einschalten kann. 10-ms-Frames erst nach dem Wake-Burst ist in Ordnung.

**Markierung der Fahrzeug-CAN-Zeilen (Berichtigung):** Der Mitschnitt 22aaf176 wurde mit dem Filter `!90,C6,12E,17A,17E,186,18A,1B0,1F6,1F8,211,217,242,29A,29C,2B7` aufgenommen (keine dieser 16 IDs im Log). Fuer diese 16 Zeilen (`0x090`, `0x0C6`, `0x12E`, `0x17A`, `0x17E`, `0x186`, `0x18A`, `0x1B0`, `0x1F6`, `0x1F8`, `0x211`, `0x217`, `0x242`, `0x29A`, `0x29C`, `0x2B7`) ist die Markierung "BMS<>EVC-Bus" daher **"unbekannt (im Mitschnitt gefiltert)"**, nicht "nein". Ob `0x090`/`0x242` vom Akku kommen: im Bench-Log 25.09. (Akku allein, ungefiltert) sendet der Akku keine dieser 16 IDs.

**Offene Punkte vor dem Bau:**
- Last: sechs 10-ms-Frames = ca. 600 Frames/s. Fruehere Wake-Fehler (CAN NATIVE BUS ERROR) kamen bei 0x090/0x242 gleichzeitig mit dem Wake-Burst; die schnellen Frames erst nach dem Wake-Burst starten (Fix-later-Punkt aus dem Chat vom 29.09.).
- Nicht bauen ohne ausdrueckliches "bauen".

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
