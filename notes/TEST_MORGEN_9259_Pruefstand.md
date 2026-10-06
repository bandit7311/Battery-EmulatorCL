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
- Missionsliste im Auto: eine Mission bei Zündung an, eine kurze Nachlauf-Mission bei Zündung aus. `9261`/`91C1` sind ein Schnappschuss-Paar, das ca. 3 min nach dem Ende einer Sitzung geschrieben wird (Stand nach 3 Dumps; Vorhersage siehe Teil 3, noch nicht bestätigt).
- Offene Bauideen (nur nach „bauen"): Zielauswahl `DB`/`DC`/`79B` bei der freien Anfrage; Zoe-Gen2-HEVC-Frames `0x373`/`0x375`/`0x376` (schwach gestützt); `0x5D7` mit Kilometern senden statt `0x426`; Inhalt von `0x426` änderbar machen.

## Teil 3 (neu, aus den Auto-Dumps vom 06.10.): Ereignis statt Dauerzustand

Im Auto zählt `9279` bei **jedem Wechsel** um 1 (Zündung an: neue Mission, Zündung aus: neue Nachlauf-Mission). BMS-Modus `9259`: **05** bei Zündung an, **04** bei Zündung aus (Akku noch wach). Der Prüfstand steht dauernd auf **04**.

1. Lies `22 92 59`, `22 92 79`, `22 92 5C`.
2. Drücke den **Wake-up-Knopf** (Aufwach-Folge `C0 → … → C7`), beobachte währenddessen alle 2 bis 3 s `22 92 59` und `22 92 79`.
3. Notiere, ob `9259` kurz auf `05` geht oder `9279` hochzählt.
4. Danach Sleep-Lauf (Schreibwert `0x00`) und nach dem Aufwachen wieder `9261`, `91C1`, `9275`, `9279`.

Vorhersage für den nächsten Auto-Dump (nach dem Schlaf): `91C1` ca. `21 47 5B`, `9261` ca. `2D 3F 59` (Schnappschuss ca. 3 min nach dem Sitzungsende).
