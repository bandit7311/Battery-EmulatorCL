# Messreihe Bench: Daily balance, zwei Laeufe (03.11. abends und 04.11. nachts) und Auto-Zeilen vom 04.11.

Quelle: Fotos und Angaben des Nutzers (Handy-Uhr, Ortszeit), eigene Rechnungen. **gemessen** = aus Foto oder Angabe abgelesen, **gerechnet** = aus den Rohwerten, **Schluss** = nicht belegt. Stand des Emulators: Commit `7ab39c0` (Daily balance, Abort-Knopf, Laufart "Sleep"); Einstellungen siehe unten.

## 1. Ablauf
| Lauf | Einstellung | Zeit (Ortszeit) |
|---|---|---|
| 1 | Time 22:10, Duration 65 min, Cycles 2, Test 10 min, Check 120 s, Laufart "Sleep" (nichts geschrieben, NVROL-Log "not sent (Sleep)") | Start 03.11. 22:10, Ende 04.11. ca. 0:40 (ueber Mitternacht, `done`-Zustand nicht gesehen, Freigabe zum Deye danach gemessen: "Charge power sent to the inverter 22.00 kW" um 0:41) |
| 2 | Cycles 3, Test 10 min (Nutzer: "ab 0:50, 3 Stueck") | Start 04.11. ca. 0:50; Ergebnis 7:58 |
- **Gemessen (Statuszeile, Lauf 1):** "cycle 2 of 2: sleeping", "first plan bit after: **test 6 s, cycle 6 s**" (erste `912B`-Antwort nach dem Wecken enthielt schon Bits, 6 s nach dem Ende der Wecken-Folge, zweimal).
- **Gerechnet:** Zeitplan passt auf den Sekundenbereich (Test: Wecken ca. 22:23; Zyklus 1: ca. 23:31:40; Zyklus 2: Beginn ca. 23:32).
- **Fehler gefunden (Nutzer 03.11. abends):** Stand "manuell, Fahren" liess den Bus-Zustand auf Fahren stehen, "awake" wurde nicht gesetzt (Code: `bus_state_now()` aendert den Zustand nur bei `!bus_state_manual`). Mit "manuell" aus lief es, und die Plan-Bits waren zu sehen, sobald der Pack wieder `04`/`02` meldete. Korrektur noch **nicht gebaut** (Entscheidung A oder B offen, Empfehlung B: nicht starten bei "manuell").

## 2. Plan (`0x912B`) nach jedem Wecken
| Zeit | Bits | Rohwert |
|---|---|---|
| 03.11. ca. 22:10 (vor dem Test) | 26 | `BF F2 0A 82 60 24 26 00 10 20 80 00` (Lesung ca. 22:09:50, in der Stille unveraendert; nach dem Wecken 22:23 dieselben 26 Bits, Alter 61 s) |
| 03.11. 23:31 (nach Zyklus 1) | **63** | `FF FF BF EF FE DA 8F E6 BC 19 70 00` (21 Zellen wie im 26-Bit-Plan, 5 nur im alten, 42 neu) |
| 04.11. 0:41 (nach Zyklus 2) | **9** | `FA C0 00 00 80 00 00 00 00 00 00 00` = Zellen 96, 95, 94, 93, 92, 90, 88, 87, 64 (alle auch im 63-Bit-Plan) |
| 04.11. 1:04 (Lauf 2, vor dem Schlaf) | 9 | unveraendert, Alter 89 s |
| 04.11. 7:58 (nach Lauf 2) | **0** | `00 ... 00`, Alter 11 s, `925C` = `01` |

## 3. Zaehler `9251` / `9252` (Einheit 1/1024 Ah bzw. 1/1024 h, raw ohne `0x80000000`)
| Zeit | `9251` | `9252` |
|---|---|---|
| 03.11. 22:24 | 0x2700 = 9,750 Ah | 0x1B68 = 6,852 h |
| 03.11. 23:32 | 0x2A30 = 10,547 Ah | 0x1FA0 = 7,906 h |
| 04.11. 0:41 | 0x3060 = 12,094 Ah | 0x23D8 = 8,961 h |
| 04.11. 7:58 | 0x3C50 = 15,078 Ah | 0x3128 = 12,289 h (`924F`/`9250` zeigen jetzt dieselben Werte) |
- **Gerechnet:** `9252` jeweils **+1080 Schritte** (1 Schritt = 1/1024 h = 3,516 s, 1080 = 63,3 min = Obergrenze) fuer 22:24 bis 23:32 und 23:32 bis 0:41; von 0:41 bis 7:58 **+3408 = 3 x 1080 + 168** (Testzyklus, 10 min Stille minus Anlauf ca. 160 bis 170 Schritte). Jeder Zyklus lief also bis zur Obergrenze.
- **Formel** (22,22 mA je Zelle und Schritt, aus frueheren Notizen, nicht gemessen): mittlere Bitzahl = `Delta 9251 / (Delta 9252 x 0,02222)`.
  - Bench Zyklus 1: 816 raw ergibt **34 Bits** (Plan davor gelesen: 26).
  - Bench Zyklus 2: 1584 raw ergibt **66 Bits** (Plan davor gelesen: 63).
  - **Schluss:** Am Bench war der abgearbeitete Plan etwas groesser als der gelesene; Ursache offen (Vermutung: neue Berechnung beim Einschlafen).
- **Verworfene `0x155`-Frames** (Wecken-Marker): 273 (22:13), 541 (22:24), 809 (23:32), 1076 (0:41), 2147 (7:58): je Wecken +267 bis +268, also **vier Weckvorgaenge** in Lauf 2.

## 4. Zellspannungen (Zellmonitor, mV)
| Zeit | tiefste | hoechste | Mittel | Spreizung | Bemerkung |
|---|---|---|---|---|---|
| 03.11. 19:09 (Datum vom Nutzer nicht genannt, **Schluss**: vor Lauf 1) | 3625 (Zellen 1, 11, 12) | 3635 (Zelle 87) | 3631,4 | 10 (Anzeige: 7 mV, "Battery is balancing now!") | 83 Zellen genau 3632, Zellen 1 bis 12 bei 3625 bis 3630 |
| 04.11. 0:44 (nach Lauf 1) | 3612 (Zelle 1) | 3625 (86, 94) | 3617,9 | 13 | 53 Zellen 3617 |
| 04.11. 0:57 (vor dem Test von Lauf 2) | 3615 (Zelle 7) | 3625 (88, 90) | 3617,8 | 10 | Planzellen im Mittel 3621,3 gegen 3617,4 |
| 04.11. 1:04 (Schlaf laeuft) | 3610 (Zelle 12) | 3623 (88, 90, 94) | 3617,3 | 13 | obere Gruppe 85 bis 96 im Mittel 3620,3 (vorher 3621,5), Zellen 1 bis 12 tiefer (3610 bis 3617) |
| 04.11. 7:58 (`9007`/`9009`) | 3604 | 3611 | 3607,7 (Zellsumme 346,344 V) | 7 | Min/Max-Zuordnung nicht bestaetigt |
- **Gemessen:** Das Balkendiagramm zeigte Cyan (Balancing) bei etwa Zelle 64 und ca. 86 bis 96 (Zuordnung aus Pixelpositionen geschaetzt), passend zum 9-Bit-Plan.
- **Offen:** Zelle 86 (3625 mV, hoechste) stand nicht im 9-Bit-Plan; warum der Pack Zellen auswaehlt (Schwelle, Totband), ist nicht geklaert.

## 5. Weitere Werte
- SOC intern / USOC: 36,880 % / 29,910 % (22:xx bis 0:41), **29,960 % / 22,690 % um 7:58** (SOC min 31,36 %, max 34,34 %). Der Balancing-Zaehler (ca. 3 Ah ueber alle Zellen) kann den Abfall von ca. 7 Prozentpunkten nicht erklaeren; Ursache offen (Deye-Strom in den Pausen?).
- `9261` = 72576 min (04.11. 0:41, `011B80`) und 72723 (`011C13`) um 7:58; `91C1` = 72907 (`011CCB`) um 7:58.
- Max Available Power 72,000 kW (0:41) und 71,540 kW (7:58).

## 6. Auto-Zeilen (Airtable) vom 04.11. 17:26 bis 19:25 UTC
Spalten ohne Kopfzeile nach Position gedeutet (**Schluss**): `912B` (64 Hex, letzte 12 Bytes), `9251`, `9252`, `9279`, `scpu9261`, `91C1`, `9261` (MCPU), `9259`, `9251`/`9252` doppelt (`924F`/`9250`), `9262`/`9263`, zwei noch ungedeutete Spalten (`00d4`, `0144` bzw. `0145` ab Ladeende), Grund, `v12`, `evc3302`.
- **Gemessen:** `912B` in allen fuenf Zeilen **null**; Zaehler `800223b0` / `8003a2c2` konstant; `9259`: 04, 05, 05, 02 (Ladestart), 04 (Ladeende); `v12` 12,38 / 13,57 / 13,58 / 13,58 / 12,58 V; `scpu9261` +1 je Minute (18:03:25 bis 19:25:41 genau +80), `91C1` (`21e7f2`) und MCPU-`9261` (`2ddff0`) unveraendert; `9279`: 04, 04, 05, 07, 08.
- **Gerechnet gegen die Zeile vom 03.11. 19:18** (`80021ed0` / `80039e8a`, Plan damals **52 Bits**): `9252` **+1080 Schritte**, `9251` **+1248 raw (+1,219 Ah)**; Formel ergibt **genau 52,0 Zellen**. **Schluss:** In diesem Lauf blieb der Plan ueber alle 1080 Schritte gleich gross (weicht am Auto nicht von der Formel ab, am Bench schon, siehe 3.). Danach war der Plan leer.
- **Gerechnet:** `scpu9261` minus MCPU-`9261` = 558 min um 17:26:27; das Einschlafen lag demnach um ca. 08:08 UTC.

## 7. Offen
- Statuszeilen beider Laeufe ("done: all cycles", Zeiten bis zum ersten Plan-Bit je Zyklus fuer Lauf 2) und Zellmonitor nach Lauf 2 liegen nicht vor.
- Warum Bench-Laeufe groesser ausfallen als der gelesene Plan (34 gegen 26, 66 gegen 63 Bits), und warum der Plan zwischen 26, 63, 9 und 0 Bits springt.
- Auswahl der Zellen (86 nicht im Plan trotz hoechster Spannung); SOC-Abfall ueber Nacht.
- Korrektur "manuell" (A oder B) und weitere Details siehe `BAULISTE.md`.
