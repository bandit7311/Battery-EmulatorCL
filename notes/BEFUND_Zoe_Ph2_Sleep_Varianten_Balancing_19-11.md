# Befund: Drei Sleep-Varianten an Zoe-Ph2-Packs (Portugal), Wirkung auf Balancing (19.11.2026)

Kennzeichnung: **gemessen** = vom Nutzer/Raul berichtet, **Schluss** = abgeleitet. Welches Pack (4 oder 5) jeweils betroffen war, ist nicht in jedem Fall sicher zugeordnet; wo unklar, steht "Pack unklar".

## Die drei getesteten Wege
1. **Sleep (normal)**, gestufter 7,2-s-Vorlauf, `HEVC frame 0x440` eingeschaltet (laut Nutzer "immer an") — setzt `0x440` B1 auf `0x32` ("unlatched for sleep transient and balance" laut ljames28-Quelle). Mehrfach getestet.
2. **Sleep instantly** (v15), kein Vorlauf, sofortige Stille, kein `0x32`. Einmal 8 min getestet.
3. **Sleep 0x9281=1** (v16), Temporisation wird vor der Stille geschrieben. Einmal getestet, v16-Anzeige bestaetigt: Schreiben von `1` **angenommen** (`accepted (6E)`, raw `03 6E 92 81 AA AA AA AA`), nach dem Aufwachen liest `$9281` wieder **`0`** (zurueckgesetzt vom Pack selbst, nicht von uns).

## Ergebnis
| Weg | Balancing-Zaehler verändert? |
|---|---|
| Sleep (normal, `0x32` gesendet), mehrfach | **nein** (keine Aenderung beobachtet) |
| Sleep instantly, 8 min | **nein** |
| Sleep 0x9281=1 | **nicht geprueft** (Nutzer hat die Werte fuer diesen Lauf nicht notiert) |

## Schluss
- Zwei von drei Wegen zeigen klar: **keine Wirkung auf die Balancing-Zaehler**, weder der sanfte noch der harte Sleep-Mechanismus loest sichtbares Balancing aus.
- Der dritte Weg (Temporisation) ist die einzige Variante mit einer Quelle, die ihn als notwendige Vorbedingung nennt (ljames28: "the battery will be confused and will not balance... you will need to send correct timing information... enable temporisation before sleep"); sein Effekt auf das Balancing selbst ist aber **noch nicht geprueft**.
- Offen bleibt die Idee, **NVROL** (statt reinem Sleep) als eigentlichen Ausloeser zu pruefen, da es den gespeicherten Zustand nullt -- das wurde in keinem der drei Tests ausprobiert.

## Fuer den naechsten Test (falls gewuenscht)
Balancing-Zaehler (`Balance capacity total/sleep`, `Balance time total/sleep`) vor und nach einem "Sleep 0x9281=1"-Lauf notieren, dazu welches Pack (4 oder 5).
