# SCPU-Dump 99dc0411 (11.10.2026 15:21 UTC, Bench-Pack) dekodiert mit RBMS_SCPU_RL_06xx_V1.2

Im Dump abgelehnt (keine Antwort): 229015, 2291BF, 2291FB, 2291FD, 229202, 229207, F182-F195.


| DID | Name | Variable | Rohbytes | Wert |
|---|---|---|---|---|
| $9000 | DIDs supported in range [$9001 - $9020] |  | 00 38 07 80 | (nicht dekodiert) |
| $9007 | Maximum cell voltage | Wxx_avg_high_voltage | 0E 5B | 3.588869025 V |
| $9008 | Number of Maximum Cell Voltage | Wxx_id_avg_high_voltage | 21 | 33 |
| $9009 | Minimum cell voltage | Wxx_avg_low_voltage | 0E 57 | 3.584962773 V |
| $900A | Number of Minimum Cell Voltage | Wxx_id_avg_low_voltage | 05 | 5 |
| $9014 | Maximum Temperature | Wxx_avg_high_temp | 03 50 | 13 °C |
| $9120 | DIDs supported in range [$9121 - $9140] |  | 00 00 02 00 | (nicht dekodiert) |
| $91A0 | DIDs supported in range [$91A1 - $91C0] |  | 81 00 00 00 | (nicht dekodiert) |
| $91B8 | Board temperature (Wxx_raw_cpu_temp) | Wxx_raw_cpu_temp | 04 30 | 27 °C |
| $91E0 | DIDs supported in range [$91E1 - $9200] |  | 7C 00 00 00 | (nicht dekodiert) |
| $91FA | Raw IRDD Um (Rm voltage)measurement (Wxx_raw_hv_iso_Um_mes) | Wxx_raw_hv_iso_Um_mes | 00 00 | 0 V |
| $91FC | Low Voltage Supply (SUB CPU) (Wxx_raw_14v_mes) | Wxx_raw_14v_mes | 31 28 | 12.28906879 V |
| $91FE | VBATP2_CMD - Bypass value (Wbx_lv_sply_byp_val) | Wbx_lv_sply_byp_val | 00 | 0 = OPEN/OFF |
| $9200 | DIDs supported in range [$9201 - $9220] |  | 00 00 07 FE | (nicht dekodiert) |
| $9201 | S1_CMD - Bypass request (Wbx_hv_iso_switch_main_byp_req) | Wbx_hv_iso_switch_main_byp_req | 00 | 0 = Bypass OFF |
| $9203 | S2_CMD - Bypass request (Wbx_hv_iso_switch_diag_byp_req) | Wbx_hv_iso_switch_diag_byp_req | 00 | 0 = Bypass OFF |
| $9204 | S2_CMD - Bypass value (Wbx_hv_iso_switch_diag_byp_val) | Wbx_hv_iso_switch_diag_byp_val | 00 | 0 = Switch OPENED |
| $9205 | P0VP_IRDD(0/60V)IRDD Bypass request (Wbx_hv_iso_supply_ctl_byp_req) | Wbx_hv_iso_supply_ctl_byp_req | 00 | 0 = Bypass OFF |
| $9206 | P0VP_IRDD - 0 or 60V IRDD Bypass value (Wbx_hv_iso_supply_ctl_byp_val) | Wbx_hv_iso_supply_ctl_byp_val | 00 | 0 = CC 60V OFF |
| $9208 | P60VP_IRDD - PWM Bypass value (Wbx_hv_iso_supply_ena_byp_val) | Wbx_hv_iso_supply_ena_byp_val | 00 | 0 = 60V Supply OFF |
| $9209 | Spare SCPU ANA input (Hot Spot Temp) (Wxx_raw_hspt_temp) | Wxx_raw_hspt_temp | 0A | -30 °C |
| $9240 | DIDs supported in range [$9241 - $9260] |  | 7E 00 00 00 | (nicht dekodiert) |
| $9259 | BMS mode | Wsx_batt_stt | 04 | 4 |
| $925A | Index of thermistor with maximal temperature value | Wxx_id_avg_high_temp | 00 | 0 |
| $925B | Index of thermistor with minimal temperature value | Wxx_id_avg_low_temp | 02 | 2 |
| $925C | Relay status, internal value | Wsx_rly_stt | 02 | 2 |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).00 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).01 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).02 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).03 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).04 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).05 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).06 | Wxx_rst_chk_sum_id | 80 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).07 | Wxx_rst_chk_sum_id | 80 | 1 = External Reset |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).08 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).09 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).10 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).11 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).12 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).13 | Wxx_rst_chk_sum_id | 20 | 1 = Power-Up Reset |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).14 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $925E | CPU2 reset context(Wxx_rst_chk_sum_id).15 | Wxx_rst_chk_sum_id | 20 | 0 = No Error |
| $9261 | Abs time since first ignition | Wxx_cm_abs_time_vhc | 2D 5A E2 | 2972386 ' |
| $9262 | Vehicle distance totalizer | Vxx_cm_vh_dist | 80 8D 94 24 | 92785 km |
| $9280 | DIDs supported in range [$9281 - $92A0] |  | 00 00 00 06 | (nicht dekodiert) |
| $9281 | Sleep condition force | Vbx_slp_cond_force | 00 | 0 = enabled |
| $F000 | DIDs supported in range [$F001 - $F020] |  | 00 04 00 00 | (nicht dekodiert) |
| $F012 | OperationalReference(Ref C if available or Ref B then 0 if no Ref) |  | 32 39 33 38 36 35 32 34 36 52 | (nicht dekodiert) |
| $F0A0 | FOTA Status |  | 00 39 | (nicht dekodiert) |
| $F0A5 | FOTA cancel counter |  | 00 | (nicht dekodiert) |
| $F18C | ECUSerialNumber |  | 32 31 32 39 39 30 32 30 31 50 44 41 30 30 39 41 52 30 30 30 | (nicht dekodiert) |
| $F197 | OBD EngineType |  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F | (nicht dekodiert) |
