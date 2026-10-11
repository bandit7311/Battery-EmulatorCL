# Befund: Ladelog am echten Twingo (11.10.2026) und Dump-Vergleich

Quellen: `598b197c-canmitlog.log` (Akku-Bus, PuTTY-Start 17:00:07), Protokoll des Nutzers (17:00 Stecker eingesteckt, 17:10 Auto aufgeschlossen, Stecker getrennt, sofort wieder verschlossen, gewartet bis Schlaf), MCPU-Dumps `1791728231` (vorher) und `1791731801` (nachher), SCPU-Dumps `1791728260` (vorher) und `1791732192` (nachher, vollstaendig). Unvollstaendige SCPU-Dumps `1791731824` und `1791732071` nicht verwendet.
Kennzeichnung: **gemessen** = steht in Log/Dump; **Schluss/Annahme** = aus Messung abgeleitet, nicht bewiesen.

> **Korrektur 11.10. abends (nach Rueckmeldung des Nutzers):** Die Deutung unten in Abschnitt 2, 3 und 5 war teilweise falsch. Richtig ist: In Ladelog 1 und 2 ist `A0/41` der **Ladezustand** (Stecker steckt). Der Bus wacht durch das Einstecken auf (Log 1 bei 611,77 s, Log 2 bei 247,64 s nach 44 s Stille); die Weckfolge beginnt mit 0x423. Die ersten 28 Frames in Log 1 (bei 0,5 s) sind der Wachzustand **vor** dem Einstecken, nicht das Laden. Die Aussage, im Laden wache der Akku ohne 0x423 auf, ist falsch. Die gueltigen Messwerte stehen in `BAULISTE.md`, Abschnitt "Zustand Laden". Ladelog 2 (`445a1527-teraterm.log`) ist vollstaendig (kein "not printed"). Offen: In Log 1 passen die vom Nutzer notierten Uhrzeiten (17:00 Stecker, 17:10 aufgeschlossen) nicht zu den Log-Zeiten (Aufwachen 611,77 s nach dem PuTTY-Start 17:00:07).

## 1. Grenzen der Daten
- Der Logger druckt nicht jeden Frame: nach den ersten 28 Frames steht `[261064 CAN frames not printed]`, danach weitere solche Meldungen (Zaehler 90 bis 8046). **Die Ladephase 17:00 bis 17:10 ist nur in den ersten 28 Frames sichtbar.** Ab 611,77 s (= 17:10:19) laeuft das Log, aber auch dort mit Luecken (fuer Zaehler und Zeitwerte reicht es).
- Die Zeiten in den Dump-Dateinamen laufen etwa 10 Minuten hinter der Uhr des Logs her (**Schluss**, aus der SCPU-Zeit zurueckgerechnet: 59 Minuten Abstand der Dumps = +59 in 9261 der SCPU). Die "nachher"-Dumps sind nach dem Schlaf des Logs (1284 s) gezogen (**Schluss**).

## 2. Laden (erste 28 Frames, 0,52 bis 0,54 s)
- **Gemessen:** vorhanden sind 0x0EC, 0x0ED, 0x157, 0x1A1, 0x1C7, 0x19F, 0x4F7, 0x428, 0x42F (EVC-Seite) und 0x155, 0x1C9, 0x0C5, 0x445, 0x464, 0x588 (Akku). **Nicht vorhanden:** 0x423, 0x426, 0x435, 0x436, 0x419, 0x500, 0x511, Zellframes, 0x424/0x425/0x43A, 0x69F.
- 0x19F steht dort schon in der Form `A0 A0 7D x 00 6A 50 FE`, 0x1C7 = `AA 23 00 08 FE 00`, 0x428 = `00 00 00 00 00 06 00 00`.
- **Schluss:** im Laden wacht der Akku ohne 0x423 auf; 0x423/0x426/0x435/0x436 sendet die EVC erst, wenn das Auto selbst aufwacht (Aufschliessen).

## 3. Aufschliessen und Folgezustand (ab 611,77 s)
Wake-Formen wie im Fahr-Log 22aaf176: 0x423 `30 7F FF FF FF E0 FF FF`, 0x4F7 `00 7F 00 00 09 FC 06 F0`, 0x1C7 `00 03 00 0E FE 00`, 0x428 `00 00 00 00 00 07 C0 00`, 0x42F Nullen, 0x445 `80 FF FF FF FF FF 80`, 0x464 `F0 1F C0`, 0x588 `FF FC 10 36`.

0x426 (B1/B2, B3-B5 = 01 6A 71 = 92.785 km, konstant; Zeit ab Log-Sekunde):

| Zeit s | B1/B2 | B7 | Bemerkung |
|---|---|---|---|
| 611,90 | 00/02 | 40 | zu |
| 613,80 | 00/03 | 40 | neuer Code |
| 613,90 | 60/63 | 40 | neuer Code |
| 614,50 | 60/61 | **80** | B7 wechselt auf 80 |
| 615,00 | 68/61 | 80 | |
| 616,10 | 60/61 | 80 | |
| 619,80 | **A0/41** | 80 | neuer Zustand, haelt bis 1149,6 s (**530 s = 8,8 min**) |
| 623,70 / 624,80 | A8/41, A0/41 | 80 | kurzer Wechsel |
| 1149,60 | 00/01 | 80 | |
| 1150,20 | 00/01 | 40 | B7 zurueck auf 40 |
| 1151,50 | 00/03 | 40 | |
| 1151,60 | 00/07 | 40 | |
| 1152,00 | 08/07 | 40 | |
| 1152,60 | 08/06 | 40 | |
| 1153,10 | 00/06 | 40 | wach, 120 s |
| 1273,20 | 00/02 | 40 | zu; 0x0EC endet 1273,20 s, letztes Frame 0x0ED `60 FF 00` bei 1284,26 s |

- **Neu gegenueber dem Fahr-Log:** B1/B2 `00/03`, `60/63`, `68/61`, `A0/41`, `A8/41`, `00/01`, `08/07`; B7 = `80` (statt `40`) von 614,5 bis 1150,2 s.
- Im Zustand A0/41 (gemessen): 0x19F in der Ruheform `00 00 7D x 00 6A 90 FE`, 0x436 B0 = `AD`, 0x1C7 `AA 23 00 04 34 00`, 0x428 `…04 8E 00`, 0x435 `FF FF 1E 6A 36 00 00 00`, 0x0ED `60 FF 80`, 0x0EC B0 = `1C`, kein 0x500.
- 0x436 (Zeit) zaehlt +1 pro Minute: `2D 5A CD` bei 611,9 s bis `2D 5A D9` bei 1271,9 s.
- UDS-Anfragen der EVC (18DADBF1): bei 624,8 bis 627,8 s `9215`, `91D1`, `91D2`, `92C1`; bei 1150,1 bis 1153,1 s `9243`, `9245`, `91CF`, `92C1`, `91C1`, `901B`. Wie im Fahr-Log liest die EVC nur, sie schreibt keine Zeit.

## 4. Dumps: gemessene Aenderungen
MCPU (148 von 335 Werten geaendert, davon 96 Zellspannungen):
- `9261` `2D 59 D8` -> `2D 5A D9` (**+257 min**); `91C1` `21 61 DA` -> `21 62 DB` (**+257 min**).
- Abstand `9261 - 91C1` = **784.382 (0x0BF7FE)**, vorher wie nachher identisch (**Schluss:** 91C1 = Fahrzeugzeit minus feste Zahl).
- Der neue `9261` ist **genau der letzte 0x436-Wert im Log** (`2D 5A D9` bei 1271,9 s). **Schluss:** die MCPU sichert die Zeit am Ende der Sitzung (Schlaf).
- `9279` 16 -> 20 (+4 Missionen); die Listen `9275` bis `9278` sind **unveraendert** (gemessen).
- `9270` (Modusverlauf): zwei neue Eintraege `04` (laut Definition "Non Isolated Charge").
- `9259` `05` -> `04`, `925C` `01` -> `02`, `9243` +1255, `9245` +141, `9005`/`9007`/`9009`/`900D` (Spannung/Strom) geaendert, `9215` +1, `91CF` unveraendert (`80 0C 28 60` = 24.899 km Packkilometer).
- `925E` bleibt `13 88 6F`.
SCPU (alle vier Dumps verglichen; `9270` gibt es in der SCPU nicht, weder in der Definition noch in den Dumps):
- `9261`: `2D 5A A3` -> `2D 5A DE` -> `2D 5A E2` -> `2D 5A E4` (folgt der Uhr live, +1 pro Minute).
- `9259` und `925C` (gemessen, nur Dumps mit Antwort): vorher `05` / `01`; Dump 3 (`1791732071`) `04` / `02`; Dump 4 (`1791732192`) wieder `05` / `01`. Dump 2 hat bei 9259 keine Antwort. Dump 3 zeigt dieselben Werte wie der MCPU-Nachher-Dump (`9259` = `04`, `925C` = `02`). **Schluss:** ein zeitlich begrenzter Zustandswechsel kurz nach dem Schlaf, der in beiden Prozessoren gleich auftrat.
- `925B` (Index kaeltester Thermistor): `00` -> `02` -> `06` (wandert von Dump zu Dump).
- Weitere geaenderte Kennungen im vollstaendigen Paar (vorher/Dump 4): Spannungen und Temperaturen (9007, 9008, 9009, 9014, 91B8, 91FC), 91FA/91FB, 920A.

## 5. Was daraus fuer die Bench folgt (Konzept, noch nichts gebaut)
1. **Sichern von 9261/91C1:** passiert im Auto am Sitzungsende mit dem letzten 0x436-Wert. Bei uns sendet 0x436 ein festes Alter; fuer die Sicherung fehlt vermutlich ein Sitzungsende wie im Auto (A0/41 -> 00/01 -> ... -> 00/02 -> Stille). **Annahme.**
2. **Neue 0x426-Zustaende** (Tabelle oben) in die Zustandsfolge aufnehmen, besonders `A0/41` mit B7 = `80` und die Abschaltfolge `00/01 -> 00/03 -> 00/07 -> 08/07 -> 08/06 -> 00/06 -> 00/02`.
3. **Ladebetrieb als eigener Zustand** (EVC-Frameliste aus Abschnitt 2, ohne 0x423/0x426/0x435/0x436): Bench-Test, ob der Akku damit ohne 0x423 aufwacht. **Ungetestet.**
4. **Offene Fragen:** was genau die Sicherung der Zeit ausloest (Ende von A0/41, das `00/02` oder die Stille); warum 9279 um 4 steigt, ohne dass 9275 bis 9278 sich aendern; ob unsere Bench mit denselben Codes dasselbe Speichern ausloest.

## 6. Messplan fuer die naechste Aufnahme
- Logger ohne Drosselung (alle Frames drucken oder nur EVC-Frames mitschreiben), damit die ganze Ladephase im Log steht.
- Dumps direkt vor dem Einstecken und direkt nach dem Schlaf, Uhren (PC und Logger) abgleichen.
- Zusaetzlich `190209` an DB und DC vor und nach dem Laden.
