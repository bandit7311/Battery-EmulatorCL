# Befund EVC-Dump a80d130b und 0x423 Byte 0 (17.10.2026)

Nur eine Notiz, nichts gebaut. Vollstaendige Tabelle: `EVC_Dump_a80d130b_dekodiert.md` (683 DIDs).

## Quellen
- Dump: `a80d130b-…EMS3194_0A40_300_V2.1…txt` (gleich `b5795956`), 04.11.2025 13:15 UTC, Auto wach, 683 Antworten.
- Datenbank: `36c02ace-EMS3194_0A40_300_V2.1_20200730T160049.xml`. Alle 683 DIDs des Dumps sind darin definiert.
- Die Dumps `…EMS3194_0A40_100…` (4 Dateien, gleiche Pruefsumme) haben keine Antworten.
- `…EMS3194_0A40_500…xml` (8ca9a879) hat dieselben LBC-Signale. EVC-Basis laut Kopf: EVC_3180_RH5_130.
- Bitzaehlung: BitOffset ab MSB des ersten Datenbytes (geprueft an $3202=05 und $343F).

## 1. Pack -> EVC: Skalierung passt zu unserem Treiber (Zuordnung aus Einheiten, nicht gleichzeitig gemessen)
| EVC-DID | Rohwert | Wert | Unser Frame |
|---|---|---|---|
| $3444 Ladeleistung max. (0,3 kW) | 2F | 14,1 kW | 0x155 B0 x 300 W |
| $2002 SOC (0,02 %) | 0BB3 | 59,9 % | 0x155 SOC-Rohwert x 0,0025 (Faktor 8) |
| $320B verfuegbare Ladeleistung (0,5 kW) | 56 | 43 kW | 0x424 B2 x 500 W |
| $320A verfuegbare Entladeleistung | 90 | 72 kW | 0x424 B3 x 500 W |
| $3206 SOH | 60 | 96 % | 0x424 B5 |
| $3203 HV-Spannung (0,5 V) | 02E1 | 368,5 V | 0x42E 10 Bit x 0,5 V |
| $343F Batterie-Ident per CAN | E7 71 A1 02 | 1735500034 = 0x6771A102 | 0x659 (67 71 A1 02, alle 3 s) |

## 2. EVC-Gueltigkeit von 0x155 (EVC-Seite)
- $33C2 K_Pct_UserSOC_bhv_vld: 0 ungueltig, 1 gueltig, 2..4 Fehler. Dump: 1. Dass er am Anfang 0 ist, ist NICHT gemessen.
- $3567 Vbx_sfty_abs_can_frm_LBC: 0 Frame da, 1 fehlt. Dump: 0. Welcher LBC-Frame, steht nicht in der XML.
- $3461 gehoert zu einem Bremssystem-Frame (UBP), nicht zum LBC.
- Zu 0x155 Byte 3 (0x94/0x54) gibt es in der XML kein Signal.

## 3. Hinweis: 0x423 Byte 0, untere 3 Bit = $3201 K_BHVState_ets_req (Schlussfolgerung, nicht belegt)
$3201: 0 nicht angefordert, 1 Slow, 2 Fast, 3 Normal Charge, 4 Quick Drop, 7 nicht verfuegbar. Dump (fahrbereit): 3.
In den Auto-Logs laufen die unteren 3 Bit von 0x423 B0 genau durch diese Werte:
- Wecken 30 (0) -> 33 (3) -> 07 (7): 22aaf176 bei -0,06 s, 598b197c bei -0,17 s.
- Fahren 0B (3). Abschalten 04 (4, BMS stoppt sofort). Ruhe 07 (7). 0x87 = 07 plus Bit 7 (anderes Signal).
Die Bitposition steht in keiner Frame-Liste. Weitere EVC->LBC-Signale: $33BE LBC sleep mode request (0 Normal, 1 14 Supervision, 2 Quick Drop), Dump 0, Frame unbekannt.

## 4. Wecken im Auto: SOC-Gueltigkeit haengt nicht an anderen Frames
- Alle Pack-Frames starten innerhalb von 0,11 s nach Wake, 0x155 ist 242/245 Frames ungueltig, erster gueltiger bei +2,94 s / +2,87 s.
- EVC-Frames kommen vor dem Pack (0x423 -0,06/-0,17 s, 0x0EC -0,12 s). 0x69F startet bei +2,8 s bzw. +0,56 s, also nicht am SOC festgemacht.
- Bench-Log `teraterm.log`, Wake bei 472,44 s: nur 1 ungueltiger 0x155, erster gueltiger nach 0,01 s -> kein Tiefschlaf-Wecken wie im Auto.

## 5. Weitere Werte im Dump (Auto wach)
- $3202 = 5 Normal, $3201 = 3, $33B8 = 3 Driving_Confirmed, $33E3 = 6 BHV_Normal confirmed, $3394 = 1, $33BC = 1, $3378 = 2 Closed.
- Schlaffreigaben $33C5, $342E, $3442, $34A5 = 1 (Refuse to sleep). $33BE = 0. $3302 = 0 Customer Wake Up.
- $2006 = 85465 km, $2181 = 0 km, $33EE Isolation 473000 Ohm, $3209 = 1 No Default.
- $F190 liefert im EVC die VIN (nicht im Akku). EVC-Software $F194 0A40, $F195 3000.

## Pack-Software (Dumps)
- Bench-Pack und Auto-Dump: F194 RL00, F195 0620. XML V1.7 kennt ausserdem 0603, 0604, 0605, 0630, 0632, 0640.
- DID $926D "Boolean to authorize DID writing sequence": Zweck unbekannt; Lesen 22 92 6D -> 00, im Werkstatt-Makro mit 0 geschrieben.
- $925E: Auto 13 88 6F (= 1280111, auch in 0x69F), Bench-Pack 03.10. 30 28 2F (3156015).

## Offen
- Balancing: Min/Max aller 96 Zellen am Bench zusammen mit 0x912B lesen (Spreizung gegen Bit-Muster).
- 22 33 C2 / 22 35 67 im Auto sofort nach dem Wecken lesen (mehrfach in den ersten 5 s).
- Pack mit mehreren Minuten Stille wecken: kommen 240..245 ungueltige 0x155?

## 6. Absender der gesendeten Frames (Pruefung 17.10.)
- Im Code ist die Quelle bei 0x157, 0x0EC, 0x0ED, 0x428, 0x42F, 0x435 nur "not sent by the battery alone (bench log 25.09.)" = Ausschluss, kein Nachweis des EVC. 0x419: "unclear". 0x69F: CanZE sagt BCM (_Frames.csv: 69F,1000,1000,BCM). 0x350: BCM.
- Veralteter Text in BATTERY.cpp (sim_signals): bei 0x19F, 0x426, 0x436, 0x423 steht "The Twingo vehicle never sends it". Die Auto-Logs (22aaf176) enthalten alle vier.
- JB2-XML (2c3e3125) = Bordlader (plug/mains-Felder, 259x "mains"); $5015 Wake/Sleep-Request vom EVC (0 GoToSleep, 3 WakeUp), $5016 State-Request (0 init, 1 Charge, 4 Wait, 5 Power Off/Sleep). Nicht der Akku.

## 7. Inverter (INV_0220_750_V1.4, Dump 5a971c55; Tabelle: INV_Dump_5a971c55_dekodiert.md)
- Misst selbst: $2004 Udc 368,84 V, $7049 Strom 0 A. Bekommt vom Akku: $70D7 369 V, $7031 Interlock Closed. Bekommt vom EVC: $2003 Drehmoment-Sollwert 0, $7025 Ladeanforderung 0, $7027 DCDC an, $7029 Inverter aus, $703C Strecke 85465 km.
- HINWEIS (Schlussfolgerung, nicht gemessen): 0x19F Byte 2 + oberes Halbbyte Byte 3 (12 Bit, Leerlauf 2000 = 0x7D0) = obere 12 Bit von $2001 Rotordrehzahl (Rohwert 7D 00 = 0 rpm; Schritt 0,5 rpm, Offset -16000, also 8 rpm/Bit). In 22aaf176 weicht der Wert oft ab (min 1069, max 4095), in den Ladelogs fast nie. Dann waere 0x19F ein Inverter-Frame (PEB), kein EVC-Frame.

## 8. BCM-Zeit, Pack-Zeit und 0x436 (BCM-Dump 35655b33, 04.11.2025 12:51:35 UTC; Tabelle: BCM_Dump_35655b33_dekodiert.md)
- Belegt: $8005 ABSOLUTETIME = 00 25 F2 75 = 2486901 min; $6012 V_TM_ABSOLUTETIMESINCE1RSTIGN = 00 25 F2 76. Klimabox 12:49:14 = 2486898 (IH_AbsoluteTimeSinceFirstIgnition): +3 in 141 s = Minuten, gleicher Zaehler.
- Belegt: Pack im Auto-Dump 04.11.2025 13:17: $9261 = 25 F2 56 = 2486870 (BCM um 13:17 etwa 2486927, also ca. 57 min Vorsprung; gegen 12:51: 31 min). $91C1 = 19 F2 52 = 1700434 (BCM minus 91C1 = 786467 min; die frueher notierten 784382 min stammen aus anderen Dumps, nicht neu nachgerechnet).
- Schlussfolgerung: 0x436 B1-3 in den Auto-Logs (22aaf176: 2969614, 598b197c: 2972365..2972377, Ladelog: 2972444..2972454) = BCM-Minuten seit erster Zuendung; rueckgerechnet 05.10. (22aaf176) und 07.10.2026 (598b197c, Ladelog). Aufnahmetage nicht bekannt, Upload 09.-15.10.
- Bench-Pack am 11.10.2026: $9261 = 2D 5A D9 = 2972377 = letzter 0x436-Wert aus 598b197c -> unser Alter-Startwert stammt wohl aus diesem Log.
- VIN steht im BCM (22 10 0C, 21 81, 21 7E), nicht im Pack.
