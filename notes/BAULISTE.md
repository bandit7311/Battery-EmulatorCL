# Bauliste (nur Vormerkung, NICHTS davon ist gebaut)

Stand 09.10.2026. Gebaut wird erst nach ausdruecklichem "bauen".
Basis-Branch fuer Code: claude/twingo-hv-modell-und-alter (HEAD e3988cb).

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

## Offen beim Nutzer
- Bench-Mitschnitt mit Logger (RX und TX) in den drei Schritten fuer 0x423 (Abschnitt 7.5)
- Pin-Belegung am LBC-Stecker (CAN H/L), damit die Aderfarben aus dem Schaltplan zugeordnet werden koennen.

## Korrektur zum Mitschnitt 22aaf176
- 0x155 (10 ms): 60.697 geloggt, ca. 77.500 erwartet in aktiver Zeit -> ca. 16.800 Frames (22 %) fehlen.
- Ursache: 88 kurze Loecher (Mittel 1,9 s, zusammen 169 s) im Mitschnitt, keine Dezimierung.
- Lange Pausen (753 s: 471-566, 748-1355, 1531-1553, 1557-1586 s) sind echte Busstille.
- Filter ab 04.10.: !90,C6,12E,17A,17E,186,18A,1B0,1F6,1F8,211,217,242,29A,29C,2B7.
- 02.10./04.10. waren Fahrzeug-CAN (0x350 etc.), nicht der Akku-Bus.
