# Quelle: Zoe Gen2 (Emulator) und ljames28 "Renault-Zoe-PH2-ZE50-Canbus-LBC-Information" (03.11.2026)

Kennzeichnung: **gemessen/gelesen** = aus Code oder Datei, **Schluss** = abgeleitet, **offen**. Alles nur gelesen, nichts geaendert. Repo `https://github.com/ljames28/Renault-Zoe-PH2-ZE50-Canbus-LBC-Information` (README, `example-code.py`, `BMS Modes.xlsx`, `Can_frames.xlsx`), lokal gelesen unter `/home/user/ljames28/`. Original-Emulator `dalathegreat/Battery-Emulator` Stand 02.11.2026 (`5983b9c`).

## 1. Emulator: Zoe Gen2 gegenueber dem Original
- Unsere Zoe-Dateien sind unveraenderte Schnappschuesse des Originals: Gen2 `.cpp`/`.h` = `9f453856` (04.07.2026), Gen2 `HTML.h` = `173b7cd3` (29.03.2026), Gen1 `.cpp` = `c2a500e5` (17.08.2026), Gen1 `.h` = `4e600b6b` (08.08.2026). Eigene Aenderungen daran gibt es nicht.
- Danach im Original: Events je Pack (13.08. bis 14.09.), Pack-Spannungsplatzhalter entfernt (23.09.), Gen2 "extended data sharing between double batteries" (29.08.), Advanced-Seite je Instanz (13.09.). Gen1: Balancing-Bitreihenfolge LSB-first (`3826ca9a`, 24.08.), Fahrzeug-Frames `0x19F`, `0x426`, `0x436`, `0x69F` ergaenzt (`e06acf9c`, `acfd596f`, 01. und 04.09.; Kommentar: ohne sie meldet der LBC `U1000`/`D000` und setzt das Caution-Bit).
- **DTC:** Zoe Gen2 hat nur Loeschen (`supports_reset_DTC`, Frame `18DADBF1 04 14 FF FF FF`), kein Lesen (`supports_read_DTC` fehlt), auch im Original.

## 2. Zoe Gen2: Zeit, Wecken, Schlaf (Emulatorcode)
- Frames: `0x0EE` (10 ms, Zaehler und CRC), `0x373` (100 ms, `C1 40 5D B2 00 01 FF E3`, B2/B3 wechseln alle 5 Frames), `0x375`, `0x376` (100 ms, Zeit), `0x5F8` (1 s, Fahrzeug-ID `16 44 90 8F`), `0x6BF` (1 s, Boost-Zeit). `RENAULT-ZOE-GEN2-BATTERY.cpp` Zeilen 408 bis 473.
- Zeit: `ZOE_376_time_now_s` zaehlt jede Sekunde +1 (Zeile 468), Start 1745452800 (24.04.2025), Produktionszeitpunkt 1614454107. `0x376` B0 bis B2 und B3 bis B5 = Minuten seit Produktion als **drei Ziffern zur Basis 255** (Zeilen 487 bis 506).
- NVROL/Sleep (Zeilen 510 bis 562): Session `10 03`, Routine `B009`, 1 s, Session `10 03`, Schreiben `9281 = 01`, dann **`0x373` B0 von `C1` auf `01`** fuer 30 s (die Frames laufen weiter, nur B0 aendert sich), danach wieder `C1`. Auch der Zeit-Zaehler laeuft dabei weiter (Zeile 468 liegt im 1-s-Block von `transmit_can`).
- Gen2 liest Balancing-Zaehler (`9250` bis `9263`), `battery_temporisation`, `battery_time`, `battery_pack_time` (nur 2 Byte), Zeilen 306 bis 373.

## 3. ljames28 ZE50 (README, `example-code.py`, `Can_frames.xlsx`, `BMS Modes.xlsx`)
- **README:** "the battery will be confused and **will not balance**. To properly emulate the car so that the battery is happy, you will need to send **correct timing information and correct checksums**." Nach NVROL-Reset muss "temporisation before sleep" aktiv sein und das Streaming von `0x373` enden; dann speichert der Pack die Daten und schlaeft.
- `0x373` B0 = `0xC1` (streaming, 3) / `0x01` (no streaming, 0). B1 = `0x40` verriegelt / `0x80` entriegelt.
- `0x376`: "Distance/Time. **Critical frame.** Accuracy is important" (`example-code.py` Zeile 417).
- **`0x440` "Relay / 12V Voltage" (100 ms, vom Emulator nicht gesendet):** B1 = `0x31` normal / `0x32` idle / `0x33` transition / `0x39` init; Kommentar im Skript (Zeile 422): "0x31 = latched, **0x32 = unlatched for sleep transient and balance**"; B2 = CRC (xor `0xAB`), B3 = Zaehler << 4. Weiter `0x4CE` (Ladeart), `0x4FB`, `0x0F5`, `0x133`; laut `Can_frames.xlsx` "Processed by LBC = Yes" fuer `0x0EE`, `0x0F5`, `0x133`, `0x373`, `0x375`, `0x376`, `0x440`, `0x4CE`, `0x4FB`, `0x5F8`, `0x6BF`.
- LBC-Zeit wird als **3 Byte** gelesen (`example-code.py` Zeilen 281 und 283, `9261`/`91C1`).
- **`BMS Modes.xlsx`:** intern 1 Init, 2 Wait, 3 Isolated Charge, 4 Non-isolated charge, 5 External charge, 6 Driving, 7 **Sleep transient (no 373)**, 8 DC, 9 DC+RISOL; zusaetzlich Spalte "BMS Mode External": Init 6, Wait 4, Isolated Charge 1, Non-isolated 2, External charge 3, Driving 5, Sleep transient 4, DC 7, DC+RISOL 9.

## 4. Schluesse fuer Punkt X (nicht bewiesen)
1. **`9259` folgt vermutlich der "BMS Mode External"-Nummerierung** (nicht der internen wie `9270`): gemessen am Twingo-Bench und Auto: `05` = Fahren/Zuendung an (External 5 = Driving), `04` = Zuendung aus, Closed, Awake, natuerlicher Aufwacher (External 4 = Wait/Sleep transient), `02` = Laden (External 2 = Non-isolated charge, Original-Pack-Dump 24.07.2025). Das erklaert die bisher "ohne Wertetabelle" gefuehrten Werte. Die Tabelle aus der MCPU-XML gilt dann nur fuer `9270`.
2. **Das Balancing passt zu "Wait/Sleep transient" (External 4):** der Bench-Pack stand nach dem Sleep auf `9259 = 04` (0:47 am 03.11.) und balancierte; im Zustand "Fahren" (`05`) legte er bisher keinen Plan an. Das ist nur eine Spur.
3. Dass der Pack ohne **richtige Zeit** nicht balanciert (README), deckt sich mit der Messung vom 03.11.: gebucht wird nach der Bus-Zeit-Differenz (56 s bei Differenz 0, 25,6 min bei +25).
4. **Offen:** Ob der Twingo-LBC ein `0x440`-aehnliches "latched/unlatched"-Signal vom EVC bekommt (im Twingo-Bus bisher nicht gefunden); ob der Twingo-LBC dieselben Schutzmechanismen wie der ZE50-LBC hat (der Zoe-Gen2-Satz weckt den Twingo-Pack nicht zum Broadcast, `BEFUND_Echter_Bus_LBC_EVC_09-10.md` Abschnitt 10.1).

## 5. Nachtrag: `Can_frames.xlsx` (Blatt CAN0, Spalte "Comment" 2) im Detail
Gelesen: Die LBC (ZE50) verarbeitet nur 11 Frames: `0x0EE`, `0x0F5`, `0x133`, `0x373`, `0x375`, `0x376`, `0x440`, `0x4CE`, `0x4FB`, `0x5F8`, `0x6BF`; dazu OBD-Frames, die nicht auf dem Bus liegen. Die Spalten "CRC Poly" und "CRC XOR" nennen CRC-8, Polynom `1D`, XOR `AC` (`0x0EE`), `16` (`0x0F5`), `4E` (`0x133`), `AB` (`0x440`), `0A` (`0x4CE`), `82` (`0x4FB`).
- **`0x4CE` "Charging type changes BMS state":** `0F/1F` = Mode 4 (idle), `2F/3F` = Mode 1 (Iso-Charge), `4F/5F` = Mode 2 (Non-iso Charge), `6F/7F` = Mode 5 (Driving), `8F/9F` = Mode 3 (External Charge), `AF/BF` = Mode 7 (DC), `CF/DF` = Mode 9 (DC+RISOL). Das ist die **externe** Nummerierung aus `BMS Modes.xlsx`; dort bestimmt der Fahrzeug-Controller den BMS-Modus ueber dieses Frame.
- **`0x440` "Relay status, internal value":** "Affects BMS State and Current offset calculations. States 0/1/2/3 internally. Options are 0x30/31/32/33. **0x31: 1 = latched. 0x32: 2 = unlatched for sleep transient and balance.** 0/3 ignored." In `Can_frames.xlsx` B1: `0x31` normal, `0x32` idle, `0x33` transition, `0x39` init.
- **`0x376`:** "The time must be **sequential** or it'll be really confused and you have to reset NVROL then do temporisation before sleep. Time is really important." Also nur vorwaerts, nie rueckwaerts senden.
- **`0x373`:** B0 `C1` = streaming (3), `01` = no streaming (0); B5 "affects Can0/UDS"; B7 = Mode (3 = ok).
- **`0x4FB` "DC Relay" / "Relay 4":** `0x4` = 1, `0x2` = 0, `0` = Fehler.

## 6. Schluss fuer die Twingo (nicht bewiesen, Gegenprobe noetig)
- **`925C` ist laut XML "Relay status, internal value" (`Wsx_rly_stt`)**, derselbe Name wie `0x440` im ZE50. Die gemessenen Werte passen zur ZE50-Beschreibung: `02` = unlatched/idle bei **Closed** und **Awake** (Handtabelle 16.10.), `01` = latched in allen Fahr- und Ladezustaenden, `03` am Bench mit HV-Zeilen ("transition", `MERKLISTE_Test.md` Zeile 68), `00` wird ignoriert.
- Damit waere der Zustand, in dem der Pack "sleep transient and balance" macht, **`9259` = 04 (Wait/Sleep transient) und `925C` = 02 (unlatched)**. Beim natuerlichen Aufwachen im Auto steht `9259` = 04 (gemessen, Airtable); `925C` wurde dort nicht gelesen.
- Bench: nach dem Sleep stand `9259` = 04 (0:47 am 03.11.) und der Pack balancierte; im Zustand "Fahren" (`05`/`01`) bisher kein Plan.
- **Zu pruefen (nur Lesen):** `22 92 5C` direkt nach dem Wecken und nach dem Sleep am Bench lesen (Taste "Relay 22 92 5C MCPU" auf der Advanced-Seite), und im Skript `925C` beim natuerlichen Aufwachen im Auto mitlesen.
- **Warnung:** Die Zeit in `0x436` nie rueckwaerts setzen (ZE50-Hinweis zu `0x376`); die Tests mit +478, +25, +70 gehen alle vorwaerts.
