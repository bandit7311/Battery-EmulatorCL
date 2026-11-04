# Quelle: CAN-Datenbank "Renault_Twingo_T4" (Fahrzeug-CAN, vom Nutzer 04.11.) und Abgleich mit unseren OVMS-Logs

**Dateien (vom Nutzer hochgeladen):** `Renault_Twingo_T4.dbc` (204 kB, hier abgelegt in `QUELLE_Twingo_T4_DBC_04-11/`), `Renault_Twingo_T4.dbf` (BusMaster-Fassung derselben Datenbank, Kopf: 213 Nachrichten, nicht gegen die DBC verglichen), `Renault_Twingo_T4.log` (nur "Conversion Warnings Log" der Umwandlung, keine Messdaten). Herkunft der Datenbank ist nicht genannt. Die DBC hat **212 Nachrichten** (eigene Zaehlung) und ist das **Fahrzeug-CAN (HS)**; unsere Pack-Frames `0x423`, `0x424`, `0x425`, `0x426`, `0x435`, `0x436`, `0x445` stehen **nicht** darin. Bitnummern der DBC: Motorola, Startbit = hoechstwertiges Bit des Signals (`Byte = Start / 8`, Feld bei `len` Bits abwaerts).

Kennzeichnung: **gemessen** = aus unseren Logs, **DBC** = steht in der Datenbank, **Schluss** = nicht belegt.

## 1. `0x350` BCM_CANHS_R_04 (DBC)
- `VehicleStateExtented` (Bits 3..0 von Byte 0): 0 Sleeping, 1 Technical Wakeup, 2 CutOffPending, **3 BatTempoLevel**, **4 AccessoryLevel**, **5 IgnitionLevel**, **6 StartingInProgress**, 7 EngineRunning.
- `BCM_WakeUpSleepCommand` (Bits 7..6 von Byte 0): 0 Go to Sleep, 3 WakeUp. `WakeUpType` (Byte 7 Bit 5): 0 Customer wake up, 1 Technical wake up. `AbsoluteTimeSince1rstIgnition` (Byte 1 bis 3, Minuten).
- Tueren: `PassengerDoorState`, `DriverDoorState` (1 closed, 2 open), `EngineHoodState` (1 opened, 2 closed), `RearRightDoorState`, `RearLeftDoorState`; **`VehicleOutsideLockedState`** (Byte 6 Bit 1: 1 = vom Kunden von aussen verriegelt).
- **Passt zu unserem Emulator:** Die Abschaltfolge `C3 -> C2 -> C0 -> 00` ist WakeUp + BatTempoLevel, dann CutOffPending, dann Sleeping, zuletzt Go to Sleep (Seitentext "vehicle state C3 (BAT TEMPO LEVEL)").
- **Schluss:** Schluessel gesteckt = BatTempoLevel, Zuendung Stufe 1 = AccessoryLevel, Stufe 2 = IgnitionLevel, Stufe 3 = StartingInProgress. Die DBC nennt nur die Namen.

## 2. Netzwerkmanagement: "Refuse to Sleep" (DBC) und was unsere Logs zeigen
Steuergeraete melden 1 = refuse, 2 = ready (3 Zustaende, 0 reserviert). Das Netz kann erst enden, wenn alle ready melden.
| ID | Signal (Bits) | Steuergeraet |
|---|---|---|
| `0x1FD` | `SCH_Refuse_to_Sleep` (Byte 1, Bits 7..6) | SCH (Ladesystem) |
| `0x211` | `EMM_Refuse_to_Sleep` (Byte 0, Bits 7..6) | EMM |
| `0x634` | `TCU_Refuse_to_Sleep` (Byte 0, Bits 7..6) | TCU |
| `0x4F8` | `Cluster_Refuse_to_Sleep` (Byte 0, Bits 7..6) | Kombiinstrument |
| `0x550` | `IVI_RefusetoSleep` (Byte 1, Bits 6..5), `Audio_WakeUpSleepCommand` (Byte 0, Bits 6..5) | **Radio / IVI** |
| `0x68B` | `BIC_Refuse_to_Sleep` (Byte 0, Bits 1..0) | BIC |
| `0x3D8` | `BMS12V_RefuseToSleep` (Byte 4, Bits 4..3) | 12-V-Speicher |
| `0x44A` | `CCM_Refuse_to_Sleep` (Byte 0, Bits 7..6) | CCM |
Weitere Frames der DBC, die einen Zusatz im Auto anzeigen koennten: `0x552` DONGLE_BCM, `0x563` DONGLE_EPS, `0x575` TRACKER_BCM.
- **Gemessen (`CANLOG_Aufwachburst_01-11_2134CET`, `CANLOG_Wecken_xrt32_02-11_1901UTC`, beide Bus 1 = Fahrzeug-CAN):** `0x1FD` wechselt beim Wecken zwischen **ready** und **REFUSE** (erste Frames `ff 80 ...` = ready, dann `fe 40 ...` = refuse; im Log vom 01.11. nach 6 s wieder ready, im Log vom 02.11. blieb refuse bis zum Logende). `0x211`, `0x634`, `0x4F8`, `0x550` melden dauernd **ready**, `0x68B` meldet 0 (reserviert). `0x3D8` und `0x44A` kommen in beiden Logs **nicht** vor.
- **Schluss:** Die lange Netz-an-Zeit im Stehen-Log (970 s gegen 130 s im Fahrlog, siehe `BEFUND_Auto-Pack_Dump_Balancing_24-10.md` Abschnitt 14/15) kann von einem dieser Steuergeraete kommen (Radio, Dongle). Die Stehen- und Fahrlogs sind auf dem Akku-Bus aufgenommen und zeigen das nicht.

## 3. `0x1FD` SCH_CANHS_R_01 (DBC) und unsere Frames
Signale: `DCDCLoad` (Byte 0, %), `SCH_Refuse_to_Sleep`, **`LeftTimeBeforeVehicleWakeUp`** (11 Bit: `(Byte 2 & 7) << 8 | Byte 3`), `StartPreHeatingCounter`, `StopPreHeatingCounter`, `LeftTimeToScheduledTime` (Byte 4 Bits 6..0, min), `PreHeatingActivation`, `ClimAvailablePower` (Byte 5, x 25 W), **`Econometer`** (Byte 6, x 1, Offset -80 kW), `AT_GearShiftRefused`, `BMS_TiAlertDisplay` (Byte 7 Bit 7).
- **Gemessen:** `LeftTimeBeforeVehicleWakeUp` = **2047** (Maximum, vermutlich "nicht verfuegbar") in allen 5 Frames des Logs vom 01.11. und in allen 17 Frames des Logs vom 02.11. Beim Einschlafen haben wir den Frame nie mitgeschrieben.
- **Schluss:** Dieses Signal koennte die Quelle des natuerlichen Aufwachers nach **480 min** sein (`BEFUND_Auto-Pack_Dump_Balancing_24-10.md`, Abschnitt 37e bis 37g): der SCH teilt mit, in wie vielen Minuten das Fahrzeug wieder geweckt werden soll. Pruefbar nur mit einem Mitschnitt beim Zuendung aus (Abschnitt 6).
- **Korrektur zu `MERKZETTEL_Zeitwerte_33BE.md` Nachtrag 10 (Schluss aus der DBC):** Das dort gefundene "Relais-Bit" `0x1FD` Byte 6 Bit 4 liegt im Signal **`Econometer`** (Byte 6, Rohwert 0x50 = 80 = 0 kW, 255 = ungueltig); `0x62D` Byte 4 Bit 1 liegt im Signal **`AvailableEnergy_HEVC`** (11 Bit, Byte 4 und 5, x 0,05 kWh). Beide aendern sich beim Zuschalten, weil sich Leistung und Energie aendern, nicht weil sie das Relais abbilden. Das echte Relaissignal ist **`PowerRelayState`** in `0x62D` (Byte 3, Bits 2..1): 0 Precharge, 1 Closed, 2 Opened, 3 Transitory. **Gemessen** im Log vom 02.11.: 3 Frames mit `PowerRelayState` = **2 (Opened)** (Pack wach, Schuetze offen), `AvailableEnergy_HEVC` 0 und dann 3,0 kWh. Ob "Closed" beim Zuschalten erscheint, ist an den alten Fahr- und Ladelogs zu pruefen (liegen hier nicht vor).

## 4. Weitere Frames
- **`0x654` MMI_SCH_CANHS_R_02:** `HVBatteryUnballastAlert` (Byte 0 Bit 3: 1 = HV Battery Unballast Alert), `ChargingPlugConnected`, `HVBatteryFailureDisplay`, `HVBatteryLow`. In unseren Logs nicht ausgewertet.
- **`0x657` BCM_CANHS_R_03:** `TechnicalWakeUpType` (Byte 1 Bits 5..4), **gemessen** in beiden Logs immer 0 (10 und 27 Frames). **`0x55D` BCM_CANHS_R_05:** `DischargedBatTempoRequest` (Byte 2 Bit 2), **gemessen** immer 1.
- **`0x500` BCM_VERLOG und `0x511` ECM_VERLOG:** `SoftwareLockValue` (32 Bit) und `SoftwareLockRequest` (48 Bit) mit Steuerbits: ein Wechselspiel BCM/ECM (Software-Lock). Das erklaert, warum die Bytes B1 bis B4 von `0x500` zufaellig wirken (`BEFUND_Echter_Bus_LBC_EVC_09-10.md`).
- **`0x4AF` LBC_CANHS_R_09** (5 Byte, konstant `95 45 4B 35 C0`): `ClosedCircuit_CellMaxVoltage_Threshold` (9 Bit), `HVB_TempMinStopCooling_Threshold` (7 Bit), `ClosedCircuit_CellMinVoltage_Threshold` (9 Bit), `HVB_MaxCapacity` (x 0,1 kWh). **`0x6C9` HEVC_A22:** hoechste und niedrigste Zellspannung (x 0,01 V + 1 V), Temperaturen, Netzspannung, Strom.
- **`0x3D8`, `0x4DA`, `0x6BD`** BMS12V_A1/A3/A2: 12-V-Speicher (Strom, Spannung, SOC, SOH, `BMS12V_RefuseToSleep`).
- **`0x55C`, `0x677`, `0x6F9`** TCU-, EMM-, CGW-WakeUpFrame (je 2 Byte). **`0x53B`** TIME_R1: Uhrzeit und Datum des Fahrzeugs.
- Zellspannungen `0x5A1` bis `0x5F7` (`BMS_A10` bis `BMS_A28`), `0x5DC` (`BMS_A30`) und `0x5DD` (`BMS_A29`) sind die Broadcast-Frames, die unser Emulator schon liest.

## 5. Was daraus fuer die offenen Fragen folgt (Schluesse)
- Das Wachbleiben des Netzes nach Zuendung aus haengt an "Refuse to Sleep" einzelner Steuergeraete. Radio (`IVI`) und Dongle (`0x552`) stehen auf der Liste der Kandidaten.
- Die 480 min bis zum natuerlichen Aufwachen koennten aus `LeftTimeBeforeVehicleWakeUp` kommen; dass sie in der Nacht 02./03.11. (USOC 15 %) ausblieben, koennte dann ein Wert im SCH sein, nicht im Pack. Beides ist nicht belegt.

## 6. Vorschlag: Mitschnitt auf dem Fahrzeug-CAN (OVMS) beim naechsten Zuendung aus
Mitschreiben: `0x1FD`, `0x211`, `0x634`, `0x4F8`, `0x550`, `0x68B`, `0x3D8`, `0x44A`, `0x654`, `0x657`, `0x62D`, `0x350`. Auswerten: wer wann auf "ready" wechselt, ob `LeftTimeBeforeVehicleWakeUp` beim Einschlafen eine Zahl wie 480 zeigt (ca. 2 min nach Zuendung aus bis zum Netz-Ende), ob `0x654` `HVBatteryUnballastAlert` setzt, `PowerRelayState`.

## 7. Offen
- Herkunft der DBC, ob sie zu unserer Twingo-Generation passt (Frames wie `0x350`, `0x500`, `0x511`, `0x69F`, `0x5xx` stimmen mit unseren Logs ueberein).
- `.dbf` gegen `.dbc` nicht verglichen; die DBC kann Signale mit Min/Max-Fehlern enthalten (Warnungen im `.log`).
