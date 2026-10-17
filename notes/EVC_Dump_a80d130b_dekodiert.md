# EVC-Dump a80d130b (Auto wach, 04.11.2025 13:15 UTC) dekodiert mit EMS3194_0A40_300_V2.1

Quelle: 36c02ace-…xml + a80d130b-…txt. Bitzaehlung: BitOffset ab MSB des ersten Datenbytes (an $3202=05 und $343F geprueft).

| DID | Name | Variable | Rohbytes | Wert |
|---|---|---|---|---|
| $2000 | DIDs supported in range [$2001 - $2020] |  | FC 37 00 01 | (nicht dekodiert) |
| $2001 | ($2001) Battery Rack temperature | Temp_TempBhv_BMS_mes | 32 | 10 °C |
| $2002 | ($2002) State Of Charge (SOC) HV battery | Pct_UserSOC_bhv_est | 0B B3 | 59.9 % |
| $2003 | ($2003) Vehicle speed | Vxx_vs | 00 00 | 0 km/h |
| $2004 | ($2004) consolidated HV voltage | Volt_HVNet_cons | 02 E1 | 368.5 V |
| $2005 | ($2005) Battery voltage 14v | Vxx_vb | 05 40 | 13.44 V |
| $2006 | ($2006) Total vehicle distance | Vxx_vh_dist | 01 4D D9 | 85465 km |
| $200B | ($200B) Accelerator pedal voltage _ track 1 | Vxx_acel_pdl_sens_v1 | 02 DC | 732 mV |
| $200C | ($200C) Accelerator pedal voltage _ track 2 | Vxx_acel_pdl_sens_v2 | 01 6E | 366 mV |
| $200E | ($200E) Key state | Vbx_igk | 01 | 1 = key on |
| $200F | ($200F) Brake pedal _ switches consolidation state | Vsx_brk_pdl | 01 | 1 = not pressed |
| $2010 | ($2010) Engine status | Vsx_eng | 00 | 0 = 0:engine stopped |
| $2020 | DIDs supported in range [$2021 - $2040] |  | CC 04 0C FF | (nicht dekodiert) |
| $2021 | ($2021) Sensors supply voltage 1 | Vxx_pws_1 | 13 87 | 4999 mV |
| $2022 | ($2022) Sensors supply voltage 2 | Vxx_pws_2 | 13 88 | 5000 mV |
| $2025 | ($2025) Brake pedal _ close active switch state | Vsx_brk_pdl_clos | 02 | 2 = pressed |
| $2026 | ($2026) Brake pedal _ open active switch state | Vsx_brk_pdl_open | 01 | 1 = not pressed |
| $202E | ($202E) Accelerator pedal position | Vxx_acel_pdl_rat | 00 00 | 0 |
| $2035 | ($2035) CCSL _ Cruise control speed setpoint | Vxx_cru_sl_driv_vs_sp_kmh | 00 | 0 km/h |
| $2036 | ($2036) Counter of inconsistencies between accelerator pedal and brake | Vxx_acel_pdl_secu_ctr | 00 | 0 |
| $2039 | ($2039) CCSL _ Steering wheel push buttons voltage | Vxx_cru_sl_whl_cmd_in | 0F 9A | 3994 mV |
| $203A | ($203A) Configuration _ Cruise control (option) | Vbx_cru_lk_stt | 01 | 1 = with |
| $203B | ($203B) Configuration _ Cruise control (OnOff button) | Vbx_cru_swi_lk_stt | 01 | 1 = with |
| $203C | ($203C) Cruise control OnOff button state | Vbx_cru_on_off_in | 00 | 0 = off |
| $203D | ($203D) Configuration _ Speed limiter (option) | Vbx_sl_lk_stt | 01 | 1 = with |
| $203E | ($203E) Configuration _ Speed limiter (OnOff button) | Vbx_sl_swi_lk_stt | 01 | 1 = with |
| $203F | ($203F) Speed limiter OnOff button state | Vbx_sl_on_off_in | 00 | 0 = off |
| $2040 | ($2040) DIDs supported in range [$2041 - $2060] |  | 80 3F 80 21 | (nicht dekodiert) |
| $2041 | ($2041) Configuration _ CCSL steering wheel push buttons | Vbx_whl_lk_stt | 01 | 1 = with |
| $204B | ($204B) CCSL _ Steering wheel push buttons state | Vxx_cru_sl_whl_cmd_raw | 00 | 0 = unpressed |
| $204C | ($204C) CCSL _ Status | Vxx_cru_sl_stt_ip | 00 | 0 = CC and SL off |
| $204D | ($204D) CCSL - State of the failures which cause irreversible CC safety failure.0 | RV/LV - Etat des défauts entraînant un défaut SdF RV irréversible | 00 | 0 = No Presence of CC force request despite the CC deactivation |
| $204D | ($204D) CCSL - State of the failures which cause irreversible CC safety failure.1 | RV/LV - Etat des défauts entraînant un défaut SdF RV irréversible | 00 | 0 = No activation of the open brake switch without CC deactivation |
| $204D | ($204D) CCSL - State of the failures which cause irreversible CC safety failure.2 | RV/LV - Etat des défauts entraînant un défaut SdF RV irréversible | 00 | 0 = No activation of the minimum travel clutch switch without CC deactivation |
| $204E | ($204E) CCSL - State of the reversible failures not due to CCSL which cause CCSL failure.0 | Real vehicle speed unavailable | 20 | 0 = No real vehicle speed unavailable |
| $204E | ($204E) CCSL - State of the reversible failures not due to CCSL which cause CCSL failure.1 | Displayed vehicle speed unavailable | 20 | 0 = No displayed vehicle speed unavailable |
| $204E | ($204E) CCSL - State of the reversible failures not due to CCSL which cause CCSL failure.2 | Real vehicle speed absence | 20 | 0 = No real vehicle speed absence |
| $204E | ($204E) CCSL - State of the reversible failures not due to CCSL which cause CCSL failure.3 | Displayed vehicle speed absence | 20 | 0 = No displayed vehicle speed absence |
| $204E | ($204E) CCSL - State of the reversible failures not due to CCSL which cause CCSL failure.4 | Change of the displayed speed unit | 20 | 0 = No change of the displayed speed unit |
| $204E | ($204E) CCSL - State of the reversible failures not due to CCSL which cause CCSL failure.5 | SL engine inhibition | 20 | 1 = SL engine inhibition |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.0 | reserved | 80 | 0 |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.1 | reserved | 80 | 0 |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.2 | Brake information unavailable | 80 | 0 = No brake information unavailable |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.3 | Brake information absence | 80 | 0 = No brake information absence |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.4 | Braking detected without brake information | 80 | 0 = No braking detected without brake information |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.5 | Sudden braking detected without brake information | 80 | 0 = No sudden braking detected without brake information |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.6 | Gearbox in limp home mode | 80 | 0 = No Multi Function Contactor failure |
| $204F | ($204F) CCSL - State of the reversible failures not due to CC which cause CC failure.7 | CC engine inhibition | 80 | 1 = CC engine inhibition |
| $2050 | ($2050) Displayed vehicle speed received on the CAN network | Vxx_mux_dspl_vs | 00 00 | 0 km/h |
| $2051 | ($2051) Displayed vehicle speed unit | Vbx_dspl_vs_in_mph | 00 | 0 = km/h |
| $205B | ($205B) Limp home activation state | Vbx_acel_pdl_lih_act | 00 | 0 = deactivated |
| $2060 | DIDs supported in range [$2061 - $2080] |  | 00 64 01 FD | (nicht dekodiert) |
| $206A | ($206A) Brake pedal _ duration of the close active switch blocked | Vxx_brk_pdl_clos_blk_dly | 00 00 | 0 s |
| $206B | ($206B) Brake pedal _ duration of the open active switch blocked | Vxx_brk_pdl_open_blk_dly | 00 00 | 0 s |
| $206E | ($206E) Configuration _ Brake pedal _ open active switch | Vbx_sens_brk_pdl_open_lk_stt | 01 | 1 = with |
| $2078 | ($2078) CCSL - State of the causes for normal CCSL deactivation.0 | Main switch changed | 00 | 0 = No Main switch changed |
| $2078 | ($2078) CCSL - State of the causes for normal CCSL deactivation.1 | Suspend button pressed | 00 | 0 = No Suspend button pressed |
| $2078 | ($2078) CCSL - State of the causes for normal CCSL deactivation.2 | Braking | 00 | 0 = No Braking |
| $2078 | ($2078) CCSL - State of the causes for normal CCSL deactivation.3 | Clutch pedal pressed | 00 | 0 = no Clutch pedal pressed or no connection |
| $2078 | ($2078) CCSL - State of the causes for normal CCSL deactivation.4 | Gearbox on a neutral position | 00 | 0 = No Gearbox on a neutral position |
| $2078 | ($2078) CCSL - State of the causes for normal CCSL deactivation.5 | Gearbox on a neutral position | 00 | 0 = No Eco mode request change |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.0 | Start button pressed | 00 | 0 = Start button not pressed |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.1 | Gearbox on a reverse position | 00 | 0 = Gearbox not on a reverse position |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.2 | ASR or AYC in regulation | 00 | 0 = ASR or AYC not in regulation |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.3 | Speed / setpoint ratio too small | 00 | 0 = Speed / setpoint ratio not too small |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.4 | Assisted or manual parking brake | 00 | 0 = No Assisted nor manual parking brake |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.5 | CC system engine control inhibition | 00 | 0 = No CC system engine control inhibition |
| $2079 | ($2079) CCSL - State of the system causes for normal CCSL deactivation.6 | SL system engine control inhibition | 00 | 0 = No SL system engine control inhibition |
| $207A | ($207A) Maximum duration of resume button pressed | Vxx_cru_sl_resu_blk_dly_max | 00 | 0 s |
| $207B | ($207B) Maximum duration of set + button pressed | Vxx_cru_sl_acel_blk_dly_max | 00 | 0 s |
| $207C | ($207C) Maximum duration of set manus button pressed | Vxx_cru_sl_decl_blk_dly_max | 00 | 0 s |
| $207D | ($207D) Maximum duration of suspend button pressed | Vxx_cru_sl_susp_blk_dly_max | 00 | 0 s |
| $207E | ($207E) Maximum value of blocked button detection counter | Vxx_cru_sl_whl_cmd_blk_ctr_max | 00 | 0 |
| $2080 | DIDs supported in range [$2081 - $20A0] |  | 00 00 20 00 | (nicht dekodiert) |
| $2093 | ($2093) Test of underload made on the freon pressure measure | VH_UnderLoadState | 03 | 3 = Test conditions satisfied and correct cooling gas load |
| $2120 | DIDs supported in range [$2121-$2140] |  | 00 1C 00 00 | (nicht dekodiert) |
| $212E | ($212E) A crash occured and the diagnostic add-on strategy is frozen | Vbx_crsh_end_rec_eprm | 00 | 0 = No crash detected |
| $2160 | DIDs supported in range [$2161 - $2180] |  | 04 01 00 01 | (nicht dekodiert) |
| $2166 | ($2166) External controls safety authorization flag | Vbx_ext_ctrl_ena | 01 | 1 = Enabled |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).0 | vehicle STOP information on the cluster | 00 | 0 = no vehicle STOP information on the cluster |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).1 | Reserved | 00 | 0 |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).2 | Reserved | 00 | 0 |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).3 | Reserved | 00 | 0 |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).4 | Reserved | 00 | 0 |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).5 | Reserved | 00 | 0 |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).6 | Reserved | 00 | 0 |
| $2170 | ($2170) CCSL - State of the system causes for normal CCSL deactivation (2nd byte).7 | Reserved | 00 | 0 |
| $2180 | DIDs supported in range [$2181 - $21A0] |  | 90 00 00 00 | (nicht dekodiert) |
| $2181 | ($2181) Mileage recording | Vxx_fsl_km | 00 00 00 00 | 0 km |
| $2184 | ($2184) Requested speed setpoint for FSL function | Vxx_max_vs | 36 B0 | 140 km/h |
| $2220 | DIDs supported in range [$2221 - $2240] |  | BF 23 FF FF | (nicht dekodiert) |
| $2221 | ($2221) Brake pedal states validity indicator is in the limp home data status | Vbx_brk_pdl_vld_fail | 00 | 0 = deactivated |
| $2223 | ($2223) Engine control request for cruise control abnormal deactivation | Vbx_cru_eng_dsb | 00 | 0 = deactivated |
| $2224 | ($2224) Engine control request for cruise control system deactivation | Vbx_cru_eng_dsb_sys | 00 | 0 = deactivated |
| $2225 | ($2225) Vehicle speed received on the CAN network not available after a filtering time | Vbx_vs_navl_fil | 00 | 0 = deactivated |
| $2226 | ($2226) Displayed vehicle speed received on the CAN network not available after a filtering time | Vbx_dspl_vs_navl_fil | 00 | 0 = deactivated |
| $2227 | ($2227) Engine control request for speed limiter abnormal deactivation | Vbx_sl_eng_dsb | 00 | 0 = deactivated |
| $2228 | ($2228) Engine control request for speed limiter system deactivation | Vbx_sl_eng_dsb_sys | 00 | 0 = deactivated |
| $222B | ($222B) Automatic or manual parking brake detected | Vbx_bpk | 00 | 0 = deactivated |
| $222F | ($222F) Authorization to connect cruise control and speed limiter options | Vbx_cru_sl_new_lk_ena | 00 | 0 = Disabled |
| $2230 | ($2230) ABS in regulation | Vbx_abs_reg | 00 | 0 = No activated |
| $2231 | ($2231) Authorized ASR torque request | Vbx_asr_tqe_req_ok | 00 | 0 = Not Autorized |
| $2232 | ($2232) AYC in regulation | Vbx_ayc_reg | 00 | 0 = No activated |
| $2233 | ($2233) Request to turn the brake lights on (1) during regenerative braking | Vbx_brk_lght_req | 00 | 0 = Brake light off |
| $2234 | ($2234) PTC accessories activation flag. | Vbx_emot_acs_act | 00 | 0 = No activated |
| $2235 | ($2235) Flag indicating that a MSR torque request is being applied | Vbx_msr_tqe_req_ok | 00 | 0 = No request of MSR torque |
| $2236 | ($2236) Foot_up situation detected | Vbx_no_driv_req | 01 | 1 = Foot up detected |
| $2237 | ($2237) Flag to limit the regenerative braking demand in case MSR function is faulty | Vbx_no_pwt_sp_forc_lim | 00 | 0 = No limitation |
| $2238 | ($2238) Gear level position | Vsx_gear | 01 | 1 = park |
| $2239 | ($2239) Powertrain setpoint (%) requested by the driver via the accelerator pedal | Vxx_acel_pdl_pwt_sp | 00 00 | 0 |
| $223A | ($223A) Force demand at the wheels (output of the drivers' arbitration) | Vxx_arb_driv_forc_sp | 89 60 | 1200 N |
| $223B | ($223B) Electrical motor (EM) torque setpoint before AJS Correction action | Vxx_arb_emot_tqe_sp | 80 00 | 0 N·m |
| $223C | ($223C) ASR setpoint torque sent to torque arbitration | Vxx_asr_tqe_sp | FF FF | 1647.96875 N·m |
| $223D | ($223D) Torque correction calculated by AJCOR function | Vxx_eajs_cor_dyn_tqe | 80 00 | 0 N·m |
| $223E | ($223E) Final torque setpoint after AJMOD action | Vxx_eajs_modu_tqe_sp | 80 00 | 0 N·m |
| $223F | ($223F) Force demand derived from accelerator pedal position | Vxx_emot_acel_pdl_forc_sp | 89 60 | 1200 N |
| $2240 | DIDs supported in range [$2241 - $2260] |  | FF FC 00 00 | (nicht dekodiert) |
| $2241 | ($2241) MSR torque setting sent to torque arbitration | Vxx_msr_tqe_sp | 00 00 | -400 N·m |
| $2242 | ($2242) Filtered electric motor (EM) speed | Vxx_emot_n_mv | 80 00 | 0 rpm |
| $2243 | ($2243) Final effective torque request to the electric motor (EM) | Vxx_last_emot_tqe_sp | 80 00 | 0 N·m |
| $2244 | ($2244) Lever position word | Vxx_levr_psn | 00 | 0 |
| $2245 | ($2245) Limited electrical motor (EM) effective torque setpoint | Vxx_lim_emot_tqe_sp | 80 00 | 0 N·m |
| $2246 | ($2246) Electrical motor (EM) maximum effective torque available | Vxx_max_emot_tqe | 80 00 | 0 N·m |
| $2247 | ($2247) Minimum effective torque that can be requested to the electrical motor (EM) | Vxx_min_emot_tqe | 80 00 | 0 N·m |
| $2248 | ($2248) Force demand (at the wheels) when powertrain setpoint is equal to zero | Vxx_no_pwt_sp_forc | 89 60 | 1200 N |
| $2249 | ($2249) Force demand when ptw setpoint is equal to 0,before saturation accounting for min tq demand available | Vxx_no_pwt_sp_forc_bfr_sat | 89 60 | 1200 N |
| $224A | ($224A) Voltage measure of main relay (VBR) | Vxx_vb_rly | 53 EA | 13.43935402 V |
| $224B | ($224B) Data used to validated a detection of braking | Vsx_brk_pdl_vld | 01 | 1 = Ok |
| $224C | ($224C) Gear lever position validity flag | Vsx_gear_vld | 01 | 1 = Ok |
| $224D | ($224D) METS Vehicle speed validity flag | Vsx_VehicleSpeed_vld | 01 | 1 = Ok |
| $224E | ($224E) Engine speed validity flag from ECC | Vsx_emot_n_vld | 01 | 1 = Ok |
| $2500 | DIDs supported in range [$2501 - $2520] |  | 00 00 00 02 | (nicht dekodiert) |
| $251F | ($251F) Vehicle speed threshold detected | Vbx_vs_thd_done | 01 | 1 = 1: Vehicle speed threshold detected |
| $2860 | DIDs supported in range [$2861 - $2880] |  | 00 00 08 00 | (nicht dekodiert) |
| $2875 | ($2875) External controls denial status flag.7 | Contrôle extérieur des défauts relevés | 00 | 0 = Temperature before turbine correcte |
| $2875 | ($2875) External controls denial status flag.6 | Contrôle extérieur des défauts relevés | 00 | 0 = Pression turbo correcte |
| $2875 | ($2875) External controls denial status flag.5 | Contrôle extérieur des défauts relevés | 00 | 0 = Normal Engine Speed |
| $2875 | ($2875) External controls denial status flag.4 | Contrôle extérieur des défauts relevés | 00 | 0 = Stop and Start not ON |
| $2875 | ($2875) External controls denial status flag.3 | Contrôle extérieur des défauts relevés | 00 | 0 = Not null vehicle speed |
| $2875 | ($2875) External controls denial status flag.2 | Contrôle extérieur des défauts relevés | 00 | 0 = Accelerator pedal Not presed |
| $2875 | ($2875) External controls denial status flag.1 | Contrôle extérieur des défauts relevés | 00 | 0 = Ignition in ON position |
| $2875 | ($2875) External controls denial status flag.0 | Contrôle extérieur des défauts relevés | 00 | 0 = MGB in neutral position OR other kind of transmission in park or in neutral position |
| $2875 | ($2875) External controls denial status flag.15 | Contrôle extérieur des défauts relevés | 00 | 0 = - |
| $2875 | ($2875) External controls denial status flag.14 | Contrôle extérieur des défauts relevés | 00 | 0 = - |
| $2875 | ($2875) External controls denial status flag.13 | Contrôle extérieur des défauts relevés | 00 | 0 = - |
| $2875 | ($2875) External controls denial status flag.12 | Contrôle extérieur des défauts relevés | 00 | 0 = - |
| $2875 | ($2875) External controls denial status flag.11 | Contrôle extérieur des défauts relevés | 00 | 0 = brake pedal not pressed |
| $2875 | ($2875) External controls denial status flag.10 | Contrôle extérieur des défauts relevés | 00 | 0 = Normal coolant temperature |
| $2875 | ($2875) External controls denial status flag.9 | Contrôle extérieur des défauts relevés | 00 | 0 = Normal Manifold pressure |
| $2875 | ($2875) External controls denial status flag.8 | Contrôle extérieur des défauts relevés | 00 | 0 = pression rail correcte |
| $2C00 | DIDs supported in range [$2C01 - $2C20] |  | 10 00 00 01 | (nicht dekodiert) |
| $2C04 | ($2C04) Current gear engaged | Vxx_gear | 01 | 0 |
| $2C20 | DIDs supported in range [$2C21 - $2C40] |  | 00 10 00 00 | (nicht dekodiert) |
| $2C2C | ($2C2C) Kick down state | Vbx_acel_pdl_kd | 00 | 0 = no kick down |
| $3000 | DIDs supported in range [$3001 - $3020] |  | 87 00 00 01 | (nicht dekodiert) |
| $3001 | ($3001) Verlog state | K_Verrlog_ets_status | 01 | 1 = Vehicle Not Locked |
| $3006 | ($3006) HV Network discharge request | K_PebDischarge_sch_req | 01 | 1 = Discharge not requested |
| $3007 | ($3007) HV Network discharge Feedback | B_feedbackdischarge_peb_req | 00 | 0 = Active Discharge Not In Progress |
| $3008 | ($3008) HV PEB voltage measure | Volt_HVNet_peb_mes | 02 E1 | 368.5 V |
| $3020 | DIDs supported in range [$3021 - $3040] |  | 79 C0 00 01 | (nicht dekodiert) |
| $3022 | ($3022) DCDC activation request | V_B_PowerOn_dcdc_req | 01 | 1 = DCDC On |
| $3023 | ($3023) 14V voltage request | V_Volt_14V_dcdc_req | 1F | 13.55 V |
| $3024 | ($3024) 14V DCDC voltage measure | Volt_14VSide_dcdc_mes | 61 | 13.7 V |
| $3025 | ($3025) 14V DCDC current measure | I_14VSide_dcdc_mes | 0E | 14 A |
| $3028 | ($3028) DCDC Load | V_Pct_DCDC_LoadForBCM_est | 15 | 8.203125 % |
| $3029 | ($3029) DCDC temperature indicator | Temp_AmbDcdc_peb_mes | 2A | 42 % |
| $302A | ($302A) General DCDC state | K_hw_DCDC_status | 02 | 2 = wake up and activated |
| $3040 | DIDs supported in range [$3041 - $3060] |  | 6A 00 00 00 | (nicht dekodiert) |
| $3042 | ($3042) HV INV current measure | I_HVSide_inv_mes | 7F FF | -0.5 A |
| $3043 | ($3043) Torque request | Tq_Final_em_req | 7F CB | -53 N·m |
| $3045 | ($3045) Electrical Motor speed | N_em_mes | 7F FF | -5 rpm |
| $3047 | ($3047) INV temperature indicator | Temp_AmbInv_peb_mes | 02 CC | 14.32 % |
| $3100 | DIDs supported in range [$3101 - $3120] |  | B3 ED E0 00 | (nicht dekodiert) |
| $3101 | ($3101) Charger Bloc state | K_BCBState_bcb_status | 04 | 4 = Diagnosis State |
| $3103 | ($3103) HV BCB voltage measure | Volt_HVNet_bcb_mes | 00 00 | 0 V |
| $3104 | ($3104) Mains Network detection | B_MainsDetected_sbc_status | 00 | 0 = Domestic Network is not present |
| $3107 | ($3107) Charge current limitation | K_10ALim_sbc_status | 02 | 2 = 16A Type |
| $3108 | ($3108) Earth plug connection | B_EarthPlugConnect_sbc_status | 01 | 1 = Ground Plug Connection Failure |
| $3109 | ($3109) BCB water temperature | Temp_Water_bcb_mes | 12 | 18 °C |
| $310A | ($310A) BCB internal temperature | Temp_AmbBcb_bcb_mes | 1E | 30 % |
| $310B | ($310B) interlock connection HV Bat, PEB | K_HVBatPEB_Interlock_status | 02 | 2 = Closed |
| $310D | ($310D) Interlock JB cover | K_JunctBox_Interlock_status | 02 | 2 = Closed |
| $310E | ($310E) Interlock JB Charge | K_SQCharge_Interlock_status | 02 | 2 = Closed |
| $3110 | ($3110) Charge current measure | I_ExtCh_bcb_mes | FF | 255 A |
| $3111 | ($3111) Quick charge start | B_QCStart_ets_req | 00 | 0 = Quick Charge Start Not Requested |
| $3112 | ($3112) Quick charge stop | B_QCStop_ets_req | 00 | 0 = Quick Charge Stop Not Requested |
| $3113 | ($3113) Quick charge current request | I_QCSetPoint_ets_req | 00 | 0 A |
| $3200 | DIDs supported in range [$3201 - $3220] |  | F5 F8 00 00 | (nicht dekodiert) |
| $3201 | ($3201) LBC mode request | K_BHVState_ets_req | 03 | 3 = Normal Charge |
| $3202 | ($3202) LBC fonctionning state | K_ECU_bhv_status | 05 | 5 = Normal |
| $3203 | ($3203) HV LBC voltage measure | Volt_HVNet_bhv_mes | 02 E1 | 368.5 V |
| $3204 | ($3204) HV LBC current measure | I_bhv_mes | 7F FE | -0.38 A |
| $3206 | ($3206) State Of Health (SOH) HV battery | Pct_SOHEnergy_bhv_est | 60 | 96 % |
| $3208 | ($3208) Caution flag state | K_HVBatteryFailure_ets_status | 00 | 0 = C_HVBatteryNoDefault |
| $3209 | ($3209) Failure flag state | K_Failure_bhv_status | 01 | 1 = No Default |
| $320A | ($320A) Available discharge power | P_AvailDischarge_bhv_est | 90 | 72 kW |
| $320B | ($320B) Available charge power | P_AvailCharge_bhv_est | 56 | 43 kW |
| $320C | ($320C) Available discharge Energy | E_AvailDischarge_bhv_est | 09 8A | 12.21 |
| $320D | ($320D) Saving of the state of the interlocks.0 | K_InterlockBatt_bhv_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.1 | K_HVBatPEB_Interlock_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.2 | K_JunctBox_Interlock_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.3 | K_Interlock_peb_emr_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.4 | K_SQCharge_Interlock_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.5 | K_JB_AC_Interlock_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.6 | K_Interlock_CI_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.7 | K_JB_PTC_Interlock_status | 00 | 0 = No default |
| $320D | ($320D) Saving of the state of the interlocks.8 | K_Interlock_PTC_status | 00 | 0 = No default |
| $3300 | ($3300) DIDs supported in range [$3301 - $3320] |  | C1 F9 03 AD | (nicht dekodiert) |
| $3301 | ($3301) USM 14V Voltage measure (from CAN) | Volt_BatteryUSM_mes | A1 | (Antwort zu kurz) |
| $3302 | ($3302) Wake Up Type | B_WakeUpType_BCM_status | 00 | 0 = Customer Wake Up |
| $3308 | ($3308) HV relay connection state | K_HVConnection_ets_status | 02 | 2 = HV_Connected |
| $3309 | ($3309) HV relay P1 command | B_HVRelayP1_sch_req | 01 | 1 = Request for closing the main relay P1 |
| $330A | ($330A) HV relay P2 command | B_HVRelayP2_sch_req | 01 | 1 = Request for closing the main relay P2 |
| $330B | ($330B) HV Battery relay A request | B_HVRelayA_sch_req | 00 | 0 = Request for opening the precharge relay A |
| $330C | ($330C) HV Network Insulation level | K_InsulFailure_ets_statusIn | 00 | 0 = No InsulationFailure |
| $330D | ($330D) 14V Unballasting request | B_14vUnballasting_HVNET_req | 00 | 0 = Not Unballasting request |
| $3310 | ($3310) 14V Batterie charge request | B_DCDC_modecharge_req | 00 | 0 = Charge not requested |
| $3317 | ($3317) QuickDrop lock global state | K_BatteryLock_MngHVB_status | 02 | 2 = BatteryLock closed |
| $3318 | ($3318) Driving pump Feedback | PWM_FeedBack_ETS_WEP1_status | 00 | 0 % |
| $3319 | ($3319) Charge pump Feedback | PWM_FeedBack_ETS_WEP2_status | 00 | 0 % |
| $331B | ($331B) Cooling valve request | B_WaterGateChgCooling_req | 00 | 0 = Request for opening the water gate of the charge mode |
| $331D | ($331D) Pulser relay request | B_PreHeat_Blower_req | 00 | 0 = Request for opening the pulser relay |
| $331E | ($331E) Heater request | B_OnOffHeater_veh_req | 00 | 0 = Request for opening the heater relay |
| $3320 | ($3320) DIDs supported in range [$3321 - $3340] |  | 4F 7F CF 49 | (nicht dekodiert) |
| $3322 | ($3322) Heater led state | B_GHIndic_GH_req | 00 | 0 = Request for switching off the heater led |
| $3325 | ($3325) Shiftlock request | Vbx_shf_lock | 00 | 0 = Request for deactivate the shiftlock (P signal) |
| $3326 | ($3326) Driving pump Request | PWM_DrivingMode_wep_req | 3C | 60 % |
| $3327 | ($3327) Charge pump Request | PWM_ChargeMode_wep_req | 0A | 10 % |
| $3328 | ($3328) Heat pump Request | PWM_Heater_WEP_req | 0A | 10 % |
| $332A | ($332A) Fan cabin status | B_Blower_veh_status | 00 | 0 = Blower OK |
| $332B | ($332B) ETS state | K_State_ets_status | 02 | 2 = ETS_Comfort |
| $332C | ($332C) Start refuse Reason | K_ReasonStartRefuse | 00 | 0 = NO_REFUSE |
| $332D | ($332D) 14V Network functionning Authorization | B_MngETS_DCDC_Act | 01 | 1 = Autorized |
| $332E | ($332E) PowerTrain functionning Authorization | B_MngETS_Driving_Act | 00 | 0 = Not Autorized |
| $332F | ($332F) Thermal Comfort functionning Authorization | K_MngETS_ThermComfort_Auth | 00 | 0 = Not Authorised |
| $3330 | ($3330) Charge Management functionning Authorization | K_MngETS_ChrgMngt_Act | 00 | 0 = Not Autorized |
| $3331 | ($3331) PreHeating functionning Authorization | K_MngETS_PreHeating_Auth | 00 | 0 = Not Authorised |
| $3332 | ($3332) Electrical Machine functionning Authorization | K_MngETS_PEB_Act | 01 | 1 = Inverter Off |
| $3335 | ($3335) 1st level insulation Alert MMI request | K_ETAlertIsolationLevel1_status | 01 | 1 = No ET Alert Isolation Level 1 Requested  |
| $3336 | ($3336) 2nd level insulation Alert MMI request | K_ETAlertIsolationLevel2_status | 01 | 1 = No ET Alert Isolation Level 2 Requested  |
| $3337 | ($3337) ETS Alert MMI request | K_ETAlertETSDefault_status | 01 | 1 = No ET Alert ETS Default Requested  |
| $3338 | ($3338) Motor Alert MMI request | K_ETAlertMotorDefault_status | 01 | 1 = No ET Alert Motor Default Requested |
| $333A | ($333A) Low Battery 14V MMI request | K_Battery14VWarningDisplay_req | 00 | 0 = Unavailable value |
| $333D | ($333D) Brake assist pump state | B_VPM_Pump_status | 00 | 0 = Pump Off |
| $3340 | ($3340) DIDs supported in range [$3341 - $3360] |  | 01 EE EF FF | (nicht dekodiert) |
| $3348 | ($3348) PTC request | Pct_Final_ptc_req | 00 | 0 % |
| $3349 | ($3349) Time Counter for the driving WEP in Low Speed | V_Timer_DrivingWEP_ON_L1 | 00 00 00 00 | 0 |
| $334A | ($334A) Time Counter for the driving WEP in Middle Speed | V_Timer_DrivingWEP_ON_L2 | 00 00 00 00 | 0 |
| $334B | ($334B) Time Counter for the driving WEP in High Speed | V_Timer_DrivingWEP_ON_L3 | 00 00 1D BA | 7610 |
| $334D | ($334D) Time Counter for the load WEP in Low Speed | V_Timer_ChargeWEP_ON_L1 | 00 00 00 00 | 0 |
| $334E | ($334E) Time Counter for the load WEP in Middle Speed | V_Timer_ChargeWEP_ON_L2 | 00 00 00 00 | 0 |
| $334F | ($334F) Time Counter for the load WEP in High Speed | V_Timer_ChargeWEP_ON_L3 | 00 00 00 1C | 28 |
| $3351 | ($3351) Time Counter for the Cooling fan in Low Speed | V_Timer_EngFan_ON_L1 | 00 00 00 00 | 0 |
| $3352 | ($3352) Time Counter for the Cooling fan in Middle Speed | V_Timer_EngFan_ON_L2 | 00 00 00 00 | 0 |
| $3353 | ($3353) Time Counter for the Cooling fan in High Speed | V_Timer_EngFan_ON_L3 | 01 90 FA FC | 26278652 |
| $3355 | ($3355) Time Counter for the PEB temperature in driving mode below 40 degrees celcius | V_Timer_TempPEB_DM_L1 | 00 00 00 61 | 97 |
| $3356 | ($3356) Time Counter for the PEB temperature in driving mode between 40 degrees celcius and 50 degrees celcius | V_Timer_TempPEB_DM_L2 | 00 36 F0 68 | 3600488 |
| $3357 | ($3357) Time Counter for the PEB temperature in driving mode between 50degrees celcius and 60degrees celcius | V_Timer_TempPEB_DM_L3 | 00 53 44 0F | 5456911 |
| $3358 | ($3358) Time Counter for the PEB temperature in driving mode between 60degrees celcius and 70degrees celcius | V_Timer_TempPEB_DM_L4 | 00 0F 25 D0 | 992720 |
| $3359 | ($3359) Time Counter for the PEB temperature in driving mode above 70 degrees celcius | V_Timer_TempPEB_DM_L5 | 00 00 00 02 | 2 |
| $335A | ($335A) Time Counter for the PEB temperature in load mode below 40 degrees celcius | V_Timer_TempPEB_CM_L1 | 00 00 00 00 | 0 |
| $335B | ($335B) Time Counter for the PEB temperature in load mode between 40 degrees celcius and 50 degrees celcius | V_Timer_TempPEB_CM_L2 | 00 06 DE 0C | 450060 |
| $335C | ($335C) Time Counter for the PEB temperature in load mode between 50 degrees celcius and 60 degrees celcius | V_Timer_TempPEB_CM_L3 | 00 33 24 A6 | 3351718 |
| $335D | ($335D) Time Counter for the PEB temperature in load mode between 60 degrees celcius and 70 degrees celcius | V_Timer_TempPEB_CM_L4 | 00 8E 41 2B | 9322795 |
| $335E | ($335E) Time Counter for the PEB temperature in load mode above 70 degrees celcius | V_Timer_TempPEB_CM_L5 | 00 00 00 00 | 0 |
| $335F | ($335F) Time Counter for the BCB temperature in driving mode below 40 degrees celcius | V_Timer_TempBCB_DM_L1 | 00 00 00 00 | 0 |
| $3360 | ($3360) DIDs supported in range [$3361 - $3380] |  | FF FF FF 8F | (nicht dekodiert) |
| $3361 | ($3361) Time Counter for the BCB temperature in driving mode between 40 degrees celcius and 50 degrees celcius | V_Timer_TempBCB_DM_L2 | 00 90 C9 05 | 9488645 |
| $3362 | ($3362) Time Counter for the BCB temperature in driving mode between 50 degrees celcius and 60 degrees celcius | V_Timer_TempBCB_DM_L3 | 00 08 66 14 | 550420 |
| $3363 | ($3363) Time Counter for the BCB temperature in driving mode between 60 degrees celcius and 70 degrees celcius | V_Timer_TempBCB_DM_L4 | 00 00 2B 91 | 11153 |
| $3364 | ($3364) Time Counter for the BCB temperature in driving mode above 70 degrees celcius | V_Timer_TempBCB_DM_L5 | 00 00 00 00 | 0 |
| $3365 | ($3365) Time Counter for the BCB temperature in load mode below 40 degrees celcius | V_Timer_TempBCB_CM_L1 | 00 00 00 00 | 0 |
| $3366 | ($3366) Time Counter for the BCB temperature in load mode between 40 degrees celcius and 50 degrees celcius | V_Timer_TempBCB_CM_L2 | 00 2A 88 B0 | 2787504 |
| $3367 | ($3367) Time Counter for the BCB temperature in load mode between 50 degrees celcius and 60 degrees celcius | V_Timer_TempBCB_CM_L3 | 00 5E 7D 43 | 6192451 |
| $3368 | ($3368) Time Counter for the BCB temperature in load mode between 60 degrees celcius and 70 degrees celcius | V_Timer_TempBCB_CM_L4 | 00 3F 3D EA | 4144618 |
| $3369 | ($3369) Time Counter for the BCB temperature in load mode above 70 degrees celcius | V_Timer_TempBCB_CM_L5 | 00 00 00 00 | 0 |
| $336A | ($336A) Maximale value of PEB temperature | V_Max_TempPEB | 3D | 61 % |
| $336B | ($336B) Maximale value of BCB temperature | V_Max_TempBCB | 37 | 55 % |
| $336C | ($336C) Quick Drop operating counter | CM_BatteryLock_Counter | 3F FF | 16383 |
| $336D | ($336D) Data store memory use to store the activation time of the pump | C_life_cycle_memory | 00 00 00 | 0 s |
| $336E | ($336E) Value of the vehicle speed before a safety level 2 hotreset | Vxx_sfty_vs_bfr_rst | 00 00 | 0 km/h |
| $336F | ($336F) Value of the safety monitoring status flags before the hotreset | Vxx_sfty_mon_sf_bfr_rst | 00 00 | 0 |
| $3370 | ($3370) Value of the electric motor speed before a safety level 2 hotreset | Vxx_sfty_emot_n_bfr_rst | 80 00 | 0 rpm |
| $3371 | ($3371) Pressure value max in the booster during failure confirmation time | K_VPM_Pressure_max | FE | 1270 mbar |
| $3372 | ($3372) Current pressure in the mastervac | K_VPM_Pressure_status | FE | 1270 mbar |
| $3373 | ($3373)The most critical state of the brake assist system between detected and confirmed states of mastervac failure | K_VPM_PressureState_max | 03 | 3 = Vacuum pressure between 'Pump On Threshold' and 'Pump Off Threshold'  |
| $3374 | ($3374) The most critical state of the Brake assist system when a failure has been confirmed | K_VPM_PressureState_status | 00 | 0 = unavailable |
| $3375 | ($3375) Memorized reason of Starting Refuse for Start Refuse History elaboration.0 | K_ReasonStartRefuse_status | 01 01 | (Antwort zu kurz) |
| $3375 | ($3375) Memorized reason of Starting Refuse for Start Refuse History elaboration.1 | K_ReasonStartRefuse_status | 01 01 | (Antwort zu kurz) |
| $3375 | ($3375) Memorized reason of Starting Refuse for Start Refuse History elaboration.2 | K_ReasonStartRefuse_status | 01 01 | (Antwort zu kurz) |
| $3375 | ($3375) Memorized reason of Starting Refuse for Start Refuse History elaboration.3 | K_ReasonStartRefuse_status | 01 | 1 = C_Driving_Refuse_Plug |
| $3375 | ($3375) Memorized reason of Starting Refuse for Start Refuse History elaboration.4 | K_ReasonStartRefuse_status | 01 | 1 = C_Driving_Refuse_Plug |
| $3376 | ($3376) Measure of the freon pressure | Pres_Freon_ac_mes | 00 29 | 4.1 bar |
| $3377 | ($3377) AC compressor Interlock State | K_Interlock_CI_status | 00 | 0 = Unvailable Value |
| $3378 | ($3378) Interlock HV battery | K_InterlockBatt_bhv_status | 02 | 2 = Closed |
| $3379 | ($3379) Interlock of PTCs | K_Interlock_PTC_status | 02 | 2 = Closed |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.0-diagnostic of NVMY | diagnostic of NVMY | 00 | 0 = No default |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.1 | Not used | 00 | 0 |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.2-Internal diag RELAY A | Internal diag RELAY A | 00 | 0 = No default |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.3-Internal diag RELAY P | Internal diag RELAY P | 00 | 0 = No default |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.4-Internal diag RELAY N | Internal diag RELAY N | 00 | 0 = No default |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.5-Internal diag Shift Lock | Internal diag Shift Lock | 00 | 0 = No default |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.6-Internal diag Water Cooling Valve | Internal diag Water Cooling Valve | 00 | 0 = No default |
| $337D | ($337D) Context not safety datas microcontroller memorised carrier 1.7-Internal diag Stop Light Relay | Internal diag Stop Light Relay | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.0-ERR_SYM_ROM | ERR_SYM_ROM[ SYM_0] | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.1-ERR_SYM_RAM | ERR_SYM_RAM[ SYM_0] | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.2-ERR_SYM_SPI-0 | ERR_SYM_SPI[ SYM_0] | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.4-ERR_SYM_SPI-2 | ERR_SYM_SPI[ SYM_2] | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.5-ERR_SYM_SPI-3 | ERR_SYM_SPI[ SYM_3] | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.6-ERR_SYM_SPI-4 | ERR_SYM_SPI[ SYM_4] | 00 | 0 = No default |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.7 | Not used | 00 | 0 |
| $337E | ($337E) Context not safety data microcontroller memorised Carrier 2.3-ERR_SYM_SPI-1 | ERR_SYM_SPI[ SYM_1] | 00 | 0 = No default |
| $337F | ($337F) Context safety datas memorised in main microcontroller | Vxx_ecm_mon_1_stt | 00 | 0 = No error |
| $3380 | ($3380) DIDs supported in range [$3381 - $33A0] |  | F7 7F FF FF | (nicht dekodiert) |
| $3381 | ($3381) Context safty datas memorised in Safety microcontroller | Vxx_ecm_mon_2_stt | 00 | 0 |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.00 CI_Error_Status | K_TCLimitMode_AC[0] - CI_Error_Status | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.11 Internal_communication_error | K_TCLimitMode_AC[0] - Internal_communication_error | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.02 High_Voltage_error | K_TCLimitMode_AC[0] - High_Voltage_error | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.03 Abnormal_Run_up | K_TCLimitMode_AC[0] - Abnormal_Run_up | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.10 Over_heat_warning | K_TCLimitMode_AC[0] - Over_heat_warning | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.04 Over_heat | K_TCLimitMode_AC[0] - Over_heat | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.05 Over_load | K_TCLimitMode_AC[0] - Over_load | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.06 Current_sensor_Error | K_TCLimitMode_AC[0] - Current_sensor_Error | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.07 Open_contact | K_TCLimitMode_AC[0] - Open_contact | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.08 Short_Circuit | K_TCLimitMode_AC[0] - Short_Circuit | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.09 Low_Voltage_error | K_TCLimitMode_AC[0] - Low_Voltage_error | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1 | K_TCLimitMode_AC[0] | 00 00 | 0 |
| $3382 | ($3382) AC compressor Error Bits Recordings.-2 | K_TCLimitMode_AC[1] | 00 00 | 0 |
| $3382 | ($3382) AC compressor Error Bits Recordings.-3 | K_TCLimitMode_AC[2] | 00 00 | 0 |
| $3382 | ($3382) AC compressor Error Bits Recordings.-4 | K_TCLimitMode_AC[3] | 00 00 | 0 |
| $3382 | ($3382) AC compressor Error Bits Recordings.-5 | K_TCLimitMode_AC[4] | 00 00 | 0 |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.12 Voltage_Restriction | K_TCLimitMode_AC[0] - Voltage_Restriction | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.13 Current_Restriction | K_TCLimitMode_AC[0] - Current_Restriction | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.14 Over_voltage_HV | K_TCLimitMode_AC[0] - Over_voltage_HV | 00 | 0 = No default |
| $3382 | ($3382) AC compressor Error Bits Recordings.-1.15 Under_voltage_HV | K_TCLimitMode_AC[0] - Under_voltage_HV | 00 | 0 = No default |
| $3383 | ($3383) Reasons of cut off PTCs.-1.0 Over temperature | Over temperature | 30 | 0 = No default |
| $3383 | ($3383) Reasons of cut off PTCs.-1.1 Under Voltage | Under Voltage | 30 | 0 = No default |
| $3383 | ($3383) Reasons of cut off PTCs.-1.2 Error PTC stage 1 | Error PTC stage 1 | 30 | 0 = No default |
| $3383 | ($3383) Reasons of cut off PTCs.-1.3 Error PTC stage 2 | Error PTC stage 2 | 30 | 0 = No default |
| $3383 | ($3383) Reasons of cut off PTCs.-1.4 Error PTC stage 3 | Error PTC stage 3 | 30 | 1 = Default present |
| $3383 | ($3383) Reasons of cut off PTCs.-1.5 Error PTC stage 4 | Error PTC stage 4 | 30 | 1 = Default present |
| $3383 | ($3383) Reasons of cut off PTCs.-1 | K_TCLimitMode_PTC[0] | 30 | 48 |
| $3383 | ($3383) Reasons of cut off PTCs.-2 | K_TCLimitMode_PTC[1] | 32 30 | 8 |
| $3383 | ($3383) Reasons of cut off PTCs.-3 | K_TCLimitMode_PTC[2] | 32 32 | 35 |
| $3383 | ($3383) Reasons of cut off PTCs.-4 | K_TCLimitMode_PTC[3] | 32 | 12 |
| $3383 | ($3383) Reasons of cut off PTCs.-5 | K_TCLimitMode_PTC[4] | 32 | 50 |
| $3384 | ($3384) Mission counter without driver door transition | K_MngETS_DDoorStatCounter | 00 | 0 |
| $3386 | ($3386) Life time of the pump read from the VPM memory via LIN | OLIN_K_LifeCycle_vpm_mes | FF FF FF | 16777215 s |
| $3387 | ($3387) Brake pedal state during diagnostic confirmation | K_VPM_BLS_max | 01 | 1 = not pressed |
| $3388 | ($3388) Bcb Activation failed Flag : charge or pre_heating not authorized any more | B_BCBFailed | 00 | 0 = no failure  |
| $338A | ($338A) Customer WakeUp flag after internal processing | B_BCM_CustomWUP | 01 | 1 = Customer WakeUp |
| $338B | ($338B) Event type technical WakeUp flag after internal processing | B_BCM_EventTechWUP | 01 | 1 = Event technical WakeUp |
| $338C | ($338C) ComfortMode failed and ended after confirmed non HV relays connexion | B_ComfortModeEnd | 00 | 0 = Confort mode ok  |
| $338D | ($338D) Comfort mode disabled | B_ComfortModeNotAllowed | 00 | 0 = Allowed |
| $338E | ($338E) Indicates the status of the driver door of the vehicle | B_DriverDoorSwitch_status | 01 | 1 = Opened |
| $338F | ($338F) Driving Restart flag (after EVC reset) | B_DrivingRestart_Act | 00 | 0 = desactivated |
| $3390 | ($3390) EVC reset flag for HV connexion after EVC reset management | B_EVCReset_Act | 00 | 0 = desactivated |
| $3391 | ($3391) Gaz Heater not allowed flag | B_GasHeater_NotAllowed | 01 | 1 = Not Allowed |
| $3392 | ($3392) 400 V Battery Cooling Needed flag | B_HVBcool_ets_req | 00 | 0 = No request |
| $3393 | ($3393) HV Network Power cut request | B_HVNET_Power_Cut_req | 00 | 0 = no request |
| $3394 | ($3394) Flag for LBC Actual status(= to 1 when LBC status is Driving ). AfterK_BHVStateConfirmed_status processing | B_LBC_Act_status_S | 01 | 1 = Ready for driving |
| $3395 | ($3395) Internal condition for Zero Ampere mode Autorisation | B_METS_ZA_Auth | 00 | 0 = Allowed |
| $3396 | ($3396) PEB State failure defect present : PEB is Absent or PEB Status is not power on . PEB status is in limp mode | B_MngETS_DefPres_PEBstatus | 01 | 1 = PEB Not Power ON |
| $3397 | ($3397) Gaz Heater Activation Authorisation flag | B_MngETS_GasHeater_Auth | 00 | 0 = Not Autorized |
| $3398 | ($3398) HV Battery cooling Authorisation flag | B_MngETS_Peltier_Auth | 00 | 0 = Not Autorized |
| $3399 | ($3399) Quick Drop flag memorisation (EEPROM memorisation) | B_MngETS_QuickDropMem | 00 | 0 = Not Memorized |
| $339A | ($339A) No Major failure | B_noMajor_failure | 01 | 1 = failure |
| $339B | ($339B) Front passenger door state from CANV | B_PassengerDoorSwitch_status | 01 | 1 = Opened |
| $339C | ($339C) PEB status after internal processing | B_PEB_Act_status_S | 00 | 0 = NOK |
| $339D | ($339D) Charging Plug detected as present by the Charger.processed | B_PlugConnected_bcb_status_S | 00 | 0 = No charging plug |
| $339E | ($339E) Quick drop condition is active | B_QuickDrop_Act | 00 | 0 = No Active |
| $339F | ($339F) Soc authorisation for pre_heating authorisation during charge | B_UserSOC_Auth_S | 00 | 0 = Not Autorized |
| $33A0 | ($33A0) DIDs supported in range [$33A1 - $33C0] |  | E7 3D AF FF | (nicht dekodiert) |
| $33A1 | ($33A1) BCM absent and vehicle stopped flag : driving interruptiion | BCMAbsentDriving_NotAllowed | 00 | 0 = Not Autorized |
| $33A2 | ($33A2) Button AC from the cluster (X61) or AC request from CAREG (L38) | B_ACbutton_ac_req | 00 | 0 = no AC requested  |
| $33A3 | ($33A3) For L38 vehicule, this is the demand from the CAREG to start a PreHeating. | B_CaregPreCondDemand_AC_req | 00 | 0 = no PreHeat demand from CAREG   |
| $33A6 | ($33A6) AC compressor DTC Rootcause | AC_LIN_Snapshot_DTC | 00 | 0 = No fault |
| $33A7 | ($33A7) AC compressor Power Consumption Estimation | P_InputPower_ac_mes | FE | 6350 W |
| $33A8 | ($33A8) Request of FAN activation from the thermal comfort | K_EngineFAN_ac_req | 00 | 0 = no FAN requested |
| $33AB | ($33AB) State of PreHeating to display | K_PreheatingProgress_status | 01 | 1 = PreHeating requested for H1 |
| $33AC | ($33AC) Status of pre_heating sent to Manage ETS | K_TC_PreHeatingAvail_status | 00 | 0 = pre-heating available |
| $33AD | ($33AD) Computation of PTC defaults.0 | Over temperature | 30 | 0 = No default |
| $33AD | ($33AD) Computation of PTC defaults.1 | Under Voltage | 30 | 0 = No default |
| $33AD | ($33AD) Computation of PTC defaults.2 | Error PTC stage 1 | 30 | 0 = No default |
| $33AD | ($33AD) Computation of PTC defaults.3 | Error PTC stage 2 | 30 | 0 = No default |
| $33AD | ($33AD) Computation of PTC defaults.4 | Error PTC stage 3 | 30 | 1 = Default present |
| $33AD | ($33AD) Computation of PTC defaults.5 | Error PTC stage 4 | 30 | 1 = Default present |
| $33AE | ($33AE) AC compressor RPM Command | N_RPMComp_ac_req | 00 00 | 0 rpm |
| $33B0 | ($33B0) evaporator temperature setpoint for compressor control | VH_EvapTempSP | 0A C8 | 3 °C |
| $33B1 | ($33B1) CAN signal for external temperature of the vehicle. | Temp_ExternalTemp_Veh_mes | 36 | 14 °C |
| $33B3 | ($33B3) water temperature setpoint for PTC control | VH_WaterTempSP | 4C | 36 °C |
| $33B5 | ($33B5) HV Relays Connection Autorisation synthesis flag | HVConnection_Auth | 01 | 1 = Autorized |
| $33B6 | ($33B6) Charge Management status for ETS | K_BCBMode_status | 00 | 0 = NoActiveMode |
| $33B7 | ($33B7) Technical WakeUp type | K_BCM_TechWUpType | 00 | 0 = not used |
| $33B8 | ($33B8) Confirmation of the state of the battery from ManageHVBattery | K_BHVStateConfirmed_status | 03 | 3 = Driving_Confirmed |
| $33B9 | ($33B9) Start_up Authorisation _ Anti_kick off management | K_CrankingAutho_AT_veh_status | 01 | 1 = CrankingAllowed Start_up allowed by anti-kick off |
| $33BA | ($33BA) Indicates if the external power supply is present and the type of power supply(slow charge/quick charge) | K_ExtEnergy_status | 00 | 0 = NoExtEnergyAvail |
| $33BB | ($33BB) Signal coming from Manage HVBattery indicating if the battery need to be charged or to be cooled | K_HVBCharge_ets_req | 01 | 1 = HVBChargeRequest |
| $33BC | ($33BC) HV connection request from LBC | K_HVConnectAuth_bhv_status | 01 | 1 = Closing of Power Contacts Allowed |
| $33BD | ($33BD) Plug status validity flag | K_METS_Plug_vld | 01 | 1 = Ok |
| $33BE | ($33BE) LBC sleep mode request | K_MngETS_BHV_SleepMode | 00 | 0 = Normal Sleep |
| $33BF | ($33BF) HV connection request | K_MngETS_HVConnection_req | 03 | 3 = Nominal connexion request |
| $33C0 | ($33C0) DIDs supported in range [$33C1 - $33E0] |  | FF FF BF AD | (nicht dekodiert) |
| $33C1 | ($33C1) K_StartingMode_BCM_req input validity state | K_MngETS_Startmod_vld | 01 | 1 = K_StartingMode_BCM_req available and valid |
| $33C2 | ($33C2) IVLD of the signal Pct_UserSOC_bhv_est | K_Pct_UserSOC_bhv_vld | 01 | 1 = valid value OK 1 |
| $33C3 | ($33C3) PEB status from CAN ETS | K_PEB_Act_status | 00 | 0 = Starting up or power off |
| $33C4 | ($33C4) Preheating requested by the vehicle | K_PreHeatSelect_veh_req | 02 | 2 |
| $33C5 | ($33C5) ETS System Sleeping authorisation synthesis | K_ReadyToSleep_ets_status | 01 | 1 = Refuse to sleep |
| $33C6 | ($33C6) Electrical Motor state requested bu user and build by the BCM, signal coming from CAN, not checked | K_StartingMode_BCM_req | 00 | 0 = Motor stop requested |
| $33C7 | ($33C7) Driver requests synthesis | K_State_OutputFAL_status | 03 | 3 = Life on Board request (HV network request) |
| $33C8 | ($33C8) ETS Organs on CAN_E Wake up or Sleep request | K_WakeUpSleep_CAN_EL_req | 03 | 3 = WakeUp |
| $33C9 | ($33C9) High level failure in the BCB | B_BCBFailureH_ets_status | 00 | 0 = No failure |
| $33CA | ($33CA) Low level failure in the BCB. | B_BCBFailureL_ets_status | 00 | 0 = No failure |
| $33CB | ($33CB) This flag from Charge Management indicates that a charge is finished. | B_ChargeFinished_status | 00 | 0 = Charge not finished |
| $33CC | ($33CC) Request for the Gasoline heater relay | B_GHRelay_GH_req | 00 | 0 = no request  |
| $33CD | ($33CD) Request of Hvac power Relay | B_HVAC_Relay_req | 00 | 0 = no request  |
| $33CE | ($33CE) Software failure detected in the quick charge stateflow | B_QCFailure_ets_status | 00 | 0 = No failure |
| $33CF | ($33CF) Specific failure during the quick charge | B_QuickChargeFailure_ets_status | 00 | 0 = No failure |
| $33D0 | ($33D0) This signal indicates that a quick drop is finished | B_QuickDropFinished_status | 00 | 0 = No quick-drop has occured |
| $33D1 | ($33D1) Specific failure during slow charge or zero ampere mode | B_SlowChargeFailure_ets_status | 00 | 0 = No failure |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.0 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.1 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.2 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.3 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.4 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.5 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.6 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.7 | B_BPLCSpotDet_CJ_read | 00 00 | (Antwort zu kurz) |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.8 | B_BPLCSpotDet_CJ_read | 00 | 0 = spot not present |
| $33D3 | ($33D3) BPLC spot presence status to be read by diag tool.9 | B_BPLCSpotDet_CJ_read | 00 | 0 = spot not present |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.0 | D_DistTotal_CJ_read | 01 4C 3A | 85050 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.1 | D_DistTotal_CJ_read | 01 4C 3A | 85050 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.2 | D_DistTotal_CJ_read | 01 4C 3A | 85050 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.3 | D_DistTotal_CJ_read | 01 4C 3A | 85050 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.4 | D_DistTotal_CJ_read | 01 4C 6E | 85102 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.5 | D_DistTotal_CJ_read | 01 4C A1 | 85153 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.6 | D_DistTotal_CJ_read | 01 4C D6 | 85206 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.7 | D_DistTotal_CJ_read | 01 4D 08 | 85256 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.8 | D_DistTotal_CJ_read | 01 4D 3B | 85307 km |
| $33D4 | ($33D4) Total travelled distance to be read by diag tool.9 | D_DistTotal_CJ_read | 01 4D D8 | 85464 km |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.0 | K_ChargeStop_CJ_read | 01 00 01 01 01 | (Antwort zu kurz) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.1 | K_ChargeStop_CJ_read | 01 00 01 01 01 | (Antwort zu kurz) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.2 | K_ChargeStop_CJ_read | 01 00 01 01 01 | (Antwort zu kurz) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.3 | K_ChargeStop_CJ_read | 01 00 01 01 01 | (Antwort zu kurz) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.4 | K_ChargeStop_CJ_read | 01 00 01 01 01 | (Antwort zu kurz) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.5 | K_ChargeStop_CJ_read | 01 | 1 = Unplugged (Vehicle side) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.6 | K_ChargeStop_CJ_read | 01 | 1 = Unplugged (Vehicle side) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.7 | K_ChargeStop_CJ_read | 01 | 1 = Unplugged (Vehicle side) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.8 | K_ChargeStop_CJ_read | 00 | 0 = Nominal end of charge (battery full) |
| $33D5 | ($33D5) Reason of end of charge to be read by diag tool.9 | K_ChargeStop_CJ_read | 01 | 1 = Unplugged (Vehicle side) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.0 | K_ChargeType_CJ_read | 00 00 00 00 | (Antwort zu kurz) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.1 | K_ChargeType_CJ_read | 00 00 00 00 | (Antwort zu kurz) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.2 | K_ChargeType_CJ_read | 00 00 00 00 | (Antwort zu kurz) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.3 | K_ChargeType_CJ_read | 00 00 00 00 | (Antwort zu kurz) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.4 | K_ChargeType_CJ_read | 00 00 00 00 | (Antwort zu kurz) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.5 | K_ChargeType_CJ_read | 00 00 00 00 | (Antwort zu kurz) |
| $33D6 | ($33D6) Type of charge to be read by diag tool.6 | K_ChargeType_CJ_read | 00 | 0 = Slow Charge |
| $33D6 | ($33D6) Type of charge to be read by diag tool.7 | K_ChargeType_CJ_read | 00 | 0 = Slow Charge |
| $33D6 | ($33D6) Type of charge to be read by diag tool.8 | K_ChargeType_CJ_read | 00 | 0 = Slow Charge |
| $33D6 | ($33D6) Type of charge to be read by diag tool.9 | K_ChargeType_CJ_read | 00 | 0 = Slow Charge |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.0 | Pct_UserSOC_CJ_read | 00 41 | 13 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.1 | Pct_UserSOC_CJ_read | 00 89 | 27.4 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.2 | Pct_UserSOC_CJ_read | 00 BC | 37.6 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.3 | Pct_UserSOC_CJ_read | 01 5D | 69.8 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.4 | Pct_UserSOC_CJ_read | 01 F3 | 99.8 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.5 | Pct_UserSOC_CJ_read | 01 80 | 76.8 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.6 | Pct_UserSOC_CJ_read | 01 18 | 56 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.7 | Pct_UserSOC_CJ_read | 00 FA | 50 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.8 | Pct_UserSOC_CJ_read | 01 F4 | 100 % |
| $33D7 | ($33D7) State of charge (UserSOC) to be read by diag tool.9 | Pct_UserSOC_CJ_read | 01 48 | 65.6 % |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.0 | Temp_BatTemp_CJ_read | 37 | 15 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.1 | Temp_BatTemp_CJ_read | 38 | 16 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.2 | Temp_BatTemp_CJ_read | 33 | 11 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.3 | Temp_BatTemp_CJ_read | 35 | 13 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.4 | Temp_BatTemp_CJ_read | 34 | 12 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.5 | Temp_BatTemp_CJ_read | 30 | 8 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.6 | Temp_BatTemp_CJ_read | 34 | 12 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.7 | Temp_BatTemp_CJ_read | 34 | 12 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.8 | Temp_BatTemp_CJ_read | 37 | 15 °C |
| $33D8 | ($33D8) Battery temperature to be read by diag tool.9 | Temp_BatTemp_CJ_read | 35 | 13 °C |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.0 | Time_ChargeTime_CJ_read | 00 00 | 0 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.1 | Time_ChargeTime_CJ_read | 00 14 | 20 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.2 | Time_ChargeTime_CJ_read | 00 0F | 15 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.3 | Time_ChargeTime_CJ_read | 00 29 | 41 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.4 | Time_ChargeTime_CJ_read | 00 69 | 105 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.5 | Time_ChargeTime_CJ_read | 00 0B | 11 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.6 | Time_ChargeTime_CJ_read | 00 16 | 22 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.7 | Time_ChargeTime_CJ_read | 00 22 | 34 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.8 | Time_ChargeTime_CJ_read | 00 7B | 123 min |
| $33D9 | ($33D9) Total duration of the charge sequence to be read by diag tool.9 | Time_ChargeTime_CJ_read | 00 5B | 91 min |
| $33DB | ($33DB) Memorized value of domestic energy consumption | E_TotalEnergyConsMMI_ets_est | 33 | 51 |
| $33DD | ($33DD) This variable allows to memorize the reseted energy consumed | CM_ResetedConsumedEnergy | 00 4C F2 | 19.698 |
| $33DE | ($33DE) This variable allows to memorize the reseted distance | CM_ResetedTravelledDistance | 00 18 0E FC | 157.67 km |
| $33E0 | ($33E0) DIDs supported in range [$33E0 - $3400] |  | 7F FF FF FF | (nicht dekodiert) |
| $33E2 | ($33E2) Flow that requests the BCB to make available certain functionalities | K_BCBStates_ets_req | 04 | 4 = Diag |
| $33E3 | ($33E3) Status of Manage HV Battery Master Stateflow | K_BHVMasterState_bhv_status | 06 | 6 = BHV_Normal confirmed |
| $33E4 | ($33E4) Cooling Management status answering to cooling request from Manage ETS | K_Cooling_Mngt_status | 00 | 0 = Cooling ON |
| $33E5 | ($33E5) Charge WEP Feedback Diag | K_Failure_Charge_wep_code | 00 | 0 = No defined value |
| $33E6 | ($33E6) Driving WEP Feedback Diag | K_Failure_Driving_wep_code | 00 | 0 = No defined value |
| $33E7 | ($33E7) Code Failure of All the sensors of temperature | K_Failure_Temp_sensor_code | 00 00 | 0 |
| $33E8 | ($33E8) JB_AC Interlock status | K_JB_AC_Interlock_status | 02 | 2 = Interlock Close |
| $33E9 | ($33E9) JB_PTC Interlock status | K_JB_PTC_Interlock_status | 02 | 2 = Interlock Close |
| $33EA | ($33EA) CAN signal for the status of the plugs connection | K_PlugConnected_bcb_status | 00 | 0 = No plug connected |
| $33EB | ($33EB) Power supply relay of fan requested by EVC | K_PowerOn_engf_req | 03 | 3 = closed relay |
| $33EC | ($33EC) Different states of CM master stateflow | K_BCBMasterState_sbc_status | 01 | 1 = BCB_Normal |
| $33ED | ($33ED) Different states of quick charge management stateflow | K_BCBQChargeState_sbc_status | 00 | 0 = NoState |
| $33EE | ($33EE) Resistance insulation value | R_Insulation_bhv_mes | 12 7A | 473000 Ohm |
| $33EF | ($33EF) Different states of slow charge management stateflow | K_BCBSlowChargeState_sbc_status | 00 | 0 = 0:NoState |
| $33F0 | ($33F0) Different states of the stand_by management stateflow | K_BCBStandbyState_sbc_status | 00 | 0 = NoState |
| $33F1 | ($33F1) Electrical Machine normalised temperature | Temp_Machine_em_mes | 01 0E | 5.4 % |
| $33F2 | ($33F2) High Voltage Battery maximal Temperature | Temp_TempBhv_Max_mes | 32 | 10 °C |
| $33F3 | ($33F3) Maximal torque without unballasting | Tq_AvailMaxUnbOff_em_est | 28 00 | 0 N·m |
| $33F4 | ($33F4) Minimal torque | Tq_AvailMin_em_est | 28 00 | 0 N·m |
| $33F5 | ($33F5) Cooling Temperature dcdc mesured | V_Temp_Cooling_dcdc_mes | 28 | 0 °C |
| $33F6 | ($33F6) Temperature of the inverter given by PEB (CAN ETS) | V_Temp_Cooling_inv_mes | 28 | 0 °C |
| $33F7 | ($33F7) Counter of Efan speed on Low and very low speed | V_Timer_EngFan_ON_L1_L2 | 00 00 35 14 | 13588 |
| $33F8 | ($33F8) Different states of the Zero Ampere mode management stateflow. | K_BCBZeroAmpereState_sbc_status | 00 | 0 = 0: NoState |
| $33F9 | ($33F9) Primary ignition supply state, signal coming from CAN, not checked | K_IgnitionState_status | 01 | 1 = +ACC off |
| $33FA | ($33FA) Pedal pressed flag internal processing | B_BrakePedalPressed_S | 00 | 0 = pedal not pressed |
| $33FB | ($33FB) Preheating requested by the vehicle. Processed | B_PreHeatSelect_veh_req_S | 00 | 0 = no request |
| $33FC | ($33FC) Electrical Motor state requested by user and build by the BCM | K_StartingMode_BCM_req_S | 02 | 2 = +APC requested |
| $33FD | ($33FD) Peltier Control Order | B_PeltierCtrlOrder | 00 | 0 = no control |
| $33FE | ($33FE) Peltier Fan control order | B_PeltierFanCtrlOrder | 00 | 0 = no control |
| $33FF | ($33FF) Major failure detected on the ETS | B_ETS_Failure_Management_flag | 00 | 0 = no failure |
| $3401 | ($3401) Manage Failure Authorization of the AC | B_HVDiag_AC_Auth_status | 01 | 1 = Autorized |
| $3402 | ($3402) Manage Failure Authorization of the 14V Network management | B_HVDiag_DCDC_Auth_status | 01 | 1 = Autorized |
| $3403 | ($3403) Manage Failure Authorization of the heater | B_HVDiag_Heater_Auth_status | 01 | 1 = Autorized |
| $3404 | ($3404) Manage Failure Authorization of the Peltier | B_HVDiag_Pelt_Auth_status | 01 | 1 = Autorized |
| $3405 | ($3405) Manage Failure Authorization of the PowerTrain | B_HVDiag_PT_Auth_status | 01 | 1 = Autorized |
| $3407 | ($3407) Manage Failure Authorization of the RSA Quick Charge | B_HVDiag_RSAQC_Auth_status | 01 | 1 = Autorized |
| $3408 | ($3408) Manage Failure Authorization of the Slow Charge | B_HVDiag_SC_Auth_status | 01 | 1 = Autorized |
| $3409 | ($3409) Signal of requesting the MMI EBAAlertLevel1 (brake failure with SERV) | K_AlerteFrein1_req | 01 | 1 = alert off |
| $340A | ($340A) Signal of requesting the MMI EBAAlertLevel1 (brake failure with STOP) | K_AlerteFrein2_req | 01 | 1 = alert off |
| $340B | ($340B) Signal of requesting the MMI QuickDropIterationExceededDisplay | K_AlertQuickDropCountMax_req | 01 | 1 = alert off |
| $340C | ($340C) Signal of requesting the MMI QuickDropLockFailureDisplay | K_AlertQuickDropFail_req | 01 | 1 = alert off |
| $340D | ($340D) Signal of requesting the MMI QuickDropUnlockedDisplay | K_AlertQuickDropUnlocked_req | 01 | 1 = alert off |
| $340E | ($340E) Signal of requesting the MMI ETAlertDCDC | K_ETAlertDCDC_req | 01 | 1 = alert off |
| $340F | ($340F) Signal of requesting the MMI ETAlertRepairETS | K_ETAlrtRepairETSDefault_status | 01 | 1 = alert off |
| $3410 | ($3410) HV Network Diagnosis synthesis | K_HVDiag_Connect_Auth_status | 01 | 1 = Connection Allowed |
| $3411 | ($3411) Distance memorized for energy consumption journal.0 | D_Distance_CsJ | 01 4C 3A | 85050 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.1 | D_Distance_CsJ | 01 4C 6E | 85102 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.2 | D_Distance_CsJ | 01 4C 6E | 85102 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.3 | D_Distance_CsJ | 01 4C A1 | 85153 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.4 | D_Distance_CsJ | 01 4C A1 | 85153 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.5 | D_Distance_CsJ | 01 4C A1 | 85153 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.6 | D_Distance_CsJ | 01 4C D6 | 85206 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.7 | D_Distance_CsJ | 01 4D 08 | 85256 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.8 | D_Distance_CsJ | 01 4D 3B | 85307 km |
| $3411 | ($3411) Distance memorized for energy consumption journal.9 | D_Distance_CsJ | 01 4D D8 | 85464 km |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.0 | P_bhv_est_tot_CsJ | 00 00 | 0 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.1 | P_bhv_est_tot_CsJ | 03 12 | 7.86 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.2 | P_bhv_est_tot_CsJ | 03 12 | 7.86 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.3 | P_bhv_est_tot_CsJ | 06 12 | 15.54 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.4 | P_bhv_est_tot_CsJ | 06 12 | 15.54 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.5 | P_bhv_est_tot_CsJ | 06 12 | 15.54 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.6 | P_bhv_est_tot_CsJ | 09 4F | 23.83 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.7 | P_bhv_est_tot_CsJ | 0C 2C | 31.16 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.8 | P_bhv_est_tot_CsJ | 0F 23 | 38.75 |
| $3412 | Memorized accumulated HV battery output energy for energy consumption journal.9 | P_bhv_est_tot_CsJ | 18 09 | 61.53 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.0 | P_Elec_inv_tot_CsJ | 00 00 | 0 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.1 | P_Elec_inv_tot_CsJ | 02 EE | 7.5 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.2 | P_Elec_inv_tot_CsJ | 02 EE | 7.5 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.3 | P_Elec_inv_tot_CsJ | 05 D4 | 14.92 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.4 | P_Elec_inv_tot_CsJ | 05 D4 | 14.92 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.5 | P_Elec_inv_tot_CsJ | 05 D4 | 14.92 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.6 | P_Elec_inv_tot_CsJ | 08 DC | 22.68 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.7 | P_Elec_inv_tot_CsJ | 0B A3 | 29.79 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.8 | P_Elec_inv_tot_CsJ | 0E 75 | 37.01 |
| $3413 | Memorized accumulated electrical inverter energy for energy consumption journal.9 | P_Elec_inv_tot_CsJ | 16 C0 | 58.24 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.0 | P_UsedPower_TC_tot_CsJ | 00 00 | 0 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.1 | P_UsedPower_TC_tot_CsJ | 00 0A | 0.1 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.2 | P_UsedPower_TC_tot_CsJ | 00 0A | 0.1 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.3 | P_UsedPower_TC_tot_CsJ | 00 0A | 0.1 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.4 | P_UsedPower_TC_tot_CsJ | 00 0A | 0.1 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.5 | P_UsedPower_TC_tot_CsJ | 00 0A | 0.1 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.6 | P_UsedPower_TC_tot_CsJ | 00 21 | 0.33 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.7 | P_UsedPower_TC_tot_CsJ | 00 21 | 0.33 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.8 | P_UsedPower_TC_tot_CsJ | 00 2C | 0.44 |
| $3414 | Memorized accumutation of the energy used by the thermal comfort for energy consumption journal.9 | P_UsedPower_TC_tot_CsJ | 00 76 | 1.18 |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.0 | Pct_UserSOC_CsJ | 0D A2 | 69.8 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.1 | Pct_UserSOC_CsJ | 13 83 | 99.9 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.2 | Pct_UserSOC_CsJ | 13 84 | 99.92 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.3 | Pct_UserSOC_CsJ | 0D 98 | 69.6 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.4 | Pct_UserSOC_CsJ | 0D 60 | 68.48 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.5 | Pct_UserSOC_CsJ | 0F 03 | 76.86 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.6 | Pct_UserSOC_CsJ | 0A F8 | 56.16 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.7 | Pct_UserSOC_CsJ | 09 CD | 50.18 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.8 | Pct_UserSOC_CsJ | 13 88 | 100 % |
| $3415 | ($3415) Memorized User SOC for energy consumption journal.9 | Pct_UserSOC_CsJ | 0C D3 | 65.66 % |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.0 | V_P_DCDC_tot_CsJ | 00 00 | 0 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.1 | V_P_DCDC_tot_CsJ | 00 11 | 0.17 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.2 | V_P_DCDC_tot_CsJ | 00 10 | 0.16 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.3 | V_P_DCDC_tot_CsJ | 00 15 | 0.21 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.4 | V_P_DCDC_tot_CsJ | 00 15 | 0.21 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.5 | V_P_DCDC_tot_CsJ | 00 15 | 0.21 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.6 | V_P_DCDC_tot_CsJ | 00 31 | 0.49 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.7 | V_P_DCDC_tot_CsJ | 00 41 | 0.65 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.8 | V_P_DCDC_tot_CsJ | 00 4F | 0.79 |
| $3416 | ($3416) Memorization of accumulated electrical DCDC energy for energy consumption journal.9 | V_P_DCDC_tot_CsJ | 00 77 | 1.19 |
| $3417 | Saving of the NAVL state of the interlocks.0 | InterlockBatt | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.1 | HVBatPEB | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.2 | JunctBox | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.3 | Interlock_peb_emr | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.4 | SQCharge | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.5 | JB_AC | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.6 | Interlock_CI | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.7 | JB_PTC | 00 | 0 = Interlock OK |
| $3417 | Saving of the NAVL state of the interlocks.8 | Interlock_PTC | 00 | 0 = Interlock OK |
| $3418 | ($3418) DCDC desactivation requested by diagnostic tool in plant | B_APVDCDCOff_dcdc_req | 00 | 0 = Activated  |
| $3419 | ($3419) Memorized crash level 1 (medium) detection | B_MemoCrashLvl1_status | 00 | 0 = no memorized  |
| $341A | ($341A) Memorized crash level 2 (violent) detection | B_MemoCrashLvl2_status | 00 | 0 = no memorized  |
| $341E | ($341E) Rotation speed of External Fan for battery cooling (Peltier) | ExternalFanRotationSpeed | 00 00 | 0 Hz |
| $341F | ($341F) Rotation speed of Internal Fan for battery cooling (Peltier) | InternalFanRotationSpeed | 00 00 | 0 Hz |
| $3420 | ($3420) DIDs supported in range [$3420 - $3440] |  | BF 84 EF FB | (nicht dekodiert) |
| $3421 | ($3421) For reading Battery temperature starting point of the Peltier System (Diagnostic Input) | VH_BattTempAverage_init | 28 | 0 °C |
| $3423 | ($3423) P_R_N_D_P_N | Vbx_levr_psn_bit1 | 00 | 0 = 12 V |
| $3424 | ($3424) P_R_N_D_L2 | Vbx_levr_psn_bit2 | 00 | 0 = 12 V |
| $3425 | ($3425) P_R_N_D_L3 | Vbx_levr_psn_bit3 | 00 | 0 = 12 V |
| $3426 | ($3426) P_R_N_D_L4 | Vbx_levr_psn_bit4 | 00 | 0 = 12 V |
| $3427 | ($3427) debounced signal from eco mode button | Vbx_eco_swt_fil | 00 | 0 = switch not pushed |
| $3428 | ($3428) Battery 14V Voltage after everychecking (internal SCH) | BatteryVoltage_USM_checked | 6A 68 | 13.62 V |
| $3429 | ($3429) DCDC ready or not (internal EVC) | DCDC_Ready | 01 | 1 = DCDC On |
| $342E | ($342E) DCDC ready to sleep (internal EVC) | K_RTS_14VNetMngt_status | 01 | 1 = Refuse to Sleep |
| $3431 | ($3431) Time since the vehicle has been parked (calculated in SCH) | V_TM_ParkingDuration | 80 00 00 27 | 39 min |
| $3432 | ($3432) Battery voltage no load estimation memorization (internal SCH) | V_U_Batt_NoLoad_est_Mem | 64 18 | 12.812 V |
| $3433 | ($3433) Battery voltage request (internal SCH) | V_U_batt_req | 1E | 13.5 V |
| $3435 | ($3435) AC compressor authorization command | K_ENB_CMP | 01 | 1 = Enable |
| $3436 | ($3436) AC compressor RPM real value | CompRPMStatus | 23 28 | 9000 rpm |
| $3437 | ($3437) AC compressor High Voltage value | V_INV_H | FE | 508 V |
| $3438 | ($3438) AC compressor Power Module Temperature | TMP_IPM | A0 | 120 °C |
| $3439 | ($3439) AC compressor Error Status | CI_Error_status | 00 | 0 = No problem |
| $343A | ($343A) AC compressor Motor Phase Current | I_INV | FE | 25.4 |
| $343B | ($343B) AC compressor State | COMP_Status | 00 | 0 = Signal Invalid |
| $343C | ($343C) Dysfunctional disconnections counter | K_Dys_Discon_Counter_est | 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.9 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.8 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.7 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.6 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.5 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.4 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.3 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.2 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.1 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343D | ($343D) Identification number of the batteries for the 10 last quick drop.0 | CM_IdentNumber_QDJ_log | 80 00 00 00 | 0 |
| $343F | ($343F) Identification number of the battery received by CAN | K_IdentNumber_bhv_status | E7 71 A1 02 | 1735500034 |
| $3440 | ($3440) DIDs supported in range [$3440 - $3460] |  | DD F5 FF FF | (nicht dekodiert) |
| $3441 | ($3441) CAN signal for the status of the K-Line communication | K_KLBus_bcb_status | 00 | 0 = No presence |
| $3442 | ($3442) ETS Sleeping authorisation according to HV Battery Management Strategies | K_ReadyToSleep_MngHVB_status | 01 | 1 = Refuse to sleep |
| $3444 | ($3444) CAN signal for the maximal charging power authorized by the HV battery | P_ChargeMax_bhv_est | 2F | 14.1 kW |
| $3445 | ($3445) CAN signal for the estimation of the BCB input power (power consumed from the Mains network) | P_MainsConsumption_bcb_est | 00 00 | 0 W |
| $3446 | ($3446) eco mode request | Vbx_eco_mod_req | 01 | 1 = Mode eco requested |
| $3448 | ($3448) Electrical motor (EM) maximum effective torque available, after eco mode limitation | Vxx_max_emot_tqe_eco | 94 00 | 160 N·m |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.9 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.8 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.7 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.6 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.5 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.4 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.3 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.2 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.1 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $3449 | ($3449) Total vehicle distance associated with each last ten Quick Drop.0 | CM_DistTotal_QDJ_log | 80 00 00 00 | 0 km |
| $344A | ($344A)Counter of fct diag which aim at detecting a too often low performance of battery cooling system in study | K_PeltierCounterM | 00 | 0 |
| $344B | ($344B) Allow to control the BattVE Relay which gives energy to several ECU linked to thermal comfort | B_BattVE_req | 01 | 1 = Closed |
| $344C | ($344C) Plug Unlock/ Trap Open Push Button request | B_METS_PlugUnlock_req | 00 | 0 = No request |
| $344E | ($344E) Energy Trap Actuator Command | B_MngETS_TrapOpen_Act | 00 | 0 = No open trap command |
| $3450 | ($3450) Configuration of the Quick Drop (Enable or disable) | CM_QuickDropConfig | 00 | 0 = No quickdrop |
| $3451 | ($3451) Estimated kilometric cruising range sent to MMI | Dist_KilomRangeMMI_ets_est | 00 73 | 115 km |
| $3452 | ($3452) Eco mode request to be sent on the CAN for CLIM and MMC | Vbx_eco_mod_can | 00 | 0 = Mode ECO not requested |
| $3453 | ($3453) EEPROM memorized value of the eco mode switch request | Vbx_eco_mod_can_mem | 00 | 0 = Mode ECO not requested |
| $3454 | ($3454)Contributions of anticipation lack and excessive speed to electrical overconsumption since cluster reset.0 | Vtx_exc_csm_aftr_rst | 00 00 0C | 0.012 |
| $3454 | ($3454)Contributions of anticipation lack and excessive speed to electrical overconsumption since cluster reset.1 | Vtx_exc_csm_aftr_rst | 00 01 06 | 0.262 |
| $3455 | ($3455) Mean eco electrical consumption (if driver has economical driving behavior) | Vxx_dspl_aut_eco_csm_mv | 15 BE | 5567 kW |
| $3456 | ($3456) Temporary estimated kilometric cruising range sent to MMI (economical driving) | Vxx_dspl_aut_eco_dist_est | 00 7B | 123 km |
| $3457 | ($3457) Mean pessimistic electrical consumption (worst possible consumption) | Vxx_dspl_aut_min_csm_mv | 31 FF | 12800 kW |
| $3458 | ($3458) Temporary estimated minimum kilometric cruising range sent to MMI | Vxx_dspl_aut_min_dist_est | 00 36 | 54 km |
| $3459 | ($3459) Mean consumption per unit of distance, used for calculation of kilometric range | Vxx_dspl_aut_nom_csm_mv | 17 58 | 5977 kW |
| $345A | ($345A) Indicator relative to anticipation for economical scoring | Vxx_dspl_eco_ant | 00 | 0 |
| $345B | ($345B) Indicator of economical monitoring | Vxx_dspl_eco_mon | 00 | 0 |
| $345C | ($345C) Indicator relative to vehicle speed for economical scoring | Vxx_dspl_eco_vs | 00 | 0 |
| $345D | ($345D) Information about the performance of Battery conditionning system memorized in E2PROM | K_CondBattCounterM | 00 | 0 |
| $345E | ($345E) FAN global request from ETS in PWM including AC, Cooling needs and Relay protection check | PWM_PowerOn_engf_req | 05 | 5 % |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.0 | V_K_M14N_RTS_Frame | DD | 1 = EM NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.1 | V_K_M14N_RTS_Frame | DD | 0 = BCB OK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.2 | V_K_M14N_RTS_Frame | DD | 1 = BHV NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.3 | V_K_M14N_RTS_Frame | DD | 1 = MCS NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.4 | V_K_M14N_RTS_Frame | DD | 1 = TC NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.5 | V_K_M14N_RTS_Frame | DD | 0 = MC OK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.6 | V_K_M14N_RTS_Frame | DD | 1 = MHVB NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.7 | V_K_M14N_RTS_Frame | DD | 1 = METS NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.8 | V_K_M14N_RTS_Frame | 01 | 1 = MHVN NOK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.9 | V_K_M14N_RTS_Frame | 01 | 0 = PWT OK |
| $345F | ($345F) This internal M14N variable is a frame composed of every RTS from all ETS system.10 | V_K_M14N_RTS_Frame | 01 | 0 = Peltier OK |
| $3460 | ($3460) DIDs supported in range [$3460 - $3480] |  | FC E7 FF EF | (nicht dekodiert) |
| $3461 | ($3461) Frame consistency ok | Vbx_ubp_chk_sum_ok | 01 | 1 = UBP frame Ok |
| $3462 | ($3462) Vehicle configured with UBP (1) or VPM (0) | Vbx_ubp_pres | 01 | 1 = configured with UBP |
| $3463 | ($3463) electrical braking torque setpoint | Vbx_ubp_whl_tq_sp_req_ok | 00 | 0 = unvalid braking torque setpoint |
| $3464 | ($3464) UBP brake pedal ratio from CAN | Vxx_mux_brk_pdl_rat | 00 | 0 |
| $3465 | ($3465) UBP wheel torque request from CAN | Vxx_mux_ubp_whl_tq_sp | 80 00 | 0 N·m |
| $3466 | ($3466) memorisation flag in NVMY of HV battery over-heating | B_MemoHVBattOVH_status | 00 | 0 = no HV battery over-heating memorisation in NVMY |
| $3469 | ($3469) Flag to prevent the activation of downgraded mode when MSR function is disabled | Vbx_no_pwt_sp_forc_lim_cut | 00 | 0 = No MSR Limp Home mode deactivation requested |
| $346A | ($346A) High Voltage Battery maximal Temperature from reductant LBC | Temp_TempBhv_Max_RCY_mes | 32 | 10 °C |
| $346B | ($346B) No pump allowed neither DCDC if this boolean is set to 0 | B_Aut_WEP_USE_mmr | 01 | 1 = pump and DCDC activation authorized |
| $346E | ($346E) CAN signal for the PEB class of failure status (on charge) | K_ChargeFailure_peb_status | 00 | 0 = Classe A |
| $346F | ($346F) Synthesis of JB charger fault type | K_ChargerFaultType_status | 00 | 0 = No fault type |
| $3471 | ($3471) Request for engine fan speed from ClimBox | Pct_EngineFanSpeedPWM_req | 00 | 0 % |
| $3472 | ($3472) Lever 1st track duty cycle | Vxx_levr_rat_1 | 06 53 | 0.1619 |
| $3473 | ($3473) Lever 2nd track duty cycle | Vxx_levr_rat_2 | 21 0D | 0.8461 |
| $3474 | ($3474) PWM gearbox lever track 1 position | Vsx_raw_gear_pwm_tmp_1 | 01 | 1 = park |
| $3475 | ($3475) PWM gearbox lever track 2 position | Vsx_raw_gear_pwm_tmp_2 | 01 | 1 = park |
| $3476 | ($3476) Counter during one trip for contributions of anticipation lack and excessive vehicle speed to overconsumption.0 | Vtx_exc_el_egy_ctr | 00 00 00 | 0 J |
| $3476 | ($3476) Counter during one trip for contributions of anticipation lack and excessive vehicle speed to overconsumption.1 | Vtx_exc_el_egy_ctr | 00 00 00 | 0 J |
| $3477 | ($3477) Raw mean electrical consumption while economical driving | Vxx_dspl_aut_eco_csm_mv_raw | 15 BF 7B | 1425275 |
| $3478 | ($3478) Raw mean consumption per unit of distance | Vxx_dspl_aut_nom_csm_mv_raw | 17 59 FF | 1530367 |
| $3479 | ($3479) Average over-consumption used for economical monitoring | Vxx_exc_csm_mon_mv | 00 00 00 | 0 |
| $347A | ($347A) Request to turn the reverse lights on (1) during reverse mode | Vbx_rvr_lght_req | 00 | 0 = Reverse light off |
| $347B | ($347B) Pulse box command | V_B_PulseBox_CDE_SOF | 00 | 0 = SOF test not requested |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.0 | U_Consistency_Flag_1 | FF | 1 = Umin not available |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.1 | U_Consistency_Flag_2 | FF | 1 = Value not consistent |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.2 | U_Consistency_Flag_3 | FF | 1 = Trigger inconsistency - Min found at beginning of interval |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.3 | U_Consistency_Flag_4 | FF | 1 = Trigger inconsistency - Min found at end of interval |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.5 | U_Consistency_Flag | FF | 1 = bcs diag2 - current measurement error |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.6 | U_Consistency_Flag_7 | FF | 1 = bcs diag3 - voltage measurement error |
| $347D | ($347D) Internal errors detected by Battery Current Sensor.7 | U_Consistency_Flag_8 | FF | 1 = bcs diag5 - BCS internal failure |
| $347E | ($347E) Memorization of next 4 years alert value in time | AbsTime_Mem | 05 B4 | 1460 d |
| $347F | ($347F) Reinitialisation of memorization of next 4 years alert value in time | M14N_AlertBatt4y_tool | 00 | 0 = Do nothing |
| $3480 | ($3480) DIDs supported in range [$3480 - $34A0] |  | FF FF FF FF | (nicht dekodiert) |
| $3481 | ($3481) Internal counter for SOF test KO | nombre_mission_X | 00 | 0 |
| $3482 | ($3482) Internal counter for SOF test failed | nombre_mission_Y | 00 | 0 |
| $3483 | ($3483) Time of the last SOF Test that occured | V_TM_Mem_Last_Test_SOF | 80 00 00 00 | 0 min |
| $3484 | ($3484) Current measurement given by BCS Battery Current Sensor | I_Imin_Cranking | 80 00 | 0 A |
| $3485 | ($3485) Voltage measurement given by BCS Battery Current Sensor | Volt_Umin_Cranking | 00 00 | 0 V |
| $3486 | ($3486) Alert for changing battery | Battery14VToBeChanged_Display | 00 | 0 = No display |
| $3487 | ($3487) Counter of PWM Efan speed on Low speeds | V_Timer_EngFan_PWM_ON_L1 | 00 09 53 08 | 611080 |
| $3488 | ($3488) Counter of PWM Efan speed on Medium speeds | V_Timer_EngFan_PWM_ON_L2 | 00 0B 6C CF | 748751 |
| $3489 | ($3489) Counter of PWM Efan speed on High speeds | V_Timer_EngFan_PWM_ON_L3 | 00 03 A1 B5 | 238005 |
| $348A | ($348A) Current memorized for SOF test.1 | I_Batt_SOF_A_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.2 | I_Batt_SOF_B_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.3 | I_Batt_SOF_C_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.4 | I_Batt_SOF_D_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.5 | I_Batt_SOF_E_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.6 | I_Batt_SOF_F_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.7 | I_Batt_SOF_G_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.8 | I_Batt_SOF_H_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.9 | I_Batt_SOF_I_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.10 | I_Batt_SOF_J_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.11 | I_Batt_SOF_K_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.12 | I_Batt_SOF_L_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.13 | I_Batt_SOF_M_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.14 | I_Batt_SOF_N_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.15 | I_Batt_SOF_O_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.16 | I_Batt_SOF_P_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.17 | I_Batt_SOF_Q_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.18 | I_Batt_SOF_R_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.19 | I_Batt_SOF_S_1000 | 80 00 | 0 A |
| $348A | ($348A) Current memorized for SOF test.20 | I_Batt_SOF_T_1000 | 80 00 | 0 A |
| $348B | ($348B) Result of measure memorized for SOF test.1 | SOF_measure_vld_A_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.2 | SOF_measure_vld_B_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.3 | SOF_measure_vld_C_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.4 | SOF_measure_vld_D_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.5 | SOF_measure_vld_E_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.6 | SOF_measure_vld_F_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.7 | SOF_measure_vld_G_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.8 | SOF_measure_vld_H_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.9 | SOF_measure_vld_I_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.10 | SOF_measure_vld_J_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.11 | SOF_measure_vld_K_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.12 | SOF_measure_vld_L_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.13 | SOF_measure_vld_M_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.14 | SOF_measure_vld_N_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.15 | SOF_measure_vld_O_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.16 | SOF_measure_vld_P_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.17 | SOF_measure_vld_Q_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.18 | SOF_measure_vld_R_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.19 | SOF_measure_vld_S_0100 | 00 00 00 | 0 |
| $348B | ($348B) Result of measure memorized for SOF test.20 | SOF_measure_vld_T_0100 | 00 00 00 | 0 |
| $348C | ($348C) Absolut time memorized for SOF test.1 | TM_Last_SOF_test_A_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.2 | TM_Last_SOF_test_B_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.3 | TM_Last_SOF_test_C_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.4 | TM_Last_SOF_test_D_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.5 | TM_Last_SOF_test_E_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.6 | TM_Last_SOF_test_F_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.7 | TM_Last_SOF_test_G_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.8 | TM_Last_SOF_test_H_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.9 | TM_Last_SOF_test_I_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.10 | TM_Last_SOF_test_J_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.11 | TM_Last_SOF_test_K_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.12 | TM_Last_SOF_test_L_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.13 | TM_Last_SOF_test_M_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.14 | TM_Last_SOF_test_N_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.15 | TM_Last_SOF_test_O_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.16 | TM_Last_SOF_test_P_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.17 | TM_Last_SOF_test_Q_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.18 | TM_Last_SOF_test_R_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.19 | TM_Last_SOF_test_S_0010 | 00 00 00 | 0 min |
| $348C | ($348C) Absolut time memorized for SOF test.20 | TM_Last_SOF_test_T_0010 | 00 00 00 | 0 min |
| $348D | ($348D) Voltage memorized for SOF test.1 | Volt_Batt_SOF_A_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.2 | Volt_Batt_SOF_B_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.3 | Volt_Batt_SOF_C_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.4 | Volt_Batt_SOF_D_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.5 | Volt_Batt_SOF_E_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.6 | Volt_Batt_SOF_F_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.7 | Volt_Batt_SOF_G_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.8 | Volt_Batt_SOF_H_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.9 | Volt_Batt_SOF_I_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.10 | Volt_Batt_SOF_J_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.11 | Volt_Batt_SOF_K_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.12 | Volt_Batt_SOF_L_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.13 | Volt_Batt_SOF_M_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.14 | Volt_Batt_SOF_N_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.15 | Volt_Batt_SOF_O_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.16 | Volt_Batt_SOF_P_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.17 | Volt_Batt_SOF_Q_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.18 | Volt_Batt_SOF_R_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.19 | Volt_Batt_SOF_S_0001 | 00 00 | 0 V |
| $348D | ($348D) Voltage memorized for SOF test.20 | Volt_Batt_SOF_T_0001 | 00 00 | 0 V |
| $348E | ($348E) DCDC voltage control request calculated and used when no default on 14V network is detected | V_U_DCDCControl_req | 1F | 13.55 V |
| $348F | ($348F)Boolean Variable:M14N resquest for activation when SOC low | B_14VBatChg_ets_req | 00 | 0 = Not Activation |
| $3490 | ($3490) Critical Failure detected on the 14V network (internal SCH) | V_K_DCDC_14v_Default_status | 00 | 0 = Unavailable value |
| $3491 | ($3491) Request of ETS to force 14v network energy management into traction battery (from ETS CAN) | V_B_DCDC_modecharge_req | 00 | 0 = Not Autorized |
| $3492 | ($3492) Boolean that indicates the loss of a correct measurement on 14V | V_B_BattSCG | 00 | 0 = Tension Battery Available |
| $3493 | ($3493) Authorization from METS to activare DCDC converter | V_B_MngETS_DCDC_Act | 01 | 1 = Autorized DCDC Activation |
| $3494 | ($3494) Instantaneous DCDC output power | V_P_DCDC_mes | 00 0E | 140 W |
| $3495 | ($3495) Current ratio from DCDC. (Idcdc / Idcdcmax) | V_Pct_DF_DCDC | 15 | 8.203125 % |
| $3496 | ($3496) 14 V battery charge Autorisation flag from METS | B_MngETS_14VBatChg_Auth | 00 | 0 = dcdc activation forbidden |
| $3497 | ($3497) Better Place Config | BetterPlaceConfig | 00 | 0 = NO BetterPlace Vehicle |
| $3498 | ($3498) This flag shows the battery relay PTC n°1(X10,L38G2,X61G3) request | B_BattCTP_relay1_req | 00 | 0 = no activation |
| $3499 | ($3499) relay thermoplunger (X07) n°1 request | B_BattTHP_relay1_req | 00 | 0 = no activation |
| $349A | ($349A) This flag shows the battery relay PTC n°2 (X10,L38G2,X61G3) request | B_BattCTP_relay2_req | 00 | 0 = no activation |
| $349B | ($349B) relay thermoplunger (X07) n°2 request | B_BattTHP_relay2_req | 00 | 0 = no activation |
| $349C | ($349C) This flag shows the battery relay PTC n°3 (X10,L38G2,X61G3) request | B_BattCTP_relay3_req | 00 | 0 = no activation |
| $349D | ($349D) relay thermoplunger (X07) n°3 request | B_BattTHP_relay3_req | 00 | 0 = no activation |
| $349E | ($349E) Battery evaporator electrical Valve status (Valve for battery HVAC) | B_BatteryCooling_EV_req | 00 | 0 = no activation |
| $349F | ($349F) Cabin evaporator electrical Valve status (Valve for cabin HVAC) | B_CabinCooling_EV_req | 00 | 0 = no activation |
| $34A0 | ($34A0) DIDs supported in range [$34A1 - $34C0] |  | 3F FF 7F FF | (nicht dekodiert) |
| $34A3 | ($34A3) Rear Evaporator temperature target | VH_HVBattEvapTempSetPoint_Bat | 09 1A | -40 °C |
| $34A4 | ($34A4) Battery cooling order | K_HVBcond_ets_Auth_req | 00 | 0 = No_Cool_Bat |
| $34A5 | ($34A5) TC flag ready to sleep | K_ReadyToSleep_TC_status | 01 | 1 = refuse to sleep |
| $34A6 | ($34A6) Battery HVAC speed rotation request (command of the blower) | k_BattCool_blower_req | 00 | 0 % |
| $34A7 | ($34A7) Battery Cooling System Type | HVBCOOLConfig | 03 | 3 = Water Cooling without chiller |
| $34A8 | ($34A8) Battery Heating System Type | HVBHEATConfig | 04 | 4 = Water Coolant Heaters(CTP) evc (1 relay) |
| $34A9 | ($34A9) Configuration of the TCU (Enable or disable) | TCUConfig | 01 | 1 = avec TCU |
| $34AA | ($34AA) Differential Protection in EVC | Diff_ProtectConfig | 00 | 0 = NO Differential Protection in EVC |
| $34AD | ($34AD) Set-point for the charge current for the JB2 | I_JB2SetPoint_ets_req | 80 00 | 0 A |
| $34AE | ($34AE) PEB frame monitoring status flags.all | Vxx_sfty_mon_emot_n_mem | 00 | 0 |
| $34AE | ($34AE) PEB frame monitoring status flags.B0 |  | 00 | 0 = Default on engine speed frame sent by PEB (see other bits) |
| $34AE | ($34AE) PEB frame monitoring status flags.B1 |  | 00 | 0 = No clock default |
| $34AE | ($34AE) PEB frame monitoring status flags.B2 |  | 00 | 0 = No CRC default |
| $34AE | ($34AE) PEB frame monitoring status flags.B4 |  | 00 | 0 = Frame presence |
| $34AF | ($34AF) EVC waiting for isolation confirmation before starting charge | B_MngETS_ChgWait_HvIns | 00 | 0 = Driving mode |
| $34B0 | ($34B0) Configuration of the Load Shedding | LoadSheddingConfig | 00 | 0 = No load shedding |
| $34B2 | ($34B2) Gasoline heater configuration V2 | GHConfig / RH4 420 | 00 | 0 = no heater |
| $34B3 | ($34B3) PTC_Config V2 | PTCConfig / RH4 420 | 02 | 2 = HV PTC with air |
| $34B4 | ($34B4) Cut off charge request by thermal comfort to start compressor with safety conditions | B_ChargeState_status | 01 | 1 = No cut off for charge requested |
| $34B5 | ($34B5) Authorization for deicing function | B_TC_CAREG_DeIcing_Auth | 03 | 3 = 3: Unavailable value |
| $34B6 | ($34B6) Pre-conditionning available energy | E_ClimAvailableEnergy_est | 00 | 0 |
| $34B7 | ($34B7) Defaults and state of the compressor description | K_AC_Mngt_status | 03 | 3 = Off with fault |
| $34B8 | ($34B8) Request for activation of preconditionning from EVC to Climbox | K_CaregPC_TC_Act | 00 | 0 = No PC requested |
| $34B9 | ($34B9) Pre Conditionning activation request from cluster | K_ClimPCMMCCustomerActiv_status | 00 | 0 = No PreHeating |
| $34BA | ($34BA) Pre-conditionning request memorized by EVC | K_ClimProgramPCDisplay_status | 01 | 1 |
| $34BB | ($34BB) Request for low level battery | K_HVBatteryLevelAlert_status | 00 | 0 = No alert |
| $34BC | ($34BC) Request for flashers when Immediate PreHeating is requested | K_ImmediatePreheatAutho_status | 01 | 1 = No pre heat requested |
| $34BD | ($34BD) Left time calculated before the customer hour programmation | K_LeftTimeBefDepart_TC_est | 7F | 127 min |
| $34BE | ($34BE) Local time received by EVC | K_LocalTime_veh_mes | 05 A0 | 1440 min |
| $34BF | ($34BF) Immediate pre-conditionning request | K_MngETS_PHByKey_Auth | 00 | 0 = C_METS_TC_NoAuth (No Authorization) |
| $34C0 | ($34C0) DIDs supported in range [$34C1 - $34E0] |  | FF FF FB DD | (nicht dekodiert) |
| $34C1 | ($34C1) Pre Conditionning activation request from climbox | K_PCButton_TC_status | 00 | 0 = No PreHeating |
| $34C2 | ($34C2) Defaults and state of the HV PTC description | K_PTC_Mngt_status | 01 | 1 = Off |
| $34C3 | ($34C3) Scheduled time 1 requested by the customer for the preconditionning system | K_ScheduledTime1_TC_est | 00 00 | 0 min |
| $34C4 | ($34C4) Scheduled time 2 requested by the customer for the preconditionning system | K_ScheduledTime2_TC_est | 00 00 | 0 min |
| $34C5 | ($34C5) Request to the BCM to start the counter for vehicle wakeup due to preconditionning | K_StartCount_TC_req | 00 | 0 |
| $34C6 | ($34C6) Request to the BCM to stop the counter for vehicle wakeup due to preconditionning | K_StopCount_TC_req | 00 | 0 |
| $34C7 | ($34C7) Supposed customer departure value | K_SupCustomerDepart_TC_req | 00 | 0 |
| $34C8 | ($34C8) Power available for Climate functions | P_Auth_ptc_est | 28 | 40 kW |
| $34C9 | ($34C9) Available charging power necessary to authorize some functions (preconditionning, battery cooling) | P_AvailChargingPwr_est | 01 FF | 511 kW |
| $34CA | ($34CA) Available power for preconditionning | P_PowerLeft_TC_est | A0 | 4000 W |
| $34CB | ($34CB) Cabin blower command | Pct_ClimAirFlow_veh_mes | 00 | 0 % |
| $34CC | ($34CC) Scheduled Time 1 memorized by EVC | Time_PCScheduledTimeTdB1_status | 00 00 | 0 min |
| $34CD | ($34CD) Scheduled Time 2 memorized by EVC | Time_PCScheduledTimeTdB2_status | 00 00 | 0 min |
| $34CE | ($34CE) Manage Failure Authorization of the HV PTC | B_HVDiag_PTC_HT_Auth_status | 01 | 1 = HV PTC authorized |
| $34CF | ($34CF) Power consumed by the 400V components for Thermal Comfort | P_UsedPower_TC_HT | 00 | 0 W |
| $34D0 | ($34D0) Internal flag : HV Battery Conditionning Authorisation flag | B_MngETS_AC_HVBCool_Auth | 00 | 0 |
| $34D1 | ($34D1) This variable allows to memorize the reseted energy consumed 2 | CM_ResetedConsumed2Energy | 00 4C F2 | 19.698 |
| $34D2 | ($34D2) This variable allows to memorize the reseted distance 2 | CM_ResetedTravelledDistance2 | 00 18 0E FC | 1576700 |
| $34D3 | ($34D3) HV Battery Conditionning Activation Autorisation | K_MngETS_AC_HVBCool_Auth | 00 | 0 = 0: No Authorisation |
| $34D4 | ($34D4) Signal of requesting the MMI DoNotTow | K_ETAlertDoNotTow_req | 01 | 1 = 1: No ET Alert Do Not Tow Requested |
| $34D5 | ($34D5) Chiller activation request | VH_ChillerActivationReq | 01 | 1 = 1: No Chiller Activation Requested |
| $34D7 | ($34D7) HVB chiller temperature set point | VH_HVBattChilTempSetPoint_Bat | 28 | 0 °C |
| $34D8 | ($34D8) Water Temperature in BCB | VH_BCB_WaterTemp | 12 | 18 °C |
| $34D9 | ($34D9) State of Chiller valves received from CAREG | VH_ChillerActivationStatus | 00 | 0 = 0: no actived valves |
| $34DA | ($34DA) Postal service configuration | LotQuickDrivingBreaks | 00 | 0 = No Lot of quick Driving Breaks |
| $34DC | ($34DC) CAN signal for PEB charge mode status V2 | K_ChargingState_peb_status | 01 | 1 = Not in charge |
| $34DD | ($34DD) Request for PEB Charge mode V2 | K_PEBChargeState_ets_req | 01 | 1 = No charge requested |
| $34DE | ($34DE) Compressor Configuration V3 | CompConfig / RH4 550 | 01 | 1 = Denso compressor |
| $34E0 | ($34E0) DIDs supported in range [$34E0 - $3500] |  | 1F FF FF FF | (nicht dekodiert) |
| $34E4 | ($34E4) Charge power available.9 | P_ChargeMaxJB2_CJ_read | 00 5F | 95 kW |
| $34E4 | ($34E4) Charge power available.8 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E4 | ($34E4) Charge power available.7 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E4 | ($34E4) Charge power available.6 | P_ChargeMaxJB2_CJ_read | 00 5F | 95 kW |
| $34E4 | ($34E4) Charge power available.5 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E4 | ($34E4) Charge power available.4 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E4 | ($34E4) Charge power available.3 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E4 | ($34E4) Charge power available.2 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E4 | ($34E4) Charge power available.1 | P_ChargeMaxJB2_CJ_read | 00 5F | 95 kW |
| $34E4 | ($34E4) Charge power available.0 | P_ChargeMaxJB2_CJ_read | 00 60 | 96 kW |
| $34E5 | ($34E5) Low level battery warning sent to the MMI | B_LowLevBattEnergyMMI_ets_warn | 00 | 0 = Not LED |
| $34E6 | ($34E6) Empty level battery warning sent to the MMI | B_EmptyLevBatEnergyMMI_ets_warn | 00 | 0 = Not LED |
| $34E7 | ($34E7) Coding Signal from CAN data CompTemperatureDischarge | Temp_CompTempDischarge_mes | 00 00 | -60 |
| $34E8 | ($34E8) Coding Signal from CAN data DeIcingRequest | K_DeIcingRequest_status | 00 | 0 = Not used |
| $34E9 | ($34E9) Ratio of PTC power used | Pct_HighVoltagePTCPWM_req | 00 | 0 |
| $34EA | ($34EA) Estimation of the current consumed by the PTC | I_PTCCurrent_ptc_est | 00 | 0 |
| $34EB | ($34EB) Power used by HVPTC | P_UsedPower_HVPTC | 00 | 0 W |
| $34EC | ($34EC) State of the AC Loop | K_ClimLoopMode_status | 00 | 0 = Unavailable |
| $34ED | ($34ED) PreheatingSelectedDisplayRequest | K_PCStateDisp_TC_status | 00 | 0 = No request to display PreHeating state |
| $34EE | ($34EE) LeftTimeBeforeVehicleWakeUp | K_LeftTimeBefWU_TC_req | 03 84 | 900 min |
| $34EF | ($34EF) HVB Water Cooling Temperature | VH_WaterHVBCoolingTemp_mes | 3A | 18 °C |
| $34F0 | ($34F0) Command of battery heater relay | V_B_14VBattHeat_relay_cmd | 00 | 0 = Request for opening the battery heater relay |
| $34F1 | ($34F1) Request to METS asking for authorisation of battery heating | B_14VBattHeat_req | 00 | 0 = Not Autorized |
| $34F2 | ($34F2) Maximal power authorised for thermal comfort | P_Auth_ptc14_charge | 11 | 17 kW |
| $34F3 | ($34F3)HV min Battery Temp checked by CAN and invalid value limp home strategiesand filtred with a diff resolution | Temp_TempBhv_Min_Ihm_flt | 0B 0E | 100 °C |
| $34F4 | ($34F4) Threshold for HVBattery conditioning received from BMS | Temp_Thld_MaxStopCool_lbc_mes | 4B | 35 °C |
| $34F5 | ($34F5) Threshold for HVBattery conditioning received from BMS (01) | Temp_Thld_MaxStopHeat_lbc_mes | 3E | 22 °C |
| $34F6 | ($34F6) Threshold for HVBattery conditioning received from BMS (02) | Temp_Thld_MaxTempCool_lbc_mes | 50 | 40 °C |
| $34F7 | ($34F7) Threshold for battery thermal management | Temp_Thld_MinStopCool_Ihm_mes | 4A | 34 °C |
| $34F8 | ($34F8) Threshold for HVBattery conditioning received from BMS (03) | Temp_Thld_MinStopCool_lbc_mes | 4A | 34 °C |
| $34F9 | ($34F9) Number of authorized stages for CTP battery | VH_PTCNumberAuthorized | 00 | 0 |
| $34FA | ($34FA) Power authorized for CTP battery activation | VH_FinalBattPTCNumberThermalReq | 00 | 0 |
| $34FB | ($34FB) Number of authorized stages for CTP battery taking power allocated into account | VH_PTCElecPwLevelLimit | 03 | 3 |
| $34FC | ($34FC) Water temperature used for Chiller | VH_Temp_BHV_CoolingLoop | 38 | 16 °C |
| $34FD | ($34FD)HV min Battery Temp checked by CAN and invalid value limp home strategiesand filtred with a diff resolution1 | VH_TempBatMin | 0B 0E | 100 °C |
| $34FE | ($34FE) Authorisation req checking for compressor in charge mode | B_clim_isol_check_req | 00 | 0 = Not Autorized |
| $34FF | ($34FF) Second water gate when battery is cooled with water | B_WaterGateChillerCool_req | 00 | 0 = Opened |
| $3500 | ($3500) DIDs supported in range [$3500 - $3520] |  | 4D FF FF FF | (nicht dekodiert) |
| $3502 | ($3502) FANConfig flag memorised into EVC EEPROM at powelatch | FANConfig | 01 | 1 = GMV PWM |
| $3505 | ($3505) PLGWConfig flag memorised into EVC EEPROM at powelatch | PLGWConfig | 00 | 0 = without PLGW |
| $3506 | ($3506) PlugTypeConfig flag memorised into EVC EEPROM at powelatch | PlugTypeConfig | 02 | 2 = Plug Type 2 WITH Temperature sensor |
| $3508 | ($3508) AEBS presence for regenerative braking |  | 00 | 0 = without AEBS |
| $3509 | ($3509) BatVE presence | BatVE | 01 | 1 = Bat VE present |
| $350A | ($350A) Battery Heating 12V presence | BattHeating12VConfig | 00 | 0 = 0: without Battery Heating |
| $350B | ($350B) Raw gear position of PWM lever corresponding to the actual lever position | Vsx_raw_gear_pwm | 01 | 1 = 1: Park |
| $350C | ($350C) flag configuration X61 with Heat pump | VH_HP_61gen3_conf | 00 | 0 |
| $350D | ($350D) Response Error HP61 | VH_B_Response_Error_HP61 | 00 | 0 |
| $350E | ($350E) Climatisation Compressor Speed Heat HP61 request | VH_ClimCompSpeedHeat_HP61_req | 00 00 | 0 |
| $350F | ($350F) Climatisation Loop Mode HP61 status | VH_ClimLoopMode_HP61_status | 00 | 0 = 0: Unavailable |
| $3510 | ($3510) Cooling Mode Request | VH_CoolingModeRequest | 00 | 0 = 0: No cooling mode requested |
| $3511 | ($3511) Delcing Request HP61 status | VH_DelcingRequestHP61_status | 00 | 0 = 0: no de-ice loop requested |
| $3512 | ($3512) BatVE ShutDown Alert HP61 | K_BatVEShutDownAlertHP61 | 02 | 2 |
| $3513 | ($3513) Cooling Mode Request (01) | K_CoolingModeRequest | 00 | 0 = 0: No cooling mode requested |
| $3514 | ($3514) DeIce Vehicle Request | K_DeIceVehicleRequest | 00 | 0 = 0: No deIce vehicle request |
| $3515 | ($3515) Deice Permission | K_DeicePermission | 01 | 1 = 1: DeIce not permetted |
| $3516 | ($3516) Heating Mode Request | K_HeatingModeRequest | 00 | 0 = 0: No heating mode requested |
| $3517 | ($3517) PTC Number Thermal BCM request | K_PTCNumberThermal_BCM_req | 04 | 4 |
| $3518 | ($3518) Water Temperature SetPoint HP61 | K_WaterTempSetPoint_HP61 | 00 24 | 36 °C |
| $3519 | ($3519) BlowerSpeed PH61 status | Pct_BlowerSpeed_PH61_status | 00 | 0 % |
| $351A | ($351A) Temperature Evaporation SetPoint HP61 request | Temp_EvapoTempSetPoint_HP61_req | 0A AA | 0 °C |
| $351B | ($351B) Temperature Hot Source HP61 | Temp_HotSource_HP61_mes | 00 | -40 °C |
| $351C | ($351C) Hot Mode HP61 | HotMode_HP61 | 00 | 0 |
| $351D | ($351D) flag configuration X61 with Heat pump | VH_HP_61gen3_conf_1 | 00 | 0 |
| $351E | ($351E) Cooling fluid config (01) | CoolingFluidConfig | 00 | 0 = 0: 1234YF |
| $351F | ($351F) AC Heat pump circuit Valve 1 managed by PAC system on LIN | K_OutputControlValve1_HP61 | 00 | 0 = 0 : no request |
| $3520 | ($3520) DIDs supported in range [$3520 - $3540] |  | FF FF 7F FF | (nicht dekodiert) |
| $3521 | ($3521) AC Heat pump circuit Valve 2 managed by PAC system on LIN | K_OutputControlValve2_HP61 | 00 | 0 = 0 : no request |
| $3522 | ($3522) IH_FailureValve1_HP61_status | VH_FailureValve1_HP61_status | 00 | 0 |
| $3523 | ($3523) IH_FailureValve2_HP61_status | VH_FailureValve2_HP61_status | 00 | 0 |
| $3524 | ($3524) VH_FailureSensor2_HP61_status | VH_FailureSensor2_HP61_status | 00 | 0 |
| $3525 | ($3525) VH_FailureSensor1_HP61_status | VH_FailureSensor1_HP61_status | 00 | 0 |
| $3526 | ($3526) Reset fault identifying number | Vxx_sfty_hv_bat_mem | 00 00 | 0 |
| $3527 | ($3527) Preheating requested by the customer | VH_PreheatByKey_req | 00 | 0 |
| $3528 | ($3528) Authorization boolean from METS for PreheatByKey f(SOC) | VH_UserSOCPHByKey_Auth | 01 | 1 |
| $3529 | ($3529) Authorization boolean from METS for Preheat f(SOC) | VH_UserSOC_Auth | 00 | 0 |
| $352A | ($352A) Less power range by AC to decrease IHM range autonomy | VH_LessPowerRangeByAC | 00 | 0 |
| $352B | ($352B) B_Batt_BlowerState for MCS | B_Batt_BlowerState | 00 | 0 |
| $352C | ($352C) B_CTP_Rly_ConfirmedFail to MCS | B_CTP_Rly_ConfirmedFail | 00 | 0 |
| $352D | ($352D) K_LessPowerRangeByAC for MMI display X07 | K_LessPowerRangeByAC | 00 | 0 |
| $352E | ($352E) ClimTempDisplay | IH_Temp_ClimDisplay_mes | 14 | 10 |
| $352F | ($352F) Internal Temp | IH_Temp_InternalTemp_mes | 28 | 0 °C |
| $3532 | ($3532) Contry configuration | ConsConfig | 00 | 0 = 0 : Remain of the world |
| $3533 | ($3533) DeIcing Activation | B_TC_PAC_DeIcing_req | 00 | 0 = 0: Not activated |
| $3534 | ($3534) HV Battery cooling State | VH_BattCoolState | 00 | 0 = 0: No HV Battery cooling |
| $3535 | ($3535) DeIce Activation Authorisation flag | VH_DeIcingAuth | 00 | 0 = 0: Not Autorized |
| $3536 | ($3536) HV Battery Condtionning mode | VH_HVBattCondMode | 03 | 3 = 3: CondNotReq |
| $3537 | ($3537) IH_Pct_EngineFanSpeedPWM_HP61 | VH_Pct_EngineFanSpeedPWM_HP61 | 00 | 0 % |
| $3538 | ($3538) IH_StateValve1_HP61_status | VH_StateValve1_HP61_status | 00 | 0 |
| $3539 | ($3539) IH_StateValve2_HP61_status | VH_StateValve2_HP61_status | 00 | 0 |
| $353A | ($353A) Physical value of the sensor NTC1 (Td) | VH_Temp_StateSensor1_HP61 | 00 3C | 0 °C |
| $353B | ($353B) Physical value of the sensor NTC2 (Ts) | VH_Temp_StateSensor2_HP61 | 7F FF | 0 °C |
| $353C | ($353C) VH_ChillerInDriveON | VH_ChillerInDriveON | 00 | 0 |
| $353D | ($353D) Number of batt CTP or Thp water requested | VH_BattPTCNumberThermalReq_flt | 00 | 0 |
| $353E | ($353E) VH_BattThpNumberThermalReq | VH_BattThpNumberThermalReq | 00 | 0 |
| $353F | ($353F) Raw gear position of lever by CAN corresponding to the actual lever position | Vsx_raw_gear_mux | 01 | 1 = 1: Park |
| $3540 | ($3540) DIDs supported in range [$3540 - $3560] |  | E0 00 3F FE | (nicht dekodiert) |
| $3541 | ($3541) Boolean of Charge limitation during Very cold preheat | B_Charge_Lim_Cold_PC_CTPHT | 00 | 0 = No limitation |
| $3542 | ($3542) Value of the safety torque structure monitoring status flags before the hot-reset | Vxx_sfty_mon_sf_tqe_bfr_rst | 00 00 | 0 |
| $3543 | ($3543) Temperature plug vehicle | Temp_Plug_Vehicle | 33 | 11 °C |
| $3553 | ($3553) Memorisation value of average consumption | E_AvgCons_mem | 00 7B | 123 |
| $3554 | ($3554) Memorisation value of average consumption 2 | E_AvgCons2_mem | 00 7B | 123 |
| $3555 | ($3555) Counter of the derating occured | K_GlobalDerating_Counter_mem | 0B 3F | 2879 |
| $3556 | ($3556) Counter for successful starts of AC charge | K_ACChg_Start_Counter_est | 00 00 | 0 |
| $3557 | ($3557) Total energy consumed , since last reset | K_EnrgyCons_total_mem | 00 00 | 0 |
| $3558 | ($3558) Total energy regenerated , since last reset | K_EnrgyRec_total_mem | 00 00 | 0 |
| $3559 | ($3559) Total auxiliary energy consumed , since last reset | K_AuxEnrgyCons_total_mem | 00 00 | 0 |
| $355A | ($355A) Number of drying cycles performed | K_DryCount_mem | 00 00 | 0 |
| $355B | ($355B) Number of relay close/open cycles performed | K_RlyCount_mem | 00 01 | 1 |
| $355C | ($355C) K_op_typ_req | K_op_typ_req | 03 | 3 = Driving |
| $355D | ($355D) K_op_typ_stt | K_op_typ_stt | 05 | 5 = Driving |
| $355E | ($355E) Batery charging | K_op_typ_stt_rcy | 05 | 5 |
| $355F | ($355F)global derating counter | K_GlobalDerating_Counter | 70 7C | 28796 |
| $3561 | ($3561) Pressure sensor value 1 | Vxx_GE_sens_pres_1 | 05 24 | 19.74 kPa |
| $3562 | ($3562) Pressure sensor value 2 | Vxx_GE_sens_pres_2 | 0B AD | 44.835 kPa |
| $3563 | ($3563) Pressure gradient value 1 | Vxx_sens_pres1_grad | 00 00 | 0 kPa |
| $3564 | ($3564) Pressure gradient value 2 | Vxx_sens_pres2_grad | 00 00 | 0 kPa |
| $3565 | ($3565) Error fault from pressure sensor 1 | Vxx_diag_GE_PS_1 | 00 00 | 0 = No error |
| $3566 | ($3566) Error fault from pressure sensor 2 | Vxx_diag_GE_PS_2 | 00 00 | 0 = No error |
| $3567 | ($3567) LBC Frame status received | Vbx_sfty_abs_can_frm_LBC | 00 | 0 = Nsx_pres |
| $3568 | ($3568) Value of Thermal propagation alert | K_TI_alert_status | 00 | 0 = No TI Alert |
| $3569 | ($3569) BMS loss after alert from Thermal propagation alert | Vxx_CANLoss_TI | 00 | 0 = False |
| $3570 | ($3570)Command of the Air Flaps in PWM | Vxx_AirFlaps_Act | 50 | 80 |
| $F000 | DIDs supported in range [$F001 - $F020] |  | 00 01 C0 00 | (nicht dekodiert) |
| $F010 | Fingerprint information.01 | Vxx_fng_1 | 0B | 11 |
| $F010 | Fingerprint information.02 | Vxx_fng_2 | 0C | 12 |
| $F010 | Fingerprint information.03 | Vxx_fng_3 | 14 | 20 |
| $F010 | Fingerprint information.04 | Vxx_fng_4 | 50 | 80 |
| $F010 | Fingerprint information.05 | Vxx_fng_5 | 4C | 76 |
| $F010 | Fingerprint information.06 | Vxx_fng_6 | 41 | 65 |
| $F010 | Fingerprint information.07 | Vxx_fng_7 | 4E | 78 |
| $F010 | Fingerprint information.08 | Vxx_fng_8 | 54 | 84 |
| $F010 | Fingerprint information.09 | Vxx_fng_9 | 20 20 | 8224 |
| $F010 | Fingerprint information.10 | Vxx_fng_10 | 20 | 32 |
| $F010 | Fingerprint information.11 | Vxx_fng_11 | 20 | 32 |
| $F010 | Fingerprint information.12 | Vxx_fng_12 | 00 | 0 |
| $F010 | Fingerprint information.13 | Vxx_fng_13 | 00 | 0 |
| $F010 | Fingerprint information.14 | Vxx_fng_14 | 00 | 0 |
| $F010 | Fingerprint information.15 | Vxx_fng_15 | 00 | 0 |
| $F010 | Fingerprint information.16 | Vxx_fng_16 | 00 | 0 |
| $F011 | the IndexSrvData.01 | Vxx_srv_data_1 | 52 | 82 |
| $F011 | the IndexSrvData.02 | Vxx_srv_data_2 | 56 | 86 |
| $F011 | the IndexSrvData.03 | Vxx_srv_data_3 | 49 | 73 |
| $F011 | the IndexSrvData.04 | Vxx_srv_data_4 | 54 | 84 |
| $F011 | the IndexSrvData.05 | Vxx_srv_data_5 | 45 | 69 |
| $F011 | the IndexSrvData.06 | Vxx_srv_data_6 | 53 | 83 |
| $F011 | the IndexSrvData.07 | Vxx_srv_data_7 | 45 | 69 |
| $F011 | the IndexSrvData.08 | Vxx_srv_data_8 | 56 | 86 |
| $F011 | the IndexSrvData.09 | Vxx_srv_data_9 | 43 | 67 |
| $F011 | the IndexSrvData.10 | Vxx_srv_data_10 | 31 | 49 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).01 | Cxx_c_id1 | 52 | 82 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).02 | Cxx_c_id2 | 30 | 48 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).03 | Cxx_c_id3 | 32 | 50 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).04 | Cxx_c_id4 | 35 | 53 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).05 | Cxx_c_id5 | 30 | 48 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).06 | Cxx_c_id6 | 33 | 51 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).07 | Cxx_c_id7 | 44 | 68 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).08 | Cxx_c_id8 | 37 | 55 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).09 | Cxx_c_id9 | 33 | 51 |
| $F012 | VehicleManufacturerECUSoftwareNumber (Ref C).10 | Cxx_c_id10 | 32 | 50 |
| $F180 | Boot identification |  | 43 6F 6E 74 69 42 6F 6F 74 2D 46 53 5F 52 45 41 34 30 5F 30 4C 30 5F 31 30 30 00 00 00 00 00 00 | (nicht dekodiert) |
| $F182 | applicationDataIdentification |  | 26 80 | (nicht dekodiert) |
| $F187 | vehicleManufacturerSparePartNumber |  | 32 33 37 44 34 30 34 34 34 52 | (nicht dekodiert) |
| $F188 | vehicleManufacturerECUSoftwareNumber |  | 32 33 37 44 34 30 34 34 34 52 | (nicht dekodiert) |
| $F18A | systemSupplierIdentifier |  | 43 4F 4E 54 49 4E 45 4E 54 41 4C 5F 45 4E 47 49 4E 45 5F 53 59 53 54 45 4D 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 | (nicht dekodiert) |
| $F18C | ECUSerialNumber |  | 32 30 35 31 34 34 30 30 30 30 30 30 30 30 30 30 30 30 30 30 | (nicht dekodiert) |
| $F190 | VIN |  | 56 46 31 41 48 30 30 30 32 36 36 38 38 33 31 36 34 | (nicht dekodiert) |
| $F191 | vehicleManufacturerECUHardwareNumber |  | 32 33 37 44 34 30 33 39 36 52 | (nicht dekodiert) |
| $F194 | systemSupplierECUSoftwareNumber |  | 30 41 34 30 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | (nicht dekodiert) |
| $F195 | systemSupplierECUSoftwareVersionNumber |  | 33 30 30 30 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 | (nicht dekodiert) |
| $F196 | exhaustRegulationOrTypeApprovalNumber |  | 48 4D 4C 47 54 39 38 36 36 52 | (nicht dekodiert) |
| $F197 | systemNameorEngineType.01 | Cxx_obd_ecu_name_1 | 48 | 72 |
| $F197 | systemNameorEngineType.02 | Cxx_obd_ecu_name_2 | 45 | 69 |
| $F197 | systemNameorEngineType.03 | Cxx_obd_ecu_name_3 | 56 | 86 |
| $F197 | systemNameorEngineType.04 | Cxx_obd_ecu_name_4 | 43 | 67 |
| $F197 | systemNameorEngineType.05 | Cxx_obd_ecu_name_5 | 00 | 0 |
| $F197 | systemNameorEngineType.06 | Cxx_obd_ecu_name_6 | 00 | 0 |
| $F197 | systemNameorEngineType.07 | Cxx_obd_ecu_name_7 | 00 | 0 |
| $F197 | systemNameorEngineType.08 | Cxx_obd_ecu_name_8 | 00 | 0 |
| $F197 | systemNameorEngineType.09 | Cxx_obd_ecu_name_9 | 00 | 0 |
| $F197 | systemNameorEngineType.10 | Cxx_obd_ecu_name_10 | 00 | 0 |
| $F197 | systemNameorEngineType.11 | Cxx_obd_ecu_name_11 | 00 | 0 |
| $F197 | systemNameorEngineType.12 | Cxx_obd_ecu_name_12 | 00 | 0 |
| $F197 | systemNameorEngineType.13 | Cxx_obd_ecu_name_13 | 00 | 0 |
| $F197 | systemNameorEngineType.14 | Cxx_obd_ecu_name_14 | 00 | 0 |
| $F197 | systemNameorEngineType.15 | Cxx_obd_ecu_name_15 | 00 | 0 |
| $F197 | systemNameorEngineType.16 | Cxx_obd_ecu_name_16 | 00 | 0 |
| $F197 | systemNameorEngineType.17 | Cxx_obd_ecu_name_17 | 00 | 0 |
| $F197 | systemNameorEngineType.18 | Cxx_obd_ecu_name_18 | 00 | 0 |
| $F197 | systemNameorEngineType.19 | Cxx_obd_ecu_name_19 | 00 | 0 |
| $F197 | systemNameorEngineType.20 | Cxx_obd_ecu_name_20 | 00 | 0 |
| $F1A0 | diagnosticVersion |  | 05 | (nicht dekodiert) |
| $F1A1 | Nissan vehicle Spare Part Number.00 | Vxx_boot_nis_vh_spre_prt_nr_0 | 32 | 50 |
| $F1A1 | Nissan vehicle Spare Part Number.01 | Vxx_boot_nis_vh_spre_prt_nr_1 | 33 | 51 |
| $F1A1 | Nissan vehicle Spare Part Number.02 | Vxx_boot_nis_vh_spre_prt_nr_2 | 37 | 55 |
| $F1A1 | Nissan vehicle Spare Part Number.03 | Vxx_boot_nis_vh_spre_prt_nr_3 | 44 | 68 |
| $F1A1 | Nissan vehicle Spare Part Number.04 | Vxx_boot_nis_vh_spre_prt_nr_4 | 34 | 52 |
| $F1A1 | Nissan vehicle Spare Part Number.05 | Vxx_boot_nis_vh_spre_prt_nr_5 | 30 | 48 |
| $F1A1 | Nissan vehicle Spare Part Number.06 | Vxx_boot_nis_vh_spre_prt_nr_6 | 34 | 52 |
| $F1A1 | Nissan vehicle Spare Part Number.07 | Vxx_boot_nis_vh_spre_prt_nr_7 | 34 | 52 |
| $F1A1 | Nissan vehicle Spare Part Number.08 | Vxx_boot_nis_vh_spre_prt_nr_8 | 34 | 52 |
| $F1A1 | Nissan vehicle Spare Part Number.09 | Vxx_boot_nis_vh_spre_prt_nr_9 | 52 | 82 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.01 | Vtx_conf_file_id[0] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.10 | Vtx_conf_file_id[9] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.09 | Vtx_conf_file_id[8] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.08 | Vtx_conf_file_id[7] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.07 | Vtx_conf_file_id[6] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.06 | Vtx_conf_file_id[5] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.05 | Vtx_conf_file_id[4] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.04 | Vtx_conf_file_id[3] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.03 | Vtx_conf_file_id[2] | 30 | 48 |
| $F1A2 | Configuration file reference information written for Nissan vehicles in plant.02 | Vtx_conf_file_id[1] | 30 | 48 |
| $F800 | DIDs supported in range [$F801 - $F820] |  | 55 43 00 70 | (nicht dekodiert) |
| $F802 | $09 - Vehicle identification number.1 |  | 56 46 31 41 48 30 30 30 32 36 36 38 38 33 31 36 34 | (nicht dekodiert) |
| $F804 | ($F804) $09 - OBD calibration identifications.0 |  | 32 33 37 44 33 30 35 32 30 52 00 00 00 00 00 00 | (Antwort zu kurz) |
| $F804 | ($F804) $09 - OBD calibration identifications.1 |  | 32 33 37 44 33 30 35 32 30 52 00 00 00 00 00 00 | (Antwort zu kurz) |
| $F804 | ($F804) $09 - OBD calibration identifications.2 |  | 32 33 37 44 33 30 35 32 30 52 00 00 00 00 00 00 | (Antwort zu kurz) |
| $F804 | ($F804) $09 - OBD calibration identifications.3 |  | 32 33 37 44 33 30 35 32 30 52 00 00 00 00 00 00 | (Antwort zu kurz) |
| $F804 | ($F804) $09 - OBD calibration identifications.4 |  | 00 00 00 00 | 0 |
| $F804 | ($F804) $09 - OBD calibration identifications.5 |  | 30 52 00 00 | 810680320 |
| $F804 | ($F804) $09 - OBD calibration identifications.6 |  | 33 30 35 32 | 858797362 |
| $F804 | ($F804) $09 - OBD calibration identifications.7 |  | 32 33 37 44 | 842217284 |
| $F80F | $09 - exhaustRegulationorTypeApprovalNumber.01 |  | 00 00 00 00 00 00 00 48 4D 4C 47 54 39 38 36 36 52 | (nicht dekodiert) |
| $F80F | $09 - exhaustRegulationorTypeApprovalNumber.00 |  | 00 00 00 00 00 00 00 48 4D 4C 47 54 39 38 36 36 52 | (nicht dekodiert) |
