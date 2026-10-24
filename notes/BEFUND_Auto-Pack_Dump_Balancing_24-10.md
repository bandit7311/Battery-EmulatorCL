# Befund: MCPU/SCPU-Dump des Auto-Packs (Recycler-Pack), Balancing-Bits bei 5 mV (24.10.2026)

Quelle: zwei Dateien vom Nutzer (`RBMS_MCPU_RL_06xx_V1.7`, `RBMS_SCPU_RL_06xx_V1.2`), abgelegt als `DUMP_MCPU_Auto-Pack_24-10.txt` und `DUMP_SCPU_Auto-Pack_24-10.txt`. Der Dateiname nennt die Dekodier-Vorlage, nicht den Aufnahmetag. Auswertung: `tools/log_ana_mcpu_dump_balancing.py`.
Kennzeichnung: **gemessen** = steht im Dump, **Schluss** = abgeleitet, **Annahme** = ungeprueft.

## 1. Welcher Pack
- **Gemessen:** `901B` = `295108369RTY032271FFFFFFF`, `F187` = `293A09578R`, `9282` = `...T2129902 01PDA009AR000`: der Recycler-Pack aus dem Auto (nicht der Original-Pack vom Juli 2025, `...TY008870`).
- **Schluss:** `91C1` = 2.206.569 min, am 11.10. 2.187.978 min; Differenz 18.591 min = 12,9 Tage, Aufnahme also etwa 24.10.

## 2. Zustand beim Dump
- **Gemessen:** `9259` = 05 und `925C` = 01 (laut Foto-Tabelle Zuendung/Go/Driving, Pack wach), in MCPU und SCPU gleich. SCPU-`9261` liegt 78 Einheiten (min) ueber MCPU-`9261`; **nicht** als Lesezeitpunkt zu deuten, siehe Nachtrag 8.
- **Gemessen:** SOC `9001` = 3759 (36,5 % nach der Skalierung des Juli-Dumps), Packspannung `9005` 347,7 V, Summe der 96 Zellen 347,78 V (`9006` 347,66 V).
- **Gemessen:** Zellen 3,6201 bis 3,6250 V (Rohwert/1024), Spreizung 4,9 mV aus der Zellliste, `9007`/`9009` 6,8 mV; SCPU: 3,9 mV (Zeitabstand zum MCPU unbekannt).
- **Schluss:** Temperatur `9012` = 792: (Rohwert/16) - 40 = 9,5 C (Skalierung am Juli-Dump geprueft: 1018 -> 23,625 C).

## 3. Balancing-Schalter `912B`
- **Gemessen:** Datenbytes 21-32 = `B9 81 60 46 88 08 C0 00 00 26 00 00`, 20 von 96 Bits gesetzt.
- **Annahme:** erstes Bit = Zelle 96 (Seitenlogik des Treibers). Dann sind Zellen 18, 19, 22, 47, 48, 52, 60, 64, 66, 67, 71, 78, 79, 81, 88, 89, 92, 93, 94, 96 am Balancieren.
- **Gemessen:** nur 2 der 20 Zellen gehoeren zu den 20 hoechsten Zellen. Bloecke zu je 12: Bits 0/3/0/2/2/4/3/6 (Block 1 bis 8), mittlere Hoehe ueber dem Minimum 1,4 / 2,4 / 2,4 / 3,0 / 2,4 / 2,7 / 2,9 / 3,1 mV. **Schluss:** schwacher Hinweis auf hoehere Zellen, bei 1 mV Aufloesung nicht belastbar.

## 4. Zaehler
| | 11.10. | 24.10. (Dump) |
|---|---|---|
| `9251` Schlaf | 130,00 Ah | 131,641 Ah |
| `9252` Schlaf | 222,11 h | 225,277 h |
| `924F`/`9250` gesamt | gleich Schlaf | gleich Schlaf |
| `9262`/`9263` Wach | 0 / 0 | 0 / 0 |
- **Gemessen:** in 12,9 Tagen +1,64 Ah und +3,17 h, nur im Schlaf-Zaehler. Der Wach-Zaehler bleibt 0, obwohl wach 20 Bits gesetzt sind.

## 5. SCPU
- **Gemessen:** nur 59 Werte, keine Balancing-Daten, kein Fehlerspeicher. `F187` = `293A05180R`, `F191` = `293A01341R`.

## 6. Folgerungen
- **Schluss:** Dieser Pack balanciert (Bits gesetzt) bei 36 % SOC und 5 mV Spreizung. Die Annahme "ab etwa 97 % SOC und 24 mV" (Zoe-Dokument, andere Firmware) trifft hier nicht zu.
- **Schluss:** Der Wake-Test bei >= 97 % ist nicht die entscheidende Probe. Besser eine Dump-Serie am Auto (siehe `MERKLISTE_Test.md`, Abschnitt "Dump-Serie am Auto").
- **Offen:** ob die Wach-Bits am Auto dasselbe sind wie die Bench-Bits (25.09., 76 Bits bei SOC 97 %, Zaehler 0); warum der Wach-Zaehler trotz Bits 0 bleibt (Vermutung: Zaehler nur aus dem Schlaf-Plan, nicht belegt).

## 7. Nachtrag: PyRen-Seiten 15 und 16 derselben Sitzung (24.10.)
- **Gemessen:** PyRen `PR280` "Absolute time since first ignition" = 2.990.951 min = `9261` des MCPU-Dumps: Dump und PyRen-Seiten stammen aus derselben Zuendungssitzung (nicht zwingend aus derselben Minute, siehe Nachtrag 8). `PR339` (BMS) = 5 = `9259`, `PR342` (Relay) = 1 = `925C`.
- **Gemessen:** SOC laut PyRen `PR297` = 34,29 %. Mein Wert 36,5 % aus `9001` beruht auf einer Skalierung, die nur an einem Punkt (Juli-Dump 98,92 %) abgeleitet ist. **Der PyRen-Wert gilt, die 36,5 % sind unsicher.** `PR340` "#BATTERY USOC" = 0,10 %, Bedeutung unbekannt.
- **Gemessen:** Strom `PR337` = -0,52 A (Dump `9284` = -0,59 A): der Strom ist unauffaellig. `PR218` zeigt -1020 A, also ein Ungueltig-/Grenzwert dieses Signals (kein Stromsensor-Befund ableitbar).
- **Gemessen:** `PR278` "Average voltage of cells" = 5,11 V und die daraus gebildeten Differenzen +-1,49 V sind offensichtlich Skalierungs-/Signalfehler der PyRen-Definition (hoechste Zelle 3,62 V stimmt: 5,11 - 1,49).
- **Gemessen:** Fahrzeug-km `925F` = 93.214 km (PyRen nennt sie "Mileage since last appearance of fault"); Pack-km `PR046` = 25.328 km; Wasser 12 C, Luefter 0.

## 8. Nachtrag: zweiter MCPU-Dump derselben Sitzung (24.10., `DUMP_MCPU_Auto-Pack_24-10_zweiter.txt`)
- **Gemessen:** `9261` und `91C1` sind in beiden MCPU-Dumps gleich (2.990.951 / 2.206.569 min), obwohl Spannung und SOC-Wert dazwischen gesunken sind. **Schluss:** diese beiden Zaehler werden nicht laufend aktualisiert (vermutlich beim Wecken festgehalten). Damit ist "derselbe Zeitpunkt" aus gleichem `9261` nicht ableitbar, und der SCPU-Wert 78 min hoeher heisst nicht, dass der SCPU-Dump 78 min spaeter gelesen wurde. Der zweite Dump passt zu den PyRen-Seiten (`9005` 347,4 V = `PR244` 347,44 V, `9284` -0,516 A = `PR337` -0,52 A).
- **Gemessen:** `912B` ist in beiden Dumps **identisch** (`B9 81 60 46 88 08 C0 00 00 26 00 00`, dieselben 20 Zellen). Die Bits blieben also in diesem Zeitabstand stehen. Der Zeitabstand ist nicht bekannt.
- **Gemessen:** Zaehler `924F` bis `9252` und `9262`/`9263` gleich; Zellsumme 347,78 -> 347,52 V (mittlere Aenderung -2,7 LSB je Zelle, 1 LSB = 0,98 mV), Spreizung 4,9 -> 6,8 mV, Strom -0,594 -> -0,516 A.
- **Gemessen:** die 20 Bit-Zellen sanken im Mittel um 2,45 LSB, die uebrigen 76 um 2,80 LSB. **Schluss:** bei 1 mV Aufloesung kein belastbarer Unterschied; ein Beleg, dass die Schalter wirklich Strom ziehen, ist das nicht.

## 9. Nachtrag: Zeitstempel, dritter MCPU-Dump und Busmitschnitt (24.10.)
- **Gemessen:** Das Praefix der Dump-Dateinamen ist die Unix-Zeit der Aufnahme (im Juli-Dump 1753382968 = 24.07.2025 18:49 UTC, passt zu den Notizen). Damit: MCPU 1 = 14:24:52, SCPU = 14:26:04, MCPU 2 = 14:48:41, EVC = 15:01:18, MCPU 3 = 15:15:59 UTC. Die Aussage "Zeitabstand unbekannt" aus Nachtrag 8 ist damit ueberholt.
- **Gemessen:** `912B` ist in allen drei MCPU-Dumps (51 min) **identisch**, ebenso in der Antwort im Busmitschnitt `teraterm.log` bei t = 218,2 s (dieselben 20 Bits). Zaehler `924F` bis `9252`, `9262`/`9263` unveraendert, `9259` = 05, `925C` = 01.
- **Gemessen:** Zellsumme 347,78 -> 347,52 -> 347,37 V, Spreizung 4,9 / 6,8 / 5,9 mV, `9001` 3759 / 3725 / 3690, Strom -594 / -516 / -505 mA. Mittlere Zelldrift in 51 min: -4,3 LSB (1 LSB = 0,98 mV), Bit-Zellen -4,30, andere -4,34: **kein messbarer Unterschied**.
- **Gemessen:** Die Bit-Zellen gehoeren in den drei Dumps zu den 20 hoechsten Zellen: 2 / 5 / 3 von 20 (Zufall waere etwa 4). **Schluss:** Die Bits folgen nicht der aktuellen Zellrangfolge. Vereinbar mit einem Bitmuster, das nicht laufend aus den Zellspannungen neu berechnet wird (zum Beispiel beim Wecken); nicht belegt.
- **Gemessen (Busmitschnitt `5fba543f-teraterm.log`, 304 s, Auto wach):** 0x424 Byte 0 = 0x11 (damit "Cell Under Voltage: OK"), 0x155 B0 = 0x49 (21,9 kW), SOC 26,14 -> 26,09 %, Strom -0,25 A, U 347,5 V; UDS-Sitzung 122,9 bis 383,0 s mit 630 Antworten (335 verschiedene DIDs).
- **Gemessen, drei SOC-Werte im Abstand von 25 min:** PyRen `PR297` 34,29 % (etwa 14:48), EVC `$2002` 24,06 % (15:01), 0x155 im Busmitschnitt 26,1 % (etwa 15:15 bis 15:20). Unterschiede nicht erklaert.

## 10. Nachtrag: EVC-Dump 15:01 UTC (`DUMP_EVC_Auto_24-10.txt`)
Dekodiert mit den Namen/Skalen der EMS3194-XML (die zum Dump gehoerige `EVC_3180_RH3_700_V3.0` liegt nicht vor; 467 von 468 DIDs haben dort einen Namen).
- **Gemessen:** HV 347,5 V, Rack-Temperatur 10 C, SOH 95 %, verfuegbare Ladeleistung `$320B` 43 kW (= MCPU `900E`), Entladeleistung `$320A` 71,5 kW, max. Ladeleistung per CAN `$3444` 21,9 kW (= 0x155 B0 und MCPU `9018` = 22,00 kW). Die zwei Ladegrenzen (0x424 B2 gegen 0x155 B0 / `9018`) sind im EVC beide sichtbar; Bau 28 nimmt das Minimum.
- **Gemessen:** Ladehistorie `$33D7` (Skala 0,2 %): 99,8 / 81,6 / 99,2 / 61,4 / 100,0 / 98,8 / 41,2 / 37,4 / 35,8 / 31,2 %; Ladeende-Grund `$33D5` (nur 5 Eintraege): 0, 1, 0, 1, 0 (0 = "Nominal end of charge (battery full)", 1 = "Unplugged"); Dauer `$33D9` 379, 21, 111, 37, 7, 87, 4, 1, 5, 25 (Einheit angenommen min). **Schluss:** der Auto-Pack wird regelmaessig voll geladen (Annahme: gleiche Reihenfolge in den Listen).

## 11. Nachtrag: 79B/7BB und 18DA15F1 (mod_ddt-Liste)
- 79B/7BB (11 Bit, LBC EV, `BMS453`): antwortet bei unserem Pack nie (Notiz 17.10., Treiberkommentar `BATTERY.h:886`); in den hochgeladenen Mitschnitten gibt es keinen einzigen 0x7BB-Frame, `0x79B` wird nur von uns gesendet (`02 21 07/41/42`).
- `18DA15F1`/`18DAF115` (ECM_29): nur als Listenzeile in `batteryemulator_vs_ovms_031026.md` erwaehnt, nie als Frame auf dem Bus.

## 12. Nachtrag: Zellwerte Broadcast gegen UDS im selben Mitschnitt (24.10., `teraterm.log`, 304 s)
Frage vom Anfang: Warum kann ein Zellwert per UDS anders sein als im Broadcast?
- **Gemessen:** Die UDS-Sitzung im Log (t = 122,9 bis 383,0 s, 630 Antworten, 335 DIDs) stimmt mit dem dritten MCPU-Dump in allen 335 DIDs inhaltlich ueberein. Damit liegen UDS-Werte und Broadcast desselben Zeitraums vor.
- **Gemessen:** Je UDS-Zellwert (`9021` bis `9083`, Rohwert/1,024 in mV) gegen den zeitlich naechsten Broadcast-Wert derselben Zelle (Zellframes, Wert = Rohwert + 2000 mV): Broadcast im Mittel **2,8 mV niedriger**, Streuung 1,8 mV, groesste Abweichung 9,2 mV; 29 von 96 Zellen liegen innerhalb 2 mV.
- **Gemessen:** Der Broadcast rauscht: Zelle 1 liefert ueber 304 s 3610, 3612, 3613, 3615, 3617 mV; mittlere Spanne einer Zelle 5,2 mV, groesste 8 mV. Spreizung bei t = 250 s: Broadcast 7 mV (3610 bis 3617), UDS 5,9 mV (3615 bis 3621).
- **Schluss:** Kleine Abweichungen (einige mV) sind normal: fester Versatz von etwa 3 mV plus Rauschen im Broadcast. Eine grosse Abweichung erklaert das nicht.
- **Nicht entscheidbar:** Ob Broadcast-Zelle n gleich UDS-Zelle n ist. Bei 6 mV Spreizung schneiden auch die umgekehrte und eine um 1 verschobene Nummerierung gleich gut ab (Streuung 2,5 und 1,7 mV gegen 1,8 mV). Dafuer braucht es einen Mitschnitt mit grosser Spreizung (zum Beispiel am vollen Pack).
- Gilt fuer den Auto-Pack wach bei etwa 34 % SOC; fuer den Bench-Pack bei hohem SOC nicht gemessen.
