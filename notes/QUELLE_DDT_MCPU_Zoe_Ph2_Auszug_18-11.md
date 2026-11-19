# Auszug aus der DDT2000-Datei "RBMS_MCPU_SW5XX_ZL_V1.5" (MCPU der Zoe Ph2 u. a.), vom Nutzer, 18.11.2026

Datei: `RBMS_MCPU_SW5XX_ZL_V1.5_20220525T080250.xml` (DDT2000-Export 25.05.2022, 1,5 MB, vom Nutzer hochgeladen, liegt nicht in den Notizen). Gruppe **RBMS1**, Funktion **"LBC (HEV)_29b"**, Adresse **219 (0xDB)**, LogicalAddress 00DB (= `0x18DADBF1` / `0x18DAF1DB`). Projekte im Kopf: **PZ1A, x07Ph2, x10Ph2, x62Ph2, x82Ph2, xCB, xFBPh2, xFK, xJA, xJB**. DID-Datenbank M_BMS_DIDs 3.2, DTC-Datenbank 5.2. 752 Anfragen (634 x `22`, 85 x `2E`). Nur gelesen, nichts am Geraet geprueft.

## Sitzung, Lesen, Loeschen (UDS)
| Name in der Datei | Anfrage | Antwort |
|---|---|---|
| StartDiagnosticSession.Default | `10 01` | `50 01` |
| StartDiagnosticSession.Programming | `10 02` | `50 02` |
| **StartDiagnosticSession.ExtendedDiagnostic** | **`10 03`** | `50 03` |
| StartDiagnosticSession.Supplier | `10 60` | `50 60` |
| ReadDTCInformation.ReportDTC | `19 02 FF` | `59 02 <Maske> ...` |
| **ClearDiagnosticInformation.All** | **`14 FF FF FF`** | **`54`** |
| weitere ReadDTC: NumberOfDTC (`19 01 FF`), Snapshot (`19 04 ...`), ExtendedData (`19 06 00 00 00 xx`), SupportedDTC (`19 0A`), FaultDetectionCounter (`19 14`) | | |

Der Zoe-Ph2-Treiber (`10 03`, dann `14 FF FF FF`, `UdsCanBattery.cpp:429-439`) entspricht also dieser Datei.

## Routinen (`31 01 B0 xx 00`)
| Anfrage | Name in der Datei |
|---|---|
| **`31 01 B0 04 00`** | **RoutineControl_Start_ResetSM1** (Reset Safety Mode 1) |
| `31 01 B0 01 00` | PrechargeRelayControlOutput |
| `31 01 B0 02 00` | MainRelayOutputControl |
| `31 01 B0 03 00` | FANControl |
| `31 01 B0 05 00` | BatteryCellChangeUpdateCommand |
| `31 01 B0 06 00` | BMSChangeUpdateCommand |
| `31 00/01/02/03 00 00` | allgemein RoutineControl (Start, Stop, Results) |

**`31 01 B0 09 00`** (der NVROL-Reset aus dem README von ljames28) **kommt in dieser Datei nicht vor.**

## DIDs zu Zeit, Strecke und Safety Mode (Name, Breite, Einheit)
| DID | Name in der Datei | Breite | Einheit und Skala |
|---|---|---|---|
| `$9261` | **Absolute Time of Vehicle saved** (`Zxx_abs_time_vhc`) | 24 Bit | **min** |
| `$91C1` | **Pack Time Life (since 1st power-up)** (`Wxx_abs_time_pack`) | 24 Bit | **min** |
| **`$925F`** | **Distance Totalizer (vehicle)** (`Wxx_dist_vhc`) | 32 Bit, mit Vorzeichen | **km, Schritt 0,01, kein Offset** |
| `$91CF` | **Battery mileage** (`Zxx_dist_pack_mem`) | 32 Bit, mit Vorzeichen | **km, Schritt 0,03125 (1/32), Offset 67 108 864** (= 0x80000000 / 32) |
| `$9262` | Total balancing quantity, Wake Up mode (`Zxx_bal_chg_tot_wk`) | 32 Bit | Schritt 0,0009765625 (1/1024), Offset 2 097 152 |
| `$9216` | Carrier to save SafetyMode1 root cause (`Ztx_sfty_apv`) | | lesen (`22 92 16`) und **schreiben** (`2E 92 16`) |
| `$925B` | MCPU reset status (`Wxx_BMS_rst_U_16`) | 16 Bit | Liste: RAM check error, ROM check error, Stack check error, Trap error, ... |

## Folgerungen (aus der Datei gelesen, nicht am Geraet geprueft)
1. **Das Register `$925F` hat die Einheit km mit 0,01 pro Zaehler**, nicht Meilen. Bei Pack 3 (raw 36 016 004) sind das **360 160,04 km**. Die Anzeige "225100,02 mi" auf unserer Seite ist eine **Umrechnung** (km durch 1,6, in Hundertsteln `raw / 160`), keine Einheit des Registers. Dass der Pack aus der gesendeten Zahl 225100 genau 360 160 km macht (Faktor 1,6), passt dazu, dass `0x376` die Strecke in Meilen traegt (Annahme des Nutzers, nicht in der Datei).
2. **`$91CF`**: Formel `(raw - 0x80000000) / 32` km ist durch die Datei bestaetigt.
3. **`$9261`** heisst "saved" (gespeicherte Fahrzeugzeit) in **Minuten**, **`$91C1`** ist die Packzeit **seit dem ersten Einschalten** in Minuten.
4. **Ph2 hat eine eigene Reset-Routine fuer das Safety-Mode-Flag: `31 01 B0 04 00` "ResetSM1"**, das Gegenstueck zu `31 05 00` ("erase safety mode") der Ph1. Der Grund fuer einen Safety Mode 1 steht in `$9216` (lesbar und beschreibbar).

## Nachtrag 19.11.: ganze XML durchsucht (`RBMS_MCPU_SW5XX_ZL_V1.5_20220525T080250.xml`, 1,58 MB, vom Nutzer erneut hochgeladen)
Gelesen in der Datei, nichts am Geraet geprueft.
- **Unsere Packs stehen in der Datei:** `AutoIdent` Zeile 130: `DiagVersion="23" Supplier="e2cad" Soft="543" Version="ZL00"` (auch 543 ZL10). `e2cad` ist die `$F18A`-Kennung, `$F194`/`$F195` der Portugal-Packs (543 / ZL00) sind damit die Software, fuer die die Datei gemacht ist. Verlauf: "Auto-Ident added for 540_ZL00 ... 543_ZL00, 543_ZL10 ... with Diag Version $17" (V1.2).
- **B004:** `RoutineControl_Start_ResetSM1`, Kommentar **"SM1 flag reset for full UDS LGC"**, gesendet `3101B00400`, erwartete Antwort `7101B00400` (MinBytes 3). In `RoutineIdentifierList` (Zeile 234): `45060` = 0xB004 = "Safety Mode 1 reset". "full UDS LGC" ist nicht erklaert (LGC vermutlich LG Chem, nicht belegt).
- **Korrektur einer frueheren Aussage:** Die Liste enthaelt auch `45065` = 0xB009 "Reinitialisation of NVROL memory" (nur als Kennung, **keine** Anfrage `3101B009..` definiert), ausserdem `45066` = 0xB00A "SW recopy into external Flash memory", `45063/45064` = Relais 3/4, `672` = "Request to change FOTA status", `674` = "Request S/W bank switch". Vorher stand hier, B009 komme in der Datei nicht vor: das galt nur fuer die Anfragen.
- **Sitzungen:** `10 01` Default, `10 02` Programming, `10 03` ExtendedDiagnostic, **`10 60` Supplier** (Zeile 17596). Der einzige Bildschirm mit "SM1" im Namen ("SM1-CANdev", Kategorie DEV, Text "CAN dev ON") hat nur eine Taste, die **`StartDiagnosticSession.Supplier` (`10 60`)** sendet. Hinweis, **kein Beweis**: die SM1-Funktion koennte die Supplier-Sitzung brauchen. Die Datei sagt zu keiner Anfrage, welche Sitzung sie braucht.
- Adressierung der Datei: SendId 416996337 (extended), ReceiveId 417001947, wie unser `0x18DADBF1`/`0x18DAF1DB`.
