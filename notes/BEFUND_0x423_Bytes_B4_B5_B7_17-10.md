# Befund 0x423: B4/B6, B5 und B7, gemessen (17.10.2026)

Nur eine Notiz, nichts gebaut. Quellen: `dc664c4d-0x423.txt` (5289 Frames, 114,97-688,90 s, entspricht dem Ladelog ab5f2c05: gleiche 21 Frames mit B0 = 04), `22aaf176-canmitlog.log` (Fahrlog), `598b197c-canmitlog.log` (Laden).

## Anlass
Eine fremde Analyse (zwei Traces, DBC "EVC_Final") behauptete: B4/B6 wechseln alle 50 ms, B7 sei eine XOR-Pruefsumme, B5 `20` = Fahren und `80` = Laden. Alles gegen die Logs geprueft.

## Ergebnis
| Behauptung | Messung | Urteil |
|---|---|---|
| B4/B6 wechseln alle 50 ms zwischen 5D und B2 | Frameabstand 100 ms; haeufigster Lauf von B4 = 5 Frames = 500 ms (1050 Mal) | falsch |
| B6 spiegelt B4 | Nur im Ladelog: B4 = B6 in 99,9 % (dc664c4d). Im Fahrlog 22aaf176 sind sie gegenlaeufig: B4 = B6 in 0 von 3297 (07/A0) und 0 von 1918 (0B/80) Frames, B6 ist das Gegenstueck (5D gegen B2) | nur im Laden-Zustand wahr |
| B7 = XOR-Pruefsumme von B0..B6 | XOR(B0..B6) = B7 in 0,0 %, Summe in 0,0 %. Gleiches `07 1E FF FD B2 A0 B2` kommt mit B7 = C9, CA, CB, CC, CD, CE vor | widerlegt; B7 laeuft langsam hoch (C9..CE, dann DA), kein Pruefwert |
| B5: 20 = Fahren, 80 = Laden | siehe Tabelle | umgekehrt |
| B1 = Hochlauf-Zaehler | B1 = 1E, 1F, 20, 22 (Ladelog), 1E, 34-37 (Fahrlog): langsamer Wert | unbelegt |
| 0x87 = Systemstart bei 614,73 s | in dieser Datei 87-Frames bei 250,6 s; bisher als kurzer Impuls (Laden/Ende, ca. 5,5 s) notiert | nicht belegt |

## B5 je Log (B0 mit B5, Anzahl)
- 22aaf176 (Fahrlog): 0B/80 = 1918, 07/A0 = 3297, 07/80 = 856, 04/A0 = 10, 30/E0 und 33/E0 je 3.
- 598b197c (Laden): 07/20 = 4356, 07/A0 = 1105, 87/80 = 48, 87/A0 = 32, 07/80 = 27, 04/A0 = 11, 87/20 = 5.
- ab5f2c05 (Ladelog): 07/20 = 2970, 07/A0 = 2160, 87/80 = 55, 87/A0 = 52, 07/80 = 33, 04/A0 = 21, 87/20 = 5.
- Zuordnung: 80 im Fahrlog bei 0B (Zuendung/GO); 20 nur in den beiden Lade-Logs; A0 Ruhe; E0 beim Wecken (30/33). Bei B5 = 20 steht B3 = FE, bei A0 steht B3 = FD (dc664c4d).

## B3 in dc664c4d (B0 B1 B3 B5 : Anzahl)
07 1E FE 20 : 2198 | 07 1E FD A0 : 2160 | 07 1F FE 20 : 763 | 87 1E FD A0 : 52 | 07 1E FD 80 : 30 | 04 1E FD A0 : 21 | 87 20 FE 80 : 21 | 87 22 FE 80 : 4.

## Folgerung
- Fuer das Balancing ergibt sich daraus nichts.
- Unser Treiber bildet das schon ab (BUS.h, Tabelle 0x423): die Zeile "laden" sendet `07 1E FF FE 5D 20 5D DA` bzw. `.. B2 20 B2 DA` (B3 = FE, B5 = 20, B4 = B6 wie im Ladelog); die Zeilen zu/wach/zuendung/go/fahrt/aus haben B4 und B6 gegenlaeufig wie im Fahrlog. "A0/41" im Kommentar zu BUS_LADEN meint 0x426, nicht 0x423.
- B7 ist in BUS.h als "Pruefwert unbekannt, wie aufgezeichnet gesendet" geführt (Zeile 19); diese Messung bestaetigt, dass es keine einfache XOR-/Summenpruefsumme ist.
