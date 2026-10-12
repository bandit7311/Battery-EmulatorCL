# Quelle "dj2cs": Frame-Auswertung und Zeitpruefungen des Akkus (Nutzer, 12.10.2026)

**Quelle:** "dj2cs" (Angabe des Nutzers). Was das ist (Person, Repo, Tool) und ob es aus der Akku-Firmware stammt, ist nicht bekannt; die "Getter"- und Pruefadressen sind von uns nicht pruefbar. Alles unten ist Hypothese, soweit nicht ausdruecklich als gemessen gekennzeichnet. Abgleich nur gegen unsere Aufnahmen (`BUS.h`, Auto-Dumps) und Bench-Messungen.

## Tabelle 1: ausgewertete Frames (wie uebergeben)

| ID | DLC | Bytes | Inhalt | Getter | Invalid |
|---|---|---|---|---|---|
| 0x436 | 4 | B1,B2,B3 | Absolutzeit Fahrzeug, 24 bit BE, Minuten | 0xB3D2C | >= 0x00FFFFFF |
| 0x426 | 6 | B3,B4,B5 | zweiter Minutenzaehler (Boost/Transition) | 0xB3D84 | >= 0x00FFFFFF |
| 0x426 | | B1 b7:5 / B2 b1:0 | Status; B2-Feld == 2 = aktiv | 0x81D50 | 7 / 3 |
| 0x69F | 4 | B0..B3 | Fahrzeug-ID, intern (v & ~0xF) + 0xF -> 28 bit wirksam | 0xB3CC4 | 0xFFFFFFFF |
| 0x423 | 7 | B0 b1:0 | == 3 blockiert die komplette Auswertung | 0x81B74 | - |
| 0x19F | 4 | (B2<<4)\|(B3>>4) | 12 bit, phys = raw*10 - 20000 | 0xB3DDC | >= 0x0FFF |

## Tabelle 2: Pruefungen (wie uebergeben)

| Pruefung | Adresse | Inhalt |
|---|---|---|
| Invalid-Sentinel | 0x8FD1C | Wert >= 0x00FFFFFF => verworfen |
| Monotonie zyklisch | 0x8FEB8 | Delta nur wenn t_neu > t_alt, strikt |
| Monotonie Hochlauf | 0x8FD6C | nur wenn t_jetzt >= NVM-Wert |
| Gleiches Fahrzeug | 0x8FEC0 | Delta nur wenn gespeicherte ID == aktuelle ID |
| Standzeit lang | 0x8FAE0 | >= 470 min => Langzeit-/OCV-Pfad |
| Standzeit kurz | 0xA0158 | >= 30 min / >= 1 min => andere SOC-Pfade |
| Saettigung | 0xCDA94 | Akkumulatoren klemmen bei 0x00FFFFFF |

## Abgleich mit unseren Daten

**Passt (gemessen):**
- `0x436` B1-B3 = Minuten, 24 Bit (SCPU-Zeit, letzter Wert im Auto-Log).
- `0x19F`: in allen 8 aufgenommenen Zustaenden B2 = `7D`, B3 oben 0, also raw 2000 und nach der Formel 0; die Weck-Form `FF FF FF Fx` ist raw 0xFFF (ungueltig).
- `0x69F` = `46 13 88 6F`: B3 hat im unteren Halbbyte schon `F`, die Formel aendert den Wert nicht.
- Die fuenf Frames (0x436, 0x426, 0x69F, 0x423, 0x19F) sind genau die Hauptkette der Bench-Zeilen. Die uebrigen EVC-Zeilen (0x157, 0x1A1, 0x1C7, 0x0EC, 0x0ED, 0x428, 0x42F, 0x435, 0x4F7, 0x500, 0x511) kommen in der Tabelle nicht vor; ob sie vollstaendig ist, ist unbekannt.
- `9261 - 91C1` ist im Auto in allen Dumps 784.382 (Nov. 2025: 786.436); das passt zu einer Packzeit, die nur gueltige Zuwaechse der Fahrzeugzeit summiert.
- `925E` stand am Bench bis 11.10. auf `00 00 00` (gespeicherte ID 0), danach `13 88 6F`; in dieser Zeit stieg `9261`/`91C1` nicht.

**Widerspricht / unklar:**
- `0x426` B3-B5 "zweiter Minutenzaehler": bei uns Kilometer (Bench: `01 1A BC` = 72.380 im Frame, `925F` = 72.380,00 km). Im Auto-Log `01 6A 70` = 92.784, `925F` dort zuletzt 92.677 km (107 km Abweichung, ungeklaert).
- `0x423` B0 Bit 1:0 `== 3` blockiert: in allen 8 Zustaenden aus dem Auto hat `0x423` B0 = `07`/`0B`, also immer `== 3`.
- `0x426` Status: B2-Feld 2 in "zu" und "wach", 1 in Zuendung/Fahrt/aus/Laden; mit "wach" ging die MCPU am Bench auf `05/01` (passt zu "2 = aktiv"). Feld 3 kommt in den Wake-Schritten `00/03` und `60/63` aus dem Auto-Log vor (laut Tabelle ungueltig).

**Nicht belegt:**
- Ob eine dieser Pruefungen E14381 ausloest (vom 02. bis 05.10. war der gespeicherte Wert 0 und E14381 trotzdem aktiv).
- Dass die Standzeit-Pfade den SOC-Sprung nach unserem Sleep (USOC 35,85 -> 42,37 %) erklaeren (Vermutung).
- Welche Bedingung am Bench das Speichern von `9261` verhindert.

## Nachtrag 12.10. abends: Beispielframes und Aussage von dj2cs (Screenshot des Nutzers)

Wie uebergeben (Text im Screenshot, Frame = ID [DLC] Bytes, dazu die Anmerkung):
- `0x436 [4] 00 C7 AF FC` - B1..B3 = 13.086.716 min, monoton.
- `0x69F [4] 0A 1B 2C 30` - Fahrzeug-ID, konstant (unteres Nibble egal).
- `0x426 [6] 00 00 02 C7 AF FC` - B2&3 = 2 (aktiv), B3..B5 = zweiter Zaehler (hier derselbe Wert wie in `0x436`).
- `0x423 [7] 00 00 00 00 28 00 00` - B0&3 = 0, keinesfalls 3.
- `0x19F [4] 00 00 7D 00` - raw 2000 -> phys 0.
- Aussage: "In deinem Mitschnitt ist auch nur der eine Zeitgeber vorhanden, daher wuerde ich sagen, du machst alles richtig und das zeigt auch, dass die DTC verschwunden ist."

**Wer dj2cs ist und ob die Beispielframes aus einem eigenen Aufbau oder aus Deutung unseres Mitschnitts stammen, ist nicht bekannt.** Der Name steht in keinem frueheren Dokument; ob es der Freund mit dem Zoe-Ph2-Emulator (zip `2d77503c`, 11.10.) ist, ist nicht belegt.

**Abgleich:**
- `0x19F`: gleich (B2 = `7D`, B3 oben 0, raw 2000).
- `0x426` B3..B5: bei uns Kilometer (Auto-Aufnahme `01 6A 70` = 92.784, Bench `01 1A BC` = 72.380 und `925F` = 72.380,00 km), nicht der Wert von `0x436`. Im Beispiel von dj2cs steht dort derselbe Wert wie in `0x436`. Wir haben nie `0x426` B3..B5 gleich `0x436` B1..B3 gesendet.
- `0x423`: bei uns (Auto-Aufnahme, alle 8 Zustaende) B0 = `07` oder `0B`, also `B0&3 = 3`; im Beispiel B0 = `00`. Widerspruch zwischen "keinesfalls 3" und der Auto-Aufnahme; E14381 ist am Bench trotz `B0&3 = 3` nach dem +1000-Test und nach dem kalten Neustart ausgeblieben.
- `0x436`: B0 = `00` im Beispiel, bei uns `80`/`8A`/`AC`/`AD` je Zustand.
- `0x69F`: andere ID, ohne Bedeutung fuer den Vergleich.

**Moeglicher Test (nicht gebaut):** `0x426` B3..B5 auf denselben Wert wie `0x436` setzen und `0x423` B0 auf `B0&3 = 0`; nur einzeln und mit E14381-Lesung (`19 02 FF`) vor und nach jedem Schritt. Ob das noetig ist, ist offen, da E14381 gerade nicht ansteht.

## Nachtrag 2 (12.10. abends): Einschlaf-Anforderung ueber 0x423 und NVM-Schreiben

Aussage von dj2cs (Text des Nutzers): Einschlaf-Anforderung per CAN; `0x423` B0 Bit 1:0 == 0 (Adresse 0x81C20) => ECU-State 8 "Shutdown pending", Bit 5:4 waehlt die Abschaltart; `0x426` 2 s aus erzwingt denselben State (0x82E00 -> 0x804A0); danach 1 s vollstaendige Busstille (0x811D4, 100 Zyklen der 10-ms-Task, Zaehler 0x400000C0, Flag 0x40001CDE), CAN-Timeout-Flags dabei unterdrueckt (0x80360 ff., sonst Fatal-Code 0x38); in dem Fenster schreibt das Steuergeraet sein NVM; 0xCFEFC ist der einzige NVM-Schreiber (kein zyklischer Writer); geschrieben werden u. a. Fehlerspeicher (Block 0xA6, 1444 B), SOC/SOH-Historie (0xA0), Zaehler (0xAB, 0xAA, 0x19), Block 0x18 mit Fahrzeug-ID und AbsTime (0xCFC4C -> 0xCBB28).

**Abgleich mit den Auto-Logs (gemessen):** `0x423` B0 geht in allen drei Logs fuer die letzten 10-11 Frames (rund 1 s) auf `04`; der Akku sendet ab genau diesem Moment nicht mehr; die EVC-Frames laufen noch etwa 1 s. Einzelheiten, Zeiten und Wake-Frames `30`/`33`: `BAULISTE.md`, Bau 7. Die NVM-Angaben (Bloecke, Adressen, ein Schreiber) sind von uns nicht pruefbar. Die frueheren Aussagen der Tabelle (`0x423` B0 & 3 == 3 blockiere die Auswertung) sind damit weiter ungeklaert; die Logs zeigen `& 3 == 3` in allen Phasen ausser dem Ende und dem allerersten Wake-Frame.
