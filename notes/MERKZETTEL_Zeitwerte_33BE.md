# Merkzettel Zeitwerte ($9261, $91C1, $91CF) und EVC $33BE

Stand 05.10.2026. Nur eine Notiz, nichts gebaut.

## Was beim Wiederaufnehmen der Zeitwerte erinnert werden soll
- EVC-DID `$33BE` "LBC sleep mode request": 2 Bit, 0 = Normal Sleep, 1 = 14 Supervision, 2 = Quick Drop,
  3 = Unavailable value. Signal vom EVC an den Akku (Quelle: EVC-XML "EMS3194_0A40_100_V1.0", Basis EVC_3180_RH5).
- Unbelegter Kandidat fuer den Frame: 0x214 (2 Byte, nur im Abschaltvorgang, 08 02 -> F8 3E). Nicht geprueft.
- Ob der Akku beim Schlaf nur speichert, wenn er so ein Signal bekommt, ist offen.

## Bisher ohne Wirkung auf $9261 / $91C1
- Uhr, glatter Zaehler, Manual 3 (alle drei nach Sleep), 12 V in der Stille aus: weiter 00 00 00.
- Schreiben $9261 und $91CF: 7F 2E 33 (security access denied). $9281 ging ohne Schluessel.

## Offen
- Weiterer EVC-Dump (Adresse 0x18DADAF1, Antwort 0x18DAF1DA). Der erste (04.11.2025 13:23 UTC) hatte keine Antworten.
- OVMS-Messung $34EF (HVB Water Cooling Temperature) zusammen mit CAN-Log, um den Frame der Wassertemperatur zu finden.
- 1B0762 (Kuehl-Thermistor, Signal Compare Failure) ist wohl ein Akku-interner Vergleich, nicht ein CAN-Signal.

## Nachtrag nach EVC-Dump a80d130b (04.11.2025 13:15 UTC, Auto wach)
- $33BE LBC sleep mode request = 0 (Normal Sleep) auch im Wachzustand -> kein Wert, der erst beim Abschalten erscheint.
- Schlaffreigaben $33C5, $3442, $34A5, $342E = 1 (Refuse to sleep) im Wachzustand; beim Abschalten vermutlich 2 (Ready to sleep). Empfaenger unbekannt.
- EVC $2006 Total vehicle distance = 85465 km (= LBC $925F = PEB $703C). Klimabox liest IH_DistanceTotalizer aus einem Bremssystem-Frame (Brake_Speed_R1, 85464 km).
- Klimabox: IH_AbsoluteTimeSinceFirstIgnition = 25 F2 72 = 2486898 min aus Frame BCM_CANHS_R_04 (zusammen mit IH_VehicleStateExtended) -> unser 0x350 ist ein BCM-Frame. LBC $9261 (2486870) = 28 min aelter.
- Wasser: $34EF = 18 C, $34FC = 16 C, BCB 18 C. $34A7 Kuehlsystemtyp = 3 (Wasser ohne Chiller). Schwellen vom Akku: 35/22/40/34/34 C.
- KORREKTUR 05.10.: "eigener Bus CAN_E" ist nur eine Vermutung (Name aus EVC-XML $33C8; Nutzer: evtl. nur Gateway-Anschluss E, Gateway ueberbrueckt, alle Busse parallel). Verifiziert: in allen drei Fahrzeug-Logs fehlen die schnellen Akku-Frames (0x155, 0x0C5, 0x1C9, 0x424, 0x425, 0x445, 0x464, 0x588, 0x43A, 0x6BE, 0x659, 0x4AE), waehrend Akku-Diagnose (0x18DADBF1/0x18DAF1DB) und die 20 langsamen 0x5xx-Frames vorhanden sind. Beste Klaerung: CAN-H/CAN-L direkt am Akkustecker im Auto mitschneiden.

## MERKLISTE (vom Nutzer bestaetigt, 05.10.2026) - EVC-DIDs, Adresse 0x18DADAF1 / Antwort 0x18DAF1DA
### Schlafen (Schlaffreigaben; im Dump bei wachem Auto = 1 "Refuse to sleep", Ziel beim Abschalten vermutlich 2 "Ready to sleep")
- $33C5  ETS System Sleeping authorisation synthesis
- $3442  ETS Sleeping authorisation according to HV Battery Management Strategies
- $34A5  TC flag ready to sleep
- $342E  DCDC ready to sleep (internal EVC)
  Werte laut XML: 0 Not used, 1 Refuse to sleep, 2 Ready to sleep. Empfaenger/Frame unbekannt. Idee: im Auto beim Abschalten mitlesen
  (22 33 C5, 22 34 42, 22 34 A5, 22 34 2E), um zu sehen, wann sie auf 2 gehen.
### Temperatur
- $34EF  HVB Water Cooling Temperature, 8 Bit, Wert - 40 = Grad C. Dump: 3A = 18 C (Aussen 14 C). Gleicher Wert wie OVMS "xrt32.bat.cooling.temp".
  Verwandt: $34FC (Wasser fuer Chiller, 16 C), $34D8 / $3109 (Wasser im Lader, 18 C).
  Idee: $34EF per OVMS lesen und gleichzeitig CAN loggen, dann im Log das Byte mit Wert + 40 suchen -> Frame der Wassertemperatur.

## Schaltplan X07 "Steuerung Elektrofahrzeug" (5AL/605) + Bauteilverzeichnis (05.10.2026)
- Die drei Plan-PDFs sind identisch (gleiche Pruefsumme). Legende: Verzeichnis_der_Bauteile.pdf (37 Seiten).
- 946 = Elektrisches Steuergeraet Fahrzeug (EVC), 938 = Antriebsbatterie (3 Niedervolt-Steckerabschnitte), 977 = Batteriekuehlfluessigkeitspumpe,
  567 = Elektrische Wasserpumpe, 2350 = Heizrelais 1 Antriebsbatterie, 645 = Zentralelektronik (UCH = BCM), 2235 = Elektrofahrzeug Batterie + Relais,
  916 = Ladeanschluss, 129 = Programmschalter Automatikgetriebe.
- Pumpe 977: Versorgung 3FB2 (abgesichert, gleiche Ader versorgt auch 567), Masse MAS, Steuerleitungen 55BJ und 55BH ueber R265/R212 zum EVC -> EVC schaltet die Batteriepumpe direkt.
- Heizrelais 2350: vom EVC angesteuert (3ALU an EVC Q1 ueber R212 B3).
- Batterie 938: Stecker 1 Pin 12 = 3FB2 (12 V abgesichert), Pin 6 = 55AP (vom EVC); Stecker 2 Pins 2/6/11 = 2ADD/2ADE/2ADF direkt zum EVC (N1/C3/F3);
  Stecker 3 Pins 5/3 = 2AD/2AC zum EVC (P3/Q2) ueber Verbinder R212 A2. Kein Gateway auf dem Plan, keine CAN-Beschriftung. Kandidat fuer das Akku-CAN-Paar: 2AC/2AD.
- UCH <-> EVC: 133B/133C an UCH-Pins 24/23 mit Abzweigen E133B/E133C -> sieht nach Fahrzeug-CAN-Paar aus (unbelegt).
- Zu klaeren: welche Pins am Bench-Akku sind CAN-H/CAN-L? Wenn 5 und 3 des dritten 938-Steckers, dann ist 2AC/2AD das Akku-CAN.

## OVMS-Module smart EQ (vehicle_smarteq) und Zoe Ph2 (05.10.2026)
- smart EQ 0x350 Byte 0: C0 SLEEPING, C1 TECHNICAL WAKE UP, C2 CUT OFF PENDING, C3 BAT TEMPO LEVEL, C4 ACCESSORY LEVEL, C5 IGNITION LEVEL,
  C6 STARTING IN PROGRESS, C7 ENGINE RUNNING, C8 AUTOSTART, C9 ENGINE SYSTEM STOP. Byte 6 == 0x96 -> "locked".
- smart EQ weckt das Auto per 0x350 "C3 00 00 00 14 70 96 85" (5x, alle 200 ms, Zeitbytes 0!) - eq_cmds_override.cpp.
- smart EQ liest vom Fahrzeugbus: 0x17E, 0x350, 0x392, 0x42E, 0x4F8, 0x5D7, 0x5DE, 0x62D, 0x637, 0x646, 0x654 (SOC Byte 3, Stecker Bit 0x20), 0x658 (Serie, SOH), 0x673.
- Batterie: 11-Bit 0x79B/0x7BB KWP-Gruppenabfragen (0x07 Zustand, 0x02 HV Contactor Cycles, 0x61 SOH...). EVC (smart): 0x7E4/0x7EC, DIDs wie in unserer EVC-XML ($3022-$3025, $339D, $34CB, $320C, $2005, $3301).
- Zoe Ph2 liest EVC 0x18DADAF1/0x18DAF1DA DID 0x2006 = Odometer (passt zu EVC-XML "Total vehicle distance").
- Keines der Module hat Wasser-/Pumpen-DIDs oder 0x4BC.
- 0x654, 0x658, 0x62D (SOC/SOH/Ladestatus) sind in den Fahrzeug-Logs vorhanden -> EVC reicht Akku-Infos auf den Fahrzeugbus weiter.
- WARNUNG in smart-EQ-Doku: Drittanbieter-OBD kann den Schuetz-Alterungszaehler des Akkus auf 0 setzen, HV-Schuetze schliessen dann nicht mehr; Reparatur = BMS Counter Reset durch Fachwerkstatt. xsq hvcycles ueberwachen.

## KORREKTUR EVC-Adresse (Screenshot ddt4all "Ecu settings", 05.10.2026)
- EVC am OBD: Xid 7E4 / Rid 7EC (11 Bit), CAN-500, Session open cmd 10 03, funktionale Adresse 95, XML EMS3194_0A40_300_V2.1.
- Die 29-Bit-IDs 0x18DADAF1/0x18DAF1DA stammen aus dem Kopf der XML 0A40_100 und waren am OBD wohl falsch -> erster EVC-Dump leer (Vermutung).
- Akku (LBC) weiter 29 Bit: 0x18DADBF1 / 0x18DAF1DB.
- Naechste EVC-Dumps (nur lesen): (1) beim Laden, (2) kurz nach dem Abschalten; je mit Uhrzeit und Zustand notieren.

## Auto-Akku-Dump 04.11.2025 13:17:41 UTC (RBMS_MCPU_RL_06xx_V1.7), Auto wach
- $9261 = 25 F2 56 = 2486870 min; $91C1 = 19 F2 52 = 1700434 min; $925F = 85465,00 km; $91CF = 80 08 95 60 = 17579 km;
  $925E = 13 88 6F; $9281 = 00; $9264 = $926B = 00 00 00.
- $9261 ist ~56 min hinter der Fahrzeugzeit (Klimabox 12:49: 2486898) -> wird beim Abschalten gesichert, nicht laufend.
- $91C1 am 04.10.2026 (Fahrlog) = 21 3A 49 = 2177609: +477175 min in 480898 min echter Zeit (Luecke 2,6 Tage) -> zaehlt etwa in Echtzeit, Stand vom letzten Abschalten.
- Pack-km 17579 vs Fahrzeug-km 85465 -> Pack bei Fahrzeug-km 67886 eingebaut (Schluss). $9261 - $91C1 = 786436 min (546 Tage).
- Der EVC-Dump (a80d130b / b5795956) ist doppelt hochgeladen, identisch.

## Neu aus dem Auto-Akku-Dump: Verlaufsspeicher (05.10.2026)
- $9275/$9276 = 32 Eintraege je 3 Byte (Pack-Time-Stempel in min), $9277/$9278 = 32 Eintraege je 4 Byte (Pack-km-Stempel, (raw^0x80000000)/32).
  Ringpuffer ueber 24005 min (16,7 Tage), neuester Eintrag 1700414 = 20 min VOR $91C1 (1700434). Abstaende der Eintraege: 20, 22, 47, 780, 787, 788, 2280 ... min.
- $9271/$9272 (je 1 Byte, -40): 8..19 C, passt zu Pack-Temperaturen (jetzt T1..T8 = 10,0..10,4 C). $9273/$9274: 22..28 C (aeltere warm, neueste 22 C) - Bedeutung unbekannt.
- Korrektur: $91C1 zaehlt weitgehend in Echtzeit (nur 2,6 Tage hinter echter Zeit nach 11 Monaten) - NICHT sicher "nur beim Abschalten gesichert".
  $9261 dagegen ist 56 min hinter der Fahrzeugzeit (BCM) -> dort gesichert, nicht live.
- EVC-Dump: $3431 Time since parked = 0x80000027 -> 39 min (Offset 0x80000000). Keine absolute Zeit im EVC gefunden.
- Zwei EVC-Eintraege in ddt4all (95 = 7E4/7EC, DA = 18DADAF1/18DAF1DA); nur 7E4/7EC hat geantwortet. LBC hat 29 Bit (DB) und im Treiber auch 11 Bit 0x79B/0x7BB (KWP).

## LBC-XML RBMS_MCPU_RL_06xx_V1.7 (22d72d9b) - Namen der DIDs (05.10.2026)
- Praefixe: Z = gespeichert, W = live. $9261 "Absolute Time of Vehicle saved" (Zxx), $91C1 "Pack Time Life (since 1st power-up)" (Wxx_abs_time_pack, live),
  $9264 "Total boost time from HEVC to BMS saved at powerlatch", $926B "Abstime at transistion start", $9281 0 = "temporisation is activated".
- Mission-Log (32 Eintraege): $9271/$9272 Initial/End mission temperature, $9273/$9274 Initial/End mission USOC (NICHT Temperatur - Korrektur),
  $9275/$9276 Initial/End mission Absolute Pack Time, $9277/$9278 Initial/End mission Distance Pack, $9270 BMS State, $926E/$926F Ladung, $9279 mission indice (Auto: 29).
- Temperaturen Auto-Akku: Zellen T1..T8 $9131..$9138 = 10,0..10,4 C; $9012/13/14 avg/min/max; $91CD "Cooling temperature" (T1_MEA, Roh $9173 2,47 V) = 15,9 C;
  $921C "Skin temperature" (T2_MEA, Roh $9174 4,09 V) = -20 C; $91B7 CPU-Board 31 C.
- Zaehler: $9210 = 167 volle, $9215 = 215 teilweise Ladungen; $925B MCPU reset status; $9279 Missionsindex.
- 1B0762 "Cooling thermistor input / 1 thermistor in deviation": Akku-interner Vergleich der Thermistoren (T1 Cooling, T2 Skin, Zellen) -> am Bench lesen: 22 91 CD, 22 91 73, 22 92 1C, 22 91 74.
- Test ob der Bench-Akku Missionen speichert: vor/nach Sleep 22 92 79 (Index), 22 92 75/76/77/78 lesen.

## KORREKTUR Steckerbelegung Antriebsbatterie (PLATFUSI.pdf S.40, "Stecker B 12-polig SCHWARZ"), 06.10.2026
- Stecker B (schwarz, "NO" im Schaltplan): 1/7 MH Masse Motor; 2 2ADD + GTR/EVS-Drucksensor 2; 3 2AD Relaissteuerung 1 Antriebsbatterie;
  5 2AC Relaissteuerung Vorladung Antriebsbatterie; 6 2ADE Signal GTR/EVS-Drucksensor 2; 8 55BW SIGNAL TEMPERATUR ANTRIEBSBATTERIE;
  11 2ADF - GTR/EVS-Drucksensor 2; 12 55BX 0-V-SIGNAL ANTRIEBSBATTERIE; 4/9/10 nicht belegt.
- WIDERRUF: 2AC/2AD sind Relaissteuerungen (Vorladung, Relais 1), NICHT das Akku-CAN. 2ADD/2ADE/2ADF = Drucksensor (+/Signal/-), NICHT Kuehlfuehler.
- Der erste Akku-Stecker im Schaltplan ist GRAU ("GR", Pins 12 und 6), nicht Stecker B. Belegung unbekannt (vermutlich Stecker A, Seite 39 der PLATFUSI.pdf).
- Kuehlfuehler-Kandidat: Stecker B Pin 8 (55BW Signal) und Pin 12 (55BX 0 V). Plan: stromlos Widerstand Pin 8 - Pin 12 messen.
- Offen: Auf welchem Stecker/Pins haengt das Bench-CAN ("5 und 3")?

## Stecker A (12-polig, GRAU) der Antriebsbatterie (PLATFUSI.pdf S.37), 06.10.2026
- Pin 1 BP1X = Dauerstrom abgesichert > Antriebsbatterie 1 (Dauerplus, = Klemme 30 des Nutzers)
- Pin 6 55AP = CAN H SIGNAL; Pin 12 55AQ = CAN L SIGNAL  (im Schaltplan: 55AP/55AQ am EVC E2/E1 und am Programmschalter 129 -> Bus mit mind. EVC, Akku, Wahlschalter)
- Pin 7 3FB1 = EINSPRITZANLAGE + > SCHUTZRELAIS (geschaltetes 12 V; EVC steuert Schutzrelais 238 ueber 3AA an EVC-Pin N4) -> im Auto geschaltet, am Bench dauernd an
- Pins 2,3,4,5,8,9,10,11 nicht belegt.
- Offen: Nutzer nannte "5 und 3" fuer Bench-CAN - passt nicht zu A6/A12. Klaeren.
- Idee fuer spaeter: nur Pin A7 (3FB1) in der Stille wegnehmen (so wie das Auto das Schutzrelais abfallen laesst).

## Stecker A Variante mit ZWEI CAN-Paaren (PLATFUSI.pdf, Screenshot 06.10.2026 00:46)
- Pin 5 53AP = CAN L, Pin 6 55AP = CAN H, Pin 11 53AM = CAN H, Pin 12 55AQ = CAN L; Pin 1 BP1X Dauerplus; Pin 7 3FB1 Schutzrelais-12V.
  => Bus 1: 55AP (H, Pin 6) / 55AQ (L, Pin 12); Bus 2: 53AM (H, Pin 11) / 53AP (L, Pin 5).
- Im Schaltplan hat der EVC beide Paare (53AM/53AP und 55AP/55AQ). Je nach Ausstattung hat der Akkustecker 1 oder 2 CAN-Paare.
- Nutzer: im zerlegten Akku war nur EIN Bus aufgelegt. Welches Paar? (zu klaeren; frueher genannte "Pin 5 und 3" passt nicht)

## Stand 06.10.2026 - Ergebnisse und offene Ideen (fuer spaeter)

### Gemessen / belegt
- T1-Ersatz (Widerstand Stecker B Pin 8 - 12): Punkte 15k=-12,1C, 12k=-8,0C, 6,67k=+3,0C, 4,29k=+13,0C. Mit Widerstand sind 1B0762/1B0715 weg. Zielwert ca. 17C ~ 3,6k (+22k parallel zu 4,29k). Kurve nur aus diesen Punkten, kein NTC-Typ bestimmt.
- Trotz T1-Fix: Sleep-Lauf (7:36 Stille) speichert nichts (9261, 91C1, 9275, 9279 alle 0).
- E14281 (CAN from Inverter, Typ 81) verschwindet mit 0x19F an. E14381 (CAN from EVC/HEVC, Typ 81 = "invalid data") verschwindet mit 0x426 AUS. Fehlertext Autel/CLIP: "946 HEVC not detected by 938 LBC2 can-hv. Invalid series data received".
- 0x426 Bytes 4-6 = km (4B C8 00 = 19400,0). 925F zeigt das live, ist NICHT im Akku gespeichert (0x426 aus -> 925F = 0).
- Bench-Statistik leer: Mileage 0, Lifetime Energy 0, Cycles 0, Energy charged/discharged 0.
- Fremdabfragen am LBC im Auto-Log (RX4, nicht Emulator): 9243, 9245, 91CF, 92C1(NRC31), 91C1, 901B ca. 0,4-0,7 s nach 0x350 -> C3/C4; im Lade-Log bei Ladebeginn 9210, 9215, 91D1, 91D2 und 1 s nach Ladestopp die Folge 9243..901B, nochmal bei 360 s. OVMS-Autor: TCU fragt "Batteriestatistik" ab.
- 0x1BFEF280/0x1BFE80F2: UDS mit ASCII (RXTCU;id=...), Namen TCU.CONFIGURATION, TCU.MQTT-TP-CONFIG, TCU.SOFTWARE ... -> TCU auf diesem Bus.
- Zwei Prozessoren auf einer Platine: MCPU 0x18DADBF1/0x18DAF1DB, SCPU ("LBC2_29b", logische Adresse 00DC) 0x18DADCF1/0x18DAF1DC. SCPU hat 9261 "Abs time since first ignition" (live), 9262 "Vehicle distance totalizer" (Vxx, 0,01 km, Offset 21474836,48), 9281 "Sleep condition force". CanZE LBC2_Fields.csv bestaetigt. OVMS Zoe Ph2 fragt SCPU (nur 9015). smart EQ nutzt 11 Bit 0x79B/0x7BB.
- Echter Twingo sendet 0x5D7 (Bytes 2-4 steigen 578694 -> 578701 -> 578720, wohl km in 0,01) - Emulator sendet 0x5D7 nicht.

### Offene Ideen, nur nach "bauen"
1. Freie Anfrage: Zielauswahl DB (29 Bit MCPU) / DC (29 Bit SCPU) / 79B (11 Bit). Dann 22 92 61, 22 92 62, 22 92 81 an SCPU lesen.
2. Zoe-Gen2-HEVC-Frames als Simulator-Zeilen: 0x373 (C1 40 5D B2 00 01 FF E3, B2/5D alle 5 Frames wechselnd), 0x375 (02 29 00 BF FE 64 00 FF), 0x376 (Zeit/Datum, Sekunden seit 25.02.2021, Basis 255), 0x0EE (10 ms, Zaehler+CRC), 0x5F8, 0x6BF. Beim Sleep 0x373 Byte 0 = 0x01 / weglassen. Hypothese: dem LBC2 fehlt ein Datenframe (HEVC-Satz).
3. 0x5D7 mit km senden statt 0x426.
4. Inhalt von 0x426 aenderbar machen (Bytes 0-3, 7 unbekannt).

### Tests fuer spaeter (ohne Umbau)
- Sleep-Lauf mit Schreibwert 0x00 (activated) statt 0x01 (MCPU-XML: 9281 0 = "temporisation is activated"; Zoe-Gen2/ljames28: 01 = enable). Vorher/nachher 9261, 91C1, 9275, 9279 lesen, Rueckleswert 9281 notieren.
- Reihenfolge fuer E14381: weitere Simulator-Zeilen (0x1F6, 0x211, 0x217, 0xC6, 0x12E, 0x29A, 0x2B7, 0x634 sind aus) alle an, dann halbieren.
- "Read DTC details" (19 06) fuer E14381.
- Dump vom Akku des Autos (MCPU + SCPU, mit Zuendung an und nach Abschalten), um 925F/9262 mit 0x5D7 zu vergleichen.
- Mitschnitt am Akkustecker im Auto (Stecker A Pin 6/12 oder 11/5) mit Pumpenlauf; EVC-Pumpen-DIDs 3318/3319/331A, 3326-3328 parallel lesen.

### Quellen-Hinweis
- Gemini-Aussagen (Handshake, 159C96 loest E14381 aus) sind ohne Beleg; Forenzeilen zeigen nur gemeinsames Auftreten.

### Safety-CPU (SCPU) - Datei RBMS_SCPU_RL_06xx_V1.2 (06.10.2026)
- Passende SCPU-Datei zum Pack (RL00, SW 0600-0640, wie MCPU RL). FL_06xx V1.0 hat dieselben 55 DIDs, anderer SW-Stand.
- Namen: MCPU = "LBC (HEV)_29b", logische Adresse 00DB, Projekt nur x07Ph2, 0x18DADBF1/0x18DAF1DB. SCPU = "LBC2_29b", 00DC, Projekte x07Ph2/x10Ph2/x62Ph2/xCB/xFBPh2/xFC/xFK/xJA/xJB, 0x18DADCF1/0x18DAF1DC.
- Folgerung (nicht sicher): Fehlertext "HEVC not detected by LBC2 CANHV" meint wohl die Safety-CPU (Funktionsname LBC2_29b); E14381 wird dann von der SCPU erkannt.
- Vom Fahrzeug an die SCPU (V-Werte) nur: 9262 Vehicle distance totalizer (km, 0,01 km, Offset 21474836,48), 9281 Sleep condition force (0=enabled, 1=disabled, Bit 31 = LSB nach CanZE-Zaehlung). Live-Zeit 9261 "Abs time since first ignition" (24 Bit, Minuten). 9261 ist 24 Bit wie 0x350 Bytes 1-3.
- Weitere lesbare SCPU-DIDs: 9259 BMS mode, 925C Relay status, 925E CPU2 reset context, 91B8 Board temp, 9015 Isolation, 9129 Slave failure, 9013/9014 min/max Temp, 9007-900A min/max Zellspannung + Index.
- MCPU 9261 = "Absolute Time of Vehicle saved" (Z, gespeichert), SCPU 9261 = live (W): gleiche Nummer, andere Bedeutung.
- Zum Lesen braucht die freie Anfrage eine Zielauswahl DB/DC/79B (nur nach "bauen").

### SCPU-Dump vom AUTO (Datei 1791304429_RBMS_SCPU_RL_06xx_V1.2, 06.10.2026 16:33 UTC) - Referenz
- 9261 = 2D 3F 16 = 2.965.270 min = 06.10.2026 16:23:08 UTC (Epoche 15.02.2021 11:13:08 UTC) -> Live-Zeit der SCPU = 0x350 Bytes 1-3 (gleiche Breite 24 Bit, gleiche Epoche, 10,7 min Abweichung zur Dump-Zeit). 0x350-Zaehler in Logs: 02.10. 2.959.415, 04.10. 2.962.016.
- 9262 = 80 8D 69 F4 -> 92.677,00 km. 0x5D7 (Bytes 2-5 als 32 Bit, >>4, 0,01 km): 02.10. 92.591,12 -> 92.592,24 km, 04.10. 92.595,34 km -> 0x5D7 traegt vermutlich den Kilometerzaehler (frueherer Deutungsversuch 5787 km war falsch). Bitlage Annahme.
- Emulator sendet 0x5D7 nicht; km kommt bei uns aus 0x426 (feste 19.400 km).
- Fehler SCPU (19 02 FF): 1BA249 Status 0x68 (bestaetigt, nicht aktuell) = Insulation resistance measurement circuit / internal electronic failure. 19 0A: SCPU unterstuetzt E14381 + E14387, E14081/87, E14481/87; E142xx NICHT (-> E14281 kommt aus dem MCPU).
- SW: F194 RL00, F195 0620, Teile 293A05180R, HW 293A01341R, 9281 = 00, 9259 (BMS mode) = 05, 925C (Relais) = 01.
- Naechste Dumps: MCPU vom Auto am selben Tag (925F, 9261 gespeichert, 91C1, 91CF, 9275-9279, 9243/45); dann Auto aus, ~10 min Ruhe, beide nochmal -> zeigt, was beim Abschalten gespeichert wird.

### MCPU-Dump vom AUTO (1791305027, 06.10.2026 16:43 UTC, ca. 10 min nach SCPU-Dump) - Referenz
- 925F = 92.677,00 km = SCPU 9262 (gleich). 925E = 13 88 6F = 0x69F Bytes 1-3. 91CF = 24.791 km (Pack-km). 91C1 = 2.180.862 min (1514,5 Tage). 9279 = 31. 9281 = 00.
- 9261 gespeichert = 2.965.244 = 06.10. 15:57:08 UTC, 26 min hinter SCPU-Live (16:23). Fruehere Dump 04.11.2025: 56 min hinter Live. -> 9261 wird NICHT beim Abschalten geschrieben (Mission endete ca. 16:31, 3 min vor Lesen); passt eher zu "beim Wecken / Missionsbeginn" oder periodisch. Offset abs-Packzeit ca. 784.419 min (+-6).
- Missionsliste (32 Eintraege, Ringpuffer, neueste bei Index 1): jede Mission = HV-Sitzung (Fahrt oder Laden), auch 0-2-min-Missionen (Messsitzungen am 04.10.). Neueste: 06.10. ~15:50-16:20, 24766->24791 km, USOC 40->26. 02.10. 16:07-17:44 USOC 11->78 = Ladung. Werte: 9275/76 Zeit (Packzeit, 3 Byte), 9277/78 km ((raw^0x80000000)/32), 9271/72 Temp (raw-40), 9273/74 USOC.
- Bench: 9279 = 0, alle Listen 0 -> am Bench startet nie eine Mission (Vermutung: HV-Schuetze schliessen nie / kein HV-Sitzungsbeginn).
- Bench ohne Umbau lesbar: 22 92 5C (Relais), 22 92 59 (BMS mode), 22 91 CA (Hauptrelais-Anforderung), 22 91 CB (Vorladerelais), 22 92 79 (Missionsindex). Im Auto (aus): 925C=01, 9259=05, 91CA=00, 91CB=00.
- KORREKTUR (Nutzer: Auto war waehrend beider Dumps durchgehend AN): Die neueste Mission (Index 1) ist die LAUFENDE. Ihr Ende ist ein mitlaufender Wert (End-km 24.791 = 91CF live; End-Zeit ca. 3 min hinter 91C1 live), kein echtes Ende. Frueherer Satz "Mission endete ca. 16:31 / Zuendung aus zwischen den Dumps" ist falsch.
- Folgerung (Hypothese): 9261 gespeichert (15:57:08) = Abs-Zeit zum Beginn der laufenden Mission/des Weckens (Mission-Start abs ca. 15:57-16:01, Offset +-6 min), NICHT beim Abschalten geschrieben. Bei frueherem Dump 56 min hinter Live = Auto war so lange wach.
- 91CA/91CB (Haupt-/Vorladerelais-Anforderung) = 0 ("Opening control") im Dump bei laufender Zuendung - Zustand des Autos beim Dump (READY?) unbekannt.
- 9259 BMS mode (05) und 925C Relay status (01): in den XMLs keine Aufzaehlung der Werte.
- Pack-Zeit laeuft langsamer als Absolut-Zeit (Abweichung bei Missionszeiten ca. 15-17 min nach 2-4 Tagen gegenueber 0x350-Zeit) -> Missionszeiten nur grob in Abs-Zeit umrechenbar.

### Zweiter MCPU-Dump vom Auto (1791308177, 06.10.2026 17:36 UTC, "Zündung an", 52,5 min nach Dump 1) - KORREKTUR der Deutung
- Unveraendert zwischen Dump 1 und 2: 9261 (2D 3E FC), 91C1 (21 46 FE), 925F (92.677 km), 91CF (24.791 km), 9259 (05), 925C (01), 9281 (00), 91CA/91CB (00). Geaendert: Zellspannungen/Temperaturen, 9279 (31 -> 00), Missionsliste.
- Neue Mission an Index 0 (ueberschreibt aeltesten Eintrag 28.09.): Start 2.180.885, Ende 2.180.950 (65 min), km 24.791 -> 24.791 (nicht gefahren), USOC 27 -> 25, T 20. Missionen gibt es also auch ohne Fahren.
- Mission Index 1 (Start 2.180.829, Ende 2.180.859, 30 min, 25 km) war bei Dump 1 schon ABGESCHLOSSEN (Ende unveraendert). Frueherer Satz "laufende Mission" war falsch.
- 9261 und 91C1 sind ein Schnappschuss-Paar: Offset X = 9261 - 91C1 = 784.382. 91C1 = Ende Mission 1 + 3 min -> Schnappschuss ca. 3 min NACH Ende der vorigen Sitzung (nicht beim Start, nicht waehrend der laufenden Sitzung: in 52 min unveraendert). Pruefung: Mission 0 = 16:20 bis 17:25 (Autozeit) passt zum Dump-2-Zeitpunkt (PC-Zeit 17:36 minus ca. 10 min Differenz Auto/PC aus SCPU-Vergleich).
- -> Rueckkehr zur Hypothese "gespeichert nach Sitzungsende (nach Temporisation, ca. 3 min)". Die Hypothese "beim Wecken/Missionsbeginn" ist zurueckgezogen.
- Offen: warum Mission 0 bei Dump 1 (Autozeit ca. 16:33) noch fehlte, obwohl Start 16:20 (Eintrag wird wohl erst bei HV-Aktivierung/READY angelegt).
- Folge fuer den Prüfstand: ohne Sitzung (9279 = 0, keine Mission) gibt es kein Sitzungsende und damit keinen Schnappschuss. Test: BMS-Modus 04 -> 05 (siehe TEST_MORGEN_9259).

### Dritter MCPU-Dump vom Auto (1791308249, 17:37:29 UTC, nur 72 s nach Dump 2): "Zündung aus, kurz vor Schlaf"
- Geaendert gegenueber Dump 2: 9259 BMS-Modus 05 -> **04**, 925C Relaisstatus 01 -> **02**, 9279 Missionsindex 00 -> **01**, neue Mission an Index 31 (Start 2.180.950 = Ende von Mission 0, Ende 2.180.952, 2 min, km 24.791 -> 24.791), 12-V (F442) 12,9 V -> 12,0 V, 9284 FF FF FD F9 -> 00 00 00 00, 91CD Kuehltemp.
- Unveraendert: 9261 (2D 3E FC), 91C1 (21 46 FE), 925F, 91CF -> Schnappschuss noch NICHT erneuert (kurz vor Schlaf).
- Mission 0 (Index 0): 16:20 bis 17:25 (Autozeit, Offset X = 784.382), 65 min, km 24.791 -> 24.791, USOC 27 -> 25.
- 9279 zaehlt bei jedem Missionsstart um 1 (31 -> 0 -> 1). Eine Mission beginnt bei Zuendung an UND direkt bei Zuendung aus (kurze "Nachlauf"-Mission bis zum Schlaf).
- BMS-Modus: 05 = Zuendung an (Sitzung), 04 = Zuendung aus / Akku wach vor dem Schlaf. Prüfstand zeigt 04 (+ Relaisstatus 01 wie "Zuendung an", gemischt).
- VORHERSAGE fuer naechsten Dump nach dem Schlaf: Schnappschuss = Ende letzte Mission + 3 min -> 91C1 ca. 2.180.955 (0x21475B), 9261 ca. 2.965.337 (0x2D3F59 = 17:30:08 Autozeit). Trifft das ein, ist belegt: gespeichert ca. 3 min nach Sitzungsende (nach Temporisation).
- Prüfstand-Ideen: 9259/9279/925C direkt nach dem Wake-up-Knopf (Wechsel/Ereignis statt Dauerzustand?) lesen; 9279 muss bei Sitzungsbeginn +1 zaehlen.

## Nachtrag 06.10.: vier weitere MCPU-Dumps (Zuendzyklus ca. 18:58-19:06 Autozeit)
Autozeit = Dateizeit minus ca. 10,2 min (Naeherung).
- Dateien ...3736 / ...3774 / ...3903 / ...4177 (Abstand 38 s / 129 s / 274 s):
  - 9259/925C/9279: 04/02/03 -> 05/01/03 -> 05/01/03 -> 04/02/04.
  - 9279 zaehlt beim Wechsel Zuendung an -> aus um 1 (03 -> 04). Zuendung an: 9259 = 05, 925C = 01. Zuendung aus: 9259 = 04, 925C = 02.
- Schnappschuss-Paar neu: 9261 = 2.965.423 (18:56 Autozeit), 91C1 = 2.181.041, Differenz weiter 784.382. In allen vier Dumps konstant.
- 925F = 92.678,00 km, 91CF = 24.792 km (jeweils +1 km gegenueber den Dumps 1-3; ob zwischen 17:37 und 18:58 gefahren wurde: Frage an Nutzer offen).
- Missionsliste (9275/9276/9278) in allen vier Dumps UNVERAENDERT: neuester Eintrag Start 2.180.885, Ende 2.180.950 (65 min, 17:25 Autozeit, 24.791 km), Liste neueste zuerst.
  Der Zaehler 9279 ging seit 17:37 von 01 auf 04, ohne dass Eintraege dazukamen.

### Korrektur der Vorhersage
- Die Vorhersage "Schnappschuss ca. 3 min nach Sitzungsende" (91C1 ca. 2.180.955) ist WIDERLEGT: tatsaechlich 2.181.041 (86 min spaeter). Die 3-min-Regel gilt nicht.
- Offen / Hypothese (nicht belegt): Missionseintraege werden erst bei sauberem Sitzungsende (Tiefschlaf/Abschaltfolge) geschrieben; der Schnappschuss wird bei einem anderen Ereignis geschrieben (z. B. Aufwachen/Zuendung an).
- Zustaende wie READY lassen sich aus den vier Dumps nicht von "Zuendung an" trennen.
