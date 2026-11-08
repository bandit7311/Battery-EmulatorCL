# Zoe Ph2 ZE50: HEV-CAN-Logs, Strom-Frames, fehlende Frames bei dala, DTC loeschen (08.11.2026)

Kennzeichnung: **gemessen** = im Log nachgerechnet, **Schluss** = gefolgert, **Annahme** = nicht belegt.
Quellen: Repo ljames28/Renault-Zoe-PH2-ZE50-Canbus-LBC-Information (README, `Can_frames.xlsx`, `example-code.py`, 11 Logs unter "Canbus Logs"), dala-Stand 2621aaf (05.11.2026), PR #2824 (Zweig `bugfix/renault-zoe2-fallback`, Kopf 7f03d14a), unsere Logs vom 09.10./11.10./16.10.

## 1. Was die Zoe-Logs sind
- README: aufgenommen im Auto auf dem **HEV-CAN nahe der Batterie** (LBC 938, HEVC 946, PEC 2325). Das ist das Gegenstueck zu unserem Bus LBC<>EVC. 29-Bit-Frames sind aus den Logs entfernt.
- `open_flap_charge.log` ist byte-identisch mit `charge_to_100.log`.
- Stromsensor: Tabelle Blatt "CAN1": `0x3C2`, 10 ms, Absender CURRENT, "LBC Current sensor on can1 only" (nur intern im Pack). Auf dem HEV-CAN ist er nicht zu hoeren.

## 2. Strom-Frames
| Twingo | Zoe Ph2 | Beleg |
|---|---|---|
| 0x155 (Pack-Strom + SOC) | **0x0F8** B4-5 (Spannung B2-3) | Tabelle "Current/Voltage/Interlock" (LBC); gemessen: faellt zum Ladeende (53,7 s), steigt zum Ladebeginn (41,10 s), Rauschen <1 Zaehler |
| 0x19F (EVC, 12-Bit-Wert um 0x7D0, Zaehler, eigene CRC) | **0x0F5** | siehe Abschnitt 4; Schluss |
| 0x0C5 B2, 0x445 B5 (grobe Kopien, ca. 0,1-0,15 s nach 0x155) | 0x612, 0x7CE B4-5 (nur CAN1) | Tabelle |
| 0x157 (Soll-Strom, Rampe, laeuft dem Strom voraus) | kein sicheres Gegenstueck | **0x0EF B6:B7** zeigt die Rampe (10 -> 4650, 145 Werte), beginnt 0,12 s vor dem Strom; aber laut Tabelle vom LBC **nicht verarbeitet** |

- Zoe 0x0F8 B4-5: Ruhe 48757 (konstant), Laden ca. 48865-48881 (Ladeende), beim Ladebeginn Rampe auf ca. 49060. Skalierung unbekannt ("/8" nach dala).
- Zoe 0x0EF B6:B7: Ladebeginn 10 -> 4650 in 4 s; Ladeende faellt 2474 -> 10 bei 53,69 s (0,02 s vor 0x0F8) bzw. 1226 -> 874 -> 554 -> 458 (508,4-508,6 s) zeitgleich mit 0x0F8.
- Zoe 0x0F5 B6:B7 steigt beim Ladebeginn gleichzeitig mit 0x0F8 (+50..+100 Zaehler, Rauschen +-17), fuer eine Zeitverschiebung nicht aufloesbar.
- Alle anderen Frames mit Rampe wechseln **nach** dem Strom: 0x144 (+0,23 s), 0x0FB (+0,27 s), 0x383 (+0,36 s), 0x4DB (+1,0 s).
- Zustandsbytes beim Ladeende: 0x4CE B0 `5F`->`1F` (+1,65..2,4 s), 0x029 B0 `C5`->`00` (+2,5..3,2 s).

## 3. Zoe 0x0F5 (8 Byte, 10 ms) - Aufbau, gemessen
- B0 `7D` (selten `7C`/`7B`), B1 unteres Nibble Zaehler 0-15 (+1), B1 oberes Nibble meist 0.
- **B2 = CRC-8, Polynom 0x1D, Start 0, ueber B0,B1,B3..B7 (B2 ausgelassen), XOR 0x16** - 56.764 von 56.764 Frames aus drei Logs.
- B3 `FF`; B4/B5 `D7`/`F8` (Ruhe, Laden), `DE`/`B0` (Zuendung an/Fahrt); B6 `7D`, B7 08..F8 in Schritten von 8 (Median Ruhe 30, Laden 58).
- Das Beispielskript des Autors (`example-code.py:455-457`) sendet `[0x7C, counter, CRC, 0xFF, 0xD7, 0xF8, 0x7D, 0x10]`.
- Tabelle: "Current?", "Defaults 0x07D0 or 0x4000 if bad. IDK where it goes."

## 4. Zoe 0x0F5 <-> Twingo 0x19F (Schluss)
- 0x19F: V = B2<<4 | B3hi, Ruhe 0x7D0 (47.037 von 60.723 Frames), Wake-Rampe ab 0x42D, ungueltig 0xFFF; Zaehler B3 unten (+5), CRC B5 (Poly 0x2F). Im Fahrlog schwache Korrelation mit dem Pack-Strom (|r| bis 0,5 bei 1 s). Am Bench verschwand E14281 mit 0x19F.
- 0x0F5: gleicher Ruhewert 0x7D0 (B0<<4|B1hi), Zaehler, eigene CRC, 10 ms.
- Folge (Annahme): U214281 ("CAN from Inverter") koennte bei der Zoe Ph2 an 0x0F5 haengen.

## 5. Was dala (Zoe Gen2) sendet und was fehlt
- dala sendet: 0x0EE, 0x373, 0x375, 0x376, 0x5F8, 0x6BF (0x133 auskommentiert, Zeile 423).
- Beispielskript sendet zusaetzlich: **0x0F5, 0x133, 0x440, 0x4CE, 0x4FB** (laut Tabelle vom LBC verarbeitet).
- Inhalt (gemessen, CRC-8 Poly 0x1D):
  - 0x4CE: B0 Ladeart (`1F` Ruhe, `5F` Laden, `7F` Fahrt), B1-B3 `FF`, B4 `DF`, B5 `FF`, B6 `E`+Zaehler, B7 CRC (XOR 0x0A, ueber B0-B6): 7214/7214.
  - 0x440: B0 `00`, B1 `31` (eingerastet) / `32` (Einschlafen), B2 CRC (XOR 0xAB, ueber B0,B1,B3-B7), B3 oben Zaehler, B4 `1F` (Relais an) / `9F` (aus), B5 B6 `FE FF`, B7 `94`: 7214/7214.
  - 0x133: B0 B1 `00 7F` oder `B1 A6`, B2 Zaehler<<2, B3 CRC (XOR 0x4E, ueber B0-B2,B4-B7), B4 B5 `00 06`, B6 B7 Geschwindigkeit (`00 00`, `00 03`, `FF FF` ungueltig); Laden immer `B1 A6 .. FF FF`: 72.146/72.148.
  - 0x4FB: DLC 6, B0 = `0x80`+Zaehler, B1 `00`, B2 `04`, B3 B4 `00 00`, B5 CRC (XOR 0x82, ueber 7 Byte mit Nullen aufgefuellt): 7196/7196. Tabelle: `04` = Relais an, `02` = aus, `00` = Fehler ("Relay 4").
- README Zeile 11: Log abspielen -> Akku "confused" und gleicht nicht ab; noetig: "correct timing information and correct checksums". **Kein Beleg**, dass ohne 0x0F5 ein Fehler entsteht.

## 6. DTC-Zuordnung (Annahme, nicht getestet)
| Fehler | Bei uns gesehen? | Moeglicher fehlender Frame |
|---|---|---|
| U214381 (E14381, "CAN from EVC/HEVC", Typ 81) | ja | HEVC-Frames 0x133, 0x4CE |
| U214281 (E14281, "CAN from Inverter") | ja (weg mit 0x19F) | 0x0F5 |
| U214287 (Typ 87 = fehlende Nachricht) | nein | wie oben |
| P1B2081 ("Relay 4") | nein | 0x4FB |
- dalas JSON `renault_zoe_gen2_dtc.json` kennt diese 6-stelligen Codes nicht (110 Eintraege mit 5 Zeichen). Der Text "CAN-Stromsensor-Frames" steht weder in dalas JSON noch in unseren Notizen.
- Vorschlag: Frames einzeln zuschalten (0x4FB, 0x4CE, 0x133, 0x0F5, 0x440) und nach jedem den Fehlerspeicher lesen.

## 7. DTC loeschen: PR #2824 gegen unseren Bench
- dala (PR-Stand): `reset_DTC()` -> `start_sequence(UDS_STATE_CLEAR_DTC_START)` -> `0x14 FF FF FF` an `uds_address` (nur `0x18DADBF1`, MCPU), **ohne vorherige Session**, `UdsCanBattery.cpp:428-435, 552-554`. Positive Antwort `0x54`: "nothing to do". Die lokale Liste (`dtc_count`/`dtc_codes`/`dtc_status`) wird nur in `handle_dtc_response` beschrieben; nach dem Loeschen **kein automatisches Neulesen** -> die Seite zeigt nach `reload_after` die alte Liste (Schluss).
- Wir: MCPU `0x18DADBF1`/`0x18DAF1DB`: Session `10 03`, nach 100 ms `04 14 FF FF FF`, Antwort `54` (`handle_dtc_ext`, `BATTERY.cpp:4081`); SCPU `0x18DADCF1`/`0x18DAF1DC`: `14 FF FF FF` per freier Anfrage Ziel 1 (`:4077`). Beides am Bench bestaetigt (MCPU 14.10., SCPU 08.10.: danach "0 DTC"/"no DTC"). Keine Auto-Auslese nach dem Loeschen.
- SCPU-Adressumschaltung bei uns: Absender-ID der Vorlage `ZOE_POLL_18DADBF1` und der Flow-Control-Vorlage wird fuer die Dauer einer Anfrage auf `0x18DADCF1` gesetzt (`:4189-4190`), danach zurueck (`uq_restore_poll_template`); Antworten `0x18DAF1DC` werden nur waehrend dieser Anfrage angenommen (`:2550`). Die dala-Oberklasse hat nur ein Adresspaar.
- Offen: ob der Pack `14` ohne Session annimmt (nicht getestet), ob aktive Fehler sofort zurueckkommen (bei uns offen: 1B0E41 MCPU, 1BB587 SCPU).

## 8. Gesendete Dateien (nicht im Repo)
Zoe Gen2 mit 0x0F5, Basis dala-Original 2621aaf: `RENAULT-ZOE-GEN2-BATTERY.h`, `.cpp`, `patch.diff` (20 Zeilen hinzugefuegt, keine entfernt). Startwerte `7C 00 00 FF D7 F8 7D 10` wie im Beispielskript; Zaehler `counter_10ms`, CRC wie in Abschnitt 3 (56.764/56.764 getestet, g++ mit dalas CRC-Tabelle, Gesamtprojekt nicht gebaut, am Pack nicht getestet).
