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
