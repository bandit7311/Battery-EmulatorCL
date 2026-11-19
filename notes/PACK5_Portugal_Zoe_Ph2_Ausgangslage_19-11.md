# Pack 5 (Portugal, zweiter Akku von Raul Caldas), Zoe Ph2, Ausgangslage 19.11.2026
Abgelesen von Claude aus Screenshots (Raul-Uhr 16:21 bis 16:23, Seite `192.168.1.59`, Firmware v13.6.0 mit Patch v12, Stark CMR, Laufzeit 8 min 43 s). **Anderer Akku als Pack 4** (`192.168.1.231`, `$9216` = 00, Pack distance 0 km, SOH 95,41 %): Raul schreibt selbst "a different safety mode than your battery, it has 10, yours has 00". Raul: der Akku war schon einmal "a couple of months ago, for SOH readings" am BE. Frame 0x376 und alle Zusatzframes (4CE, 4FB, 133, 440) sind **aus**; kein Wechselrichter angeschlossen.

## Startseite
Scaled SOC 3,53 % (real 22,83 %), **SOH 91,15 %**, 342,4 V, 0,0 A, 0 W, Zellen min/max 3562/3570 mV, **Delta 8 mV**, Temperatur 20/20 C, "Battery idle". Cellmonitor: SoC 2583 ppt, usable 1937 ppt, Pack 3424 dV, 12 V 13598 mV, Max cell 3656 / Min 3647 mV (Cellmonitor-Seite, anderer Zeitpunkt).

## Zeit, Strecke, Safety (Block unter Temporisation)
| Zeile | Wert |
|---|---|
| Tage / Minuten (Felder) | leer / 0 |
| LBC1 time | **0 min** (raw 00 00 00) ✗ |
| LBC2 time | **0 min** ✗ |
| Pack time ($91C1) | **2377458 min** (raw 24 46 F2) = 1651 Tage + 18 min ✓ |
| LBC1 distance | 0,00 mi ✗ |
| LBC2 distance | 0,00 km (raw 0x80000000 + 0) ✗ |
| **Pack distance ($91CF)** | **12374920 km** (raw 2543481088) ✗ |
| **Safety mode 1 root cause ($9216)** | **10 (raw 0A)** |
- Die zwei Erase-Knoepfe v1 und v2 sind da, "not sent yet".
- Pack time aufgerundet auf volle Tage: (2377458 + 1439) / 1440 = **1652**. Tage fuer 0x376 muessen mindestens so hoch sein.

## Zaehler und Zustand (untere Haelfte)
Balance capacity total/sleep **500,04 Ah** (raw 2147995696), time total/sleep **655,12 h** (raw 2148154496), **wake 0,00 Ah / 0,00 h** (raw 2147483648). BMS state **4**. Energy complete 141, partial 976, Slave failures 0, Fan speed 255, period 1000, control 0, duty 3. SoC min/max 2500/2660. Temporisation 0 Activated. TX 232/s, RX 191/s, awake, Sleep mode "Sleep", Auto wake 10 s.

## DTC (Raul 16:23, Read DTC vor 18 s)
- **LBC, 8 Codes:** P1B0301 Confirmed (Current sensor input, general electrical failure), P1B1F81 Confirmed (CAN current sensor MCPU, invalid serial data), P1B0E41 Confirmed (external NV memory, checksum), **U214381 Active** (CAN from EVC/HEVC), P1B1E1C Confirmed (current sensor 12 V supply, voltage out of range), U214281 Confirmed (CAN from Inverter), U214287 Confirmed (CAN from Inverter, missing message), P1B0016 Confirmed (low voltage supply input). 8/8 beschrieben.
- **LBC2, 5 Codes:** U214481 Confirmed, **U214381 Active**, **U214387 Active**, P1BA481 Confirmed, P1BA016 Confirmed (alle "Unknown").

## Einordnung
- Pack distance 12,4 Mio. km ist unmoeglich; aehnlicher Fall wie Pack 2 (20360111 km, `BUILDLIST`/`RenaultZoeGen2Test`). Die Vermutung des Nutzers, ein einmaliger BE-Anschluss mit falscher Zeit/Strecke habe den Zaehler zerstoert, passt zu Raul ("war einmal am BE") und zum Muster, ist aber nicht bewiesen.
- `$9216` = 10 ist der **erste Fall mit Wert ungleich 0 an einem Akku, den wir live haben**: hier koennen die Erase-Knoepfe v1/v2 etwas zeigen.
