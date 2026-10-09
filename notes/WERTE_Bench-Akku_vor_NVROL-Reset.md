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

## Muss man die Werte zurueckschreiben? Was der Akku selbst uebernimmt (Stand 09.10.)

Nutzer-Idee: Wenn wir den richtigen emulierten Wert senden, uebernimmt der Akku ihn selbst. Einordnung nach Belegen:

### Wird uebernommen (gemessen)
- **Fahrzeug-km `925F`:** 19.400,00 km am 01.10. = 16 Bit `4B C8` aus unserem Standard-`0x426` (Log `cbd10d87-canlog_0d00h04m17s`).
- **SCPU-Zeit `9261`:** Quelle sind die Bytes 1-3 von `0x436` (BEFUND_SCPU_0436_Bench_08-10.md).

### Wird bisher nicht uebernommen (gemessen)
- **Pack-km `91CF`:** am 01.10. = 0, obwohl `925F` = 19.400 km. **Vermutung (nicht belegt):** zaehlt Fahrstrecke (Zuwachs) und braucht einen Fahrzustand, den wir nie senden.
- **MCPU-Zeit `9261`/`91C1`:** seit Wochen unveraendert `14 02 70`. Ob dieser Wert aus unserem `0x436` (Bytes `14 ...`, Zaehler `0x0270`) stammt, ist Hypothese. Die MCPU nimmt aktuell nichts Neues auf.
- **`91C1` (Pack-Zeit seit erstem Einschalten):** kein Beleg, dass er von aussen kommt (wirkt wie interner Zaehler).

### Misst der Akku selbst (Schluss, nicht gemessen)
- Zyklen, Energien (geladen/entladen/regeneriert), Balancing-Zaehler: kein Frame bekannt, der sie vorgibt. Sie beginnen bei 0 neu zu zaehlen, sobald die Zeitbasis passt; die Werte aus der Tabelle oben kommen davon nicht zurueck.

### Tests (nichts gebaut, Bauliste B)
1. `0x426` mit **steigenden Kilometern** in den Zustaenden Zuendung/Fahrt senden; pruefen, ob `91CF` mitzaehlt.
2. `0x436` im echten Format (`80 [Alter] 00 00`) senden; pruefen, ob `9261`/`91C1` in der MCPU anlaufen.
3. Zum Vergleich weiter `925F` und `9261` der SCPU mitlesen (diese folgen bereits).

## Werte, die zu diesem Bench-Akku passen (Vorschlag fuer die Voreinstellung der Felder, 10.10.)

Rechnung (nachgerechnet am 10.10.):
- **km:** Pack-Laufleistung `91CF` = 6.844 km (`0x1ABC`), unveraendert zwischen 24.07. und 05.08. Der Akku wurde laut Nutzer mit wenigen km gekauft. **Annahme:** Lief der Pack seit neu im selben Spenderfahrzeug, entspricht 6.844 km dem Kilometerstand des Spenderfahrzeugs. `0x426` setzt `925F` (Fahrzeugstrecke), nicht `91CF`; `91CF` zaehlt der Akku selbst (am 01.10. trotz 19.400 km in `925F` = 0). Vorschlag: Voreinstellung des km-Feldes (`0x426`, bisher 19.400) auf **6.844** (weiter einstellbar). Der Nutzer wollte frueher 19.400 km am Bench behalten; Entscheidung offen.
- **Zeit (`91C1`, Pack-Zeit seit erstem Einschalten):** 912.981 min am 24.07. Bei natuerlicher Fortsetzung ergibt das 1.019.541 min am 06.10. und 1.025.301 min am 10.10. (78 Tage x 1.440 min seit dem 24.07.), **falls** der Zaehler in Kalenderzeit weiterlaeuft; zaehlt er nur Betriebszeit, liegt der Wert darunter. Offen.
- **Auffaellig (gemessen):** Am 06.10. stand `9261` = `91C1` = 1.311.344 min, also **291.803 min (rund 203 Tage) ueber** der natuerlichen Fortsetzung (1.019.541). Das passt nicht zu einem reinen Weiterzaehlen; moeglicher Zusammenhang mit dem Wert, den unser Emulator an den Akku schickte (Hypothese, nicht belegt: die Bytes `14 02 70` entsprechen Byte 1-3 unseres alten `0x436`-Zaehlers `86 14 02 70 ...`). Die bisherige Regel (Alter 1.314.935 = 1.311.344 + Zeit + 1 Tag, "nicht unter der Packzeit") beruht auf diesem Wert.
- **`9261` (Fahrzeugzeit):** fuer diesen Akku vor dem Reset nicht gelesen, kein passender Wert bekannt.
