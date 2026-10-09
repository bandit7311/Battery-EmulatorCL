# Baubericht Prio 1 und Prio 2 (10.10.2026)

Branch `twingo-echter-bus-modus` (Basis e3988cb), Commit 14f3aaf. Der alte Branch `claude/twingo-hv-modell-und-alter` bleibt unveraendert als Rueckfall. 404 Tests gruen (Basis 362).

## Prio 1: echter Bus

- 30 sendbare Zeilen im Fahrzeugformat und Fahrzeugtakt: 25 neue R-Zeilen (Index 46-70) plus die fuenf Bus-Zeilen 0x19F, 0x426, 0x436, 0x423, 0x69F. Der Inhalt haengt vom Zustand ab (zu, wach, Zuendung 1, Zuendung 3, GO, Fahrt, aus). Zaehler und CRC gibt es nur dort, wo sie gemessen sind (0x19F, 0x0EC, 0x157, 0x511, 0x426, 0x436). Alles andere wird als aufgezeichnet wiederholt.
- 10-ms-Frames starten erst nach dem Wake-Burst.
- Globaler Schalter "Format der BMS<>EVC-Frames: Fahrzeug (Standard) / Zoe alt" fuer 0x423, 0x426, 0x436, 0x19F, 0x69F.
- Maske auf 128 Bit (NVM-Schluessel TWINGOSIMHI2 und TWINGOSIMHI3 neu). Zeilen 0-45 behalten ihren Index.
- Empfangsverfolgung: LED (gruen = Frame mit dieser ID kam von aussen; Schwelle 3 Zyklen, mindestens 2 s, 3-s-Frames 10 s), "Erstmals gesehen" relativ (T+12,43 s) mit NTP-Uhr als Zusatz, Liste "Weitere empfangene IDs". T+0 wird nur durch "Alle Zeilen aus" gesetzt. Zeitleiste mit aktueller Zeit und letzter Aktion. Zeilen mit "Erstmals gesehen" juenger als 15 s sind hellgelb. Abfrage per JSON alle 1 s (Route `/twingoRxStatus`).
- Voreinstellungen: km 0x426 = 6844, Alter 1025301 (+1 pro Minute ab Unix 1791590400). Kein Sicherheitstag und keine Anhebung aus dem Pack mehr. Die NVM-Schluessel fuer das Alter sind umbenannt (TWAGEPV2/TWAGEPT2), ein alter gespeicherter Bezug wird ignoriert.
- Simulator-Seite neu: Block 1 mit allen 50 Adressen (30 sendbar, 20 Zellframes nur LED), Block 2 "simulator alt" mit 41 Zeilen, alle "nein" in der Spalte BMS<>EVC-Bus.

## Prio 2: Zellwerte aus dem Broadcast

- Eigener Schalter "Zellwerte-Quelle: UDS (heute) / Broadcast".
- Dekodierung je Frame: Z1=(B0<<4)|(B1>>4), Z2=((B1&0F)<<8)|B2, Z3=(B3<<4)|(B4>>4), Z4=((B4&0F)<<8)|B5, Z5=(B6<<4)|(B7>>4), mV = Rohwert + 2000. 0x5DD traegt nur eine Zelle.
- Gemessen: Bench-Frame `87 28 74 87 48 74 87 4F` ergibt 4162/4164/4164/4164/4164 mV. Auto-Frame `60 36 03 60 36 03 60 3F` ergibt 3539 mV je Zelle.
- Platzhalter beim Aufwecken (FF.. und 00..0F) liegen ausserhalb von 2500-4500 mV und werden ignoriert.
- Werte gehen in `cell_voltages_mV[]`. Mit allen 96 Zellen werden Packspannung, Min und Max daraus berechnet, der Wechselrichter bekommt sie wie bisher.
- Im Broadcast-Modus werden die UDS-Zellabfragen (0x9021-0x9083) im Rundlauf uebersprungen. Cellwatch bleibt davon unberuehrt.

## Offene Punkte und Risiken

- Der Bench-Pack hielt am 06.10. 1.311.344 min, der Standard ist jetzt 1.025.301. Moeglicherweise nimmt der Pack einen kleineren Wert als den gespeicherten nicht an. Das ist ungeprueft.
- 0x423 Byte 7, 0x500, 0x1A1, 0x419: Pruefsumme unbekannt, aufgezeichnete Werte werden wiederholt.
- Frames mit Akku-Herkunft sind senbar. Mit angeschlossenem echten Akku kollidieren sie.
- Zellnummern: Reihenfolge der Frames ist 0x5F7 = Zellen 1-5 ... 0x5D9 = 36-40, 0x5EC = 41-45 (Annahme: ersetzt A18 = 0x5D7 des Zoe-Codes), ... 0x5A1 = 91-95, 0x5DD = 96. Im BEFUND Abschnitt 19.1 steht fuer 0x5EC "36-40". Das ist ein Zaehlfehler: A18 ist 41-45, der Code nutzt 41-45.
- Nicht Teil dieses Baus (Prio 3): 0x62B-Testzeile, Altersfaktor 1/10, DF/DA-Ziele, Inverter-Dump, Shutdown-Taster.
- Die Bus-Zeilen wurden nur am Rechner getestet (Einheitstests und Seite im Browser mit simuliertem Status), noch nicht am Bench.
