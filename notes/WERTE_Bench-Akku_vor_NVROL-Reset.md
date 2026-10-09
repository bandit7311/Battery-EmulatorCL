# Lebensdaten des Bench-Akkus vor dem NVROL-Reset (Stand 09.10.2026)

Zweck: Die Werte stehen nur noch in Logs, im Akku selbst sind sie seit dem NVROL-Reset (23./24.09.) auf 0. Kennzeichnung: **gemessen** = steht im Log; **Chat** = steht im Chat-Export, nicht neu geprueft; **Nutzer** = Aussage des Nutzers, nicht belegt.

| Wert | DID | Rohwert | Umgerechnet | Quelle | Anmerkung |
|---|---|---|---|---|---|
| Pack-Laufleistung | `91CF` | `80 03 57 80` | **6.844 km** ((Roh XOR 0x80000000) / 32) | gemessen: `cf794fb4`/`dbd33adf` (24.07.), `be14806b` (05.08.) | In beiden Laeufen gleich. Skalierung /32 laut Chat am Auto-Pack geprueft (`80089560` = 17.579 km). Nutzer: Akku wurde mit wenigen km gekauft, 6.844 km koennte der Originalstand sein. |
| Pack-Zeit seit erstem Einschalten | `91C1` | `0D EE 55` | **912.981 min = 634,0 Tage** | gemessen: `cf794fb4`/`dbd33adf` (24.07.), einmal gelesen | Kein Verlauf vorhanden. 1:1 Minuten laut Chat. |
| Balancing gesamt | `924F` / `9250` | `80 00 B4 00` / `80 01 C2 29` | **45,0 Ah / 112,5 h** ((Roh XOR 0x80000000) / 1024) | gemessen: 05.08. (`be14806b`); `9250` auch 24.07. | Skalierung /1024 laut Chat bestaetigt |
| Balancing im Schlaf | `9251` / `9252` | `80 00 B4 00` / `80 01 C2 29` | 45,0 Ah / 112,5 h | gemessen: 05.08.; `9252` auch 24.07. | gleich wie gesamt: alles im Schlaf |
| Balancing im Wachzustand | `9262` | `80 00 00 00` | 0 | gemessen: 24.07. | |
| Zyklen | – | – | 99 | Chat (NVROL-Log 23.09.) | |
| Energie geladen | `9243` | – | 1026,59 kWh | Chat (NVROL-Log 23.09.) | |
| Energie entladen | `9245` | – | 1178,56 kWh | Chat (NVROL-Log 23.09.) | |
| Energie regeneriert | `9247` | – | 204,18 kWh | Chat (NVROL-Log 23.09.) | |
| BMS-Zustand | `9259` | `00` | 0 | gemessen: 24.07. | Bedeutung offen |
| Interner SOC | `9001` | `23 27` | 8999 (bei 0,01 %/Bit: 89,99 %) | gemessen: 05.08. | Skalierung nur aus einem Chat-Anzeigewert abgeleitet |
| Zellspannungen UDS | `9021...9083` | Rohwerte 4192-4208 | 4094-4109 mV (x 0,976563) | gemessen: 05.08. | |

## Stand nach dem Reset
- `91CF`, `91C1`, `9261`, Zyklen, Energien, Balancing-Zaehler: **0** bzw. `80000000` (gemessen 28.09., 29.09., 01.10.; Nutzer 09.10.).
- `925F` (Fahrzeug-km) = 19.400,00 km: kommt aus unserem `0x426` (`4B C8`), nicht aus der Lebensdauer des Packs.

## Offen
- Ob und wie sich die Werte zurueckschreiben lassen: Werkstatt-Dumps sind Schreibmakros (Service 0x2E). `2E 9261` scheiterte mit NRC 0x33 (Security Access). Fuer `91CF` und die Zaehler nicht geprueft. Kein Seed/Key-Raten; nur mit Nutzer vor Ort und ausdruecklichem Go.
- Verlauf von `91C1`/`9261` vor dem Reset (mehrere Lesungen) liegt nicht vor.
