# Fremdanalyse "Zoe LBC: wie das Zellbalancing funktioniert" (22.10.2026)

Quelle: hochgeladene Datei `3fa95bcd-rbms_balancing.html` (Firmware-Analyse von `rbms_mcpu_293A0108_2RL.bin`, MCPU `293A01082R`, Zoe). **Nicht unser Akku:** der Twingo-Bench-Akku hat MCPU `293A01958R`, Software RL00/0620. Die Aussagen sind ungeprueft (Binaerdatei liegt nicht vor), sie dienen als Hypothese.
Kennzeichnung: **Dokument** = Behauptung der Quelle, **gemessen** = aus unseren Daten, **Schluss** = abgeleitet.

## 1. Behauptungen des Dokuments
- Passives Balancing ueber Bleed-Widerstaende, etwa 100 mA, in den CSC-Modulen (12 Module, 96 Zellen).
- Zeitgesteuert: Die LBC rechnet einmal die Minuten je Zelle aus, zaehlt sie herunter und schaltet ohne Nachmessen ab.
- Formel: Minuten = SOH[%] x (dSOC - Totband)[%] x 7,62. Totband 2 % SOC, bei Pack-SOC 30 bis 40 % 5 %. Das entspricht je nach SOC etwa 10 bis 24 mV (Tabelle im Dokument: 99 % SOC = 24 mV, 50 % = 10 mV, 30 bis 40 % = 15 bis 20 mV).
- Takt 30 min an / 30 min Pause (Pause zaehlt Wanduhrzeit).
- Neu gerechnet wird nur beim Aufwachen und bei Fahrtbeginn nach vollstaendigem Laden.
- Beim Einschlafen endet das Balancing sofort. Restzeiten werden nur bei einer der Abschaltarten im EEPROM gesichert und beim Hochlauf nur bei 30 bis 40 % SOC uebernommen.
- Laden, Fahren und wacher Stand balancieren gleich (keine Abhaengigkeit von Strom oder Schuetz).
- Von aussen sichtbar: Diagnose `21 07` (dieselben 12 Bytes wie die Schaltmaske, ohne Freischaltung lesbar), CAN intern `0x110/0x111`. Kein Dienst startet oder stoppt das Balancing. Offener CCP-Kanal auf `0x300/0x301` (laut Dokument lesen und schreiben beliebiger Speicherstellen, auch der Balancing-Zaehler).

## 2. Was zu unseren Daten passt
- **Gemessen:** Bench-Akku bei 3,625 V: Spreizung 7 mV (Zellmonitor 20.10.). Das liegt unter der Schwelle von 15 bis 20 mV bei 30 bis 40 % SOC. **Schluss:** dass dort nichts balanciert wird und die Zaehler `924F` bis `9252` auf 0 stehen, waere nach dem Dokument zu erwarten.
- **Schluss (nicht belegt):** Wenn die Minuten nur beim Aufwachen berechnet werden, bewertet ein dauerhaft wacher Akku am Bench spaeter entstehende Unterschiede nicht neu.
- **Schluss:** Die 30/30-Pause erklaert, warum eine Stichprobe von `912B` alle 20 s (Bau 26) mal Bits zeigt und mal nicht.

## 3. Was nicht passt oder offen ist
- **Gemessen (Auto-Akku):** Die Zaehler im Schlaf laufen (+2,03 Ah, +2,83 h in 5 Tagen). Das Dokument sagt, im Schlaf werde nicht balanciert. Die Twingo-Firmware weicht hier ab, oder das Dokument irrt.
- **Gemessen:** Bench-Akku zeigte bei SOC 97 % etwa 4,185 V (`9006` / 96). Die OCV-Tabelle des Dokuments endet bei 4,166 V (100 %). Die Kennlinie passt nicht direkt auf unseren Akku.
- Fruehere Beobachtung: 76 gesetzte Bits bei SOC 97 % im ESP-Log, aber Zaehler bleiben 0. Wie `924F` bis `9252` zu den Bits und Minuten des Dokuments stehen, ist offen.

## 4. Tests (kein Bau)
1. **Wake-Test:** auf >= 97 % SOC laden, bis die Spreizung ueber 25 mV liegt; Schlaf (`9281=1`) und Wecken; danach `912B` und `22 92 4F` bis `52` lesen und mit vorher vergleichen. Erscheinen nach dem Wecken Bits, rechnet die Firmware vermutlich nur beim Aufwachen.
2. **`21 07`** am Bench-Akku lesen (KWP-Dienst 0x21, Local ID 0x07). Ob der Twingo-Akku ihn beantwortet, ist unbekannt. Zweite Quelle fuer die Schaltmaske.
3. Zaehler `924F` bis `9252` und `9259` vor und nach jedem Test notieren.

## 5. Nicht tun
- **CCP-Kanal `0x300/0x301` nicht beschreiben.** Eingriff in das Steuergeraet eines Hochvolt-Akkus ohne Rueckfallebene; ob der Kanal beim Twingo existiert, ist nicht bekannt.
