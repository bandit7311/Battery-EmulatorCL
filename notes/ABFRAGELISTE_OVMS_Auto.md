# Abfrageliste OVMS (Auto-Pack), Stand 30.10.2026

Bus: Das OVMS haengt am **Fahrzeug-CAN** (OBD-Stecker des Autos), der Bench-Emulator direkt am **LBC<>EVC-Bus** (Aussage des Nutzers, 30.10.). Das Wecken per `xrt32 wakeup` (`0x350`) bzw. `wake` (`0x634`) sind Fahrzeug-CAN-Frames.
Format: `obdii can1 request device -E <tx> <rx> 22<DID>`; MCPU tx `18dadbf1` rx `18daf1db`; SCPU tx `18dadcf1` rx `18daf1dc`. Pack muss wach sein (vorher wecken, ca. 3 s warten).

## Ablauf je Lesung
0. Wecken: `xrt32 wakeup`
1. SCPU-Uhr: `obdii can1 request device -E 18dadcf1 18daf1dc 229261`
2. MCPU Kernwerte: DID `912B`, `9251`, `9252`, `9279`
3. MCPU Uhr/Zustand: `91C1`, `9261`, `9259`
4. Zaehler (bei Bedarf, z. B. taeglich): `924F`, `9250`, `9262`, `9263`
5. Ladungen (bei Bedarf): `9210`, `9215`
6. Missionen (wenn `9279` gestiegen ist): `9270`, `9273`, `9274`, `9277`, `9278`, `926E`, `926F`, `9275`, `9276`

Dazu in die Tabelle: UTC-Zeit, Wecken-Methode (A = `0x350`, B = `0x634`), Werte oder Timeout.

## Umrechnung
- `9251` Ah und `9252` h: (Rohwert - 0x80000000) / 1024. Ein Schritt = 1/1024.
- `91C1`/`9261`: Minuten (Latch bzw. laufende Uhr der SCPU).
- `9270`: 04 Laden, 06 Fahren. `9273`/`9274`: USOC in % (Start/Ende). `9277`/`9278`: km = (Rohwert - 0x80000000) / 32.
