# Merkliste Test (Bench, angelegt 12.10.)

Nur Vorgemerktes zum Testen am Bench. Nichts davon ist gemacht, solange kein Ergebnis eingetragen ist. Nur Fakten aus Code und Messungen; Vermutungen sind als solche gekennzeichnet.

## T1: Haengt E14381 an der Zeit in 0x436? (Vermutung des Nutzers, 12.10., nicht belegt)

**Vermutung (Nutzer):** Der Akku vergleicht die Zeit aus 0x436 mit einem Wert von der letzten Sitzung. Ist die gesendete Zeit nach einem Neustart kleiner, kommt E14381.

**Was dafuer spricht (gemessen):**
- Zeit in 0x436 um +1000 min: E14381 (MCPU) von `2F` auf `2E`, quittierbar; alle anderen Eingriffe liessen `2F` (`TEST_BENCH_10-10_Aufwecken_und_Zustaende.md`, Abschnitt "Ergebnis Bench 12.10.").
- Danach kam E14381 nach dem Quittieren nicht zurueck.
- Gesendetes Alter am 12.10.: 1370828 (Voreinstellung 1025301).

**Was dagegen spricht oder offen ist (Code und Messung):**
- Automatik: Alter = Referenz im Flash + Uhr, nie rueckwaerts (`BATTERY.cpp:1306`, nur im RAM abgesichert). Manuell: Startwert + Minuten ab gespeicherter Unix-Zeit (`:1290`). Ohne gueltige Uhr wird kein Alter gesendet (`:1293`, `:1300`). Ein kleinerer Wert nach dem Neustart entsteht nur durch eine getippte kleinere Zahl.
- `9261` und `91C1` (MCPU) stehen auf `0`; ein gemerkter Vergleichswert ist in diesen DIDs nicht sichtbar.
- Basis- und Zielwert des +1000-Tests sind nicht festgehalten; die Schwelle ist offen.
- E14381 war frueher mit 0x426 an aktiv und mit 0x426 aus weg, ohne Aenderung der Zeit (`MERKZETTEL_Zeitwerte_33BE.md`, Z. 126, 284).

**Ablauf (nichts im Akku wird geschrieben, nur Frames aendern sich):**
1. Vor dem Neustart "Vehicle age" von der Seite notieren, nach dem Neustart nochmal. Ist der Wert gleich oder groesser?
2. 0x426 an, E14381 nicht aktiv (`19 02 FF` bzw. Haken "Read ALL DTC statuses"): Basiswert V notieren.
3. Zeit in 0x436 auf V - 1000 setzen, ca. 30 s warten, `19 02 FF` lesen: Status von E14381 notieren (kommt `2F` wieder?).
4. Zeit wieder auf V (oder V + 1000), lesen, notieren.
5. Schwelle eingrenzen durch Halbieren der Differenz; jeden Schritt mit Basis-, Zielwert, Status, Zeilenstand notieren.

**Eintragen:** Basiswert, Zielwert, Status E14381 (MCPU und SCPU), `9259`, `925B`, `[DC] 925E`.

## T2: Zeit speichern lassen (bereits vorgeschlagen, mit der Endfolge aus Bau 1)
- Alle 17 EVC-Zeilen an, Einschlafweg "Ladelog 2", Sleep-Lauf mit 9281, Stille bis zum Ende ohne 12 V zu trennen, Wake.
- Danach lesen: `9261`, `91C1`, `22 92 5B` (MCPU), `[DC] 22 92 5E`, `19 02 09` (beide).
- Stand 12.10.: nach 9281, 3 min Stille und Wake stehen `9261` / `91C1` auf `0` (Reset-Bits unveraendert `00 00` / `20 80`, DTC-Listen leer).

- Bedingungen fuer Zeit-Zuwaechse nach der Quelle "dj2cs" (Hypothese, `QUELLE_dj2cs_Frames_und_Zeitpruefungen.md`): `0x69F` an und mit der ID aus `925E` (jetzt `13 88 6F`); Zeit in `0x436` streng steigend; beim Hochlauf gesendete Zeit >= gespeicherter Wert; keine Werte ab `0xFFFFFF`.

- Nach Bau 7 (Schlaf-Anforderung `0x423` B0 = `04` in der letzten Sekunde, siehe `BAULISTE.md`, Bau 7): derselbe Lauf, danach zusaetzlich `19 02 09` beider Prozessoren lesen (der Fehlerspeicher wird laut Quelle im selben Fenster geschrieben).
- **Messwerte 13.10. (Firmware f8b9acd, Bau 6 und 7; Sleep mit 9281, Weg lang, ca. 183 s Busverkehr, dann Stille, Auto-Wake):**
  - Vor dem Sleep (nur Update, ohne Schlaf, nach Neustart): MCPU `9261` = `91C1` = 62436 min (`00 F3 E4`), blieb bei zwei Lesungen unveraendert; SCPU `9261` = 62441, spaeter 62446 (laeuft live); `925F` MCPU 6852 km, SCPU 6854 km; `19 02 09` beide ohne Fehler.
  - Nach dem Sleep: MCPU `9261` = `91C1` = 62455 min (`00 F3 E7`), SCPU `9261` = 62456 (`00 F3 F8`); `925F` = `9262` = 6854,00 km; `925B` = `00 00`; `925E` = `13 88 6F`; `19 02 09` beide ohne Fehler (war schon vorher so).
  - Befund: MCPU-Zeit ist nach dem Sleep gemeinsam von 62436 auf 62455 gesprungen (Schnappschuss, 1 min unter der SCPU). Einzelner Lauf; Ursache nicht bewiesen (Neustart hatte den Wert 62436 vorher auch gesetzt; seit 12.10. mehrere Aenderungen). `9261 - 91C1` = 0 (Auto: 784.382).
  - Stille-Anzeige im Lauf: "SILENT for 00:22 (safety limit 05:00)"; Akku-Frames endeten gemeinsam bei T+183,0 s (Zellframes 180,1 s).
- **Messwerte 13.10., zweiter Lauf (gleiche Firmware f8b9acd, Sleep mit 9281, Weg lang):**
  - Vor dem Lauf: MCPU `9261` = `91C1` = 62455 (`00 F3 F7`); `91CF` = 350,00 km; `925F` = 6854,00 km; BMS-Zustand durchgehend `06`; Balancing-Zaehler 0.
  - Nach dem Lauf: MCPU `9261` = `91C1` = 62542 (`00 F4 4E`); SCPU `9261` = 62543 (`00 F4 4F`); `91CF` = 350,00 km (`80 00 2B C0`, unveraendert); `925F` = `9262` = 6856,00 km; `925B` = `00 00`; SCPU `925E` = `20 80`; `19 02 09` beide ohne Fehler ("DTC Read: OK, 0 DTC(s)"); `0x155` verworfene Frames 287 (vorher 1; Zeitpunkt nicht geprueft).
  - Befund: MCPU-Zeit wieder gemeinsam gesprungen (62455 -> 62542), wieder 1 min unter der SCPU. Damit zweimal reproduziert (Weg lang). Nicht geprueft: Weg Ladelog 2; E14381 war vor dem Lauf nicht vorhanden, daher keine Aussage zum Fehler.
  - Anzeigefehler "BMS SILENT since 71481:14" / "last one 6233.6 s ago" in der Stille: siehe BAULISTE Bau 8.
- **Fahrzeugalter am Bench (Entscheidung des Nutzers, 13.10.):** Das Alter, das der Emulator in `0x350`, `0x436`, `0x523`, `0x376` sendet, ist ein von Hand gesetzter Startwert (manueller Modus, im Flash als `TWAGEMV` gespeichert), absichtlich auf etwa 43 Tage reduziert (vorher ca. 1,3 Mio. min). Grund: Das BMS wuerde den Akku bei einem Alter von 3 Jahren staerker degradieren. Der Wert zaehlt +1 pro Minute nach der Uhr des Emulators (Unix-Zeit, sonst `millis()`). Die SCPU zaehlt diesen Wert live mit; die MCPU uebernimmt beim Sleep einen Schnappschuss davon (zweimal gemessen, MCPU 1 min unter der SCPU). Stand 13.10.: SCPU ca. 62.543 min (43 d 10 h).

## T3: Reset-Register nach jedem Schritt lesen (bereits vorgeschlagen)
- Nach Sleep-Lauf, 9281-Schreiben und Wake je `22 92 5B` (MCPU) und `[DC] 22 92 5E`: zeigt, welcher Schritt zuerst ein Reset-Bit setzt und ob dann 1B0E41 / 1BB587 zurueckkommen.
- Stand 12.10.: nach kaltem Neustart `00 00` / `20 80`, beide Fehlerspeicher leer.
