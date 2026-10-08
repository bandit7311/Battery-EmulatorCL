# Test für morgen: Was setzt den BMS-Modus am Prüfstand von 04 auf 05?

Stand 06.10.2026. Nur Lesen und Haken auf `/simulator`, **kein Umbau nötig**. Nichts am Akku schreiben außer dem unten genannten Sleep-Lauf.

## Ausgangslage (gemessen)

| Wert | Prüfstand | Auto (Zündung an) |
|---|---|---|
| `22 92 59` BMS-Modus | **04** | **05** (heute und Nov. 2025) |
| `22 92 5C` Relaisstatus | 01 | 01 |
| `22 91 CA` / `22 91 CB` Relais-Anforderung | 00 / 00 | 00 / 00 |
| `22 92 79` Missionsindex | **00** | 31 |
| `22 92 61` gespeicherte Zeit | 00 00 00 | 2D 3E FC (= Beginn der laufenden Sitzung, ca. ±6 min) |

Vermutung (nicht belegt): `05` heißt „Fahrzeug aktiv, Sitzung läuft". Dann startet erst dort eine Mission und die Zeit wird gespeichert.

## Ablauf Teil 1: BMS-Modus (jeweils nach 30 s Wartezeit lesen)

Lesen jeweils: `22 92 59`, `22 92 79`, `22 92 61`, `22 91 C1`.

| Schritt | Haken auf `/simulator` | `9259` | `9279` | `9261` | Bemerkung |
|---|---|---|---|---|---|
| 1 | alle Zeilen aus, **nur `0x350`** an | | | | Erwartung: 04 |
| 2a | + `0x19F`, `0x426`, `0x436`, `0x423` | | | | `E14281`/`E14381` merken (`19 02 FF`) |
| 2b | + EVC-Zeilen `0x18A`, `0x1F8`, `0x42E`, `0x427`, `0x432`, `0x650`, `0x1FD` | | | | |
| 2c | + alle übrigen Zeilen | | | | |

Springt `9259` auf `05`: Auslöser gefunden. Dann Gruppe wieder halbieren, bis die eine Zeile feststeht.
Danach Sleep-Lauf und die vier Zeitwerte lesen.

## Ablauf Teil 2: Sleep-Lauf mit richtigem Schreibwert

Laut Haupt-CPU-XML ist `9281`: `0` = „temporisation is **activated**", `1` = „deactivated". Der letzte Lauf hatte `0x01` (nach XML: ausgeschaltet). Die Zoe-Anleitung sagt das Gegenteil.

1. `/simulator`/More Battery Info: **`0x00 (activated)`** anhaken.
2. Vorher lesen: `22 92 61`, `22 91 C1`, `22 92 75`, `22 92 79`.
3. Sleep-Lauf, bis zum Aufwachen durchlaufen lassen. Rücklesewert von `9281` notieren (soll `00` sein).
4. Dieselben vier Werte nochmal lesen.

## Zusatz im Auto (falls möglich)

`22 92 59` bei Zündung aus (Akku noch wach), beim Laden und im READY-Zustand lesen. Dann ordnen wir `04`, `05` und weitere Werte den Fahrzeugzuständen zu. Außerdem Dump Haupt-CPU (`0x18DADBF1`) und Safety-CPU (`0x18DADCF1`) direkt nach dem Einschalten (erste Minute) und später.

## Ergebnisse eintragen

- Ergebnis Teil 1:
- Ergebnis Teil 2:
- Auffälligkeiten:

## Hintergrund (kurz)

- Auto-Dumps: `925F` Fahrzeug-km = `9262` Safety-CPU = 92.677,00 km. Die Live-Zeit der Safety-CPU (`9261`) ist der Minutenzähler aus `0x350`.
- Missionsliste im Auto: eine Mission bei Zündung an, eine kurze Nachlauf-Mission bei Zündung aus. `9261`/`91C1` sind ein Schnappschuss-Paar. Die frühere "ca. 3 min nach Sitzungsende"-Regel ist durch vier weitere Dumps widerlegt (Schnappschuss 86 min später); Auslöser offen.
- Offene Bauideen (nur nach „bauen"): Zielauswahl `DB`/`DC`/`79B` bei der freien Anfrage; Zoe-Gen2-HEVC-Frames `0x373`/`0x375`/`0x376` (schwach gestützt); `0x5D7` mit Kilometern senden statt `0x426`; Inhalt von `0x426` änderbar machen.

## Teil 3 (neu, aus den Auto-Dumps vom 06.10.): Ereignis statt Dauerzustand

Im Auto zählt `9279` beim **Ende** einer Mission um 1 (nicht beim Start); während der laufenden Mission bleibt er stehen. BMS-Modus `9259`: **05** bei Zündung an, **04** bei Zündung aus (Akku noch wach). Der Prüfstand steht dauernd auf **04**.

1. Lies `22 92 59`, `22 92 79`, `22 92 5C`.
2. Drücke den **Wake-up-Knopf** (Aufwach-Folge `C0 → … → C7`), beobachte währenddessen alle 2 bis 3 s `22 92 59` und `22 92 79`.
3. Notiere, ob `9259` kurz auf `05` geht oder `9279` hochzählt.
4. Danach Sleep-Lauf (Schreibwert `0x00`) und nach dem Aufwachen wieder `9261`, `91C1`, `9275`, `9279`.

Vorhersage widerlegt: erwartet `91C1` ca. `21 47 5B`, tatsächlich `21 47 B1` (2.181.041), `9261` = `2D 3F AF`.

**Zusatz für den Prüfstand:** Vor und nach jedem Schritt zusätzlich `22 92 75`/`22 92 76` (neuester Missionseintrag) und `22 92 79` lesen. Im Auto zählt `9279` beim **Ende** einer Mission und es erscheint ein Ringeintrag (Ring wird abwärts beschrieben, Index 31, 30, 29, 28 …). Die Liste deshalb komplett lesen, nicht nur den Anfang.

## ACHTUNG vor jedem Test (Regel vom 08.10.)

Der Emulator sendet nach einem Neustart mit `0x350` an wieder den **Uhrwert** (ca. 2,97 Mio. min). Das soll nicht mehr passieren. Deshalb vor dem Einschalten von `0x350`: auf `/simulator` unter "Manual vehicle age" **1311345** eintragen und **Set** drücken (zählt dann +1 pro Minute), erst danach `0x350` und alle anderen Zeilen einschalten. Nach jedem Neustart wiederholen. Test dazu: Sleep-Lauf, danach `22 92 61` und `22 91 C1` lesen.
