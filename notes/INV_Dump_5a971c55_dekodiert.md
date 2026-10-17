# Inverter-Dump 5a971c55 (04.11.2025, ca. 13:03 UTC) dekodiert mit INV_0220_750_V1.4

Quelle: 383cc5ee-…xml (= 34f702f7) + 5a971c55-…txt. ACHTUNG: Vektor-DIDs ($7052, $7058, $7066, $7067) sind unplausibel dekodiert, nicht verwenden.

| DID | Name | Variable | Rohbytes | Wert |
|---|---|---|---|---|
| $2000 | DIDs supported in range [$2001 - $2020] |  | 00 00 00 3E | (nicht dekodiert) |
| $2001 | ($2001) Rotor mechanical rotation speed | Vxx_emot_n | 7D 00 | 0 rpm |
| $2002 | ($2002) Emotor torque estimation, based on measured current values | Vxx_esti_emot_tq | 3F 80 | 0 N·m |
| $2003 | ($2003) Torque setpoint from EVC | Vxx_emot_tq_sp | 09 C4 | 0 N·m |
| $2004 | ($2004) Battery voltage sensor | Vxx_udc | 2E 1B | 368.84375 V |
| $2005 | ($2005) DCDC output tension measure | Vxx_dcdc_volt_ou_mes_raw | 39 6C | 14700 mV |
| $2060 | DIDs supported in range [$2061 - $2080] |  | 00 FF 00 00 | (nicht dekodiert) |
| $2070 | ($2070) Immobilizer - diagnosis availability | Vbx_swlk_diag_avl | 01 | 1 = available |
| $2071 | ($2071) Immobilizer - Byte 1 used to allow diagnosis | Vxx_swlk_diag_1 | 00 | 0 |
| $2072 | ($2072) Immobilizer - Byte 2 used to allow diagnosis | Vxx_swlk_diag_2 | 1E | 30 |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.0 | Vxx_swlk_diag_3 | 35 | 1 = Immobilizer engine running authorized |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.1 | Vxx_swlk_diag_3 | 35 | 0 = Not used |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.2 | Vxx_swlk_diag_3 | 35 | 1 = Cleappli set |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.3 | Vxx_swlk_diag_3 | 35 | 0 = Not used |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.4 | Vxx_swlk_diag_3 | 35 | 3 = Not virgin ECM - Secret key learnt in non erasable area |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.6 | Vxx_swlk_diag_3 | 35 | 0 = ECM not protected |
| $2073 | ($2073) Immobilizer - Byte 3 used to allow diagnosis.7 | Vxx_swlk_diag_3 | 35 | 0 = Not used |
| $2074 | ($2074) Immobilizer - engine not running due to ECM | Vbx_swlk_diag_ecm | 00 | 0 = no |
| $2075 | ($2075) Immobilizer - engine not running due to BCM in secure mode | Vbx_swlk_diag_tool | 00 | 0 = no |
| $2076 | ($2076) Immobilizer - engine not running due to no BCM authorization | Vbx_swlk_diag_ccu | 00 | 0 = no |
| $2077 | ($2077) Immobilizer - engine not running due to a CAN network problem with the BCM | Vbx_swlk_diag_mux | 00 | 0 = no |
| $7001 | ($7001) Command of PEC fan | Vxx_cfa_pwm_sp | 0E | 14 % |
| $7002 | ($7002) Wired APC signal | Vbx_dcdc_apc_raw | 01 | 1 = APC on |
| $7003 | ($7003) Measured rotor current (sensor 1) | Wxx_i_f_1 | 00 55 | 0.0830078125 A |
| $7004 | ($7004) Measured rotor current (sensor 2) | Wxx_i_f_2 | 00 4A | 0.072265625 A |
| $7005 | ($7005) Verlog Status | Vbx_eng_run_ena_swlk | 01 | 1 = Vehicle unlocked |
| $7006 | ($7006) Wire frame APC state | Vbx_wf_apc | 01 | 1 = APC on |
| $7007 | ($7007) Measured stator current on phase U | Wxx_i_mot_ph_1 | 3E 6A | -0.6875 A |
| $7008 | ($7008) Measured stator current on phase V | Wxx_i_mot_ph_2 | 3E 61 | -0.96875 A |
| $7009 | ($7009) Measured stator current on phase W | Wxx_i_mot_ph_3 | 3E 64 | -0.875 A |
| $700A | ($700A) Feedback fan raw diagnostic | Vbx_raw_cfa_nok | 00 | 0 = OK |
| $700B | ($700B) Excitation temperature | Vxx_wf_exct_temp | 00 6A 86 | 13.26171875 °C |
| $700C | ($700C) Inverter temperature sensor 1 | Vxx_wf_invt_temp_1 | 00 70 6E | 16.21484375 °C |
| $700D | ($700D) MICRO temperature | Vxx_wf_mc_temp | 00 93 FA | 33.98828125 °C |
| $700E | ($700E) PCB temperature | Vxx_wf_pcb_temp | 00 84 F9 | 26.48632812 °C |
| $700F | ($700F) Stator temperature sensor 1 | Vxx_sttr_temp_1 | 00 6A AB | 13.33398438 °C |
| $7010 | ($7010) Stator temperature sensor 2 | Vxx_sttr_temp_2 | 00 63 06 | 9.51171875 °C |
| $7011 | ($7011) Raw rotor position resolver angular position measurement | Wxx_sens_emot_psn_raw | 80 73 | 256.8984375 ° |
| $7012 | ($7012) Status flag of the Smartcoder | Wxx_sens_emot_psn_sf | 00 | 0 |
| $7013 | ($7013) Raw rotor position resolver cosine voltage | Wxx_sens_emot_psn_raw_cos_0 | 07 70 | 0.4648437024 V |
| $7014 | ($7014) Raw rotor position resolver cosine voltage with 25µs delay | Wxx_sens_emot_psn_raw_cos_25 | 06 5A | 0.3969726156 V |
| $7015 | ($7015) Raw rotor position resolver excitation peak voltage | Wxx_sens_emot_psn_raw_exc | 0D 70 | 0.839843664 V |
| $7016 | ($7016) result of the check on CAN communication with EVC | Vbx_can_emot_tq_sp_nok | 00 | 0 = Check is Ok |
| $7017 | ($7017) Failure vector allowing exact identification of the failure.1 | Vxx_dcdc_diag_1 | 00 00 | 0 |
| $7018 | ($7018) Failure vector allowing exact identification of the failure.2 | Vxx_dcdc_diag_2 | 00 | 0 |
| $7019 | ($7019) Failure vector allowing exact identification of the failure.3 | Vxx_dcdc_diag_3 | 00 | 0 |
| $701A | ($701A) Failure vector allowing exact identification of the failure.4 | Vxx_dcdc_diag_4 | 00 00 | 0 |
| $701B | ($701B) Failure vector allowing exact identification of the DCDC event | Vxx_dcdc_diag_5 | 00 | 0 |
| $701C | ($701C) Saved rotor position offset.CRC |  | B3 DE 62 | (nicht dekodiert) |
| $701C | ($701C) Saved rotor position offset.value.physical | Sxx_emot_psn_ofs | B3 DE | 46046 |
| $701D | ($701D) Counter of FAN fail times to cool the machine stator in hard conditions.1 | Sxx_fan_nok_fail_1 | 00 00 | 0 |
| $701E | ($701E) Counter of FAN fail times to cool the machine stator in hard conditions.2 | Sxx_fan_nok_fail_2 | 00 00 | 0 |
| $701F | ($701F) Counter of FAN fail times to cool the machine stator in hard conditions.3 | Sxx_fan_nok_fail_3 | 00 00 | 0 |
| $7021 | ($7021) Saved resistance rotor.value |  | 08 98 9F | (nicht dekodiert) |
| $7021 | ($7021) Saved resistance rotor.CRC |  | 08 98 9F | (nicht dekodiert) |
| $7022 | ($7022) Active discharge request from EVC to PEB | Vbx_actv_dchg_req | 00 | 0 = Discharge not requested |
| $7023 | ($7023) Effective active discharge feedback to EVC | Vbx_actv_dchg_stt | 00 | 0 = Active Discharge Not In Progress |
| $7024 | ($7024) APC state (from CAN) | Vbx_apc_stt | 01 | 1 = APC on |
| $7025 | ($7025) Charge request from EVC to PEB | Vbx_chg_req | 00 | 0 = No charge requested |
| $7026 | ($7026) Effective Charge feedback to EVC | Vbx_chg_stt | 00 | 0 = No charge in progress |
| $7027 | ($7027) On/Off request of the DCDC from EVC | Vbx_dcdc_actv_req_evc | 01 | 1 = DCDC On |
| $7028 | ($7028) status verlog from PEB to EVC | Vbx_eng_run_ena_swlk | 01 | 1 = Vehicle unlocked |
| $7029 | ($7029) On/Off power train activation request from EVC | Vbx_invt_act | 00 | 0 = Inverter Off |
| $702B | ($702B) Auto Diag enable | Vsx_auto_diag_ena | 00 | 0 = Nsx_auto_diag_nok (0) : Auto diag not allowed |
| $702D | ($702D) Failure class of the DCDC converter | Vsx_dcdc_err_stt | 00 | 0 = Nsx_dcdc_err_stt_ok  : No Fault type |
| $702E | ($702E) DCDC state | Vsx_dcdc_stt | 04 | 4 = Pwer ON (Nsx_dcdc_stt_on) |
| $702F | ($702F) High Voltage Network Default | Vsx_hv_dfl_sfty | 01 | 1 = Nsx_hv_ok (1): No Default |
| $7030 | ($7030) Failure class of the Inverter | Vsx_invt_flt_typ | 00 | 0 = Nsx_invt_flt_type_0 (0): no downgraded mode |
| $7031 | ($7031) Interlock plug service status, from LBC (condition for reprogramming) | Vsx_lbc_ilck_stt | 00 | 0 = Closed |
| $7032 | ($7032) Torque Default | Vsx_tq_dfl_sfty | 00 | 0 = No default |
| $7033 | ($7033) DCDC input voltage | Vxx_dcdc_volt_in_mes_raw | 02 E8 | 372 V |
| $7034 | ($7034) Voltage demand for the DCDC converter from the EVC | Vxx_dcdc_volt_sp | 38 D6 | 14550 mV |
| $7035 | ($7035) Maximal available calculated torque in generator mode | Vxx_emot_max_gen_tq | A0 | 160 N·m |
| $7036 | ($7036) Maximal available calculated torque in motor mode | Vxx_emot_max_mot_tq | A0 | 160 N·m |
| $7037 | ($7037) Rotor position offset learning + CRC written by diag tool.1 |  | B4 00 00 | (nicht dekodiert) |
| $7037 | ($7037) Rotor position offset learning + CRC written by diag tool.2 |  | B4 00 00 | (nicht dekodiert) |
| $7038 | ($7038) Rotor copper temperature raw estimation | Vxx_esti_rot_cu_temp_raw | 17 DC | 7.71875 °C |
| $7039 | ($7039) Combination of inverter, Excitation, PCB, MC and DRI temperatures, so in % | Vxx_invt_norm_temp | 03 6E | 878 % |
| $703A | ($703A) Combination of rotor and stator temperatures, so in % | Vxx_mot_norm_temp | 01 51 | 337 % |
| $703B | ($703B) Resistance rotor learning + CRC written by diag tool.1 |  | 1E 00 00 | (nicht dekodiert) |
| $703B | ($703B) Resistance rotor learning + CRC written by diag tool.2 |  | 1E 00 00 | (nicht dekodiert) |
| $703C | ($703C) Vehicle distance totalizer from EVC | Vxx_vh_dist | 01 4D D9 | 85465 km |
| $703D | ($703D) The water temperature produced by the Rectifier | Vxx_wtr_temp | 11 | 17 °C |
| $703E | ($703E) Mode of the power train (motor or generator) | Vbx_emot_mod | 00 | 0 = Motor |
| $703F | ($703F) SPI communication default | Wbx_spi_dfl_sfty | 00 | 0 = No default |
| $7041 | ($7041) Rotor overcurrent validated | Wbx_sw_ocp_exct_mem | 00 | 0 = No default |
| $7042 | ($7042) Stator overcurrent validated | Wbx_sw_ocp_invt_mem | 00 | 0 = No default |
| $7043 | ($7043) activated downgraded mode FPGA number | Wsx_dm_act_fpga | 00 | 0 = Nsx_no_sfty_dfl : no downgraded mode |
| $7044 | ($7044) activated downgraded mode number | Wsx_dm_act_sfty | 00 | 0 = Nsx_no_sfty_dfl : no downgraded mode |
| $7045 | ($7045) Validity status of the rotor electrical rotation speed | Wsx_emot_n_dfl_sfty | 00 | 0 = Nsx_emot_n_chk_ok: OK |
| $7046 | ($7046) Excitation Current Default | Wsx_i_f_dfl_sfty | 00 | 0 = Nsx_ok (0) : no excitation sensor default |
| $7047 | ($7047) Phase Current Default | Wsx_i_mot_ph_dfl_sfty | 00 | 0 = Nsx_ok (0) : no sensor default |
| $7048 | ($7048) Validity status of the rotor electrical angular position | Wsx_theta_dfl_sfty | 00 | 0 = Nsx_dgt_emot_psn_chk_ok: OK |
| $7049 | ($7049) Combinated current value on the HV Network of Inverter and Excitation | Wxx_i_pwt | 3E 80 | 0 A |
| $704A | ($704A) TETA CRC received |  | 00 | (nicht dekodiert) |
| $704B | ($704B) Received TETA position offset | Vxx_emot_psn_ofs_rcp | B4 00 | 46080 |
| $704C | ($704C) Received rotor reference resistance CRC |  | 00 | (nicht dekodiert) |
| $704D | ($704D) Received value for reference rotor resistance | Vxx_rot_r_ref_rcp | 1C 00 | 7680 Ohm |
| $7050 | ($7050) Value for reference rotor resistance (for safety monitoring) | Sxx_rot_r_ref_sfty | 08 98 | 4.296875 Ohm |
| $7051 | ($7051) Position sensor type | Sbx_sens_emot_psn_typ | 01 | 1 = 1: Sensor type B = MINEBEA |
| $7052 | ($7052) Vector of 5 average dcdc temperature on 1s in the minute before break event for post mortem.4 | Stx_pm_dcdc_temp_clk_moy | 00 54 00 | 1024 °C |
| $7052 | ($7052) Vector of 5 average dcdc temperature on 1s in the minute before break event for post mortem.0 | Stx_pm_dcdc_temp_clk_moy | 00 50 00 | 0 °C |
| $7052 | ($7052) Vector of 5 average dcdc temperature on 1s in the minute before break event for post mortem.1 | Stx_pm_dcdc_temp_clk_moy | 00 50 00 | 0 °C |
| $7052 | ($7052) Vector of 5 average dcdc temperature on 1s in the minute before break event for post mortem.2 | Stx_pm_dcdc_temp_clk_moy | 00 50 00 | 0 °C |
| $7052 | ($7052) Vector of 5 average dcdc temperature on 1s in the minute before break event for post mortem.3 | Stx_pm_dcdc_temp_clk_moy | 00 50 00 | 0 °C |
| $7053 | ($7053) Vector of 5 average driver board temperature on 1s in the minute before break event for post mortem.0 | Stx_pm_dri_temp_clk_moy | 00 50 00 | 0 °C |
| $7053 | ($7053) Vector of 5 average driver board temperature on 1s in the minute before break event for post mortem.1 | Stx_pm_dri_temp_clk_moy | 00 50 00 | 0 °C |
| $7053 | ($7053) Vector of 5 average driver board temperature on 1s in the minute before break event for post mortem.2 | Stx_pm_dri_temp_clk_moy | 00 50 00 | 0 °C |
| $7053 | ($7053) Vector of 5 average driver board temperature on 1s in the minute before break event for post mortem.3 | Stx_pm_dri_temp_clk_moy | 00 50 00 | 0 °C |
| $7053 | ($7053) Vector of 5 average driver board temperature on 1s in the minute before break event for post mortem.4 | Stx_pm_dri_temp_clk_moy | 00 80 D8 | 12504 °C |
| $7054 | ($7054) Vector of 5 average excitation temperature on 1s in the minute before break event for post mortem.0 | Stx_pm_exct_temp_clk_moy | 00 50 00 | 0 °C |
| $7054 | ($7054) Vector of 5 average excitation temperature on 1s in the minute before break event for post mortem.1 | Stx_pm_exct_temp_clk_moy | 00 50 00 | 0 °C |
| $7054 | ($7054) Vector of 5 average excitation temperature on 1s in the minute before break event for post mortem.2 | Stx_pm_exct_temp_clk_moy | 00 50 00 | 0 °C |
| $7054 | ($7054) Vector of 5 average excitation temperature on 1s in the minute before break event for post mortem.3 | Stx_pm_exct_temp_clk_moy | 00 50 00 | 0 °C |
| $7054 | ($7054) Vector of 5 average excitation temperature on 1s in the minute before break event for post mortem.4 | Stx_pm_exct_temp_clk_moy | 00 5E 53 | 3667 °C |
| $7055 | ($7055) Vector of 5 average inverter temperature on 1s in the minute before break event for post mortem.0 | Stx_pm_invt_temp_clk_moy | 00 50 00 | 0 °C |
| $7055 | ($7055) Vector of 5 average inverter temperature on 1s in the minute before break event for post mortem.1 | Stx_pm_invt_temp_clk_moy | 00 50 00 | 0 °C |
| $7055 | ($7055) Vector of 5 average inverter temperature on 1s in the minute before break event for post mortem.2 | Stx_pm_invt_temp_clk_moy | 00 50 00 | 0 °C |
| $7055 | ($7055) Vector of 5 average inverter temperature on 1s in the minute before break event for post mortem.3 | Stx_pm_invt_temp_clk_moy | 00 50 00 | 0 °C |
| $7055 | ($7055) Vector of 5 average inverter temperature on 1s in the minute before break event for post mortem.4 | Stx_pm_invt_temp_clk_moy | 00 66 3E | 5694 °C |
| $7056 | ($7056) Vector of 5 average pcb temperature on 1s in the minute before break event for post mortem.0 | Stx_pm_pcb_temp_clk_moy | 00 50 00 | 0 °C |
| $7056 | ($7056) Vector of 5 average pcb temperature on 1s in the minute before break event for post mortem.1 | Stx_pm_pcb_temp_clk_moy | 00 50 00 | 0 °C |
| $7056 | ($7056) Vector of 5 average pcb temperature on 1s in the minute before break event for post mortem.2 | Stx_pm_pcb_temp_clk_moy | 00 50 00 | 0 °C |
| $7056 | ($7056) Vector of 5 average pcb temperature on 1s in the minute before break event for post mortem.3 | Stx_pm_pcb_temp_clk_moy | 00 50 00 | 0 °C |
| $7056 | ($7056) Vector of 5 average pcb temperature on 1s in the minute before break event for post mortem.4 | Stx_pm_pcb_temp_clk_moy | 00 79 6E | 10606 °C |
| $7057 | ($7057) Vector of 5 average wtr temperature on 1s in the minute before break event for post mortem.0 | Stx_pm_wtr_temp_clk_moy | 00 50 00 | 0 °C |
| $7057 | ($7057) Vector of 5 average wtr temperature on 1s in the minute before break event for post mortem.1 | Stx_pm_wtr_temp_clk_moy | 00 50 00 | 0 °C |
| $7057 | ($7057) Vector of 5 average wtr temperature on 1s in the minute before break event for post mortem.2 | Stx_pm_wtr_temp_clk_moy | 00 50 00 | 0 °C |
| $7057 | ($7057) Vector of 5 average wtr temperature on 1s in the minute before break event for post mortem.3 | Stx_pm_wtr_temp_clk_moy | 00 50 00 | 0 °C |
| $7057 | ($7057) Vector of 5 average wtr temperature on 1s in the minute before break event for post mortem.4 | Stx_pm_wtr_temp_clk_moy | 00 68 00 | 6144 °C |
| $7058 | ($7058) 1s average dcdc temperature on break event for post mortem | Sxx_pm_dcdc_temp_brk_moy | 00 A2 00 | 20992 °C |
| $7059 | ($7059) 1s average driver board temperature on break event for post mortem | Sxx_pm_dri_temp_brk_moy | 00 8E 72 | 15986 °C |
| $705A | ($705A) Derating counter for post mortem | Sxx_pm_drt_ctr | 0C 01 | 3073 |
| $705B | ($705B) Blocked rotor counter for post mortem | Sxx_pm_drt_igbt_ctr | 00 00 | 0 |
| $705C | ($705C) Water overheat temperature counter for post mortem | Sxx_pm_drt_tco_ctr | 00 00 | 0 |
| $705D | ($705D) 1s average excitation temperature on break event for post mortem | Sxx_pm_exct_temp_brk_moy | 00 6A 5B | 6747 °C |
| $705E | ($705E) 1s average inverter temperature on break event for post mortem | Sxx_pm_invt_temp_brk_moy | 00 70 5F | 8287 °C |
| $705F | ($705F) 1s average pcb temperature on break event for post mortem | Sxx_pm_pcb_temp_brk_moy | 00 84 FA | 13562 °C |
| $7061 | ($7061) Maximum driver board temperature during last derating | Sxx_pm_temp_dri_drt_max | 00 86 98 | 13976 °C |
| $7062 | ($7062) Maximum excitation temperature during last derating | Sxx_pm_temp_exct_drt_max | 00 5B D4 | 3028 °C |
| $7063 | ($7063) Maximum inverter temperature during last derating | Sxx_pm_temp_inv_drt_max | 01 7C 00 | 76800 °C |
| $7064 | ($7064) Maximum pcb temperature during last derating | Sxx_pm_temp_pcb_drt_max | 00 7E 1D | 11805 °C |
| $7065 | ($7065) Maximum water temperature during last derating | Sxx_pm_wtr_temp_brk_moy | 00 72 00 | 8704 °C |
| $7066 | ($7066) Charge or non zero torque time counter for post mortem analysis | Sxx_pm_tra_chg_t_ctr | 80 C0 96 D9 | -2134862119 s |
| $7067 | ($7067) APC and zero torque time counter for post mortem analysis | Sxx_pm_apc_ztq_t_ctr | 80 0D 29 DE | -2146620962 s |
| $7068 | ($7068) Software overcurrent protection counter for post mortem analysis | Sxx_pm_sw_ocp_invt_ctr | 00 00 | 0 |
| $7069 | ($7069) Hardware (by the fpga) overcurrent protection counter for post mortem analysis | Sxx_pm_hw_ocp_invt_ctr | 16 93 | 5779 |
| $706A | ($706A) Hardware (by the fpga) overvoltage protection counter for post mortem analysis | Sxx_pm_hw_ovp_ctr | 01 8B | 395 |
| $706B | ($706B) Software overvoltage protection counter for post mortem analysis | Sxx_pm_sw_ovp_ctr | 00 04 | 4 |
| $706C | ($706C) Alim 21 monitoring | Vbx_alim_21_mon | 00 | 0 = No default |
| $706D | ($706D) Alim 21 with redundance monitoring | Vbx_alim_21_mon_red | 00 | 0 = No default |
| $706E | ($706E) DID for activation of bypass frame | Vbx_byp_frm_act | 00 | 0 = Frame inactivated |
| $706F | ($706F) Excitation high arm IGBT driver fault | Vbx_exct_flt_dri_h | 00 | 0 = No default |
| $7070 | ($7070) Excitation low arm IGBT driver fault | Vbx_exct_flt_dri_l | 00 | 0 = No default |
| $7071 | ($7071) Inverter high arm IGBT driver fault | Vbx_invt_flt_dri_h | 00 | 0 = No default |
| $7072 | ($7072) Inverter low arm IGBT driver fault | Vbx_invt_flt_dri_l | 00 | 0 = No default |
| $7073 | ($7073) Excitation over current protection (before flipflop) High | Vbx_ocp_exct_h | 00 | 0 = No default |
| $7074 | ($7074) Excitation over current protection (before flipflop) Low | Vbx_ocp_exct_l | 00 | 0 = No default |
| $7075 | ($7075) Inverter over current protection (before flipflop) Arm U | Vbx_ocp_invt_u | 00 | 0 = No default |
| $7076 | ($7076) Inverter over current protection (before flipflop) Arm V | Vbx_ocp_invt_v | 00 | 0 = No default |
| $7077 | ($7077) Inverter over current protection (before flipflop) Arm W | Vbx_ocp_invt_w | 00 | 0 = No default |
| $7078 | ($7078) Over voltage protection (before flipflop) | Vbx_ovp_udc | 00 | 0 = No default |
| $7079 | ($7079) Copy Over voltage protection (before flipflop) | Vbx_pwm_dfl_sfty_mem | 00 | 0 = No default |
| $707A | ($707A) Excitation driver state (ready or not) | Vbx_rdy_exct | 01 | 1 = Ready |
| $707B | ($707B) Inverter high IGBT driver state (ready or not) | Vbx_rdy_invt_h | 01 | 1 = Ready |
| $707C | ($707C) Inverter low IGBT driver state (ready or not) | Vbx_rdy_invt_l | 01 | 1 = Ready |
| $707D | ($707D) Supply monitoring default | Vbx_sply_mon | 00 | 0 = No default |
| $707E | ($707E) Alimentation 11 state | Vbx_sta_stop_alim_11 | 01 | 1 = Start |
| $707F | ($707F) Alimentation 21 state | Vbx_sta_stop_alim_21 | 01 | 1 = Start |
| $7081 | ($7081) Bypass mode | Vsx_byp_mod | 00 | 0 = Bypass off |
| $7082 | ($7082) FPGA fault sent by SPI | Vsx_hw_fpga_id | 00 | 0 = no fault |
| $7083 | ($7083) INV Automation State | Vsx_invt_stt | 05 | 5 = Stand By |
| $7084 | ($7084) DCDC Input current measure | Vxx_dcdc_cur_in_mes_raw | 00 00 | 0 A |
| $7085 | ($7085) DCDC output current measure | Vxx_dcdc_cur_ou_est_raw | 00 15 | 21 A |
| $7086 | ($7086) DCDC power card temperature | Vxx_dcdc_temp_pw_sens | 29 | 41 °C |
| $7087 | ($7087) Observer estimation for stack rotor temperature | Vxx_esti_rot_ac_temp | 1A BF | 1727 °C |
| $7088 | ($7088) Rotor copper temperature estimation | Vxx_esti_rot_cu_temp | 1A C1 | 1729 °C |
| $7089 | ($7089) Stator copper temperature estimation | Vxx_esti_sttr_cu_temp | 1A A5 | 1701 °C |
| $708A | ($708A) FPGA firmware version Identifier | Vxx_fpga_fw_id | 27 40 | 10048 |
| $708B | ($708B) Rotor position measure | Vxx_sens_emot_psn | 52 B3 | 21171 |
| $708C | ($708C) Copy Raw rotor position resolver excitation peak voltage | Vxx_sens_emot_psn_raw_exc | 0D 6E | 3438 V |
| $708D | ($708D) Rotor position measure (for safety monitoring) | Vxx_sens_emot_psn_sfty | 51 83 | 20867 |
| $708E | ($708E) Spare signal for driver board temperature | Vxx_wf_dri_temp | 00 8E 74 | 15988 °C |
| $708F | ($708F) Opening relays wire request | Wbx_wi_hv_rly_open | 00 | 0 = Not open |
| $7091 | ($7091) Last driver board temperature on 1s in the minute before break event for post mortem | Sxx_pm_dri_temp_clk_moy | 00 80 D8 | 12504 °C |
| $7092 | ($7092) CAN Etude activation | Sbx_can_etu_ena | 00 | 0 = deactivated |
| $7093 | ($7093) Number of Derating CFA functionnal failure 1 cycles saved | Sxx_fan_der_fail_1 | 00 00 | 0 |
| $7094 | ($7094) Number of Derating CFA functionnal failure 2 cycles saved | Sxx_fan_der_fail_2 | 00 00 | 0 |
| $7095 | ($7095) Number of Derating CFA functionnal failure 3 cycles saved | Sxx_fan_der_fail_3 | 00 00 | 0 |
| $7096 | ($7096) Normalized inverter-Excitation temperature | Vxx_invt_exct_norm_temp | 02 16 | 534 % |
| $7097 | ($7097) Normalized PCB-Micro_driverboard temperature | Vxx_pcb_mc_dri_norm_temp | 03 6E | 878 % |
| $7098 | ($7098) Excitation currents safety check | Wsx_i_f_chk_sfty | 00 | 0 = Both currents are consistent  (difference <thershold) |
| $7099 | ($7099) Phase currents safety check | Wsx_i_mot_ph_chk_sfty | 00 | 0 = the summ of the 3 currents is close to zero (< threshold) |
| $709A | ($709A) Max CPU load at 10ms | Vtx_TaskMaxDurationIT_A[12] | 4F | 79 µs |
| $709B | ($709B) CPU load at 10ms | Vtx_TaskDurationIT_A[12] | 46 | 70 µs |
| $709C | ($709C) Gestion of current sensors | Vsx_pwm_oc_auto_diag | 00 | 0 = No Default |
| $709D | ($709D) redundant supply 21 failure counter for post mortem analysis | Sxx_pm_alim_21_red_ctr | 00 00 | 0 |
| $709E | ($709E) SeedBreaker to explain the reset cause | Vxx_invt_rst_ctx_1 | 00 00 | 0 |
| $709F | ($709F) Max of CPU load in micro-seconds (Vtx_TaskMaxDurationIT_A[12]) | Vxx_invt_rst_ctx_2 | 00 00 | 0 |
| $70A1 | ($70A1) Max of 10ms task (Vtx_TaskMaxDurationIT_A[3]) | Vxx_invt_rst_ctx_3 | 00 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vbx_rot_temp_dfl_sfty_(bit0) | Vxx_invt_rst_ctx_4 | 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vsx_emot_n_dfl_sfty_(bit1) | Vxx_invt_rst_ctx_4 | 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vbx_dfl_i_f_low_(bit2) | Vxx_invt_rst_ctx_4 | 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vbx_alim_21_mon_(bit3) | Vxx_invt_rst_ctx_4 | 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vbx_alim_21_mon_red_(bit4) | Vxx_invt_rst_ctx_4 | 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vbx_sply_mon_(bit5) | Vxx_invt_rst_ctx_4 | 00 | 0 |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Stack Overflow on State_(bit6-7) | Vxx_invt_rst_ctx_4 | 00 | 0 = No overflow |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vtx_TaskMaxDurationIT_A[3]_(bit8-11) | Vxx_invt_rst_ctx_4 | 00 | 0 ms |
| $70A2 | ($70A2) Vxx_invt_rst_ctx_4.($70A2) Vxx_10ms_max_duration_(bit12-15) | Vxx_invt_rst_ctx_4 | 00 | 0 ms |
| $70A3 | ($70A3) Maximum normalized torque corresponding to max current saturation value | Vxx_norm_tq_cur_sat_max | 80 00 | 1.000001434 |
| $70A4 | ($70A4) Minimum normalized torque corresponding to max current saturation value | Vxx_norm_tq_cur_sat_min | 00 00 | -1 |
| $70A5 | ($70A5) Maximum instant value of normalized torque for cell overvoltage | Vxx_norm_tq_max_cell_ov_fac | 80 00 | 1.000001434 |
| $70A6 | ($70A6) Maximum instant value of normalized torque for temperature derating | Vxx_norm_tq_max_temp | 80 00 | 1.000001434 |
| $70A7 | ($70A7) Maximum instant value of normalized torque for voltage derating | Vxx_norm_tq_max_udc | 80 00 | 1.000001434 |
| $70A8 | ($70A8) Minimum instant value of normalized torque for cell overvoltage | Vxx_norm_tq_min_cell_ov_fac | 80 00 | 1.000001434 |
| $70A9 | ($70A9) Vxx_invt_rst_ctx_5 | Vxx_invt_rst_ctx_5 | 00 00 | 0 |
| $70CA | ($70CA) Number of the bloc which has nvrol error | Vxx_inv_nvrol_err_bloc | 00 | 0 |
| $70CB | ($70CB) Nvrol error counters.8 bits MSB.8 bits LSB | Vxx_inv_nvrol_err_cnt | 00 00 | 0 |
| $70D5 | ($70D5) Rotor position sensor safety check | Vbx_emot_psn_chk_sfty | 01 | 1 = Both positions are consistent  (difference <thershold) |
| $70D6 | ($70D6) Rotor Positions Diag | Vsx_dgt_emot_psn_sens_chk_sfty | 00 | 0 = both diag are OK |
| $70D7 | ($70D7) HVNetworkVoltage from Battery | Vxx_hv_bat | 02 E2 | 369 V |
| $70D8 | ($70D8) HVNetworkVoltage from DCDC | Vxx_hv_dcdc | 02 EA | 373 V |
| $70E7 | ($70E7) Normalized rotor temperature | Vxx_rot_norm_temp | 00 00 | 0 % |
| $70E8 | ($70E8) Stator temperature sensor 1 for freeze frame management | Vxx_sttr_temp_1_ffm | 00 D4 | 13 °C |
| $70E9 | ($70E9) Stator temperature sensor 2 for freeze frame management | Vxx_sttr_temp_1_ffm | 00 C5 | 9.25 °C |
| $70EA | ($70EA) spare signal for driver board temperature for freeze frame management | Vxx_sttr_temp_1_ffm | 01 1D | 31.25 °C |
| $70EB | ($70EB) Excitation temperature for freeze frame management | Vxx_sttr_temp_1_ffm | 00 D5 | 13.25 °C |
| $70EC | ($70EC) Inverter temperature sensor 1 for freeze frame management | Vxx_sttr_temp_1_ffm | 00 E0 | 16 °C |
| $70ED | ($70ED) MICRO temperature for freeze frame management | Vxx_sttr_temp_1_ffm | 01 27 | 33.75 °C |
| $70EE | ($70EE) PCB temperature for freeze frame management | Vxx_sttr_temp_1_ffm | 01 0A | 26.5 °C |
| $70EF | ($70EF) Counter giving the nb of times Wsx_cuc_f_dfl is equal to Nsx_i_f_df | Sxx_cuc_f_dfl_3_ctr | 40 19 | 16409 |
| $70F0 | ($70F0) Absolute time since first Ignition | Vxx_t_abs_igk_1 | 00 25 F2 81 | 2149970561 min |
| $70F1 | ($70F1) Max ratio between the square of excitation sgnl and the sun of squares of rotor reslver cos and sin sgnl | Vxx_emot_psn_sq_rat_max | 00 00 | 0 |
| $70F2 | ($70F2) Environment temperature | Vxx_air_tenv | 1B 00 | 14 °C |
| $70F3 | ($70F3) Ratio between square of excitation signal and sum of squares of rotor reslver cos and sine signals | Vxx_emot_psn_sq_rat | 01 60 | 0.6875 |
