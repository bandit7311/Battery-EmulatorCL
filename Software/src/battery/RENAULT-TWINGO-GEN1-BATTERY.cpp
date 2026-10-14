#include "RENAULT-TWINGO-GEN1-BATTERY.h"
#include <stdarg.h>
#include <string.h>
#include "../datalayer/datalayer.h"
#include "../datalayer/datalayer_extended.h"
#include "../devboard/hal/hal.h"
#include "../devboard/utils/common_functions.h"  // crc8_table_SAE_J1850_ZER0
#include "../devboard/utils/events.h"
#include "../devboard/utils/logging.h"
#include "../devboard/webserver/BatteryHtmlRenderer.h"
#include "../devboard/webserver/simulator_html.h"
#ifndef UNIT_TEST
#include <Preferences.h>
#include <WiFi.h>  // WiFi.status() for the NTP start
#endif
#include <time.h>

/* Information in this file is based of the OVMS V3 vehicle_renaultzoe.cpp component 
https://github.com/openvehicles/Open-Vehicle-Monitoring-System-3/blob/master/vehicle/OVMS.V3/components/vehicle_renaultzoe/src/vehicle_renaultzoe.cpp
The Zoe BMS apparently does not send total pack voltage, so we use the polled 96x cellvoltages summed up as total voltage
Still TODO:
- Automatically detect what vehicle and battery size we are on (Zoe 22/41 , Kangoo 33, Fluence ZE 22/36)

 Do not change code below unless you are sure what you are doing */
void RenaultTwingoGen1Battery::
    update_values() {  //This function maps all the values fetched via CAN to the correct parameters used for modbus
  datalayer_battery->status.soh_pptt = (LB_SOH * 100);  // Increase range from 99% -> 99.00%

  datalayer_battery->status.real_soc = (uint16_t)(LB_Display_SOC * 0.25f);  // 0.0025% per bit -> pptt (0.01% units)
  // Alternative: datalayer_battery->status.real_soc = (LB_SOC * 100); // Use raw BMS Chemical SOC% (0x654)

  datalayer_battery->status.current_dA = (((int32_t)LB_Current_raw * 10) / 4) - 5000;

  //Calculate the remaining Wh amount from SOC% and max Wh value.
  datalayer_battery->status.remaining_capacity_Wh = static_cast<uint32_t>(
      (static_cast<double>(datalayer_battery->status.real_soc) / 10000) * datalayer_battery->info.total_capacity_Wh);

  datalayer_battery->status.max_discharge_power_W = LB_Discharge_allowed_W;

  datalayer_battery->status.max_charge_power_W = LB_Regen_allowed_W;

  datalayer_battery->status.temperature_min_dC = LB_Cell_minimum_temperature * 10;
  datalayer_battery->status.temperature_max_dC = LB_Cell_maximum_temperature * 10;

  if (LB_Cell_minimum_voltage < 4400) {  //Value is initialized large for some reason
    datalayer_battery->status.cell_min_voltage_mV = LB_Cell_minimum_voltage;
  }

  if (LB_Cell_maximum_voltage < 4400) {  //Value is initialized large for some reason
    datalayer_battery->status.cell_max_voltage_mV = LB_Cell_maximum_voltage;
  }

#ifdef TWINGO_EXTENDED_CELL_POLLING
  // Single pack temperature sensors (0x9131-0x9138): first drop every sensor that has stopped
  // answering, then - only if all 8 are plausible and fresh - derive pack min/max from them
  // (0.1 degC resolution) instead of the 0x424 broadcast values assigned further up (1 degC
  // resolution). Otherwise the broadcast values simply stay in place.
  {
    unsigned long now = millis();
    uint8_t mask = datalayer_battery->status.temperature_sensors_valid_mask;
    for (uint8_t i = 0; i < EXT_TEMP_SENSOR_COUNT; i++) {
      if ((mask & (1u << i)) && ((now - ext_temp_last_ms[i]) > EXT_TEMP_STALE_MS)) {
        mask &= (uint8_t) ~(1u << i);
      }
    }
    datalayer_battery->status.temperature_sensors_valid_mask = mask;
    if (mask == 0xFF) {
      int16_t t_min = datalayer_battery->status.temperature_sensors_dC[0];
      int16_t t_max = t_min;
      for (uint8_t i = 1; i < EXT_TEMP_SENSOR_COUNT; i++) {
        int16_t t = datalayer_battery->status.temperature_sensors_dC[i];
        if (t < t_min) {
          t_min = t;
        }
        if (t > t_max) {
          t_max = t;
        }
      }
      datalayer_battery->status.temperature_min_dC = t_min;
      datalayer_battery->status.temperature_max_dC = t_max;
    }
  }

  // Once every one of the 96 cells has replied at least once via the extended
  // channel, use the real per-cell values instead of the coarse 0x425-broadcast
  // approximation (10mV resolution, always a multiple of 10) for pack voltage,
  // cell min and cell max. Until then (e.g. the ~20s after boot before the
  // first full poll cycle completes), keep using the broadcast values set
  // above (already assigned to cell_min/max_voltage_mV further up).
  if (ext_cells_seen >= 96) {
    uint32_t summed_mV = 0;
    uint16_t min_mV = datalayer_battery->status.cell_voltages_mV[0];
    uint16_t max_mV = datalayer_battery->status.cell_voltages_mV[0];
    for (uint8_t i = 0; i < datalayer_battery->info.number_of_cells; ++i) {
      uint16_t v = datalayer_battery->status.cell_voltages_mV[i];
      summed_mV += v;
      if (v < min_mV) {
        min_mV = v;
      }
      if (v > max_mV) {
        max_mV = v;
      }
    }
    calculated_total_pack_voltage_mV = summed_mV;
    datalayer_battery->status.cell_min_voltage_mV = min_mV;
    datalayer_battery->status.cell_max_voltage_mV = max_mV;
  } else {
    calculated_total_pack_voltage_mV = ((LB_Cell_minimum_voltage + LB_Cell_maximum_voltage) / 2) * 96;
  }
#else
  calculated_total_pack_voltage_mV = ((LB_Cell_minimum_voltage + LB_Cell_maximum_voltage) / 2) * 96;
#endif
  datalayer_battery->status.voltage_dV = ((calculated_total_pack_voltage_mV / 100));  // mV to dV

#ifdef TWINGO_EXTENDED_CELL_POLLING
  // Sleep run in progress (Sleep / Sleep 0x9281=1 / NVROL reset, from the button press through the whole
  // shutdown sequence, true silence and wake burst - UserRequestNVROLReset is only cleared again by
  // finish_nvrol_silence() once the wake burst has fully completed). While the battery itself is not being
  // polled, current/voltage/SOC above stay frozen at their last polled values (LB_* simply stop updating) -
  // that's correct for voltage and SOC, but current must not keep reporting whatever it happened to be at
  // the moment of the last poll. Force current and both power limits to 0 so the inverter is actively told
  // "neither charge nor discharge" instead of computing from a stale, possibly non-zero current. The CAN
  // frames towards the inverter keep going at their normal rate throughout (see PylonInverter::transmit_can(),
  // which reads these fields unconditionally) - only their content changes here.
  if (UserRequestNVROLReset) {
    datalayer_battery->status.current_dA = 0;
    datalayer_battery->status.max_discharge_power_W = 0;
    datalayer_battery->status.max_charge_power_W = 0;
  }
#endif
}

uint16_t RenaultTwingoGen1Battery::handle_pid(uint16_t pid, uint32_t value, const uint8_t* data, uint16_t length) {
  // Called by the UDS superclass for every successful PID response. `data`
  // points at the raw value bytes, starting right after the echoed local
  // identifier (the response is `61 <local ID> <value...>`).
  switch (pid) {
    case GROUP1_CELLVOLTAGES_1_POLL:  // 0x41, cells 1-62
      if (length >= 124) {
        for (uint8_t cell = 0; cell < 62; cell++) {
          datalayer_battery->status.cell_voltages_mV[cell] = (data[cell * 2] << 8) | data[cell * 2 + 1];
        }
        // Cell 47 measurement is inbetween pack halves. If low, fuse blown
        if (datalayer_battery->status.cell_voltages_mV[47] < 100) {
          set_event(EVENT_BATTERY_FUSE, datalayer_battery->status.cell_voltages_mV[47]);
        } else {
          clear_event(EVENT_BATTERY_FUSE);
        }
      }
      break;
    case GROUP2_CELLVOLTAGES_2_POLL:  // 0x42, cells 63-96
      if (length >= 68) {
        for (uint8_t cell = 0; cell < 34; cell++) {
          datalayer_battery->status.cell_voltages_mV[62 + cell] = (data[cell * 2] << 8) | data[cell * 2 + 1];
        }
      }
      break;
    case GROUP3_METRICS:  // 0x61, mileage + alltime energy
      if (length >= 17) {
        battery_mileage_in_km = (data[11] << 8) | data[12];
        kWh_from_beginning_of_battery_life = (data[15] << 8) | data[16];
      }
      break;
    case GROUP6_BALANCING: {  // 0x07, one bit per cell, LSB first within each byte
      bool any_balancing = false;
      for (uint8_t cell = 0; cell < 96; cell++) {
        if ((cell >> 3) >= length) {
          break;
        }
        bool is_balancing = (data[cell >> 3] >> (cell & 7)) & 0x01;
        datalayer_battery->status.cell_balancing_status[cell] = is_balancing;
        if (is_balancing) {
          any_balancing = true;
        }
      }
      datalayer_battery->status.balancing_status = any_balancing ? BALANCING_STATUS_ACTIVE : BALANCING_STATUS_READY;
      break;
    }
    default:  //Unknown PID, ignore
      break;
  }
  return 0;  //Continue scanning the PID list in order
}

#ifdef TWINGO_EXTENDED_CELL_POLLING
// ---------------------------------------------------------------------------
// Extended-address (0x18DADBF1/0x18DAF1DB) reply handling. See the comment
// block at the top of RENAULT-TWINGO-GEN1-BATTERY.h for what this is and why
// it exists alongside (not instead of) the KWP2000 UDS code above.
// ---------------------------------------------------------------------------

// Single-frame reply (fits in one CAN frame): `data`/`length` point at the
// payload bytes after the echoed SID+PID.
void RenaultTwingoGen1Battery::handle_extended_single_frame(uint16_t pid, const uint8_t* data, uint16_t length) {
  // A DTC Read is in flight and this single-frame reply isn't a normal PID response at all: for a
  // positive ReadDTCInformation reply, byte[1]=SID(0x59), byte[2]=subfunction(0x02), byte[3]=mask(0x09)
  // - which the generic caller already reinterprets as "pid", high byte 0x02, coincidentally never a
  // real PID. 01.10. real captured reply: byte[3] (the DTCStatusAvailabilityMask the ECU reports it
  // supports) came back 0xFF, NOT an echo of the 0x09 we requested - per the UDS 0x19/0x02 spec this
  // byte is the ECU's own supported-status mask, not a request echo, so only the subfunction byte
  // (pid's high byte, 0x02) is checked now, not the full 0x0209 - the exact-match version silently
  // dropped a fully, correctly reassembled reply because of this.
  // For a negative response, byte[1]=0x7F, byte[2]=<requested SID>=0x19, byte[3]=NRC - so pid's high
  // byte is 0x19. Both are handled here instead of falling through to the PID switch below.
  if (dtc_ext_state == DTC_EXT_READ_CMD_SENT) {
    if ((pid >> 8) == 0x19) {
      snprintf(dtc_ext_log_read, sizeof(dtc_ext_log_read), "NEGATIVE SID=0x19 NRC=0x%02X", pid & 0xFF);
      ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // restore poll template
      dtc_ext_state = DTC_EXT_IDLE;
      return;
    }
    if ((pid >> 8) == 0x02) {
      handle_dtc_read_response(data, length);
      return;
    }
  }
  if (pid >= 0x9021 && pid <= 0x9083) {
    // Individual cell voltage. Three offsets (0x9040/0x9060/0x9080) are
    // skipped in the BMS's own PID numbering; account for that. Verified
    // against Battery-Emulator's own RENAULT-ZOE-GEN2-BATTERY.cpp, which
    // polls the same 96 PIDs on the same LBC protocol.
    if (length < 2) {
      return;
    }
    int16_t cell_index = (int16_t)pid - 0x9021;
    if (pid > 0x903F) {
      cell_index -= 1;  // Account for missing 0x9040
    }
    if (pid > 0x905F) {
      cell_index -= 1;  // Account for missing 0x9060
    }
    if (pid > 0x907F) {
      cell_index -= 1;  // Account for missing 0x9080
    }
    if (cell_index < 0 || cell_index >= 96) {
      return;  // Shouldn't happen given the range check above, but be safe.
    }
    uint16_t cell_mV = (uint16_t)(((data[0] << 8) | data[1]) * 0.976563f);
    unsigned long now = millis();
    if (ext_first_cell_reply_ms == 0) {
      ext_first_cell_reply_ms = (now != 0) ? now : 1;  // 0 is reserved for "no reply yet"
    }
    // Boot plausibility filter (see header): only 3.0-4.3V count until all 96 cells have been
    // read once or 60s have passed since the first cell reply. Dropped values are neither
    // stored nor counted; an already stored plausible value stays untouched.
    // The filter is also armed again for 60s after the NVROL quiet phase (BMS wake-up, see finish_nvrol_silence).
    bool boot_phase = ((ext_cells_seen < 96) && ((now - ext_first_cell_reply_ms) < EXT_BOOT_FILTER_TIMEOUT_MS)) ||
                      (ext_filter_rearm_active && ((now - ext_filter_rearm_start_ms) < EXT_BOOT_FILTER_TIMEOUT_MS));
    if (boot_phase && (cell_mV < EXT_BOOT_CELL_MV_MIN || cell_mV > EXT_BOOT_CELL_MV_MAX)) {
#ifdef EXTENDED_UDS_DEBUG
      logging.printf("EXT UDS: boot filter dropped cell %d = %u mV\n", cell_index + 1, (unsigned)cell_mV);
#endif
      return;
    }
    if (datalayer_battery->status.cell_voltages_mV[cell_index] == 0 && ext_cells_seen < 96) {
      ext_cells_seen++;
    }
    datalayer_battery->status.cell_voltages_mV[cell_index] = cell_mV;
    if (datalayer_extended.twingoGen1.cellwatch_enabled &&
        (cell_index + 1) == datalayer_extended.twingoGen1.cellwatch_cell) {
      datalayer_extended.twingoGen1.cellwatch_last_mV = cell_mV;
      datalayer_extended.twingoGen1.cellwatch_sample_count++;
      datalayer_extended.twingoGen1.cellwatch_last_sample_ms = (uint32_t)now;
    }
    return;
  }

  if (pid >= EXT_POLL_TEMP_FIRST && pid < (uint16_t)(EXT_POLL_TEMP_FIRST + EXT_TEMP_SENSOR_COUNT)) {
    // Pack temperature sensor 1-8: 16-bit raw, degC = raw * 0.0625 - 40 (same as OVMS RT32), stored
    // in d°C = raw * 0.625 - 400. Raw 0 (-40.0 degC) and everything above +100.0 degC (e.g. 0xFFFF)
    // is treated as "no valid value" - the sensor then shows "--" and blocks the min/max feed.
    if (length < 2) {
      return;
    }
    uint8_t sensor = (uint8_t)(pid - EXT_POLL_TEMP_FIRST);
    uint16_t raw = (uint16_t)((data[0] << 8) | data[1]);
    int32_t temp_dC = (int32_t)(raw * 0.625f + 0.5f) - 400;
    if (temp_dC > -400 && temp_dC <= 1000) {
      datalayer_battery->status.temperature_sensors_dC[sensor] = (int16_t)temp_dC;
      datalayer_battery->status.temperature_sensors_valid_mask |= (uint8_t)(1u << sensor);
      ext_temp_last_ms[sensor] = millis();
    } else {
      datalayer_battery->status.temperature_sensors_valid_mask &= (uint8_t) ~(1u << sensor);
    }
    return;
  }

  switch (pid) {
    case EXT_POLL_TIME:  // 0x9261 and 0x91C1: shown raw, up to 4 data bytes of a single-frame reply
    case EXT_POLL_PACK_TIME:
      if (length >= 1) {
        uint8_t idx = (pid == EXT_POLL_PACK_TIME) ? 1 : 0;
        uint8_t n = (length > 4) ? 4 : (uint8_t)length;
        uint32_t v = 0;
        for (uint8_t i = 0; i < n; i++) {
          v = (v << 8) | data[i];
        }
        time_pid_raw[idx] = v;
        time_pid_len[idx] = n;
        age_note_pack_read(millis());
      }
      break;
    // Display-only PIDs from the real "RBMS_MCPU_RL" dumps (28.09.): every one of them is stored as the raw
    // big-endian value of however many data bytes the reply carries (1, 2 or 4 here), the formula from the
    // header comment is applied only when rendering the HTML page.
    case EXT_POLL_MILEAGE_PACK:
    case EXT_POLL_MILEAGE_VEHICLE:
    case EXT_POLL_LV_SUPPLY:
    case EXT_POLL_PACK_VOLTAGE:
    case EXT_POLL_CELL_V_A:
    case EXT_POLL_CELL_V_B:
    case EXT_POLL_CELL_V_A_NR:
    case EXT_POLL_CELL_V_B_NR:
    case EXT_POLL_SOH_AVG:
    case EXT_POLL_MAX_CHARGE_POWER:
    case EXT_POLL_MAX_GEN_POWER:
    case EXT_POLL_MAX_AVAIL_POWER:
    case EXT_POLL_SOC_AVG:
    case EXT_POLL_USOC_AVG:
    case EXT_POLL_SOC_MIN:
    case EXT_POLL_SOC_MAX:
    case EXT_POLL_BATTERY_CURRENT:
      if (length >= 1) {
        uint32_t v = 0;
        for (uint8_t i = 0; i < length; i++) {
          v = (v << 8) | data[i];
        }
        ExtValue* target;
        switch (pid) {
          case EXT_POLL_MILEAGE_PACK:
            target = &ext_mileage_pack;
            break;
          case EXT_POLL_MILEAGE_VEHICLE:
            target = &ext_mileage_vehicle;
            break;
          case EXT_POLL_LV_SUPPLY:
            target = &ext_lv_supply;
            break;
          case EXT_POLL_PACK_VOLTAGE:
            target = &ext_pack_voltage;
            break;
          case EXT_POLL_CELL_V_A:
            target = &ext_cell_v_a;
            break;
          case EXT_POLL_CELL_V_B:
            target = &ext_cell_v_b;
            break;
          case EXT_POLL_CELL_V_A_NR:
            target = &ext_cell_v_a_nr;
            break;
          case EXT_POLL_CELL_V_B_NR:
            target = &ext_cell_v_b_nr;
            break;
          case EXT_POLL_SOH_AVG:
            target = &ext_soh_avg;
            break;
          case EXT_POLL_MAX_CHARGE_POWER:
            target = &ext_max_charge_power;
            break;
          case EXT_POLL_MAX_GEN_POWER:
            target = &ext_max_gen_power;
            break;
          case EXT_POLL_MAX_AVAIL_POWER:
            target = &ext_max_avail_power;
            break;
          case EXT_POLL_SOC_AVG:
            target = &ext_soc_avg;
            break;
          case EXT_POLL_USOC_AVG:
            target = &ext_usoc_avg;
            break;
          case EXT_POLL_SOC_MIN:
            target = &ext_soc_min;
            break;
          case EXT_POLL_SOC_MAX:
            target = &ext_soc_max;
            break;
          default:  // EXT_POLL_BATTERY_CURRENT
            target = &ext_battery_current;
            break;
        }
        target->raw = v;
        target->valid = true;
      }
      break;
    case EXT_POLL_CYCLES:  // 0x9210, 16-bit count, no scaling
      if (length >= 2) {
        battery_charge_cycles = (data[0] << 8) | data[1];
      }
      break;
    case EXT_POLL_ENERGY_CHARGED:  // 0x9243, 32-bit, x0.001 kWh
      if (length >= 4) {
        battery_energy_charged_kWh =
            (((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3]) * 0.001f;
      }
      break;
    case EXT_POLL_ENERGY_DISCHARGED:  // 0x9245, 32-bit, x0.001 kWh
      if (length >= 4) {
        battery_energy_discharged_kWh =
            (((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3]) * 0.001f;
      }
      break;
    case EXT_POLL_ENERGY_REGENERATED:  // 0x9247, 32-bit, x0.001 kWh
      if (length >= 4) {
        battery_energy_regenerated_kWh =
            (((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3]) * 0.001f;
      }
      break;
    case EXT_POLL_TEMPORISATION:  // 0x9281, top bit of byte 0 (mirrors Zoe Gen2's own reading of this PID)
      if (length >= 1) {
        // Raw byte is kept: the flag is bit 0 per the CanZE field list (bit 31 of the frame, 0 = temporisation
        // active, 1 = deactivated), not the top bit that the Zoe Ph2 driver evaluates.
        battery_temporisation = data[0];
        priority_answered(1);
      }
      break;
    case EXT_POLL_BAL_CAP_TOTAL:   // 0x924F-0x9252 and 0x9262/0x9263: balancing counters, raw 32-bit
    case EXT_POLL_BAL_TIME_TOTAL:  // (see bal_counter_value). 0x9262/0x9263 aren't contiguous with the
    case EXT_POLL_BAL_CAP_SLEEP:   // rest, so their array index is set explicitly instead of by offset.
    case EXT_POLL_BAL_TIME_SLEEP:
    case EXT_POLL_BAL_CAP_WAKE:
    case EXT_POLL_BAL_TIME_WAKE:
      if (length >= 4) {
        uint8_t idx;
        if (pid == EXT_POLL_BAL_CAP_WAKE) {
          idx = 4;
        } else if (pid == EXT_POLL_BAL_TIME_WAKE) {
          idx = 5;
        } else {
          idx = (uint8_t)(pid - EXT_POLL_BAL_CAP_TOTAL);
        }
        bal_raw[idx] =
            ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
        bal_valid[idx] = true;
        if (pid == EXT_POLL_BAL_CAP_SLEEP) {
          priority_answered(2);
        } else if (pid == EXT_POLL_BAL_TIME_SLEEP) {
          priority_answered(3);
        }
      }
      break;
    default:  // Unknown/unrequested PID, ignore
      break;
  }
}

// Called once a full multi-frame ISO-TP reassembly has completed.
//
// Byte layout confirmed two ways: (1) it matches Battery-Emulator's own
// RENAULT-ZOE-GEN2-BATTERY.cpp POLL_BALANCE_SWITCHES handling exactly -
// cells 0-31 from the LAST 4 bytes of the 3rd continuation frame, cells
// 32-87 from ALL 7 bytes of the 4th continuation frame, cells 88-95 from
// the FIRST byte of the 5th continuation frame, MSB-first within each byte.
// (2) applying it to a real captured 0x912B response from this exact
// battery produced a stable, structured, repeatable (non-zero, non-random)
// 96-bit pattern across 5 independent polls 2 minutes apart - our original
// guess (first 12 bytes from the start of the payload, LSB-first) always
// read all-zero on this battery because that part of the payload genuinely
// is unused padding; the real per-cell bits sit further in.
//
// Index order: Zoe Gen2's own driver documents (and corrects for) the LBC
// delivering these 96 bits in REVERSE cell order (scan position 0 = cell
// 96, scan position 95 = cell 1), while cell_voltages_mV[]/the rest of the
// datalayer is ordered cell 1->96. We apply the same (95 - scan_index)
// flip here. No bit-VALUE inversion - raw bit 1 means "balancing", matching
// Zoe Gen2's own unmodified convention. (An earlier version of this
// function instead inverted the bit value without reversing the index;
// that was the wrong fix for what looked like backwards data - reverting
// it here now that the actual cause is confirmed against Zoe Gen2's code.)
void RenaultTwingoGen1Battery::handle_extended_multiframe_complete() {
#ifdef EXTENDED_UDS_DEBUG
  logging.printf("EXT UDS RX reassembled (%u bytes): ", ext_isotp_received_len);
  for (uint16_t i = 0; i < ext_isotp_received_len; i++) {
    logging.printf("%02X ", ext_isotp_buffer[i]);
  }
  logging.println();
#endif
  if (ext_isotp_received_len < 3) {
    return;  // Not even enough for an echoed SID+PID
  }
  uint16_t pid = (ext_isotp_buffer[1] << 8) | ext_isotp_buffer[2];
  if ((pid >> 8) == 0x02 && dtc_ext_state == DTC_EXT_READ_CMD_SENT) {
    // Positive ReadDTCInformation reply that needed multiple frames (more than ~1 DTC worth of data) -
    // buffer[0]=SID(0x59), [1]=subfunction(0x02), [2]=DTCStatusAvailabilityMask, [3..]=DTC entries
    // (3-byte code + 1-byte status each). This is the case actually confirmed on this battery (01.10.
    // capture): First Frame + our Flow Control + 3 Consecutive Frames reassembled cleanly to the full
    // 27 bytes the First Frame announced - but buffer[2] (the ECU's reported supported-status mask) came
    // back 0xFF, not an echo of the 0x09 we requested, so only the subfunction byte is checked here now,
    // not the full previous pid==0x0209 (that exact-match version silently dropped this exact reply).
    handle_dtc_read_response(&ext_isotp_buffer[3], (uint16_t)(ext_isotp_received_len - 3));
    return;
  }
  if (pid == EXT_POLL_BMS_STATE) {
    // 0x9270: 32 bytes after SID+PID (35 bytes in total)
    if (ext_isotp_received_len >= 3 + 32) {
      memcpy(bms_state_raw, &ext_isotp_buffer[3], sizeof(bms_state_raw));
      bms_state_valid = true;
      priority_answered(0);
    }
    return;
  }
  if (pid != EXT_POLL_BALANCE_SWITCHES) {
    return;  // Nothing else expected to arrive as multi-frame
  }

  uint16_t payload_len = ext_isotp_received_len - 3;  // Bytes available after SID+PID
  if (payload_len < 32) {
    // The bits we actually need go up to payload byte 32 (buffer[34]) - see
    // the byte-layout comment above. Don't trust a partial reassembly,
    // flag it instead of silently showing stale/wrong data.
    datalayer_battery->status.balancing_status = BALANCING_STATUS_ERROR;
    return;
  }

  bool any_balancing = false;
  // Scan positions 0-31: last 4 bytes of the 3rd continuation frame -> buffer[23..26].
  // Scan position i holds the bit for physical cell (96 - i), i.e. array index (95 - i).
  for (uint8_t i = 0; i < 32; i++) {
    bool balancing = (ext_isotp_buffer[23 + (i >> 3)] >> (7 - (i & 7))) & 0x01;
    datalayer_battery->status.cell_balancing_status[95 - i] = balancing;
    if (balancing) {
      any_balancing = true;
    }
  }
  // Scan positions 32-87: all 7 bytes of the 4th continuation frame -> buffer[27..33]
  for (uint8_t i = 32; i < 88; i++) {
    uint8_t j = i - 32;
    bool balancing = (ext_isotp_buffer[27 + (j >> 3)] >> (7 - (j & 7))) & 0x01;
    datalayer_battery->status.cell_balancing_status[95 - i] = balancing;
    if (balancing) {
      any_balancing = true;
    }
  }
  // Scan positions 88-95: first byte of the 5th continuation frame -> buffer[34]
  for (uint8_t i = 88; i < 96; i++) {
    bool balancing = (ext_isotp_buffer[34] >> (7 - (i - 88))) & 0x01;
    datalayer_battery->status.cell_balancing_status[95 - i] = balancing;
    if (balancing) {
      any_balancing = true;
    }
  }
  datalayer_battery->status.balancing_status = any_balancing ? BALANCING_STATUS_ACTIVE : BALANCING_STATUS_READY;
}

// Fills a DTC list from the repeated 4-byte entries (3-byte code + 1-byte status) of a ReadDTCInformation reply;
// `reported` = number of entries the reply held (may exceed the slots of the list).
static void fill_dtc_list(DATALAYER_BATTERY_DTC_TYPE& list, const uint8_t* data, uint16_t len, uint16_t reported) {
  int count = len / 4;
  if (count > list.MAX_DTC_COUNT) {
    count = list.MAX_DTC_COUNT;
  }
  if (count < 0) {
    count = 0;
  }
  for (int i = 0; i < count; i++) {
    uint16_t offset = (uint16_t)(i * 4);
    if ((uint16_t)(offset + 3) >= len) {
      break;
    }
    list.dtc_codes[i] = ((uint32_t)data[offset] << 16) | ((uint32_t)data[offset + 1] << 8) | (uint32_t)data[offset + 2];
    list.dtc_status[i] = data[offset + 3];
  }
  list.dtc_count = (uint8_t)count;
  list.dtc_reported_count = reported;
  list.dtc_read_failed = false;
  list.dtc_last_read_millis = millis();
}

// Dispatches an incoming 0x18DAF1DB frame: single-frame, ISO-TP First Frame
// (sends the Flow Control reply and starts reassembly), or ISO-TP
// Consecutive Frame (continues reassembly, dispatches once complete).
void RenaultTwingoGen1Battery::handle_extended_reply(CAN_frame rx_frame) {
  datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;

  if (dtc_ext_state == DTC_EXT_USER_CMD_SENT) {
    // Free read request: own collector with a 256-byte buffer (03.10.). Only the
    // reply to our request is taken; any other frame continues below like a normal frame.
    if (handle_user_query_reply(rx_frame)) {
      return;
    }
  }

  uint8_t pci = rx_frame.data.u8[0];

  if (pci < 0x10) {
    // Single frame: pci is the payload length (echoed SID + PID + data).
    if (pci < 3) {
      return;  // Too short to even contain SID+PID
    }
#ifdef EXTENDED_UDS_DEBUG
    if (rx_frame.data.u8[1] == 0x7F) {
      // Negative response: 03 7F <requested SID> <NRC>
      logging.printf("EXT UDS: negative response, requested SID=0x%02X NRC=0x%02X\n", rx_frame.data.u8[2],
                     rx_frame.data.u8[3]);
    } else {
      logging.printf("EXT UDS RX single-frame: PID=0x%02X%02X data=", rx_frame.data.u8[2], rx_frame.data.u8[3]);
      for (uint8_t i = 4; i < (uint8_t)(pci + 1); i++) {
        logging.printf("%02X ", rx_frame.data.u8[i]);
      }
      logging.println();
    }
#endif
    uint16_t pid = (rx_frame.data.u8[2] << 8) | rx_frame.data.u8[3];
    handle_extended_single_frame(pid, &rx_frame.data.u8[4], (uint16_t)(pci - 3));
    ext_isotp_in_progress = false;  // A single-frame reply closes out any pending wait
    return;
  }

  if ((pci & 0xF0) == 0x10) {
    // First frame of a multi-frame response.
    ext_isotp_expected_len = (uint16_t)((pci & 0x0F) << 8) | rx_frame.data.u8[1];
#ifdef EXTENDED_UDS_DEBUG
    logging.printf("EXT UDS RX first-frame: expected_len=%u (buffer=%u)\n", ext_isotp_expected_len,
                   (unsigned)sizeof(ext_isotp_buffer));
#endif
    if (ext_isotp_expected_len == 0 || ext_isotp_expected_len > sizeof(ext_isotp_buffer)) {
#ifdef EXTENDED_UDS_DEBUG
      logging.printf("EXT UDS: reassembly abandoned, %u bytes wouldn't fit %u-byte buffer\n", ext_isotp_expected_len,
                     (unsigned)sizeof(ext_isotp_buffer));
#endif
      ext_isotp_in_progress = false;  // Wouldn't fit our buffer; give up cleanly rather than overflow it
      return;
    }
    for (uint8_t i = 0; i < 6; i++) {
      ext_isotp_buffer[i] = rx_frame.data.u8[2 + i];
    }
    ext_isotp_received_len = 6;
    ext_isotp_in_progress = true;
    ext_isotp_started_ms = millis();
    transmit_can_frame(&ZOE_POLL_FLOW_CONTROL);
    return;
  }

  if ((pci & 0xF0) == 0x20) {
    // Consecutive frame.
    if (!ext_isotp_in_progress) {
      return;  // Stray CF with no matching First Frame; ignore it
    }
    uint16_t remaining = ext_isotp_expected_len - ext_isotp_received_len;
    uint8_t copy_len = (remaining < 7) ? (uint8_t)remaining : 7;
    for (uint8_t i = 0; i < copy_len; i++) {
      ext_isotp_buffer[ext_isotp_received_len + i] = rx_frame.data.u8[1 + i];
    }
    ext_isotp_received_len += copy_len;
    if (ext_isotp_received_len >= ext_isotp_expected_len) {
      handle_extended_multiframe_complete();
      ext_isotp_in_progress = false;
    }
    return;
  }
}

// Minimal, dedicated response parser used only while UserRequestNVROLReset is
// active. Records what actually came back for nvrol_awaiting_step as a short
// readable string, later shown in get_uds_info_html(). Distinguishes a
// positive response (raw bytes) from a negative one (SID 0x7F + NRC) -
// that distinction is the whole point: it tells us whether e.g. the routine
// or the write is actively rejected, versus never answered at all.
void RenaultTwingoGen1Battery::handle_nvrol_reply(CAN_frame rx_frame) {
  if (NVROLstateMachine == 5) {
    // Quiet phase: only record what the BMS sends on 0x18DAF1DB (e.g. a late answer to routine B009),
    // never answer it - no flow control, nothing is transmitted.
    if (nvrol_silence_reply_total < 0xFFFF) {
      nvrol_silence_reply_total++;
    }
    if (nvrol_silence_reply_count < NVROL_SILENCE_REPLY_MAX) {
      NvrolSilenceReply& r = nvrol_silence_reply[nvrol_silence_reply_count++];
      r.t_ms = (uint16_t)(millis() - nvrol_silence_start_ms);
      for (uint8_t i = 0; i < 8; i++) {
        r.d[i] = rx_frame.data.u8[i];
      }
    }
    return;
  }
  uint8_t step = nvrol_awaiting_step;
  if (step >= NVROL_LOG_STEPS) {
    return;
  }
  uint8_t pci = rx_frame.data.u8[0];
  if (pci >= 0x10) {
    snprintf(nvrol_log[step], sizeof(nvrol_log[step]), "unexpected multi-frame (PCI=0x%02X)", pci);
  } else if (pci < 3) {
    snprintf(nvrol_log[step], sizeof(nvrol_log[step]), "short reply (%u bytes)", pci);
  } else if (rx_frame.data.u8[1] == 0x7F) {
    // Negative response: 03 7F <requested SID> <NRC>
    snprintf(nvrol_log[step], sizeof(nvrol_log[step]), "NEGATIVE SID=0x%02X NRC=0x%02X", rx_frame.data.u8[2],
             rx_frame.data.u8[3]);
  } else {
    snprintf(nvrol_log[step], sizeof(nvrol_log[step]), "OK raw=%02X %02X %02X %02X %02X %02X %02X", rx_frame.data.u8[1],
             rx_frame.data.u8[2], rx_frame.data.u8[3], rx_frame.data.u8[4], rx_frame.data.u8[5], rx_frame.data.u8[6],
             rx_frame.data.u8[7]);
    if (step == 4 && pci >= 4 && rx_frame.data.u8[1] == 0x62 && rx_frame.data.u8[2] == 0x92 &&
        rx_frame.data.u8[3] == 0x81) {
      temporisation_readback = rx_frame.data.u8[4];  // 0x9281 as the BMS holds it right after the write
    }
  }
}

// Sends the two documented write sequences (NVROL reset, then enable
// "temporisation before sleep") over the same extended channel used for
// polling - see the comment block above UserRequestNVROLReset in the header
// for what this is, what it's based on, and what's deliberately left out.
// Timing (100ms/1s/100ms gaps between steps) matches Battery-Emulator's own
// proven Zoe Ph2 sequence exactly; only the final 30s software-sleep step is
// omitted on purpose.
// Enter the shutdown sequence (state 7): 0x350 will start walking C3 -> C2 -> C0 -> 00.
// Configured sleep failsafe window (minutes, "More Battery Info" web UI, persisted to NVM), clamped to
// 1-1440 min (24h). Falls back to SLEEP_MANUAL_FAILSAFE_DEFAULT_MIN if the stored value is 0 or out of
// that range (e.g. before the setting has ever been written).
unsigned long RenaultTwingoGen1Battery::sleep_manual_failsafe_ms(void) {
  uint16_t configured_min = datalayer_extended.twingoGen1.sleep_failsafe_minutes;
  if (configured_min < 1 || configured_min > 1440) {
    configured_min = SLEEP_MANUAL_FAILSAFE_DEFAULT_MIN;
  }
  return (unsigned long)configured_min * 60UL * 1000UL;
}

// Length of the true silence: the duration of the daily sleep if this run was started by it, otherwise the manual
// failsafe window.
unsigned long RenaultTwingoGen1Battery::silence_limit_ms(void) {
  if (daily_sleep_run && datalayer_extended.twingoGen1.daily_sleep_duration_min > 0) {
    return (unsigned long)datalayer_extended.twingoGen1.daily_sleep_duration_min * 60UL * 1000UL;
  }
  return sleep_manual_failsafe_ms();
}

// Once per second: at the configured local time of day (Bau 9) raise the kilometres of 0x426 and start a
// "Sleep 0x9281=1" run on the long way. Needs the checkbox "enable", a set clock and a duration of at least 1 minute. Fires once per minute of
// the day; a run that is still going at that time skips the day.
void RenaultTwingoGen1Battery::daily_sleep_service(void) {
  auto& t = datalayer_extended.twingoGen1;
  uint32_t secs = 0;
  if (!t.daily_sleep_enabled || t.daily_sleep_duration_min == 0 || !get_wall_clock_seconds_of_day(secs)) {
    daily_sleep_fired = false;
    return;
  }
  if (!twingo::daily_sleep_due(t.daily_sleep_enabled, t.daily_sleep_time_min, t.daily_sleep_duration_min, secs)) {
    daily_sleep_fired = false;  // the minute is over: the next day may fire again
    return;
  }
  if (daily_sleep_fired || UserRequestNVROLReset || NVROLstateMachine != 0) {
    return;
  }
  daily_sleep_fired = true;
  uint64_t km = (uint64_t)odo_426_km + t.daily_sleep_km;
  odo_426_km = km > ODO_426_BUS_MAX_KM ? ODO_426_BUS_MAX_KM : (uint32_t)km;
  sim_settings_save();
  daily_sleep_prev_path = t.bus_sleep_path;
  t.bus_sleep_path = twingo_bus::SLEEP_LANG;
  start_nvrol_run(2);
  daily_sleep_run = UserRequestNVROLReset;  // false if the start was refused
  if (!daily_sleep_run) {
    t.bus_sleep_path = daily_sleep_prev_path;
  }
  note_action("daily sleep");
}

void RenaultTwingoGen1Battery::effective_vehicle_id(uint8_t out[4]) {
  const auto& t = datalayer_extended.twingoGen1;
  const bool typed = t.vehicle_id[0] != 0 || t.vehicle_id[1] != 0 || t.vehicle_id[2] != 0 || t.vehicle_id[3] != 0;
  for (uint8_t i = 0; i < 4; i++) {
    out[i] = (t.vehicle_id_custom && typed) ? t.vehicle_id[i] : TWINGO_VEHICLE_ID_DEFAULT[i];
  }
}

void RenaultTwingoGen1Battery::apply_vehicle_id() {
  uint8_t id[4];
  effective_vehicle_id(id);
  for (uint8_t i = 0; i < 4; i++) {
    ZOE_69F_BCM_GATEWAY.data.u8[i] = id[i];
  }
}

void RenaultTwingoGen1Battery::restore_sleep_path_after_run() {
  if (daily_sleep_run || button_sleep_run) {
    datalayer_extended.twingoGen1.bus_sleep_path = daily_sleep_prev_path;  // the run used the long way
    daily_sleep_run = false;
    button_sleep_run = false;
  }
}

bool RenaultTwingoGen1Battery::sleep_button_init() {
  if (esp32hal == nullptr) {
    return false;
  }
  const gpio_num_t pin = esp32hal->SLEEP_BUTTON_PIN();
  if (!esp32hal->alloc_pins("Sleep button", pin)) {
    return false;
  }
  pinMode(pin, INPUT_PULLUP);  // the button connects the pin to GND
  return true;
}

bool RenaultTwingoGen1Battery::sleep_button_raw_pressed() {
  return digitalRead(esp32hal->SLEEP_BUTTON_PIN()) == LOW;
}

// Push button "Sleep" (Bau 12): a long press starts "Sleep 0x9281=1" on the long way, like the button of the page; the
// silence lasts the "Sleep failsafe manual" time. Ignored while any run is going (also in the silence): no waking and no
// second start by the button. Needs the checkbox on the Advanced page.
void RenaultTwingoGen1Battery::sleep_button_service(unsigned long currentMillis) {
  if (!datalayer_extended.twingoGen1.sleep_button_enabled) {
    sleep_button.reset();  // a press that began while the checkbox was off does not count later
    return;
  }
  if (!sleep_button_pin_ready) {
    if (sleep_button_init_failed) {
      return;
    }
    sleep_button_pin_ready = sleep_button_init();
    sleep_button_init_failed = !sleep_button_pin_ready;
    if (!sleep_button_pin_ready) {
      return;
    }
  }
  if (!sleep_button.update(sleep_button_raw_pressed(), (uint32_t)currentMillis)) {
    return;
  }
  if (UserRequestNVROLReset || NVROLstateMachine != 0) {
    return;  // a run is going (shutdown, silence, wake-up): the press is ignored
  }
  auto& t = datalayer_extended.twingoGen1;
  daily_sleep_prev_path = t.bus_sleep_path;
  t.bus_sleep_path = twingo_bus::SLEEP_LANG;
  start_nvrol_run(2);
  button_sleep_run = UserRequestNVROLReset;  // false if the start was refused
  if (!button_sleep_run) {
    t.bus_sleep_path = daily_sleep_prev_path;
  }
  note_action("sleep button");
}

void RenaultTwingoGen1Battery::start_powerdown(void) {
  powerdown_stage = 0;
  powerdown_start_ms = millis();
  powerdown_stage_start_ms = powerdown_start_ms;
  bus_sleep_start_state = bus_state_shown;  // the shutdown walk down starts from here
  NVROLstateMachine = 7;
}

// Enter the wake-up burst (state 8): 0x350 = C0 once, then C3 x10. Also starts the "how fast does the
// BMS come back" timers, so they cover the whole wake-up, not just what happens after it.
void RenaultTwingoGen1Battery::start_wake_burst(void) {
  if (dtc_ext_state != DTC_EXT_IDLE) {  // a read request of the silence (point 8) is dropped when the wake-up begins
    uq_restore_poll_template();
    dtc_ext_state = DTC_EXT_IDLE;
  }
  // True silence ends here (transmission resumes with the first burst frame), so this is when the
  // displayed "silent for" duration should stop counting - not once the burst itself has also finished.
  nvrol_silence_end_ms = millis();
  wake_burst_index = 0;
  wake_burst_last_ms = 0;
  wake_tracking = true;
  wake_start_ms = millis();
  wake_first_rx = -1;
  wake_first_uds = -1;
  wake_priority_done = -1;
  wake_priority_timeout = false;
  bus_restart_sessions();
  NVROLstateMachine = 8;
}

void RenaultTwingoGen1Battery::transmit_reset_nvrol_frames(void) {
  switch (NVROLstateMachine) {
    case 0:
      startTimeNVROL = millis();
      // Values right before the reset, for the before/after comparison on the web page
      nvrol_before.state_valid = bms_state_valid;
      memcpy(nvrol_before.state, bms_state_raw, sizeof(nvrol_before.state));
      for (uint8_t i = 0; i < 4; i++) {
        nvrol_before.bal_raw[i] = bal_raw[i];
        nvrol_before.bal_valid[i] = bal_valid[i];
      }
      nvrol_before.temporisation = battery_temporisation;
      memset(nvrol_silence_rx_per_s, 0, sizeof(nvrol_silence_rx_per_s));
      memset(nvrol_silence_rx_per_10s, 0, sizeof(nvrol_silence_rx_per_10s));
      memset(nvrol_silence_ids, 0, sizeof(nvrol_silence_ids));
      nvrol_silence_id_count = 0;
      nvrol_silence_id_other = 0;
      nvrol_silence_end_ms = 0;
      wake_tracking = false;
      temporisation_readback = 0x100;
      nvrol_silence_rx_total = 0;
      nvrol_silence_last_rx_ms = 0;
      nvrol_silence_last_rx_pre_wake_ms = 0;
      nvrol_silence_reply_count = 0;
      nvrol_silence_reply_total = 0;
      nvrol_silence_done = false;
      for (uint8_t i = 0; i < NVROL_LOG_STEPS; i++) {
        strncpy(nvrol_log[i], "no response", sizeof(nvrol_log[i]) - 1);
        nvrol_log[i][sizeof(nvrol_log[i]) - 1] = '\0';
      }
      nvrol_awaiting_step = 0;
      if (nvrol_mode == 1) {
        // "Sleep": nothing else is sent, go straight into the shutdown sequence (0x350 walking
        // C3 -> C2 -> C0 -> 00), true silence follows once it reaches 00.
        for (uint8_t i = 0; i < NVROL_LOG_STEPS; i++) {
          strncpy(nvrol_log[i], "not sent (Sleep)", sizeof(nvrol_log[i]) - 1);
        }
        ext_isotp_in_progress = false;
        start_powerdown();
        break;
      }
      // NVROL reset, part 1: open diagnostic session (SID 0x10) before RoutineControl B009. Default
      // subfunction 0x03 (Extended) - the session type that was tried so far (B009 answered NEGATIVE,
      // NRC 0x7F serviceNotSupportedInActiveSession, unconfirmed whether Programming (0x02) fixes that).
      // Configurable via "More Battery Info" (datalayer_extended.twingoGen1.nvrol_b009_use_programming_session).
      {
        uint8_t session_sub = datalayer_extended.twingoGen1.nvrol_b009_use_programming_session ? 0x02 : 0x03;
        ZOE_POLL_18DADBF1.data = {0x02, 0x10, session_sub, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
      }
#ifdef EXTENDED_UDS_DEBUG
      logging.println("NVROL: step 0 - open session");
#endif
      NVROLstateMachine = 1;
      break;
    case 1:  // wait 100ms for step 0's response
      if ((millis() - startTimeNVROL) > INTERVAL_100_MS && nvrol_mode == 2) {
        // "Sleep 0x9281=1": no reset routine, straight to the temporisation write (session is already open).
        // The write value is 0x00, not 0x01: a real "RBMS_MCPU_RL" ECU dump (28.09.) labels the live value
        // 0x00 of this PID as "temporisation is activated" - the button name is kept as-is (it predates
        // this finding), only the byte sent has changed.
        strncpy(nvrol_log[1], "skipped (Sleep 0x9281=1)", sizeof(nvrol_log[1]) - 1);
        strncpy(nvrol_log[2], "skipped (Sleep 0x9281=1)", sizeof(nvrol_log[2]) - 1);
        ZOE_POLL_18DADBF1.data = {0x04, 0x2E, 0x92, 0x81, datalayer_extended.twingoGen1.nvrol_temporisation_write_value,
                                  0xAA, 0xAA, 0xAA};
        nvrol_written_value = datalayer_extended.twingoGen1.nvrol_temporisation_write_value;
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        nvrol_awaiting_step = 3;
        startTimeNVROL = millis();
        NVROLstateMachine = 4;
        break;
      }
      if ((millis() - startTimeNVROL) > INTERVAL_100_MS) {
        // NVROL reset, part 2: RoutineControl (SID 0x31) start routine (0x01) 0xB009
        ZOE_POLL_18DADBF1.data = {0x04, 0x31, 0x01, 0xB0, 0x09, 0x00, 0xAA, 0xAA};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        nvrol_awaiting_step = 1;
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: step 1 - start routine B009 (NVROL reset)");
#endif
        startTimeNVROL = millis();
        NVROLstateMachine = 9;
      }
      break;
    case 9:  // wait 500ms, then ask the routine itself for its result (RequestRoutineResults, subfunction 0x03)
      if ((millis() - startTimeNVROL) > 500) {
        ZOE_POLL_18DADBF1.data = {0x04, 0x31, 0x03, 0xB0, 0x09, 0x00, 0xAA, 0xAA};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        nvrol_awaiting_step = 5;
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: step 1b - request routine B009 results");
#endif
        startTimeNVROL = millis();
        NVROLstateMachine = 2;
      }
      break;
    case 2:  // wait 1s for step 1's response
      if ((millis() - startTimeNVROL) > INTERVAL_1_S) {
        // Enable temporisation before sleep, part 1: open extended diagnostic session again
        ZOE_POLL_18DADBF1.data = {0x02, 0x10, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        nvrol_awaiting_step = 2;
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: step 2 - open session again");
#endif
        startTimeNVROL = millis();
        NVROLstateMachine = 3;
      }
      break;
    case 3:  // wait 100ms for step 2's response
      if ((millis() - startTimeNVROL) > INTERVAL_100_MS) {
        // Enable temporisation before sleep, part 2: WriteDataByIdentifier (SID 0x2E) PID 0x9281.
        // Default 0x00 ("temporisation is activated" per a real ECU dump, see the comment above);
        // configurable to 0x01 via "More Battery Info" (datalayer_extended.twingoGen1.nvrol_temporisation_write_value).
        ZOE_POLL_18DADBF1.data = {0x04, 0x2E, 0x92, 0x81, datalayer_extended.twingoGen1.nvrol_temporisation_write_value,
                                  0xAA, 0xAA, 0xAA};
        nvrol_written_value = datalayer_extended.twingoGen1.nvrol_temporisation_write_value;
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        nvrol_awaiting_step = 3;
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: step 3 - write temporisation (configured value)");
#endif
        startTimeNVROL = millis();
        NVROLstateMachine = 4;
      }
      break;
    case 4:  // wait 100ms for step 3's response, then read 0x9281 back
      if ((millis() - startTimeNVROL) > INTERVAL_100_MS) {
        // Read the temporisation flag back right away: does the BMS keep what was just written?
        ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x92, 0x81, 0x00, 0x00, 0x00, 0x00};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        nvrol_awaiting_step = 4;
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: step 4 - read 0x9281 back");
#endif
        startTimeNVROL = millis();
        NVROLstateMachine = 6;
      }
      break;
    case 6:  // wait 150ms for the read-back, then start the shutdown sequence
      if ((millis() - startTimeNVROL) > 150) {
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: sequence complete, shutdown sequence starts");
#endif
        // Restore the poll frame to its normal read template - we're done with the special sequence.
        ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        ext_isotp_in_progress = false;
        start_powerdown();
      }
      break;
    case 7: {  // Shutdown sequence: the stages C3 -> C2 -> C0 -> 00 run as a clock (no 0x350 frame since Bau 13), the own
      // frames keep running (see transmit_can()) until the 00 stage begins, then true silence (state 5) follows.
      unsigned long now = millis();
      static const unsigned long stage_duration_ms[4] = {POWERDOWN_C3_MS, POWERDOWN_C2_MS, POWERDOWN_C0_MS,
                                                         POWERDOWN_00_MS};
      static const unsigned long stage_duration_car_ms[4] = {POWERDOWN_C3_MS_CAR, POWERDOWN_C2_MS_CAR,
                                                             POWERDOWN_C0_MS_CAR, POWERDOWN_00_MS_CAR};
      if (nvrol_wake_request) {
        start_wake_burst();
        break;
      }
      const unsigned long stage_len =
          (powerdown_stage == 3)
              ? twingo_bus::closing_stage_ms(datalayer_extended.twingoGen1.bus_sleep_path, shutdown_like_car)
              : (shutdown_like_car ? stage_duration_car_ms[powerdown_stage] : stage_duration_ms[powerdown_stage]);
      if (now - powerdown_stage_start_ms >= stage_len) {
        if (powerdown_stage + 1 >= 4) {
          // The 00 stage's own duration has elapsed too: stop everything, true silence begins.
          nvrol_silence_start_ms = now;  // nvrol_silence_last_rx_ms stays: the display needs the real last frame
          NVROLstateMachine = 5;
        } else {
          powerdown_stage++;
          powerdown_stage_start_ms = now;
        }
      }
      break;
    }
    case 5:  // true silence: nothing is transmitted at all (see transmit_can()), only the timer runs
      if (nvrol_wake_request || (millis() - nvrol_silence_start_ms) >= silence_limit_ms()) {
#ifdef EXTENDED_UDS_DEBUG
        logging.println("NVROL: true silence over, waking up");
#endif
        start_wake_burst();
      }
      break;
    case 8: {  // Wake-up burst: 0x350 = C0 once, then C3 x10, 200ms apart, before normal polling resumes
      unsigned long now = millis();
      if (shutdown_like_car) {
        // Mode "like the car": the 12-step wake-up of the real vehicle log (WAKE_STEPS_CAR), about 50 ms apart for
        // the first four frames, then every 100 ms.
        unsigned long interval =
            (wake_burst_index < WAKE_FIRST_FRAMES_CAR) ? WAKE_FIRST_FRAMES_INTERVAL_CAR_MS : WAKE_FRAME_INTERVAL_CAR_MS;
        if (wake_burst_index < wake_total_frames_car() && now - wake_burst_last_ms >= interval) {
          wake_burst_last_ms = now;
          wake_burst_index++;  // the step of the wake-up clock (no 0x350 frame since Bau 13)
        }
        if (wake_burst_index >= wake_total_frames_car()) {
          finish_nvrol_silence();
        }
        break;
      }
      if (now - wake_burst_last_ms >= WAKE_BURST_INTERVAL_MS) {
        wake_burst_last_ms = now;
        wake_burst_index++;  // the step of the wake-up clock (no 0x350 frame since Bau 13)
      }
      if (wake_burst_index > WAKE_BURST_COUNT) {
        finish_nvrol_silence();
      }
      break;
    }
    default:  // Something went wrong; reset state machine
      NVROLstateMachine = 0;
      nvrol_mode = 0;
      UserRequestNVROLReset = false;
      restore_sleep_path_after_run();
      break;
  }
}

// End of the quiet phase: from the next transmit_can() call on all frames are sent again, which wakes
// the BMS. It then behaves like after a boot, so the values are marked as unread, the cell plausibility
// filter is armed again and a few PIDs are asked with priority until they have answered.
// Called once the wake-up burst (state 8) has finished: the BMS/vehicle has had its wake signal, normal
// polling resumes from here. wake_tracking/wake_start_ms/wake_first_* were already set by
// start_wake_burst() when the burst began, so the "how fast did it come back" timers cover the burst too.
void RenaultTwingoGen1Battery::finish_nvrol_silence(void) {
  // Upstream's CAN_error_counter (safety.cpp, MAX_CAN_FAILURES=50) is never reset anywhere in
  // production code - once it crosses the threshold, EVENT_CAN_CORRUPTED_WARNING stays "active"
  // forever and its Count display climbs by 1 on every 1Hz update_machineryprotection() pass, even
  // with zero new corrupted frames (confirmed 01.10.: record_silence_frame showed 0 RX for 10+ min
  // while Count kept climbing). Reset it here, right after a successful wake-up resumes normal
  // communication - a fresh polling cycle is the natural "clean slate" point, same spirit as the
  // other per-wake resets below (bms_state_valid, bal_valid, temp filters).
  datalayer_battery->status.CAN_error_counter = 0;
  // Point 7 (09.10.): the events raised by the silence and the wake-up (missing battery, CAN errors, ...) are
  // acknowledged like the "clear all events" button of the events page does, as soon as the polling resumes.
  reset_all_events();
  restore_sleep_path_after_run();
  nvrol_silence_done = true;
  nvrol_last_mode = nvrol_mode;
  nvrol_mode = 0;
  nvrol_wake_request = false;
  bms_state_valid = false;
  for (uint8_t i = 0; i < 4; i++) {
    bal_valid[i] = false;
  }
  battery_temporisation = 0x100;
  ext_filter_rearm_active = true;
  ext_filter_rearm_start_ms = millis();
  temp_boot_filter_active = true;  // the temperature filter of 0x424 is armed again like the cell filter
  temp_boot_first_frame_ms = 0;
  ext425_boot_filter_active = true;  // same re-arm for the 0x425 pack-voltage boot filter
  ext425_boot_first_frame_ms = 0;
  time_pid_len[0] = 0;  // show the time PIDs as "not yet read" until they have been polled again
  time_pid_len[1] = 0;
  ext_isotp_in_progress = false;
  ext_poll_index = 0;
  ext_priority_pending_mask = 0x0F;
  ext_priority_next = 0;
  ext_priority_start_ms = millis();
  previousMillisExtPoll = millis();  // first poll after the restart follows 200ms later
  UserRequestNVROLReset = false;
  NVROLstateMachine = 0;
}

// Vehicle age (0x350 bytes 1-3, 0x523, 0x376), minutes - see the comment at the declaration and twingo::age_auto().
bool RenaultTwingoGen1Battery::shutdown_like_car = false;
bool RenaultTwingoGen1Battery::time_436_follow_age = false;
uint32_t RenaultTwingoGen1Battery::tx_69f_count = 0;
uint64_t RenaultTwingoGen1Battery::sim_mask_saved = 0;
uint64_t RenaultTwingoGen1Battery::sim_mask_saved_hi = 0;
bool RenaultTwingoGen1Battery::sim_mask_saved_valid = false;
bool RenaultTwingoGen1Battery::time_436_active = false;
uint16_t RenaultTwingoGen1Battery::time_436_phase = 0;
uint32_t RenaultTwingoGen1Battery::time_436_value = 0;
uint8_t RenaultTwingoGen1Battery::bus_state_shown = 1;
uint32_t RenaultTwingoGen1Battery::odo_426_km = 6844;  // pack mileage $91CF of this bench pack (10.10.)
uint8_t RenaultTwingoGen1Battery::odo_426_b7 = 0x40;
bool RenaultTwingoGen1Battery::age_manual_active = false;
uint32_t RenaultTwingoGen1Battery::age_manual_start_min = 0;
unsigned long RenaultTwingoGen1Battery::age_manual_set_ms = 0;
bool RenaultTwingoGen1Battery::age_manual_ms_valid = false;
int64_t RenaultTwingoGen1Battery::age_manual_anchor_unix = 0;

void RenaultTwingoGen1Battery::age_load_from_nvm() {
#ifndef UNIT_TEST
  Preferences prefs;
  if (!prefs.begin("batterySettings", true)) {
    return;
  }
  auto& t = datalayer_extended.twingoGen1;
  t.age_pack_value = prefs.getUInt("TWAGEPV2", t.age_pack_value);
  t.age_pack_unix = prefs.getUInt("TWAGEPT2", t.age_pack_unix);
  time_436_follow_age = prefs.getBool("TW436A", false);
  if (prefs.getBool("TWAGEMA", false)) {
    age_manual_active = true;
    age_manual_start_min = prefs.getUInt("TWAGEMV", 0);
    age_manual_anchor_unix = (int64_t)prefs.getUInt("TWAGEMT", 0);
    age_manual_ms_valid = false;  // counts by the clock after a restart
  }
  // settings of the /simulator page (11.10.): the typed values stay over a restart, as they were set
  time_436_active = prefs.getBool("TW436ACT", false);
  time_436_value = prefs.getUInt("TW436VAL", 0);
  if (time_436_value > AGE_MANUAL_MAX) {
    time_436_value = AGE_MANUAL_MAX;
  }
  odo_426_km = prefs.getUInt("TWO426KM", odo_426_km);
  odo_426_b7 = (uint8_t)prefs.getUInt("TWO426B7", odo_426_b7);
  t.bus_state_manual = prefs.getBool("TWBUSMAN", t.bus_state_manual);
  // Bau 18: own vehicle ID of 0x69F
  t.vehicle_id_custom = prefs.getBool("TWVEHIDEN", t.vehicle_id_custom);
  if (prefs.isKey("TWVEHID")) {
    const uint32_t v = prefs.getUInt("TWVEHID", 0);
    t.vehicle_id[0] = (uint8_t)(v >> 24);
    t.vehicle_id[1] = (uint8_t)(v >> 16);
    t.vehicle_id[2] = (uint8_t)(v >> 8);
    t.vehicle_id[3] = (uint8_t)v;
  }
  prefs.end();
#endif
}

void RenaultTwingoGen1Battery::sim_settings_save() {
#ifndef UNIT_TEST
  Preferences prefs;
  if (!prefs.begin("batterySettings", false)) {
    return;
  }
  prefs.putBool("TW436ACT", time_436_active);
  prefs.putUInt("TW436VAL", time_436_value);
  prefs.putUInt("TWO426KM", odo_426_km);
  prefs.putUInt("TWO426B7", odo_426_b7);
  prefs.putBool("TWBUSMAN", datalayer_extended.twingoGen1.bus_state_manual);
  prefs.end();
#endif
}

void RenaultTwingoGen1Battery::age_save_to_nvm() {
#ifndef UNIT_TEST
  Preferences prefs;
  if (!prefs.begin("batterySettings", false)) {
    return;
  }
  auto& t = datalayer_extended.twingoGen1;
  prefs.putUInt("TWAGEPV2", t.age_pack_value);
  prefs.putUInt("TWAGEPT2", t.age_pack_unix);
  prefs.putBool("TW436A", time_436_follow_age);
  prefs.putBool("TWAGEMA", age_manual_active);
  prefs.putUInt("TWAGEMV", age_manual_start_min);
  prefs.putUInt("TWAGEMT", (uint32_t)age_manual_anchor_unix);
  prefs.end();
#endif
}

void RenaultTwingoGen1Battery::age_manual_set(uint32_t minutes, int64_t unix_s) {
  age_manual_start_min = (minutes > AGE_MANUAL_MAX) ? AGE_MANUAL_MAX : minutes;
  age_manual_set_ms = millis();
  age_manual_ms_valid = true;
  age_manual_anchor_unix = (unix_s >= twingo::UNIX_PLAUSIBLE_MIN) ? unix_s : 0;
  age_manual_active = true;
  age_save_to_nvm();
}

const uint16_t RenaultTwingoGen1Battery::RX_CELL_IDS[RenaultTwingoGen1Battery::RX_CELL_ID_COUNT] = {
    0x5F7, 0x5F4, 0x5F2, 0x5F1, 0x5F0, 0x5ED, 0x5EA, 0x5D9, 0x5EC, 0x5D6,
    0x5CC, 0x5CB, 0x5C9, 0x5B7, 0x5B5, 0x5B4, 0x5AC, 0x5AD, 0x5A1, 0x5DD};
RenaultTwingoGen1Battery::RxSeen RenaultTwingoGen1Battery::rx_row[RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT] = {};
RenaultTwingoGen1Battery::RxSeen RenaultTwingoGen1Battery::rx_cell[RenaultTwingoGen1Battery::RX_CELL_ID_COUNT] = {};
uint16_t RenaultTwingoGen1Battery::rx_other_id[RenaultTwingoGen1Battery::RX_OTHER_MAX] = {};
RenaultTwingoGen1Battery::RxSeen RenaultTwingoGen1Battery::rx_other[RenaultTwingoGen1Battery::RX_OTHER_MAX] = {};
uint8_t RenaultTwingoGen1Battery::rx_other_count = 0;
uint32_t RenaultTwingoGen1Battery::rx_t0_ms = 0;
uint32_t RenaultTwingoGen1Battery::rx_action_ms = 0;
char RenaultTwingoGen1Battery::rx_action_text[48] = {0};

static void rx_touch(RenaultTwingoGen1Battery::RxSeen& r, uint32_t now_ms) {
  if (r.count == 0) {
    r.first_ms = now_ms;
  }
  r.last_ms = now_ms;
  if (r.count < 0xFFFFFFFFUL) {
    r.count++;
  }
}

void RenaultTwingoGen1Battery::rx_note(uint32_t id, uint32_t now_ms) {
  for (uint8_t i = 0; i < SIM_SIGNAL_COUNT; i++) {
    if (sim_signals[i].id == id) {
      rx_touch(rx_row[i], now_ms);
      return;
    }
  }
  for (uint8_t i = 0; i < RX_CELL_ID_COUNT; i++) {
    if (RX_CELL_IDS[i] == id) {
      rx_touch(rx_cell[i], now_ms);
      return;
    }
  }
  for (uint8_t i = 0; i < rx_other_count; i++) {
    if (rx_other_id[i] == id) {
      rx_touch(rx_other[i], now_ms);
      return;
    }
  }
  if (rx_other_count < RX_OTHER_MAX) {
    rx_other_id[rx_other_count] = (uint16_t)id;
    rx_other[rx_other_count] = RxSeen{0, 0, 0};
    rx_touch(rx_other[rx_other_count], now_ms);
    rx_other_count++;
  }
}

void RenaultTwingoGen1Battery::rx_reset(uint32_t now_ms) {
  for (uint8_t i = 0; i < SIM_SIGNAL_COUNT; i++) {
    rx_row[i] = RxSeen{0, 0, 0};
  }
  for (uint8_t i = 0; i < RX_CELL_ID_COUNT; i++) {
    rx_cell[i] = RxSeen{0, 0, 0};
  }
  rx_other_count = 0;
  rx_t0_ms = now_ms;
  rx_action_ms = 0;
  rx_action_text[0] = 0;
}

void RenaultTwingoGen1Battery::note_action(const char* text) {
  rx_action_ms = millis();
  strncpy(rx_action_text, text, sizeof(rx_action_text) - 1);
  rx_action_text[sizeof(rx_action_text) - 1] = 0;
}

static void rx_json_entry(String& s, bool& first_entry, long idx, const RenaultTwingoGen1Battery::RxSeen& r,
                          uint32_t t0) {
  if (r.count == 0) {
    return;
  }
  if (!first_entry) {
    s += ",";
  }
  first_entry = false;
  s += "[" + String(idx) + "," + String((long)((int32_t)(r.first_ms - t0))) + "," +
       String((long)((int32_t)(r.last_ms - t0))) + "," + String((unsigned long)r.count) + "]";
}

// Compact JSON for the /simulator page: r = rows (index in sim_signals), c = cell frames (index in RX_CELL_IDS),
// o = other IDs (the ID itself), each [key, first seen, last seen, count] with times in ms relative to T+0;
// u = Unix time of "now" (0 = clock not set).
String RenaultTwingoGen1Battery::rx_status_json(uint32_t now_ms) {
  time_t unix_now = time(nullptr);  // NTP clock for the page (0 as long as it is not set)
  if (unix_now < 1700000000) {
    unix_now = 0;
  }
  String s = "{\"now\":" + String((long)((int32_t)(now_ms - rx_t0_ms))) + ",\"u\":" + String((long)unix_now) +
             ",\"act\":" + String((long)(rx_action_ms == 0 ? -1L : (long)((int32_t)(rx_action_ms - rx_t0_ms)))) +
             ",\"acttxt\":\"";
  char txt[sizeof(rx_action_text)];
  uint8_t n = 0;
  for (const char* c = rx_action_text; *c && n + 1 < sizeof(txt); c++) {
    if (*c != '"' && *c != '\\') {
      txt[n++] = *c;  // no quotes or backslashes inside the JSON string
    }
  }
  txt[n] = 0;
  s += txt;
  s += "\",\"r\":[";
  bool first_entry = true;
  for (uint8_t i = 0; i < SIM_SIGNAL_COUNT; i++) {
    rx_json_entry(s, first_entry, i, rx_row[i], rx_t0_ms);
  }
  s += "],\"c\":[";
  first_entry = true;
  for (uint8_t i = 0; i < RX_CELL_ID_COUNT; i++) {
    rx_json_entry(s, first_entry, i, rx_cell[i], rx_t0_ms);
  }
  s += "],\"o\":[";
  first_entry = true;
  for (uint8_t i = 0; i < rx_other_count; i++) {
    rx_json_entry(s, first_entry, rx_other_id[i], rx_other[i], rx_t0_ms);
  }
  s += "],\"t69\":" + String((unsigned long)tx_69f_count) + ",\"bs\":" + String((int)bus_state_shown) +
       ",\"bm\":" + String(datalayer_extended.twingoGen1.bus_state_manual ? 1 : 0) + "}";
  return s;
}

void RenaultTwingoGen1Battery::sim_all_off() {
  if (!sim_mask_saved_valid) {
    sim_mask_saved = datalayer_extended.twingoGen1.simulator_enabled_mask;
    sim_mask_saved_hi = datalayer_extended.twingoGen1.simulator_enabled_mask_hi;
    sim_mask_saved_valid = true;
  }
  datalayer_extended.twingoGen1.simulator_enabled_mask = 0;
  datalayer_extended.twingoGen1.simulator_enabled_mask_hi = 0;
  rx_reset(millis());  // T+0: only "All rows off" sets the time to zero (decision of the user, 10.10.)
  note_action("all rows off");
}

void RenaultTwingoGen1Battery::sim_restore() {
  if (sim_mask_saved_valid) {
    datalayer_extended.twingoGen1.simulator_enabled_mask = sim_mask_saved;
    datalayer_extended.twingoGen1.simulator_enabled_mask_hi = sim_mask_saved_hi;
    sim_mask_saved_valid = false;
  }
}

void RenaultTwingoGen1Battery::time_436_follow_set(bool on) {
  time_436_follow_age = on;
  if (!on && !time_436_active) {
    ZOE_436_VEHICLE_STATUS.data.u8[1] = 0x14;  // back to 14 00 xx with the minute counter
    ZOE_436_VEHICLE_STATUS.data.u8[2] = (zoe_436_counter >> 8) & 0xFF;
    ZOE_436_VEHICLE_STATUS.data.u8[3] = zoe_436_counter & 0xFF;
  }
  age_save_to_nvm();
}

void RenaultTwingoGen1Battery::time_436_set(uint32_t minutes) {
  time_436_value = minutes > AGE_MANUAL_MAX ? AGE_MANUAL_MAX : minutes;
  time_436_active = true;
  time_436_phase = 0;  // counting by the minute factor starts from the typed value
  ZOE_436_VEHICLE_STATUS.data.u8[1] = (uint8_t)(time_436_value >> 16);
  ZOE_436_VEHICLE_STATUS.data.u8[2] = (uint8_t)(time_436_value >> 8);
  ZOE_436_VEHICLE_STATUS.data.u8[3] = (uint8_t)time_436_value;
  sim_settings_save();
}

void RenaultTwingoGen1Battery::time_436_clear() {
  time_436_active = false;
  ZOE_436_VEHICLE_STATUS.data.u8[1] = 0x14;
  ZOE_436_VEHICLE_STATUS.data.u8[2] = (zoe_436_counter >> 8) & 0xFF;
  ZOE_436_VEHICLE_STATUS.data.u8[3] = zoe_436_counter & 0xFF;
  sim_settings_save();
}

// A new factor must not lower the age that is already being sent: the current value becomes the new start and the
// counting starts from now with the new factor.
void RenaultTwingoGen1Battery::age_set_factor(uint16_t factor) {
  auto& t = datalayer_extended.twingoGen1;
  if (factor < 1) {
    factor = 1;
  }
  uint32_t current = 0;
  const time_t now_t = time(nullptr);  // static function: same source as the web handler of the manual age
  const int64_t now_unix = (int64_t)now_t;
  const bool have_clock = now_unix >= twingo::UNIX_PLAUSIBLE_MIN;
  const unsigned long now_ms = millis();
  bool have_age = false;
  if (age_manual_active) {
    if (have_clock && age_manual_anchor_unix != 0) {
      current = twingo::age_plain(age_manual_start_min, age_manual_anchor_unix, now_unix, t.age_minute_factor);
      have_age = true;
    } else if (age_manual_ms_valid) {
      current = twingo::age_clamp24(
          (uint64_t)age_manual_start_min +
          (uint64_t)(((now_ms - age_manual_set_ms) / 60000UL) / twingo::age_factor_sane(t.age_minute_factor)));
      have_age = true;
    }
    t.age_minute_factor = factor;
    if (have_age) {
      age_manual_set(current, (have_clock && age_manual_anchor_unix != 0) ? now_unix : 0);  // keeps the way of counting
    }
  } else {
    if (have_clock) {
      current = twingo::age_auto(t.age_pack_value, (int64_t)t.age_pack_unix, now_unix, t.age_minute_factor);
      t.age_minute_factor = factor;
      t.age_pack_value = current;
      t.age_pack_unix = (uint32_t)now_unix;
      age_save_to_nvm();
    } else {
      t.age_minute_factor = factor;
    }
  }
}

void RenaultTwingoGen1Battery::age_manual_clear() {
  age_manual_active = false;
  age_save_to_nvm();
}

bool RenaultTwingoGen1Battery::unix_now_valid(int64_t& now_unix) {
  time_t t = 0;
  if (!get_unix_time(t) || (int64_t)t < twingo::UNIX_PLAUSIBLE_MIN) {
    return false;
  }
  now_unix = (int64_t)t;
  return true;
}

// Age as 0x350 bytes 1-3 carry it. Manual override: start value + full minutes (by the clock from the Unix time it
// was typed at, otherwise by run time). Automatic: pack reference carried by the clock plus the safety lead, never
// lower than what was already sent. Without a usable source: false, the caller sends no frame with an age.
bool RenaultTwingoGen1Battery::vehicle_age_available(unsigned long nowMillis, uint32_t& minutes) {
  auto& t = datalayer_extended.twingoGen1;
  int64_t now_unix = 0;
  const bool clock_ok = unix_now_valid(now_unix);
  uint32_t v;
  if (age_manual_active) {
    if (clock_ok && age_manual_anchor_unix != 0) {
      v = twingo::age_plain(age_manual_start_min, age_manual_anchor_unix, now_unix, t.age_minute_factor);
      t.age_source = 3;
    } else if (age_manual_ms_valid) {
      v = twingo::age_clamp24(
          (uint64_t)age_manual_start_min +
          (uint64_t)(((nowMillis - age_manual_set_ms) / 60000UL) / twingo::age_factor_sane(t.age_minute_factor)));
      t.age_source = 3;
    } else {
      return false;
    }
  } else {
    if (!clock_ok) {
      return false;
    }
    v = twingo::age_auto(t.age_pack_value, (int64_t)t.age_pack_unix, now_unix, t.age_minute_factor);
    t.age_source = 1;
  }
  if (v < t.age_last_sent && !age_manual_active) {
    v = t.age_last_sent;  // never backwards in the automatic mode
  }
  t.age_last_sent = v;
  minutes = v;
  return true;
}

// Called after 0x9261 / 0x91C1 were stored: if the pack holds more than we currently send, it becomes the new reference
// (raised once, the safety lead is added again by age_auto). A pack that only stores what we sent never raises it.
void RenaultTwingoGen1Battery::age_note_pack_read(unsigned long nowMillis) {
  if (age_manual_active || !twingo::AGE_RAISE_FROM_PACK) {
    return;  // since 10.10. only the pack time of this bench pack is sent (no raise from the pack reading)
  }
  int64_t now_unix = 0;
  if (!unix_now_valid(now_unix)) {
    return;
  }
  uint32_t sent = 0;
  if (!vehicle_age_available(nowMillis, sent)) {
    return;
  }
  const uint32_t cand = twingo::age_pack_candidate(time_pid_raw[0], time_pid_len[0], time_pid_raw[1], time_pid_len[1]);
  if (twingo::age_should_raise(cand, sent)) {
    auto& t = datalayer_extended.twingoGen1;
    t.age_pack_value = cand;
    t.age_pack_unix = (uint32_t)now_unix;
    t.age_source = 2;
    age_save_to_nvm();
  }
}

// Switch of a row of the /simulator page (bit n of simulator_enabled_mask = sim_signals[n]). The I rows
// (0-9) switch the real senders of this driver, the other rows are handled in send_simulator_signals().
bool RenaultTwingoGen1Battery::sim_enabled(uint8_t row) {
  return sim_row_enabled(row);
}

bool RenaultTwingoGen1Battery::sim_row_enabled(uint8_t row) {
  if (row >= 128) {
    return false;
  }
  if (row >= 64) {
    return ((datalayer_extended.twingoGen1.simulator_enabled_mask_hi >> (row - 64)) & 1ULL) != 0;
  }
  return ((datalayer_extended.twingoGen1.simulator_enabled_mask >> row) & 1ULL) != 0;
}

void RenaultTwingoGen1Battery::sim_row_set(uint8_t row, bool on) {
  if (row >= 128) {
    return;
  }
  uint64_t& word = (row >= 64) ? datalayer_extended.twingoGen1.simulator_enabled_mask_hi
                               : datalayer_extended.twingoGen1.simulator_enabled_mask;
  const uint64_t bit = 1ULL << (row & 63);
  if (on) {
    word |= bit;
  } else {
    word &= ~bit;
  }
}

// Whether the rows of the bus of the simulator table may be sent in the current state: they run through the stages C3,
// C2 and C0 of the shutdown sequence (the 00 stage is send_bus_closing()) and none starts before the first step of
// the wake-up.
bool RenaultTwingoGen1Battery::sim_row_allowed_now() const {
#ifdef TWINGO_EXTENDED_CELL_POLLING
  if (NVROLstateMachine == 7) {
    return powerdown_stage < 3;
  }
  if (NVROLstateMachine == 8 && wake_burst_index == 0) {
    return false;
  }
#endif
  return true;
}

// Wake-up of the real vehicle log of 04.10. (first wake-up, 211.71-235.86 s), mode "like the car": 12 steps with the number of
// frames of each step (0 = the hold step, counted from WAKE_HOLD_C3_MS). Since Bau 13 only the clock of the steps is left (the
// frames of 0x350 are not sent any more); it gives the length of the wake-up (state 8).
const RenaultTwingoGen1Battery::WakeStepCar RenaultTwingoGen1Battery::WAKE_STEPS_CAR[WAKE_STEP_COUNT_CAR] = {
    {2}, {2}, {4}, {7}, {10}, {20}, {0}, {3}, {4}, {4}, {17}, {6}};

static uint16_t wake_step_frames_car(uint8_t count, unsigned long hold_ms, unsigned long interval_ms) {
  return count != 0 ? count : (uint16_t)(hold_ms / interval_ms);
}

uint16_t RenaultTwingoGen1Battery::wake_total_frames_car() {
  uint16_t total = 0;
  for (uint8_t i = 0; i < WAKE_STEP_COUNT_CAR; i++) {
    total += wake_step_frames_car(WAKE_STEPS_CAR[i].count, WAKE_HOLD_C3_MS, WAKE_FRAME_INTERVAL_CAR_MS);
  }
  return total;
}

static const char* bms_state_name(uint8_t v) {
  switch (v) {
    case 1:
      return "Init";
    case 2:
      return "Wait";
    case 3:
      return "Isolated Charge";
    case 4:
      return "Non Isolated Charge";
    case 5:
      return "External Charge";
    case 6:
      return "Driving";
    case 7:
      return "SleepTransient";
    case 8:
      return "DC Charging without RISOL";
    case 9:
      return "DC Charging with RISOL";
    default:
      return "?";
  }
}

static void append_hex_bytes(String& s, const uint8_t* d, uint8_t n) {
  char b[4];
  for (uint8_t i = 0; i < n; i++) {
    snprintf(b, sizeof(b), "%02X", d[i]);
    s += b;
    if (i + 1 < n) {
      s += ' ';
    }
  }
}

// Balancing counters (0x924F-0x9252) are signed 32-bit values with an offset of -2^31 and a factor of
// 1/1024 (CanZE field list): value = (raw XOR 0x80000000) / 1024. The raw value is always shown too.
static float bal_counter_value(uint32_t raw) {
  return (float)((double)(raw ^ 0x80000000u) / 1024.0);
}

// Live BMS state (all 32 bytes, first and last decoded) and the balancing counters.
void RenaultTwingoGen1Battery::append_live_html(String& s) {
  s += "BMS State (0x9270): ";
  if (!bms_state_valid) {
    s += "not yet read";
  } else {
    append_hex_bytes(s, bms_state_raw, 32);
    s += " (first: ";
    s += bms_state_name(bms_state_raw[0]);
    s += ", last: ";
    s += bms_state_name(bms_state_raw[31]);
    s += ")";
  }
  s += "<br>";
  for (uint8_t line = 0; line < 3; line++) {
    uint8_t cap = line * 2;
    uint8_t tim = line * 2 + 1;
    if (line == 0) {
      s += "Balancing total (0x924F/0x9250): ";
    } else if (line == 1) {
      s += "Balancing in sleep (0x9251/0x9252): ";
    } else {
      s += "Balancing while awake (0x9262/0x9263): ";
    }
    if (!bal_valid[cap] || !bal_valid[tim]) {
      s += "not yet read<br>";
      continue;
    }
    s += String(bal_counter_value(bal_raw[cap]), 3);
    s += " Ah / ";
    s += String(bal_counter_value(bal_raw[tim]), 3);
    s += " h (raw ";
    char b[24];
    snprintf(b, sizeof(b), "%08lX / %08lX", (unsigned long)bal_raw[cap], (unsigned long)bal_raw[tim]);
    s += b;
    s += ")<br>";
  }
  for (uint8_t idx = 0; idx < 2; idx++) {
    s += (idx == 0) ? "Time (0x9261): " : "Pack time (0x91C1): ";
    if (time_pid_len[idx] == 0) {
      s += "not yet read<br>";
      continue;
    }
    char b[24];
    snprintf(b, sizeof(b), "%0*lX", (int)(time_pid_len[idx] * 2), (unsigned long)time_pid_raw[idx]);
    s += b;
    s += " (";
    s += String((unsigned long)time_pid_raw[idx]);
    s += ")<br>";
  }
  s += "SOH candidate (0x658 byte 4): ";
  if (soh_658 == 0xFF) {
    s += "not received";
  } else if (soh_658 == 0x7F) {
    s += "invalid (127)";
  } else {
    s += String(soh_658);
    s += " &#37;";  // never a raw percent sign in this page, see append_ext_value()
  }
  s += "<br>";
  append_ext_value(s, "Pack Mileage (0x91CF)", ext_mileage_pack, (ext_mileage_pack.raw ^ 0x80000000u) / 32.0, "km", 2);
  append_ext_value(s, "Vehicle Distance Totalizer (0x925F)", ext_mileage_vehicle, ext_mileage_vehicle.raw * 0.01, "km",
                   2);
  append_ext_value(s, "Low Voltage Supply (0x9011)", ext_lv_supply, ext_lv_supply.raw / 1024.0, "V");
  append_ext_value(s, "Pack Voltage, cell sum (0x9006)", ext_pack_voltage, ext_pack_voltage.raw * 0.976563 / 1000.0,
                   "V");
  append_ext_value(s, "Cell Voltage A (0x9007, min/max not confirmed)", ext_cell_v_a,
                   ext_cell_v_a.raw * 0.976563 / 1000.0, "V");
  append_ext_value(s, "Cell Voltage B (0x9009, min/max not confirmed)", ext_cell_v_b,
                   ext_cell_v_b.raw * 0.976563 / 1000.0, "V");
  append_ext_value(s, "Cell Voltage A index (0x9008)", ext_cell_v_a_nr, (double)ext_cell_v_a_nr.raw, "");
  append_ext_value(s, "Cell Voltage B index (0x900A)", ext_cell_v_b_nr, (double)ext_cell_v_b_nr.raw, "");
  append_ext_value(s, "Battery SOH avg (0x9003)", ext_soh_avg, ext_soh_avg.raw / 100.0, "%");
  append_ext_value(s, "Max Charge Power (0x9018)", ext_max_charge_power, ext_max_charge_power.raw / 100.0, "kW");
  append_ext_value(s, "Max Generated Power (0x900E)", ext_max_gen_power, ext_max_gen_power.raw / 100.0, "kW");
  append_ext_value(s, "Max Available Power (0x900F)", ext_max_avail_power, ext_max_avail_power.raw / 100.0, "kW");
  append_ext_value(s, "Battery SOC, internal (0x9001)", ext_soc_avg, ext_soc_avg.raw * 0.01 - 3.0, "%");
  append_ext_value(s, "Battery USOC, dashboard (0x9002, display only)", ext_usoc_avg, ext_usoc_avg.raw * 0.01, "%");
  append_ext_value(s, "Battery SOC min (0x91B9)", ext_soc_min, ext_soc_min.raw * 0.01 - 3.0, "%");
  append_ext_value(s, "Battery SOC max (0x91BA)", ext_soc_max, ext_soc_max.raw * 0.01 - 3.0, "%");
  {
    const double battery_current_A = ((double)(int32_t)ext_battery_current.raw * 0.025 - 1200.0) * -1.0;
    if (ext_battery_current.valid && !twingo::battery_current_plausible(battery_current_A)) {
      char b[64];
      snprintf(b, sizeof(b), "invalid (raw 0x%08lX)", (unsigned long)ext_battery_current.raw);
      s += "Battery Current (0x900D, display only, see current_dA ToDo): ";
      s += b;
      s += "<br>";
    } else {
      append_ext_value(s, "Battery Current (0x900D, display only, see current_dA ToDo)", ext_battery_current,
                       battery_current_A, "A");
    }
    s += "0x155 frames dropped as invalid (current raw 0xFFF or SOC raw above 40000): ";
    s += String((unsigned long)datalayer_extended.twingoGen1.frame_155_dropped);
    s += "<br>";
  }
}

void RenaultTwingoGen1Battery::append_ext_value(String& s, const char* label, const ExtValue& v, double value,
                                                const char* unit, int decimals) {
  s += label;
  s += ": ";
  if (!v.valid) {
    s += "not yet read<br>";
    return;
  }
  s += String(value, decimals);
  if (unit[0] != '\0') {
    s += " ";
    // A raw '%' must never appear in this page: the web server's template engine takes the text between two '%'
    // for a placeholder and swallows it (03.10.: it ate the NVROL / DTC block with the "Read ALL DTC statuses"
    // checkbox and the DTC table head). "&#37;" is decoded by the browser, the template engine ignores it.
    char escaped[40];  // the units are short ("kW", "km", "%"); built in a buffer, String += char differs per core
    size_t n = 0;
    for (const char* p = unit; *p != '\0' && n + 6 < sizeof(escaped); p++) {
      if (*p == '%') {
        memcpy(&escaped[n], "&#37;", 5);
        n += 5;
      } else {
        escaped[n++] = *p;
      }
    }
    escaped[n] = '\0';
    s += escaped;
  }
  s += "<br>";
}

void RenaultTwingoGen1Battery::priority_answered(uint8_t bit) {
  ext_priority_pending_mask &= (uint8_t) ~(1u << bit);
  if (ext_priority_pending_mask == 0 && wake_tracking && wake_priority_done < 0) {
    wake_priority_done = (int32_t)(millis() - wake_start_ms);
  }
}

void RenaultTwingoGen1Battery::record_silence_frame(const CAN_frame& f) {
  unsigned long now = millis();
  unsigned long dt = now - powerdown_start_ms;  // one continuous timeline from the shutdown sequence's start
  unsigned long sec = dt / 1000;
  if (sec < SILENCE_SEC_BUCKETS && nvrol_silence_rx_per_s[sec] < 0xFFFF) {
    nvrol_silence_rx_per_s[sec]++;
  }
  unsigned long b10 = dt / 10000;
  if (b10 < SILENCE_10S_BUCKETS && nvrol_silence_rx_per_10s[b10] < 0xFFFF) {
    nvrol_silence_rx_per_10s[b10]++;
  }
  nvrol_silence_rx_total++;
  nvrol_silence_last_rx_ms = now;
  if (NVROLstateMachine != 8) {
    nvrol_silence_last_rx_pre_wake_ms = now;  // the verdict "Result" ignores the frames of the wake-up burst
  }

  const uint8_t n = (f.DLC > 8) ? 8 : f.DLC;  // only the bytes the frame really carries
  SilenceId* rec = nullptr;
  for (uint8_t i = 0; i < nvrol_silence_id_count; i++) {
    if (nvrol_silence_ids[i].id == f.ID) {
      rec = &nvrol_silence_ids[i];
      break;
    }
  }
  if (!rec) {
    if (nvrol_silence_id_count >= SILENCE_ID_MAX) {
      nvrol_silence_id_other++;
      return;
    }
    rec = &nvrol_silence_ids[nvrol_silence_id_count++];
    rec->id = f.ID;
    rec->count = 0;
    rec->changes = 0;
    rec->chg_count = 0;
    rec->chg_next = 0;
    memset(rec->first, 0, 8);
    memset(rec->last, 0, 8);
    memcpy(rec->first, f.data.u8, n);
    memcpy(rec->last, f.data.u8, n);
  }
  // Data change: remember the last three (time, new data, which bytes differed)
  uint8_t mask = 0;
  for (uint8_t i = 0; i < n; i++) {
    if (rec->last[i] != f.data.u8[i]) {
      mask |= (uint8_t)(1u << i);
    }
  }
  if (mask != 0) {
    if (rec->changes < 0xFFFF) {
      rec->changes++;
    }
    SilenceChange& ch = rec->chg[rec->chg_next];
    rec->chg_next = (uint8_t)((rec->chg_next + 1) % 3);
    if (rec->chg_count < 3) {
      rec->chg_count++;
    }
    ch.t_ms = (uint32_t)dt;
    ch.mask = mask;
    memset(ch.d, 0, 8);
    memcpy(ch.d, f.data.u8, n);
  }
  memcpy(rec->last, f.data.u8, n);
  rec->dlc = f.DLC;
  rec->last_ms = now;
  rec->count++;
}

// Value of the last 0x9281 write as the label shows it: the byte really sent, not the setting of the page.
static String write_value_text(uint8_t v) {
  if (v == 0xFF) {
    return String("(not written yet)");
  }
  char b[24];
  snprintf(b, sizeof(b), "0x%02X%s", (unsigned)v, v == 0 ? " (activated)" : "");
  return String(b);
}

// Temporisation (0x9281) as received: raw byte plus bit 0 and bit 7, no interpretation.
static String temporisation_text(uint16_t v) {
  if (v >= 0x100) {
    return String("not yet read");
  }
  char b[48];
  snprintf(b, sizeof(b), "raw 0x%02X (bit0=%u, bit7=%u)", (unsigned)v, (unsigned)(v & 1), (unsigned)((v >> 7) & 1));
  return String(b);
}

// Hex bytes; bytes whose bit is set in `mask` are put in [brackets] (the ones that changed).
static void append_hex_marked(String& s, const uint8_t* d, uint8_t n, uint8_t mask) {
  char b[8];
  for (uint8_t i = 0; i < n; i++) {
    snprintf(b, sizeof(b), (mask & (1u << i)) ? "[%02X]" : "%02X", d[i]);
    s += b;
    if (i + 1 < n) {
      s += ' ';
    }
  }
}

static void append_secs(String& s, long ms) {
  char b[24];
  snprintf(b, sizeof(b), "%ld.%ld s", ms / 1000, (ms % 1000) / 100);
  s += b;
}

static void append_mmss(String& s, unsigned long ms) {
  char b[16];
  unsigned long t = ms / 1000;
  snprintf(b, sizeof(b), "%02lu:%02lu", t / 60, t % 60);
  s += b;
}

// Sleep / quiet phase display: timer since nothing is sent, live BMS frame counter, per second and per
// 10 s history, when the BMS went silent, every CAN ID heard (which stops first, what changes), late
// replies and the values from before the run.
void RenaultTwingoGen1Battery::append_quiet_html(String& s) {
  const bool quiet = (NVROLstateMachine == 5);
  const bool active = UserRequestNVROLReset;
  const uint8_t mode = active ? nvrol_mode : nvrol_last_mode;
  const unsigned long now = millis();
  char b[120];

  if (mode == 1) {
    s += "Sleep (nothing sent at all, until Wake up): ";
  } else if (mode == 2) {
    s += "Sleep 0x9281=1 (temporisation written, then nothing sent until Wake up): ";
  } else {
    s += "NVROL reset (nothing sent after the sequence, until Wake up): ";
  }
  if (quiet) {
    unsigned long elapsed = now - nvrol_silence_start_ms;
    s += "<b>SILENT for ";
    append_mmss(s, elapsed);
    s += "</b> (safety limit ";
    append_mmss(s, silence_limit_ms());
    s += ") - nothing is sent, do not cut the 12V supply<br>";
  } else if (NVROLstateMachine == 7) {
    static const char* stage_names[4] = {"C3 (BAT TEMPO LEVEL)", "C2 (CUT OFF PENDING)", "C0 (SLEEPING)", "00"};
    s += "<b>vehicle state ";
    s += stage_names[powerdown_stage];
    s += "</b>, this stage for ";
    append_mmss(s, now - powerdown_stage_start_ms);
    s += ", shutdown running for ";
    append_mmss(s, now - powerdown_start_ms);
    s += " - our own frames still run, do not cut the 12V supply<br>";
  } else if (NVROLstateMachine == 8) {
    snprintf(b, sizeof(b), "<b>waking up</b> - sending burst frame %u/%u<br>", (unsigned)wake_burst_index,
             (unsigned)(WAKE_BURST_COUNT + 1));
    s += b;
  } else if (active) {
    s += "sequence running, silence follows<br>";
  } else if (!nvrol_silence_done) {
    s += "not run yet<br>";
  } else {
    s += "finished, silent for ";
    append_mmss(s, nvrol_silence_end_ms - nvrol_silence_start_ms);
    s += "<br>";
  }
  // During the wake-up burst (state 8) the info from the just-finished silence phase (frame counts,
  // ID table, ...) and the wake-timing line below are still worth showing, even though nvrol_silence_done
  // is only set true once the burst itself has also finished.
  // The recording (buckets/ID table below) now runs the whole way from the shutdown sequence's start
  // (state 7) through true silence (state 5), so it is shown live during state 7 too, not just once the
  // whole run is finished or genuinely silent.
  if (!quiet && !nvrol_silence_done && NVROLstateMachine != 8 && NVROLstateMachine != 7) {
    return;
  }

  const unsigned long duration_ms =
      (quiet || NVROLstateMachine == 7) ? (now - powerdown_start_ms) : (nvrol_silence_end_ms - powerdown_start_ms);

  snprintf(b, sizeof(b), "BMS frames received: <b>%lu</b>", (unsigned long)nvrol_silence_rx_total);
  s += b;
  if (quiet || NVROLstateMachine == 7) {
    if (nvrol_silence_rx_total == 0) {
      s += " - none so far";
    } else {
      unsigned long age = now - nvrol_silence_last_rx_ms;
      snprintf(b, sizeof(b), ", last one %lu.%lu s ago", age / 1000, (age % 1000) / 100);
      s += b;
      if (age > 3000) {
        if (quiet && nvrol_silence_last_rx_ms <= nvrol_silence_start_ms) {
          // The last frame came before the quiet phase began (the BMS stopped during the shutdown sequence).
          const unsigned long before_ms = nvrol_silence_start_ms - nvrol_silence_last_rx_ms;
          snprintf(b, sizeof(b), " - <b>BMS SILENT: last frame %lu.%lu s before the quiet phase began</b>",
                   before_ms / 1000, (before_ms % 1000) / 100);
          s += b;
        } else {
          s += " - <b>BMS SILENT since ";
          // Relative to true silence's own start while quiet; during state 7, true silence has not begun yet, so use
          // the shutdown sequence's start instead - the same base the table below uses.
          append_mmss(s, quiet ? (nvrol_silence_last_rx_ms - nvrol_silence_start_ms)
                               : (nvrol_silence_last_rx_ms - powerdown_start_ms));
          s += "</b>";
        }
      }
    }
  }
  s += "<br>Frames per second (first 60 s): ";
  unsigned long shown = duration_ms / 1000 + 1;
  if (shown > 60) {
    shown = 60;
  }
  for (unsigned long i = 0; i < shown; i++) {
    snprintf(b, sizeof(b), "%u", (unsigned)nvrol_silence_rx_per_s[i]);
    s += b;
    if (i + 1 < shown) {
      s += ',';
      if ((i + 1) % 10 == 0) {
        s += "<br>";  // one group of ten seconds per line (the line of 60 numbers did not wrap)
      }
    }
  }
  s += "<br>";
  if (duration_ms >= 60000) {
    s += "Frames per 10 s (from 01:00): ";
    unsigned long last_bucket = duration_ms / 10000;
    if (last_bucket >= SILENCE_10S_BUCKETS) {
      last_bucket = SILENCE_10S_BUCKETS - 1;
    }
    for (unsigned long i = 6; i <= last_bucket; i++) {
      snprintf(b, sizeof(b), "%u", (unsigned)nvrol_silence_rx_per_10s[i]);
      s += b;
      if (i < last_bucket) {
        s += ',';
        if ((i - 5) % 10 == 0) {
          s += "<br>";  // ten values per line (the line of a long run went over the edge of the page)
        }
      }
    }
    s += "<br>";
  }

  // The "Result" verdict is only meaningful once the run has actually finished (or is genuinely silent):
  // during state 7 the BMS is expected to keep sending normally the whole time (our own frames still run),
  // so there is nothing to conclude yet.
  // Bau 12: the verdict looks only at the frames up to the end of the silence (not at the burst of the wake-up).
  if (!quiet && nvrol_silence_done) {
    if (nvrol_silence_last_rx_pre_wake_ms == 0) {
      s += "Result: no BMS frame at all - it was already silent when the shutdown sequence began.<br>";
    } else if (nvrol_silence_last_rx_pre_wake_ms <= nvrol_silence_start_ms) {
      // The BMS's last frame arrived before true silence even began - it stopped during the shutdown
      // announcement itself (C3/C2/C0/00), most likely right when our own frames stopped at 00.
      unsigned long before_ms = nvrol_silence_start_ms - nvrol_silence_last_rx_pre_wake_ms;
      snprintf(b, sizeof(b),
               "Result: BMS's last frame was %lu.%lu s BEFORE the quiet phase began - it was already silent "
               "during the shutdown announcement (that alone does not prove the pack slept).<br>",
               before_ms / 1000, (before_ms % 1000) / 100);
      s += b;
    } else {
      unsigned long last_ms = nvrol_silence_last_rx_pre_wake_ms - nvrol_silence_start_ms;
      unsigned long silence_duration_ms = nvrol_silence_end_ms - nvrol_silence_start_ms;
      if (last_ms + 3000 < silence_duration_ms) {
        snprintf(b, sizeof(b),
                 "Result: BMS went silent after %lu.%lu s (measured from when the quiet phase began) - it stopped "
                 "sending (that alone does not prove the pack slept).<br>",
                 last_ms / 1000, (last_ms % 1000) / 100);
      } else {
        snprintf(b, sizeof(b), "Result: BMS kept sending until the end - it did not go to sleep.<br>");
      }
      s += b;
    }
  }

  if (wake_tracking) {
    s += "Wake up: first BMS frame after ";
    if (wake_first_rx >= 0) {
      append_secs(s, wake_first_rx);
    } else {
      s += "- (none yet)";
    }
    s += ", first UDS reply after ";
    if (wake_first_uds >= 0) {
      append_secs(s, wake_first_uds);
    } else {
      s += "- (none yet)";
    }
    s += ", priority values ";
    if (wake_priority_done >= 0) {
      s += "complete after ";
      append_secs(s, wake_priority_done);
    } else if (wake_priority_timeout) {
      s += "NOT complete (gave up after 30 s)";
    } else {
      s += "pending";
    }
    s += "<br>";
  }

  if (nvrol_silence_id_count > 0) {
    s += "Frames by CAN ID, earliest silent first (count / data changes / last seen / last data):<br>";
    s += "Changed bytes of IDs with few changes are shown in [brackets], the last three changes per ID:<br>";
    uint8_t order[SILENCE_ID_MAX];
    for (uint8_t i = 0; i < nvrol_silence_id_count; i++) {
      order[i] = i;
    }
    for (uint8_t i = 1; i < nvrol_silence_id_count; i++) {  // insertion sort by last seen, ascending
      uint8_t v = order[i];
      int8_t k = i - 1;
      while (k >= 0 && nvrol_silence_ids[order[k]].last_ms > nvrol_silence_ids[v].last_ms) {
        order[k + 1] = order[k];
        k--;
      }
      order[k + 1] = v;
    }
    for (uint8_t n = 0; n < nvrol_silence_id_count; n++) {
      const SilenceId& r = nvrol_silence_ids[order[n]];
      unsigned long t = r.last_ms - powerdown_start_ms;
      snprintf(b, sizeof(b), "0x%03lX: %lu / %u / T+%lu.%lu s / ", (unsigned long)r.id, (unsigned long)r.count,
               (unsigned)r.changes, t / 1000, (t % 1000) / 100);
      s += b;
      append_hex_bytes(s, r.last, r.dlc > 8 ? 8 : r.dlc);
      s += "<br>";
      if (r.changes > 0 && r.changes <= 10) {  // status-like ID: show what changed and when
        const uint8_t len = r.dlc > 8 ? 8 : r.dlc;
        for (uint8_t k = 0; k < r.chg_count; k++) {
          const SilenceChange& ch = r.chg[(r.chg_count < 3) ? k : (uint8_t)((r.chg_next + k) % 3)];
          snprintf(b, sizeof(b), "&nbsp;&nbsp;change at T+%lu.%lu s: ", (unsigned long)(ch.t_ms / 1000),
                   (unsigned long)((ch.t_ms % 1000) / 100));
          s += b;
          append_hex_marked(s, ch.d, len, ch.mask);
          s += "<br>";
        }
      }
    }
    if (nvrol_silence_id_other > 0) {
      snprintf(b, sizeof(b), "(+ %lu frames of further IDs, table is full)<br>", (unsigned long)nvrol_silence_id_other);
      s += b;
    }
  }

  if (nvrol_silence_reply_total > 0) {
    snprintf(b, sizeof(b), "Replies on 0x18DAF1DB during the quiet phase: %u<br>", (unsigned)nvrol_silence_reply_total);
    s += b;
    for (uint8_t i = 0; i < nvrol_silence_reply_count; i++) {
      snprintf(b, sizeof(b), "t=%u.%u s: ", (unsigned)(nvrol_silence_reply[i].t_ms / 1000),
               (unsigned)((nvrol_silence_reply[i].t_ms % 1000) / 100));
      s += b;
      append_hex_bytes(s, nvrol_silence_reply[i].d, 8);
      s += "<br>";
    }
  }

  s += "Before the run - BMS state: ";
  if (nvrol_before.state_valid) {
    append_hex_bytes(s, nvrol_before.state, 32);
  } else {
    s += "not read";
  }
  s += "<br>Before the run - balancing in sleep: ";
  if (nvrol_before.bal_valid[2] && nvrol_before.bal_valid[3]) {
    s += String(bal_counter_value(nvrol_before.bal_raw[2]), 3);
    s += " Ah / ";
    s += String(bal_counter_value(nvrol_before.bal_raw[3]), 3);
    s += " h";
  } else {
    s += "not read";
  }
  s += "<br>Before the run - temporisation: ";
  s += temporisation_text(nvrol_before.temporisation);
  s += "<br>";
}

// While a reset is running (and until the priority PIDs have answered afterwards) the box refreshes
// itself once per second, so the countdown is visible without reloading the page.
void RenaultTwingoGen1Battery::append_refresh_script_html(String& s, bool busy) {
  if (!busy) {
    return;
  }
  s += "<script>(function(){function tick(){fetch(location.href,{cache:'no-store'})"
       ".then(function(r){return r.text();}).then(function(t){"
       "var d=new DOMParser().parseFromString(t,'text/html');"
       "var nb=d.getElementById('nvrolBox');var cur=document.getElementById('nvrolBox');"
       "if(!nb||!cur){return;}cur.innerHTML=nb.innerHTML;"
       "var a=nb.getAttribute('data-active');cur.setAttribute('data-active',a);"
       "if(a==='1'){setTimeout(tick,1000);}}).catch(function(){setTimeout(tick,2000);});}"
       "setTimeout(tick,1000);})();</script>";
}
#endif

void RenaultTwingoGen1Battery::handle_incoming_can_frame(CAN_frame rx_frame) {
  if (!rx_frame.ext_ID) {
    rx_note(rx_frame.ID, millis());
  }
#ifdef TWINGO_EXTENDED_CELL_POLLING
  if (!rx_frame.ext_ID && datalayer_extended.twingoGen1.cell_source_broadcast) {
    // Cell source "broadcast": the 20 cell frames (5 cells each, 0x5DD one) replace the UDS cell poll. Placeholders
    // of the wake-up (FF.. / 00..0F) decode outside the plausible window and are skipped.
    const int idx = twingo_bus::cell_frame_index((uint16_t)rx_frame.ID);
    if (idx >= 0) {
      uint16_t mv[5];
      const uint8_t n = twingo_bus::decode_cell_frame((uint8_t)idx, rx_frame.data.u8, mv);
      for (uint8_t k = 0; k < n; k++) {
        const uint8_t cell = (uint8_t)(twingo_bus::first_cell((uint8_t)idx) + k);
        if (mv[k] == 0 || cell >= 96) {
          continue;
        }
        if (datalayer_battery->status.cell_voltages_mV[cell] == 0 && ext_cells_seen < 96) {
          ext_cells_seen++;
        }
        datalayer_battery->status.cell_voltages_mV[cell] = mv[k];
      }
    }
  }
  if (NVROLstateMachine == 5 || NVROLstateMachine == 7 || NVROLstateMachine == 8) {
    // Recording runs from the start of the shutdown announcement (state 7: C3/C2/C0/00) all the way
    // through true silence (state 5) AND the wake-up burst (state 8), on one continuous timeline - so
    // we can see what the BMS actually sends right after Wake up (e.g. the first 0x155 frames that
    // feed real_soc), not just whether/when it reacts. Kept as a separate condition from the
    // wake_first_rx tracking below (not else-if) so a frame seen during state 8 still counts for both.
    record_silence_frame(rx_frame);
  }
  if (wake_tracking && wake_first_rx < 0 && NVROLstateMachine != 5 && NVROLstateMachine != 7) {
    wake_first_rx = (int32_t)(millis() - wake_start_ms);  // first frame after Wake up
  }
#endif
  // UDS frames (0x7BB replies) are handled by the superclass.
  if (handle_incoming_uds_can_frame(rx_frame)) {
    return;
  }

  switch (rx_frame.ID) {
    case 0x155: {  //10ms - Charging power, current and SOC - Confirmed sent by: Fluence ZE40, Zoe 22/41kWh, Kangoo 33kWh
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      memcpy(rx_155_last, rx_frame.data.u8, 8);  // raw bytes for the page (also of frames that are dropped below)
      rx_155_seen = true;
      // A frame with the "invalid" markers (current raw 0xFFF or SOC raw above 40000, i.e. 100 %; seen right
      // after a wake-up and with only 0x350 on: SOC raw 0xFFF8 = 163.82 %) is dropped as a whole, the last valid
      // values stay (08.10.).
      const uint16_t current_raw = ((rx_frame.data.u8[1] & 0x0F) << 8) | rx_frame.data.u8[2];
      const uint16_t soc_raw = ((rx_frame.data.u8[4] << 8) | rx_frame.data.u8[5]);
      if (!twingo::frame_155_valid(current_raw, soc_raw)) {
        datalayer_extended.twingoGen1.frame_155_dropped++;
        break;
      }
      LB_Charging_Power_W = rx_frame.data.u8[0] * 300;
      LB_Current_raw = current_raw;
      LB_Display_SOC = soc_raw;
      break;
    }

    case 0x42E:  //NOTE: Not present on 41kWh battery!
      LB_Battery_Voltage = (((((rx_frame.data.u8[3] << 8) | (rx_frame.data.u8[4])) >> 5) & 0x3ff) * 0.5);  //0.5V/bit
      LB_Average_Temperature = (((((rx_frame.data.u8[5] << 8) | (rx_frame.data.u8[6])) >> 5) & 0x7F) - 40);
      break;
    case 0x424:  //100ms - Charge limits, Temperatures, SOH - Confirmed sent by: Fluence ZE40, Zoe 22/41kWh, Kangoo 33kWh
      LB_Heartbeat = rx_frame.data.u8[6];  // Alternates between 0x55 and 0xAA every 500ms (Same as on Nissan LEAF)
      if ((LB_Heartbeat != 0x55) && (LB_Heartbeat != 0xAA)) {
        datalayer_battery->status.CAN_error_counter++;
        break;
      }
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      LB_CUV = (rx_frame.data.u8[0] & 0x03);
      LB_HVBIR = (rx_frame.data.u8[0] & 0x0C) >> 2;
      LB_HVBUV = (rx_frame.data.u8[0] & 0x30) >> 4;
      LB_EOCR = (rx_frame.data.u8[0] & 0xC0) >> 6;
      LB_HVBOC = (rx_frame.data.u8[1] & 0x03);
      LB_HVBOT = (rx_frame.data.u8[1] & 0x0C) >> 2;
      LB_HVBOV = (rx_frame.data.u8[1] & 0x30) >> 4;
      LB_COV = (rx_frame.data.u8[1] & 0xC0) >> 6;
      LB_Regen_allowed_W = rx_frame.data.u8[2] * 500;
      LB_Discharge_allowed_W = rx_frame.data.u8[3] * 500;
      LB_SOH = rx_frame.data.u8[5];
      {
        // Boot plausibility filter for the temperatures (see the comment in the header): while it is active
        // only -20..+60 degC are accepted, it ends with the first plausible frame or 60 s after the first
        // 0x424 frame; from then on every value is taken over unfiltered.
        int16_t t_min = (int16_t)rx_frame.data.u8[4] - 40;
        int16_t t_max = (int16_t)rx_frame.data.u8[7] - 40;
        bool accept_temperatures = true;
        if (temp_boot_filter_active) {
          unsigned long now = millis();
          if (temp_boot_first_frame_ms == 0) {
            temp_boot_first_frame_ms = (now != 0) ? now : 1;  // 0 is reserved for "no frame yet"
          }
          bool plausible = (t_min >= TEMP_BOOT_MIN_C && t_min <= TEMP_BOOT_MAX_C && t_max >= TEMP_BOOT_MIN_C &&
                            t_max <= TEMP_BOOT_MAX_C);
          if (plausible || (now - temp_boot_first_frame_ms) >= TEMP_BOOT_FILTER_TIMEOUT_MS) {
            temp_boot_filter_active = false;
          } else {
            accept_temperatures = false;
          }
        }
        if (accept_temperatures) {
          LB_Cell_minimum_temperature = t_min;
          LB_Cell_maximum_temperature = t_max;
        }
      }
      break;
    case 0x425: {  //100ms Cellvoltages and kWh remaining - Confirmed sent by: Fluence ZE40 & Zoe Gen1
      // Boot plausibility filter (see the comment in the header): while active, only
      // EXT_425_BOOT_MIN_MV..EXT_425_BOOT_MAX_MV is accepted; it ends with the first plausible frame or
      // 60s after the first 0x425 frame, same mechanism as the 0x424 temperature filter above.
      uint16_t candidate_max = (((((rx_frame.data.u8[4] & 0x03) << 7) | (rx_frame.data.u8[5] >> 1)) * 10) + 1000);
      uint16_t candidate_min = (((((rx_frame.data.u8[6] & 0x01) << 8) | rx_frame.data.u8[7]) * 10) + 1000);
      bool accept_425 = true;
      if (ext425_boot_filter_active) {
        unsigned long now = millis();
        if (ext425_boot_first_frame_ms == 0) {
          ext425_boot_first_frame_ms = (now != 0) ? now : 1;  // 0 is reserved for "no frame yet"
        }
        bool plausible = (candidate_min >= EXT_425_BOOT_MIN_MV && candidate_min <= EXT_425_BOOT_MAX_MV &&
                          candidate_max >= EXT_425_BOOT_MIN_MV && candidate_max <= EXT_425_BOOT_MAX_MV);
        if (plausible || (now - ext425_boot_first_frame_ms) >= EXT_425_BOOT_FILTER_TIMEOUT_MS) {
          ext425_boot_filter_active = false;
        } else {
          accept_425 = false;
        }
      }
      if (accept_425) {
        LB_Cell_maximum_voltage = candidate_max;
        LB_Cell_minimum_voltage = candidate_min;
      }
      break;
    }
    case 0x427:  // NOTE: Not present on 41kWh battery!
      LB_kWh_Remaining = (((((rx_frame.data.u8[6] << 8) | (rx_frame.data.u8[7])) >> 6) & 0x3ff) * 0.1);
      break;
    case 0x445:                            //100ms - Confirmed sent by: Fluence ZE40 & Zoe Gen1
      LB_Heartbeat = rx_frame.data.u8[2];  // Alternates between 0x55 and 0xAA every 500ms (Same as on Nissan LEAF)
      if ((LB_Heartbeat != 0x55) && (LB_Heartbeat != 0xAA)) {
        datalayer_battery->status.CAN_error_counter++;
        break;
      }
      datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
      break;
    case 0x654:  //SOC
      LB_SOC = rx_frame.data.u8[3];
      break;
    case 0x658:  // SOH candidate (OVMS: byte 4 & 0x7F, 127 = invalid) - display only, not used for anything else
      if (rx_frame.DLC >= 5) {
        soh_658 = rx_frame.data.u8[4] & 0x7F;
      }
      break;
#ifdef TWINGO_EXTENDED_CELL_POLLING
    case 0x18DAF1DB:  // Extended UDS LBC reply (cell voltages / balancing / lifetime metrics)
      if (wake_tracking && wake_first_uds < 0 && NVROLstateMachine != 5) {
        wake_first_uds = (int32_t)(millis() - wake_start_ms);  // first reply after Wake up
      }
      if (UserRequestNVROLReset && !diag_silence_active()) {
        // While the NVROL sequence is running, responses are Session
        // Control/RoutineControl/WriteDataByIdentifier replies, not
        // ReadDataByIdentifier ones - handle_extended_reply() would
        // misparse them (it assumes a PID sits at bytes 2-3).
        handle_nvrol_reply(rx_frame);
      } else if (dtc_ext_state == DTC_EXT_ERASE_CMD_SENT) {
        // Erase replies are always tiny (positive "54" / negative "7F 14 NRC"), never multi-frame -
        // the simple dedicated handler is enough, no need for the generic reassembly machinery.
        handle_dtc_ext_reply(rx_frame);
      } else {
        // Covers normal PID polls AND DTC Read replies (dtc_ext_state == DTC_EXT_READ_CMD_SENT): a
        // DTC Read reply can be multi-frame (confirmed 01.10. on this battery), so it needs the
        // generic reassembly path's flow-control handling - handle_extended_single_frame() and
        // handle_extended_multiframe_complete() both recognize and redirect it (see their DTC checks).
        handle_extended_reply(rx_frame);
      }
      break;
    case 0x18DAF1DC:  // reply of the safety CPU, only while a free request to DC is running
      if (uq_active_dc && (dtc_ext_state == DTC_EXT_USER_CMD_SENT || dtc_ext_state == DTC_EXT_USER_SESSION_SENT)) {
        handle_extended_reply(rx_frame);
      }
      break;
#endif
    default:
      break;
  }
}

#ifdef TWINGO_TIME_FRAMES
// ---------------------------------------------------------------------------
// Time frames (see the define at the top of the header). The three hooks below are the only places that
// touch WiFi / NTP / the system clock, the unit tests replace them.
// ---------------------------------------------------------------------------
bool RenaultTwingoGen1Battery::network_ready() {
#ifndef UNIT_TEST
  return WiFi.status() == WL_CONNECTED;
#else
  return false;
#endif
}

// Same call and servers as the Dala display firmware: German local time with daylight saving. It only sets the
// time zone and starts the SNTP client, which then keeps synchronising by itself.
void RenaultTwingoGen1Battery::start_ntp() {
#ifndef UNIT_TEST
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
#endif
}

bool RenaultTwingoGen1Battery::get_unix_time(time_t& now_utc) {
  time_t now = time(nullptr);
  if (now < TIME_VALID_AFTER) {
    return false;  // not set yet (the ESP32 starts at 1970)
  }
  now_utc = now;
  return true;
}

bool RenaultTwingoGen1Battery::get_wall_clock_seconds_of_day(uint32_t& secs) {
  time_t now = time(nullptr);
  if (now < TIME_VALID_AFTER) {
    return false;  // not set yet (the ESP32 starts at 1970)
  }
  struct tm t;
  localtime_r(&now, &t);
  secs = (uint32_t)t.tm_hour * 3600UL + (uint32_t)t.tm_min * 60UL + (uint32_t)t.tm_sec;
  return true;
}

// Once per second: remember when the fallback clock started and, as soon as the WiFi station is connected, start
// NTP once (also when the WiFi only comes up after the boot).
void RenaultTwingoGen1Battery::time_service(unsigned long currentMillis) {
  if (currentMillis - previousMillis_time_service < INTERVAL_1_S) {
    return;
  }
  previousMillis_time_service = currentMillis;
  daily_sleep_service();
  if (!ntp_started && network_ready()) {
    start_ntp();
    ntp_started = true;
  }
}

#endif  // TWINGO_TIME_FRAMES

// The 30 /simulator signals: the five Zoe-form rows (tag I, indices 0-4 = SIM_ROW_*; sent for real by transmit_can() /
// send_bus_vehicle_frames(), their checkbox switches exactly that sending, the placeholder bytes in these rows are not
// used) and the 25 frames of the LBC<>EVC bus (tag R). The 41 rows of the vehicle CAN of the old simulator were removed
// with Bau 13 (14.10.). sender = sending ECU where a source exists, info = what the value means - only confirmed facts.
const RenaultTwingoGen1Battery::SimSignal RenaultTwingoGen1Battery::sim_signals[SIM_SIGNAL_COUNT] = {
    // --- 5 frames of the Zoe form (I rows; the old Zoe format or the format of the bus) ---
    {0x19F,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'I',
     false,
     "0x19F (upstream PR #2907)",
     true,
     "Zoe Gen1 only",
     "Zoe Gen1 frame from PR #2907 (labelled PEB inverter), rolling counter in byte 3. The Twingo vehicle never sends "
     "it. Off in the default set; no finding of its own yet.",
     SIM_END_LEGACY},
    {0x426,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'I',
     false,
     "0x426 (upstream PR #2907)",
     true,
     "Zoe Gen1 only",
     "Zoe Gen1 frame from PR #2907 (labelled EVC power mux). The Twingo vehicle never sends it. Finding (bench): bytes "
     "4-6 carry the mileage (km * 256, the pack showed exactly 19,400 km as $925F), and with this frame on the pack "
     "sets E14381, E14281 and 1B0715. Off in the default set; mileage and byte 7 are editable below.",
     SIM_END_LEGACY},
    {0x436,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'I',
     false,
     "0x436 (upstream PR #2907)",
     true,
     "Zoe Gen1 only",
     "Zoe Gen1 frame from PR #2907 (labelled EVC status, runtime clock). The Twingo vehicle never sends it. Finding: "
     "bytes 2-3 are a minute counter (+1 every 60 s since the start of the emulator). Off in the default set.",
     SIM_END_LEGACY},
    {0x423,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'I',
     false,
     "0x423",
     true,
     "Zoe Gen1 only",
     "Zoe Gen1 wake-up frame (code comment: the BMS answers diagnostics only while it receives it). The Twingo vehicle "
     "never sends it. Finding: it wakes the pack (bytes 4 and 6 swap between B2 and 5D every 5 frames). Off in the "
     "default set (0x387); switch it on when a test needs the wake-up.",
     SIM_END_LEGACY},
    {0x69F,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     1000,
     'I',
     false,
     "0x69F (upstream PR #2907)",
     false,
     "BCM (CanZE)",
     "Bytes 1-3 = vehicle ID (LBC $925E). The vehicle log and this driver (since 04.10.): 46 13 88 6F. Before it sent "
     "the Zoe value 71 30 28 2F.",
     SIM_END_BUS},
    // --- 25 frames of the LBC<>EVC bus (R rows) ---
    {0x155,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     true,
     "0x155 bus: power/current/SOC",
     true,
     "Battery (bench log 25.09.)",
     "Charging power (byte 0 x 300 W), current (bytes 1-2, 0.25 A per bit, offset -500 A), SOC (bytes 4-5, "
     "0.0025 percent per bit) - decoded by this driver. The battery sends it on its own (bench log 25.09., "
     "battery alone); sent from here it collides with a real battery. Content per state from the car log.",
     SIM_END_BUS},
    {0x0C5,
     7,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     true,
     "0x0C5 bus",
     true,
     "Battery (bench log 25.09.)",
     "Counter/checksum-like bytes, meaning unknown. The battery sends it on its own (bench log 25.09.). "
     "Content per state from the car log.",
     SIM_END_BUS},
    {0x1C9,
     2,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     true,
     "0x1C9 bus",
     true,
     "Battery (bench log 25.09.)",
     "Two bytes, counter/checksum-like, meaning unknown. The battery sends it on its own (bench log "
     "25.09.). The Renault CAN list has a steering frame with this ID (EPS_R5, 10 ms); whether it is the "
     "same frame is unknown.",
     SIM_END_BUS},
    {0x157,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     false,
     "0x157 bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Byte 1 high nibble = counter (+5 per frame), no checksum found, bytes 0 and 3 vary slowly, meaning "
     "unknown. In the car log of the LBC<>EVC bus.",
     SIM_END_BUS},
    {0x1A1,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     false,
     "0x1A1 bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Measurement-like bytes, no counter, no checksum found, meaning unknown. In the car log of the "
     "LBC<>EVC bus.",
     SIM_END_BUS},
    {0x1C7,
     6,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     false,
     "0x1C7 bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Almost constant per state, meaning unknown. In the car log of the LBC<>EVC bus.",
     SIM_END_BUS},
    {0x0EC,
     3,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     false,
     "0x0EC bus (counter + CRC)",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Byte 1 high nibble = counter (+1 per frame), byte 2 = CRC-8 (poly 0x1D, start 0, final XOR 0xBE) "
     "over bytes 0-1 (100 percent of 57,317 frames of the car log). Meaning unknown. First frame at wake-up in "
     "the car. Wake-up form: the first 6 frames after the row is switched on carry byte 0 = 10, like in the car.",
     SIM_END_BUS},
    {0x0ED,
     3,
     {0, 0, 0, 0, 0, 0, 0, 0},
     10,
     'R',
     false,
     "0x0ED bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Byte 1 changes FF -> CC with the GO switch in the car, meaning otherwise unknown. In the car log of "
     "the LBC<>EVC bus. Wake-up form after the row is switched on: 10 x A3 FF 00, then 21 x A3 FF 80, "
     "then the value of the state.",
     SIM_END_BUS},
    {0x419,
     6,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x419 bus",
     true,
     "unclear (not seen from the battery alone)",
     "Six bytes, values vary with the state, meaning unknown. Whether the EVC or the battery sends it is "
     "unclear.",
     SIM_END_BUS},
    {0x424,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     true,
     "0x424 bus: limits/SOH/temperature",
     true,
     "Battery (bench log 25.09.)",
     "Byte 2 x 500 W regeneration limit, byte 3 x 500 W discharge limit, byte 5 SOH, bytes 4/7 "
     "minimum/maximum temperature (+40), byte 6 heartbeat 0x55/0xAA - decoded by this driver. The battery "
     "sends it on its own. Content per state from the car log (heartbeat stays constant here).",
     SIM_END_BUS},
    {0x425,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     true,
     "0x425 bus: cell min/max",
     true,
     "Battery (bench log 25.09.)",
     "Cell maximum/minimum in bytes 4-7 (10 mV raster, +1000 mV) and more - decoded by this driver. The "
     "battery sends it on its own. Content per state from the car log.",
     SIM_END_BUS},
    {0x428,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x428 bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Constant 00 00 00 00 00 06 00 00 (a different pattern in the first frames after the wake-up), "
     "meaning unknown.",
     SIM_END_BUS},
    {0x42F,
     6,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x42F bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Constant 00 20 FF 80 0F B0 (zeros in the first frames after the wake-up), meaning unknown.",
     SIM_END_BUS},
    {0x435,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x435 bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Changes with the wake-up and the door (byte 2 1D/1E, byte 3 EB/0B, byte 5 FC/00), meaning unknown.",
     SIM_END_BUS},
    {0x43A,
     6,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     true,
     "0x43A bus",
     true,
     "Battery (bench log 25.09.)",
     "Counter/checksum-like, meaning unknown. The battery sends it on its own. Content per state from the "
     "car log.",
     SIM_END_BUS},
    {0x445,
     7,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     true,
     "0x445 bus: heartbeat",
     true,
     "Battery (bench log 25.09.)",
     "Byte 2 alternates 0x55/0xAA (checked by this driver). The battery sends it on its own. Content per "
     "state from the car log (heartbeat stays constant here).",
     SIM_END_BUS},
    {0x464,
     3,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     true,
     "0x464 bus",
     true,
     "Battery (bench log 25.09.)",
     "Counter/checksum-like, meaning unknown. The battery sends it on its own. Content per state from the "
     "car log.",
     SIM_END_BUS},
    {0x4F7,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x4F7 bus",
     true,
     "not sent by the battery alone (bench log 25.09.)",
     "Byte 4 changes with ignition 1 (B0 -> 90), meaning unknown.",
     SIM_END_BUS},
    {0x500,
     5,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x500 bus (ignition on only)",
     false,
     "not sent by the battery alone (bench log 25.09.)",
     "Five bytes, random-looking, no counter, no checksum found; only while the ignition is on (starts "
     "about 15 s after ignition 1 in the car). Meaning unknown.",
     SIM_END_BUS},
    {0x511,
     7,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     false,
     "0x511 bus (six counters)",
     false,
     "not sent by the battery alone (bench log 25.09.)",
     "Byte 0 = 04 at rest and 00 from ignition 3; bytes 1-6 are six counters with a fixed step each (+63, "
     "+201, +185, +107, +13, +227 per frame). The same format is seen on the vehicle CAN. Meaning unknown.",
     SIM_END_BUS},
    {0x588,
     4,
     {0, 0, 0, 0, 0, 0, 0, 0},
     100,
     'R',
     true,
     "0x588 bus",
     true,
     "Battery (bench log 25.09.)",
     "Counter/checksum-like, meaning unknown. The battery sends it on its own. Content per state from the "
     "car log.",
     SIM_END_BUS},
    {0x6BE,
     6,
     {0, 0, 0, 0, 0, 0, 0, 0},
     1000,
     'R',
     true,
     "0x6BE bus",
     true,
     "Battery (bench log 25.09.)",
     "Six bytes, changes with the state, meaning unknown. The battery sends it on its own. Content per "
     "state from the car log.",
     SIM_END_BUS},
    {0x4AE,
     8,
     {0, 0, 0, 0, 0, 0, 0, 0},
     3000,
     'R',
     true,
     "0x4AE bus",
     true,
     "Battery (bench log 25.09.)",
     "Constant after the wake-up (all FF in the first frame), meaning unknown. The battery sends it on its "
     "own.",
     SIM_END_BUS},
    {0x4AF,
     5,
     {0, 0, 0, 0, 0, 0, 0, 0},
     3000,
     'R',
     true,
     "0x4AF bus",
     false,
     "Battery (bench log 25.09.)",
     "Constant after the wake-up (all FF in the first frame), meaning unknown. The battery sends it on its "
     "own.",
     SIM_END_BUS},
    {0x659,
     4,
     {0, 0, 0, 0, 0, 0, 0, 0},
     3000,
     'R',
     true,
     "0x659 bus",
     true,
     "Battery (bench log 25.09.)",
     "Constant after the wake-up (all FF in the first frame), meaning unknown. The battery sends it on its "
     "own.",
     SIM_END_BUS},
};

// Every send place of 0x426/0x435/0x436 goes through here: the moment is noted (micros()) for the measurement of the own
// send intervals, then the frame is sent.
void RenaultTwingoGen1Battery::send_measured(CAN_frame* f) {
  tx_measure.note(f->ID, (uint32_t)micros());
  transmit_can_frame(f);
}

void RenaultTwingoGen1Battery::send_simulator_signals(unsigned long currentMillis) {
  // Only the rows of the LBC<>EVC bus (tag R) are sent here; the five Zoe-form rows (tag I) are sent by
  // transmit_can() / send_bus_vehicle_frames() and switched by their checkbox there. The true-silence rule (none starts
  // before the first wake frame, none runs in the 00 stage) is sim_row_allowed_now().
  for (uint8_t i = 0; i < SIM_SIGNAL_COUNT; i++) {
    if (sim_signals[i].tag != 'R') {
      continue;
    }
    if (!sim_row_enabled(i)) {
      const twingo_bus::FrameDef* off_def = twingo_bus::find_frame((uint16_t)sim_signals[i].id);
      if (off_def != nullptr) {
        bus_seq[off_def - twingo_bus::FRAMES] =
            0;  // switched on again = a new session: 0x0EC / 0x0ED start with the wake-up form
      }
      continue;
    }
    if (!sim_row_allowed_now()) {
      continue;
    }
    if (currentMillis - sim_last_send_ms[i] < sim_signals[i].interval_ms) {
      continue;
    }
    sim_last_send_ms[i] = currentMillis;
    CAN_frame f = {.FD = false, .ext_ID = false, .DLC = sim_signals[i].dlc, .ID = sim_signals[i].id};
    memcpy(f.data.u8, sim_signals[i].data, 8);
    // Frame of the LBC<>EVC bus (10.10.): real format, content per state, counters/CRC where known.
    const twingo_bus::FrameDef* def = twingo_bus::find_frame((uint16_t)sim_signals[i].id);
    if (def == nullptr) {
      continue;
    }
    int32_t o426_unused;
    const uint8_t state = bus_state_now(currentMillis, o426_unused);
    if (!twingo_bus::active_in_state(*def, state)) {
      continue;
    }
    if (def->kind == twingo_bus::K_0EC && bus_0ec_held_back(currentMillis)) {
      continue;  // wake-up through the plug: 0x0EC starts 2.05 s after the first frame
    }
    const uint8_t def_idx = (uint8_t)(def - twingo_bus::FRAMES);
    if (bus_state_seen[def_idx] != state) {
      bus_state_seen[def_idx] = state;
      bus_state_frames[def_idx] = 0;  // the state was entered: 0x0EC starts its driving ramp from the beginning
    }
    twingo_bus::build_frame(*def, state, bus_seq[def_idx]++, 0, odo_426_km, f.data.u8, bus_state_frames[def_idx]++);
    f.DLC = def->dlc;
    send_measured(&f);
  }
}

// The five Zoe-form frames in the format of the real bus (global switch "like the car", default): 0x19F every 10 ms,
// 0x423/0x426/0x436 every 100 ms (0x69F is the same in both formats and is sent by the 1 s block). Content per state
// from twingo_bus::FRAMES, kilometres from the 0x426 field, age (0x436) from the manual 0x436 time or the vehicle
// age; each row is still switched by its own checkbox (rows 3, 6, 4, 5).
void RenaultTwingoGen1Battery::send_bus_vehicle_frames(unsigned long now, bool in_00_stage) {
  if (datalayer_extended.twingoGen1.bus_format_zoe_old) {
    return;
  }
  struct Item {
    uint16_t id;
    uint8_t row;
    uint8_t slot;
  };
  static const Item ITEMS[4] = {
      {0x19F, SIM_ROW_19F, 0}, {0x423, SIM_ROW_423, 1}, {0x426, SIM_ROW_426, 2}, {0x436, SIM_ROW_436, 3}};
  int32_t o426 = -1;
  const uint8_t state = in_00_stage ? (uint8_t)twingo_bus::BUS_ZU : bus_state_now(now, o426);
  for (const Item& it : ITEMS) {
    if (!sim_enabled(it.row)) {
      continue;
    }
    const twingo_bus::FrameDef* def = twingo_bus::find_frame(it.id);
    if (def == nullptr) {
      continue;
    }
    if (in_00_stage && bus_closing_over(it.id, false, now)) {
      continue;  // closing stage: this frame is over (order of the way "Ladelog 2")
    }
    if (now - bus_last_ms[it.slot] < def->interval_ms) {
      continue;
    }
    bus_last_ms[it.slot] = now;
    uint32_t age_min = 0;
    if (def->kind == twingo_bus::K_436) {
      if (time_436_active) {
        age_min = time_436_value;
      } else if (!vehicle_age_available(now, age_min)) {
        continue;  // no age known: no frame
      }
    }
    CAN_frame f = {.FD = false, .ext_ID = false, .DLC = def->dlc, .ID = def->id};
    const size_t idx = def - twingo_bus::FRAMES;
    if (bus_state_seen[idx] != state) {
      const uint8_t prev = bus_state_seen[idx];
      bus_state_seen[idx] = state;
      // 0x19F: the ignition ramp is only run on the way up; coming down from a higher state A0 stays
      bus_state_frames[idx] = (prev != 255 && prev > twingo_bus::BUS_ZUENDUNG1) ? twingo_bus::Z1_19F_RAMP_DONE : 0;
    }
    const uint32_t seq = bus_seq[idx]++;
    if (def->kind == twingo_bus::K_423 && !in_00_stage && !datalayer_extended.twingoGen1.bus_state_manual &&
        seq < twingo_bus::WAKE_423_FRAMES) {
      // Bau 7: the first two 0x423 of every wake-up are 30 7F FF FF FF E0 FF FF and, 50 ms later, 33 7F ...; the regular
      // frames follow about 10 ms after the second one (all three car logs).
      twingo_bus::wake_form_423((uint8_t)seq, f.data.u8);
      if (now >= def->interval_ms) {
        bus_last_ms[it.slot] = now - (def->interval_ms - (seq == 0 ? 50UL : 10UL));
      }
      send_measured(&f);
      continue;
    }
    // 0x423: the two wake-up frames above are not part of the 100 ms pattern (B4/B6 change every 5 frames)
    const uint32_t seq_regular = (def->kind == twingo_bus::K_423 && seq >= twingo_bus::WAKE_423_FRAMES &&
                                  !datalayer_extended.twingoGen1.bus_state_manual)
                                     ? seq - twingo_bus::WAKE_423_FRAMES
                                     : seq;
    twingo_bus::build_frame(*def, state, seq_regular, age_min, odo_426_km, f.data.u8, bus_state_frames[idx]++, o426,
                            odo_426_b7, def->kind == twingo_bus::K_436 ? bus_b0_436(now) : (int16_t)-1);
    if (in_00_stage) {
      twingo_bus::apply_closing_form_at(*def, f.data.u8, (uint32_t)(now - powerdown_stage_start_ms),
                                        datalayer_extended.twingoGen1.bus_sleep_path);
    }
    send_measured(&f);
  }
}

uint8_t RenaultTwingoGen1Battery::bus_state_now(unsigned long now, int32_t& o426) {
  auto& tg = datalayer_extended.twingoGen1;
  o426 = -1;
  uint8_t state = tg.bus_state;
  if (tg.bus_session_restart) {
    tg.bus_session_restart = false;
    bus_session_started = false;  // "manual" switched off: the sequence starts again
  }
  if (!tg.bus_state_manual) {
#ifdef TWINGO_EXTENDED_CELL_POLLING
    if (NVROLstateMachine == 7) {
      state =
          twingo_bus::sleep_state((uint32_t)(now - powerdown_start_ms), bus_sleep_start_state, o426, tg.bus_sleep_path);
    } else
#endif
    {
      if (!bus_session_started) {
        bus_session_started = true;
        bus_session_start_ms = now;
      }
      state = twingo_bus::wake_state((uint32_t)(now - bus_session_start_ms), o426, tg.bus_wake_target);
    }
  }
  if (state >= twingo_bus::BUS_STATE_COUNT) {
    state = twingo_bus::BUS_WACH;
  }
  bus_state_shown = state;
  return state;
}

int16_t RenaultTwingoGen1Battery::bus_b0_436(unsigned long now) {
  const auto& tg = datalayer_extended.twingoGen1;
  if (tg.bus_state_manual) {
    return -1;
  }
#ifdef TWINGO_EXTENDED_CELL_POLLING
  if (NVROLstateMachine == 7) {
    return tg.bus_sleep_path == twingo_bus::SLEEP_LADELOG2
               ? (int16_t)twingo_bus::laden_end_b0((uint32_t)(now - powerdown_start_ms))
               : (int16_t)-1;
  }
#endif
  if (bus_session_started && tg.bus_wake_target == twingo_bus::TARGET_LADEN) {
    return (int16_t)twingo_bus::laden_wake_b0((uint32_t)(now - bus_session_start_ms));
  }
  return -1;
}

bool RenaultTwingoGen1Battery::bus_0ec_held_back(unsigned long now) {
  const auto& tg = datalayer_extended.twingoGen1;
  return !tg.bus_state_manual && bus_session_started && tg.bus_wake_target == twingo_bus::TARGET_LADEN &&
         (now - bus_session_start_ms) < 2050;
}

bool RenaultTwingoGen1Battery::bus_closing_over(uint16_t id, bool is_0ec, unsigned long now) {
#ifdef TWINGO_EXTENDED_CELL_POLLING
  if (NVROLstateMachine == 7 && powerdown_stage >= 3) {
    return (now - powerdown_stage_start_ms) >=
           twingo_bus::closing_end_ms(id, is_0ec, datalayer_extended.twingoGen1.bus_sleep_path);
  }
#endif
  return false;
}

// A new session of the bus (the wake-up after Sleep / NVROL reset): every frame starts again with its wake-up form
// (0x0EC 10 xx, 0x0ED A3 FF xx) and the driving ramp of 0x0EC starts from the beginning.
void RenaultTwingoGen1Battery::bus_restart_sessions() {
  for (uint8_t i = 0; i < twingo_bus::FRAME_COUNT; i++) {
    bus_seq[i] = 0;
    bus_state_frames[i] = 0;
    bus_state_seen[i] = 255;
  }
  bus_session_started = false;  // the state sequence walks from "closed" to "driving" again
}

// The 00 stage of the shutdown sequence on the bus (log 22aaf176: 0x426 goes to "zu", 0x0EC ends with it, 0x0ED is the
// last frame and carries on for about a second): the rows of block 1 that are switched on go on in the state
// "closed", without 0x0EC, and 0x0ED in its closing form. The five Zoe-form rows follow in
// send_bus_vehicle_frames(). After the stage the true silence begins.
void RenaultTwingoGen1Battery::send_bus_closing(unsigned long now) {
  for (uint8_t i = 0; i < SIM_SIGNAL_COUNT; i++) {
    if (sim_signals[i].tag != 'R' || !sim_row_enabled(i)) {
      continue;
    }
    const twingo_bus::FrameDef* def = twingo_bus::find_frame((uint16_t)sim_signals[i].id);
    if (def == nullptr || def->kind == twingo_bus::K_0EC) {
      continue;  // 0x0EC ends when the car closes
    }
    if (bus_closing_over(def->id, false, now)) {
      continue;  // order of the way "Ladelog 2": most frames about 10 s, 0x157 and 0x0ED last
    }
    if (now - sim_last_send_ms[i] < def->interval_ms) {
      continue;
    }
    sim_last_send_ms[i] = now;
    CAN_frame f = {.FD = false, .ext_ID = false, .DLC = def->dlc, .ID = def->id};
    twingo_bus::build_frame(*def, twingo_bus::BUS_ZU, bus_seq[def - twingo_bus::FRAMES]++, 0, odo_426_km, f.data.u8);
    twingo_bus::apply_closing_form_at(*def, f.data.u8, (uint32_t)(now - powerdown_stage_start_ms),
                                      datalayer_extended.twingoGen1.bus_sleep_path);
    send_measured(&f);
  }
}

// The last 0x155 of the battery, byte 7 to byte 0, each in decimal (13.10., for the page).
String RenaultTwingoGen1Battery::rx_155_text() const {
  if (!rx_155_seen) {
    return "0x155 (B7..B0): no frame yet";
  }
  String t = "0x155 (B7..B0):";
  for (int i = 7; i >= 0; i--) {
    t += " ";
    t += String((unsigned)rx_155_last[i]);
  }
  return t;
}

// What the OLED display shows during a Sleep run (13.10.): "Sleep requested" with 00:00:00 while the shutdown sequence runs,
// "Sleep pls wait" with the time of the silence from its start (the end time is the duration of this run: the "Sleep failsafe
// minutes" for the page button and the push button, the duration of the daily sleep for the daily sleep) in the true
// silence; nothing otherwise (the display then writes the events again, which clears the text and the time).
void RenaultTwingoGen1Battery::update_sleep_display(unsigned long now) {
  auto& st = datalayer.system.status;
#ifdef TWINGO_EXTENDED_CELL_POLLING
  if (NVROLstateMachine == 7) {
    st.sleep_display_phase = 1;
    st.sleep_display_elapsed_s = 0;
    st.sleep_display_total_s = (uint32_t)(silence_limit_ms() / 1000UL);
    return;
  }
  if (NVROLstateMachine == 5) {
    st.sleep_display_phase = 2;
    st.sleep_display_elapsed_s = (uint32_t)((now - nvrol_silence_start_ms) / 1000UL);
    st.sleep_display_total_s = (uint32_t)(silence_limit_ms() / 1000UL);
    return;
  }
#else
  (void)now;
#endif
  st.sleep_display_phase = 0;
  st.sleep_display_elapsed_s = 0;
  st.sleep_display_total_s = 0;
}

void RenaultTwingoGen1Battery::transmit_can(unsigned long currentMillis) {
  update_sleep_display(currentMillis);
  sleep_button_service(currentMillis);
  {
    int32_t o426_tick;
    bus_state_now(currentMillis, o426_tick);  // keeps the state sequence and the page display running
  }
#ifdef TWINGO_TIME_FRAMES
  time_service(currentMillis);  // NTP start / fallback clock, transmits nothing itself
#endif
#ifdef TWINGO_EXTENDED_CELL_POLLING
  // NVROL/Sleep sequences: transmit nothing extra of our own (no legacy UDS) while a run is starting or
  // truly silent. Only the timer runs in true silence; frames from the BMS are still received and counted.
  if (UserRequestNVROLReset && nvrol_mode == 1 && NVROLstateMachine == 0) {
    // "Sleep": go straight into the shutdown sequence before anything else can be transmitted.
    transmit_reset_nvrol_frames();
  }
  if (NVROLstateMachine == 5) {
    // True silence lasts until Wake up is pressed: on purpose the "battery alive" watchdog must not trip.
    datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
    transmit_reset_nvrol_frames();
    if (diag_in_silence && dtc_ext_state != DTC_EXT_IDLE) {
      handle_dtc_ext(currentMillis);  // a read request started in the silence (point 8) runs on
    }
    return;
  }
#endif

#ifdef TWINGO_EXTENDED_CELL_POLLING
  // During the shutdown sequence's final "00" stage the vehicle has effectively gone silent too - stop our
  // own broadcast frames here already, not just once true silence (state 5) begins right after. In the mode
  // "like the car" (shutdown_like_car) the rows that run until the end of the bus (SIM_END_BUS) and 0x69F still run in
  // this stage; the 100 ms frames, 0x53B and the fast frames are off there, see below.
  const bool in_00_stage = (NVROLstateMachine == 7 && powerdown_stage >= 3);
  const bool suppress_own_broadcast = in_00_stage && !shutdown_like_car;
#else
  const bool in_00_stage = false;
  const bool suppress_own_broadcast = false;
#endif
  if (in_00_stage) {
    datalayer_battery->status.CAN_battery_still_alive = CAN_STILL_ALIVE;
    send_bus_closing(currentMillis);               // the bus closes: 0x426 = zu, 0x0EC ends, 0x0ED goes on
    send_bus_vehicle_frames(currentMillis, true);  // 0x19F/0x423/0x426/0x436 in the state "closed"
    // 0x69F goes on until its end in the closing stage (10.5 s on the way "Ladelog 2", 50.4 s on the long and short way),
    // also when the other own frames stop here.
    if (suppress_own_broadcast && sim_enabled(SIM_ROW_69F) && !bus_closing_over(0x69F, false, currentMillis) &&
        currentMillis - previousMillis1000_69f >= INTERVAL_1_S) {
      previousMillis1000_69f = currentMillis;
      transmit_can_frame(&ZOE_69F_BCM_GATEWAY);
      tx_69f_count++;
    }
  }
  if (!suppress_own_broadcast) {
    // Send 100ms CAN Message (the BMS only answers diagnostic requests while it
    // receives this wakeup frame)
    if (!in_00_stage && currentMillis - previousMillis100 >= INTERVAL_100_MS) {
      previousMillis100 = currentMillis;
      // The four Zoe frames below (423/19F/426/436) never occur in the real Twingo vehicle log; each one has
      // its own /simulator checkbox (rows 6/3/4/5), counters keep running while a frame is switched off.
      const bool zoe_old = datalayer_extended.twingoGen1.bus_format_zoe_old;  // else send_bus_vehicle_frames()
      if (zoe_old && sim_enabled(SIM_ROW_423)) {
        transmit_can_frame(&ZOE_423);
      }

      if ((counter_423 / 5) % 2 == 0) {  // Alternate every 5 messages between these two
        ZOE_423.data.u8[4] = 0xB2;
        ZOE_423.data.u8[6] = 0xB2;
      } else {
        ZOE_423.data.u8[4] = 0x5D;
        ZOE_423.data.u8[6] = 0x5D;
      }
      counter_423 = (counter_423 + 1) % 10;

      // Broadcast 100ms vehicle frames (PEB Inverter 0x19F, EVC Power Mux 0x426, EVC Status 0x436)
      // Rolling 4-bit sequence counter (cycles 0-15)
      ZOE_19F_INVERTER.data.u8[3] = (zoe_19F_counter++ & 0x0F);
      if (zoe_old && sim_enabled(SIM_ROW_19F)) {
        transmit_can_frame(&ZOE_19F_INVERTER);
      }
      if (zoe_old && sim_enabled(SIM_ROW_426)) {
        const uint32_t km256 = (odo_426_km > ODO_426_MAX_KM ? ODO_426_MAX_KM : odo_426_km) * 256UL;
        ZOE_426_POWER_MUX.data.u8[4] = (uint8_t)(km256 >> 16);
        ZOE_426_POWER_MUX.data.u8[5] = (uint8_t)(km256 >> 8);
        ZOE_426_POWER_MUX.data.u8[6] = (uint8_t)km256;
        ZOE_426_POWER_MUX.data.u8[7] = odo_426_b7;
        send_measured(&ZOE_426_POWER_MUX);
      }
      if (zoe_old && sim_enabled(SIM_ROW_436)) {
        bool send_436 = true;
        if (time_436_follow_age && !time_436_active) {
          uint32_t age_min = 0;
          if (vehicle_age_available(currentMillis, age_min)) {
            ZOE_436_VEHICLE_STATUS.data.u8[1] = (uint8_t)(age_min >> 16);
            ZOE_436_VEHICLE_STATUS.data.u8[2] = (uint8_t)(age_min >> 8);
            ZOE_436_VEHICLE_STATUS.data.u8[3] = (uint8_t)age_min;
          } else {
            send_436 = false;  // no age known: no frame
          }
        }
        if (send_436) {
          send_measured(&ZOE_436_VEHICLE_STATUS);
        }
      }
    }

    if (!in_00_stage) {
      send_bus_vehicle_frames(currentMillis, false);  // 0x19F/0x423/0x426/0x436 in the format of the real bus
    }

    // Update EVC 0x436 vehicle runtime clock every 60s
    if (currentMillis - previousMillis60000_436 >= INTERVAL_60_S) {
      previousMillis60000_436 = currentMillis;
      zoe_436_counter++;
      if (time_436_active) {
        // Bau 11: the typed time of 0x436 counts +1 only every N minutes (minute factor of the age)
        if (++time_436_phase >= twingo::age_factor_sane(datalayer_extended.twingoGen1.age_minute_factor)) {
          time_436_phase = 0;
          if (time_436_value < AGE_MANUAL_MAX) {
            time_436_value++;
          }
        }
        ZOE_436_VEHICLE_STATUS.data.u8[1] = (uint8_t)(time_436_value >> 16);
        ZOE_436_VEHICLE_STATUS.data.u8[2] = (uint8_t)(time_436_value >> 8);
        ZOE_436_VEHICLE_STATUS.data.u8[3] = (uint8_t)time_436_value;
      } else {
        ZOE_436_VEHICLE_STATUS.data.u8[2] = (zoe_436_counter >> 8) & 0xFF;
        ZOE_436_VEHICLE_STATUS.data.u8[3] = zoe_436_counter & 0xFF;
      }
    }

    // Broadcast 1000ms BCM Gateway alive token
    if (currentMillis - previousMillis1000_69f >= INTERVAL_1_S) {
      previousMillis1000_69f = currentMillis;
      if (sim_enabled(SIM_ROW_69F) &&
          !(in_00_stage && bus_closing_over(0x69F, false, currentMillis))) {  // /simulator row 0x69F
        transmit_can_frame(&ZOE_69F_BCM_GATEWAY);
        tx_69f_count++;
      }
    }

    send_simulator_signals(
        currentMillis);  // the frames of the LBC<>EVC bus whose row is switched on (SIM_ON_* / the page)
  }  // !suppress_own_broadcast

#ifdef TWINGO_EXTENDED_CELL_POLLING
  if (UserRequestNVROLReset) {
    // NVROL reset / Sleep / shutdown sequence / wake burst in progress: run its state machine instead of
    // normal extended polling, since both share the ZOE_POLL_18DADBF1 frame object below.
    transmit_reset_nvrol_frames();
  } else {
    handle_dtc_ext(currentMillis);  // DTC Read/Erase probe (see read_DTC()/reset_DTC()), if one is running
    // Extended-address polling: cycle through the 119 poll targets (96 cell
    // voltages + balancing + 4 lifetime metrics + temporisation + 8 pack
    // temperatures + BMS state + 6 balancing counters + 2 time PIDs), one every 200ms (same
    // cadence as Battery-Emulator's own Zoe Ph2 driver) -> ~23.8s per full cycle.
    // Right after the NVROL quiet phase a few PIDs are asked first, see ext_priority_list.
    //
    // Paused entirely (not just Cellwatch) while a DTC exchange is in flight (dtc_ext_state != IDLE):
    // 01.10. bug found - round-robin/Cellwatch kept sending their own unrelated requests on the same
    // CAN ID while a DTC multi-frame reply was still being reassembled. A normal single-frame poll
    // response in between Consecutive Frames would set ext_isotp_in_progress = false (see
    // handle_extended_reply), silently aborting the DTC reassembly already in progress - matches a
    // real-world "no response" result that followed a successful "unexpected multi-frame" one. Same
    // "pausieren, nicht verzahnen" principle as Cellwatch, just gated on dtc_ext_state too now.
    if (dtc_ext_state != DTC_EXT_IDLE) {
      // Nothing to do here - handle_dtc_ext() above drives the whole exchange by itself.
    } else if (datalayer_extended.twingoGen1.cellwatch_enabled) {
      // Cellwatch active: the normal round-robin is fully paused - previousMillisExtPoll/ext_poll_index
      // are simply never touched here, so it resumes from exactly where it left off once Cellwatch is
      // turned off again (Variante 1: pausieren, nicht verzahnen). Only the selected cell is requested,
      // back-to-back, limited only by CELLWATCH_MIN_GAP_MS.
      if (currentMillis - previousMillisCellwatch >= CELLWATCH_MIN_GAP_MS) {
        previousMillisCellwatch = currentMillis;
        uint8_t cell = datalayer_extended.twingoGen1.cellwatch_cell;
        if (cell < 1) {
          cell = 1;
        }
        if (cell > 96) {
          cell = 96;
        }
        uint16_t pid = ext_poll_list[cell - 1];  // cell 1..96 -> the first 96 entries, in order
        // Bytes 0/1 (PCI/SID) set explicitly every time, not assumed left over from the previous send -
        // ZOE_POLL_18DADBF1 is a shared frame object; DTC Read/Erase (or any future sequence) can leave
        // it on a different SID (see handle_dtc_ext's bug fix comment), which would otherwise silently
        // corrupt every poll after it.
        ZOE_POLL_18DADBF1.data.u8[0] = 0x03;
        ZOE_POLL_18DADBF1.data.u8[1] = 0x22;
        ZOE_POLL_18DADBF1.data.u8[2] = (uint8_t)((pid >> 8) & 0xFF);
        ZOE_POLL_18DADBF1.data.u8[3] = (uint8_t)(pid & 0xFF);
        transmit_can_frame(&ZOE_POLL_18DADBF1);
#ifdef EXTENDED_UDS_DEBUG
        logging.printf("EXT UDS TX (cellwatch): PID=0x%04X\n", pid);
#endif
      }
    } else if (currentMillis - previousMillisExtPoll >= EXT_POLL_INTERVAL_MS) {
      previousMillisExtPoll = currentMillis;
      uint16_t pid;
      if (ext_priority_pending_mask != 0 && (currentMillis - ext_priority_start_ms) < EXT_PRIORITY_TIMEOUT_MS) {
        // After the quiet phase: ask the priority PIDs that have not answered yet, round-robin.
        uint8_t tries = 0;
        while (!(ext_priority_pending_mask & (1u << ext_priority_next)) && tries < EXT_PRIORITY_COUNT) {
          ext_priority_next = (ext_priority_next + 1) % EXT_PRIORITY_COUNT;
          tries++;
        }
        pid = ext_priority_list[ext_priority_next];
        ext_priority_next = (ext_priority_next + 1) % EXT_PRIORITY_COUNT;
      } else {
        if (ext_priority_pending_mask != 0) {
          wake_priority_timeout = true;  // gave up after 30s
        }
        ext_priority_pending_mask = 0;
        pid = ext_poll_list[ext_poll_index];
        ext_poll_index = (ext_poll_index + 1) % EXT_POLL_LIST_LENGTH;
        // Cell source "broadcast": the cell voltages arrive by themselves, skip their PIDs (0x9021..0x9083).
        for (uint16_t guard = 0; datalayer_extended.twingoGen1.cell_source_broadcast && pid >= 0x9021 &&
                                 pid <= 0x9083 && guard < EXT_POLL_LIST_LENGTH;
             guard++) {
          pid = ext_poll_list[ext_poll_index];
          ext_poll_index = (ext_poll_index + 1) % EXT_POLL_LIST_LENGTH;
        }
      }
      ZOE_POLL_18DADBF1.data.u8[0] = 0x03;  // see the matching comment in the Cellwatch branch above
      ZOE_POLL_18DADBF1.data.u8[1] = 0x22;
      ZOE_POLL_18DADBF1.data.u8[2] = (uint8_t)((pid >> 8) & 0xFF);
      ZOE_POLL_18DADBF1.data.u8[3] = (uint8_t)(pid & 0xFF);
      transmit_can_frame(&ZOE_POLL_18DADBF1);
#ifdef EXTENDED_UDS_DEBUG
      logging.printf("EXT UDS TX: PID=0x%04X\n", pid);
#endif
    }

    // Abandon a stalled multi-frame reassembly rather than let it block forever.
    if (ext_isotp_in_progress && (currentMillis - ext_isotp_started_ms >= EXT_ISOTP_TIMEOUT_MS)) {
      ext_isotp_in_progress = false;
      datalayer_battery->status.balancing_status = BALANCING_STATUS_ERROR;
#ifdef EXTENDED_UDS_DEBUG
      logging.printf("EXT UDS: reassembly timed out after %lums (%u/%u bytes received)\n", EXT_ISOTP_TIMEOUT_MS,
                     ext_isotp_received_len, ext_isotp_expected_len);
#endif
    }
  }
#endif

  // UDS PID polling and DTC handling
  transmit_uds_can(currentMillis);
}

template <typename T>
inline String& operator<<(String& str, const T& value) {
  str += value;
  return str;
}

String RenaultTwingoGen1Battery::get_uds_info_html() {
  String content;
  content.reserve(9000);

  // clang-format off
  content << "Cell Under Voltage: " << (LB_CUV >= 2 ? "FAULT" : "OK") << "<br>"
             "Cell Over Voltage: " << (LB_COV >= 2 ? "FAULT" : "OK") << "<br>"
             "Pack Under Voltage: " << (LB_HVBUV >= 2 ? "FAULT" : "OK") << "<br>"
             "Pack Over Voltage: " << (LB_HVBOV >= 2 ? "FAULT" : "OK") << "<br>"
             "Pack Over Current: " << (LB_HVBOC >= 2 ? "FAULT" : "OK") << "<br>"
             "Over Temp: " << (LB_HVBOT >= 2 ? "FAULT" : "OK") << "<br>"
             "Isolation: " << (LB_HVBIR >= 2 ? "FAULT" : "OK") << "<br>"
             "End Of Charge: " << (LB_EOCR >= 2 ? "YES" : "NO") << "<br>"
             "Battery Mileage: " << battery_mileage_in_km << " km<br>"
             "Lifetime Energy: " << kWh_from_beginning_of_battery_life << " kWh<br>";
  // clang-format on

#ifdef TWINGO_EXTENDED_CELL_POLLING
  // Everything below sits in one box that refreshes itself while an NVROL reset is running (countdown of
  // the quiet phase) and until the priority PIDs have answered afterwards.
  const bool nvrol_busy = UserRequestNVROLReset || (ext_priority_pending_mask != 0);
  // clang-format off
  content << "<div id='nvrolBox' data-active='" << (nvrol_busy ? "1" : "0") << "'>"
             "Charge Cycles: " << battery_charge_cycles << "<br>"
             "Energy Charged: " << battery_energy_charged_kWh << " kWh<br>"
             "Energy Discharged: " << battery_energy_discharged_kWh << " kWh<br>"
             "Energy Regenerated: " << battery_energy_regenerated_kWh << " kWh<br>"
             "Temporisation (0x9281): " << temporisation_text(battery_temporisation) << "<br>";
  // clang-format on

  // NVROL/Sleep settings: 0x9281 write value and B009 diagnostic session type (each an exclusive pair of
  // checkboxes - clicking one always forces the other off), plus the manual sleep failsafe window.
  // Persisted to NVM, same pattern as the BYD Atto3 auto-calibrate settings.
  {
    const uint8_t wv = datalayer_extended.twingoGen1.nvrol_temporisation_write_value;
    bool write0 = (wv == 0);
    bool prog = datalayer_extended.twingoGen1.nvrol_b009_use_programming_session;
    content +=
        "<h4>0x9281 write value (NVROL reset + Sleep 0x9281=1): "
        "<input type='checkbox' id='twingoWrite00' onclick='twingoSetWriteValue(0)' ";
    content += write0 ? "checked>" : ">";
    content +=
        " 0x00 (activated) "
        "<input type='checkbox' id='twingoWrite01' onclick='twingoSetWriteValue(1)' ";
    content += (wv == 1) ? "checked>" : ">";
    content += " 0x01 <input type='checkbox' id='twingoWrite80' onclick='twingoSetWriteValue(128)' ";
    content += (wv == 0x80) ? "checked>" : ">";
    content += " 0x80</h4>";

    content +=
        "<h4>B009 diagnostic session: "
        "<input type='checkbox' id='twingoB009Ext' onclick='twingoSetB009Session(0)' ";
    content += prog ? ">" : "checked>";
    content +=
        " Extended (0x03) "
        "<input type='checkbox' id='twingoB009Prog' onclick='twingoSetB009Session(1)' ";
    content += prog ? "checked>" : ">";
    content += " Programming (0x02)</h4>";

    {
      // Bau 18: vehicle ID of 0x69F; the switch "use own vehicle ID", the typed ID (always editable) and Set
      uint8_t shown[4];
      const auto& tv = datalayer_extended.twingoGen1;
      const bool typed = tv.vehicle_id[0] || tv.vehicle_id[1] || tv.vehicle_id[2] || tv.vehicle_id[3];
      for (uint8_t i = 0; i < 4; i++) {
        shown[i] = typed ? tv.vehicle_id[i] : TWINGO_VEHICLE_ID_DEFAULT[i];
      }
      char idtxt[16];
      snprintf(idtxt, sizeof(idtxt), "%02X %02X %02X %02X", shown[0], shown[1], shown[2], shown[3]);
      uint8_t sent[4];
      effective_vehicle_id(sent);
      char senttxt[16];
      snprintf(senttxt, sizeof(senttxt), "%02X %02X %02X %02X", sent[0], sent[1], sent[2], sent[3]);
      content += "<h4><input type='checkbox' id='twingoVidEn' onclick='twingoSetVehicleId(true)' ";
      content += tv.vehicle_id_custom ? "checked>" : ">";
      content += " use own vehicle ID (0x69F): <input type='text' id='twingoVid' size='11' maxlength='11' value='";
      content += idtxt;
      content += "'> <button onclick='twingoSetVehicleId(false)'>Set</button> <span id='twingoVidState'>sending ";
      content += senttxt;
      content += tv.vehicle_id_custom ? " (own ID)" : " (default of the car)";
      content += "</span></h4>";
    }

    content +=
        "<h4>Sleep failsafe manual / push button hardware (IO15) --> auto wake after: "
        "<input type='number' id='twingoSleepMinutes' min='1' max='1440' value='";
    content += String(datalayer_extended.twingoGen1.sleep_failsafe_minutes);
    content += "'> min <button onclick='twingoSetSleepMinutes()'>Set</button></h4>";
    content += "<h4><input type='checkbox' id='twingoSleepButton' onclick='twingoSetSleepButton(this.checked)' ";
    content += datalayer_extended.twingoGen1.sleep_button_enabled ? "checked>" : ">";
    content += " push button Sleep (IO15, button to GND): long press 2 s = Sleep 0x9281=1, long way</h4>";

    {
      // Bau 9: daily sleep (time of day, duration, kilometres) in one line, and the minute factor of the age
      const uint16_t tmin = datalayer_extended.twingoGen1.daily_sleep_time_min;
      char hhmm[8];
      snprintf(hhmm, sizeof(hhmm), "%02u:%02u", (unsigned)(tmin / 60) % 24, (unsigned)(tmin % 60));
      content += "<h4>Daily sleep</h4><h4><input type='checkbox' id='twingoDsEn' ";
      content += datalayer_extended.twingoGen1.daily_sleep_enabled ? "checked" : "";
      content += "> enable Time <input type='time' id='twingoDsTime' value='";
      content += hhmm;
      content += "'> Duration <input type='number' id='twingoDsDur' min='1' max='1440' value='";
      content += String(datalayer_extended.twingoGen1.daily_sleep_duration_min);
      content += "'> min Km <input type='number' id='twingoDsKm' min='0' max='65535' value='";
      content += String(datalayer_extended.twingoGen1.daily_sleep_km);
      content +=
          "'> <button onclick='twingoSetDailySleep()'>Set</button></h4>"
          "<h4>Age minute factor: <input type='number' id='twingoAgeFactor' min='1' max='1440' value='";
      content += String(datalayer_extended.twingoGen1.age_minute_factor);
      content +=
          "'> <button onclick='twingoSetAgeFactor()'>Set</button> (1 = real time, 10 = +1 only every 10 min)</h4>";
    }

    // Bau 15: the control block of the bus (moved here from the /simulator page)
    content += simulator_controls_html();

    content +=
        "<h4>Cellwatch (fast single-cell poll): "
        "<input type='checkbox' id='twingoCellwatchEnable' onclick='twingoSetCellwatchEnable(this.checked)' ";
    content += datalayer_extended.twingoGen1.cellwatch_enabled ? "checked>" : ">";
    content += " enable, cell <input type='number' id='twingoCellwatchCell' min='1' max='96' value='";
    content += String(datalayer_extended.twingoGen1.cellwatch_cell);
    content +=
        "'> <button onclick='twingoSetCellwatchCell()'>Set</button> "
        "<button onclick=\"window.open('/cellwatch','_blank')\">Open Cellwatch page</button>"
        " - pauses the normal cell round-robin while enabled</h4>";

#ifdef TWINGO_SIM_PAGE
    content +=
        "<h4><button onclick=\"window.open('/simulator','_blank')\">Open CAN Signal Simulator page</button>"
        " - " +
        String((int)SIM_SIGNAL_COUNT) + " individually toggleable cyclic signals</h4>";
#endif

    // Free request (03.10., write added 05.10.): input field, Query button and answer field. The read services
    // 0x22 and 0x19 and the write service 0x2E for a fixed list of identifiers are accepted (see
    // start_user_query()). The answer is polled from /twingoQueryResult.
    content +=
        "<h4><input type='checkbox' id='twingoSilenceDiag' "
        "onclick=\"fetch('/editTwingoSilenceDiag?value='+(this.checked?1:0))\" ";
    content += diag_in_silence ? "checked>" : ">";
    content +=
        " Allow read requests in the silence of a Sleep run (off by default, runtime only; the silence is not "
        "silent while a request is open; writes stay refused)</h4>";
    content +=
        "<h4>Free request (0x22 / 0x19 read, 0x2E write): <select id='twingoQueryTarget' "
        "onchange=\"fetch('/twingoQueryTarget?value='+this.value)\"><option value='0'";
    content += uq_target == 0 ? " selected" : "";
    content += ">DB (MCPU)</option><option value='1'";
    content += uq_target == 1 ? " selected" : "";
    content +=
        ">DC (safety CPU)</option></select> <input type='text' id='twingoQueryHex' size='20' "
        "maxlength='24' placeholder='22925E'> <button onclick='twingoQuery(-1)'>Query</button></h4>"
        "<table style='border-collapse:collapse'><tr>"
        "<td><button onclick=\"twingoQuick('19 02 09',0)\">DTC 19 02 09 MCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 61',0)\">Time 22 92 61 MCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 59',0)\">State 22 92 59 MCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 5C',0)\">Relay 22 92 5C MCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 5F',0)\">Km 22 92 5F MCPU</button></td></tr><tr>"
        "<td><button onclick=\"twingoQuick('19 02 09',1)\">DTC 19 02 09 SCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 61',1)\">Time 22 92 61 SCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 59',1)\">State 22 92 59 SCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 5C',1)\">Relay 22 92 5C SCPU</button></td>"
        "<td><button onclick=\"twingoQuick('22 92 62',1)\">Km 22 92 62 SCPU</button></td></tr><tr>"
        "<td><button onclick=\"twingoQuick('10 03',0)\">Session 10 03</button></td>"
        "<td><button onclick='twingoTimeNow()'>Set time now</button></td>"
        "<td></td><td></td><td></td></tr></table>";
    content +=
        "<p style='margin:0 0 6px 0;font-size:0.85em;'>Write: <code>2E 92 61 00 00 03</code> = $9261 to 3. Allowed "
        "identifiers: 9261, 9264, 926B, 91C1, 91CF, 925F, 9281, 925E. The value is read first and shown as \"before\"; "
        "the "
        "write is only sent when that read works and the entered value has the same length, and the value is read "
        "back after the write. A write asks for confirmation. \"Set time now\" writes $9261 with the vehicle age "
        "that is being sent. Not undoable except by writing the old value back.</p>";
    content += "<h4>Answer: <span id='twingoQueryResult'>";
    content += user_query_result();
    content += "</span></h4>";

    content += "<script>";
    content += "function twingoSetWriteValue(v){";
    content += "document.getElementById('twingoWrite00').checked=(v===0);";
    content += "document.getElementById('twingoWrite01').checked=(v===1);";
    content += "document.getElementById('twingoWrite80').checked=(v===128);";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoNvrolWriteValue?value='+v,true);x.send();}";
    content += "function twingoSetB009Session(p){";
    content += "document.getElementById('twingoB009Ext').checked=(p===0);";
    content += "document.getElementById('twingoB009Prog').checked=(p===1);";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoB009Session?value='+p,true);x.send();}";
    content += "function twingoSetSleepMinutes(){";
    content += "var m=document.getElementById('twingoSleepMinutes').value;";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoSleepMinutes?value='+m,true);x.send();}";
    content += "function twingoSetDailySleep(){";
    content += "var t=document.getElementById('twingoDsTime').value;";
    content += "var d=document.getElementById('twingoDsDur').value;";
    content += "var k=document.getElementById('twingoDsKm').value;";
    content += "var e=document.getElementById('twingoDsEn').checked?1:0;";
    content +=
        "var x=new XMLHttpRequest();x.open('GET','/editTwingoDailySleep?time='+t+'&dur='+d+'&km='+k+'&en='+e,true);"
        "x.send();}";
    content += "function twingoSetAgeFactor(){";
    content += "var f=document.getElementById('twingoAgeFactor').value;";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoAgeFactor?value='+f,true);x.send();}";
    content += "function twingoSetVehicleId(fromBox){var b=document.getElementById('twingoVidEn');";
    content += "var f=document.getElementById('twingoVid');var s=document.getElementById('twingoVidState');";
    content += "var u='/editTwingoVehicleId?en='+(b.checked?1:0)+'&id='+encodeURIComponent(f.value);";
    content += "fetch(u).then(function(r){return r.text();}).then(function(t){";
    content +=
        "if(t.indexOf('OK')===0){s.textContent=t.substring(3);}else{s.textContent=t;if(fromBox){b.checked=!b.checked;}}"
        "});}";
    content += "function twingoSetSleepButton(v){";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoSleepButton?value='+(v?1:0),true);x.send();}";
    content += "function twingoSetCellwatchEnable(v){";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoCellwatchEnable?value='+(v?1:0),true);x.send();}";
    content += "function twingoSetDtcAllStatus(v){";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoDtcAllStatus?value='+(v?1:0),true);x.send();}";
    content += "setInterval(function(){var r=document.getElementById('twingoTxStat');if(!r){return;}";
    content +=
        "fetch('/twingoTxStat').then(function(a){return a.text();}).then(function(x){r.textContent=x;});},1000);";
    content += "function twingoTxReset(){fetch('/twingoTxStat?reset=1');}";
    content += "setInterval(function(){var r=document.getElementById('twingoRx155');if(!r){return;}";
    content += "fetch('/twingoRx155').then(function(a){return a.text();}).then(function(x){r.textContent=x;});},1000);";
    content += "function twingoPoll(which,span){var n=0;var h=setInterval(function(){";
    content += "fetch('/twingoQueryResult?which='+which).then(function(a){return a.text();}).then(function(x){";
    content += "span.textContent=x;if((x!=='requested'&&x!=='')||++n>40){clearInterval(h);}});},250);}";
    content += "function twingoQuery(t){var v=document.getElementById('twingoQueryHex').value;";
    content += "try{localStorage.setItem('twingoQueryHex',v);}catch(e){}";
    content += "var r=document.getElementById('twingoQueryResult');";
    content += "if(/^\\s*2E/i.test(v)&&!confirm('Write '+v+' to the pack?')){return;}";
    content += "var q='/twingoQuery?hex='+encodeURIComponent(v);if(t===0||t===1){q+='&target='+t;}";
    content += "fetch(q).then(function(a){return a.text();}).then(function(m){";
    content += "if(m!=='OK'){r.textContent=m;return;}r.textContent='requested';twingoPoll('free',r);});}";
    content += "function twingoQuick(h,t){document.getElementById('twingoQueryHex').value=h;twingoQuery(t);}";
    content += "function twingoTimeNow(){var r=document.getElementById('twingoQueryResult');";
    content += "if(!confirm('Write $9261 with the vehicle age that is being sent?')){return;}";
    content += "fetch('/twingoSetTimeNow').then(function(a){return a.text();}).then(function(t){";
    content += "if(t!=='OK'){r.textContent=t;return;}r.textContent='requested';twingoPoll('free',r);});}";
    content += "window.addEventListener('load',function(){try{var s=localStorage.getItem('twingoQueryHex');";
    content += "if(s){document.getElementById('twingoQueryHex').value=s;}}catch(e){}});";
    content += "function twingoSetCellwatchCell(){";
    content += "var c=document.getElementById('twingoCellwatchCell').value;";
    content += "var x=new XMLHttpRequest();x.open('GET','/editTwingoCellwatchCell?value='+c,true);x.send();}";
    content += "</script>";
  }

  append_live_html(content);
  // clang-format off
  content << "NVROL Log - Session1: " << nvrol_log[0] << "<br>"
             "NVROL Log - Routine B009: " << nvrol_log[1] << "<br>"
             "NVROL Log - Routine B009 results: " << nvrol_log[5] << "<br>"
             "NVROL Log - Session2: " << nvrol_log[2] << "<br>"
             "NVROL Log - Write 9281=" << write_value_text(nvrol_written_value) << ": " << nvrol_log[3] << "<br>"
             "NVROL Log - Read back 0x9281: " << nvrol_log[4] << "<br>"
             "Temporisation right after the write (read back): " << temporisation_text(temporisation_readback) << "<br>"
             "<input type='checkbox' id='twingoDtcAllStatus' onclick='twingoSetDtcAllStatus(this.checked)' "
             << (datalayer_extended.twingoGen1.dtc_ext_read_mask == 0xFF ? "checked>" : ">") <<
             " Read ALL DTC statuses (mask 0xFF) instead of only Active/Confirmed (0x09) on next Read DTC<br>"
             "DTC Read (ext. protocol, Read DTC button): " << dtc_ext_log_read << "<br>"
             "DTC Erase (ext. protocol, Erase DTC button): " << dtc_ext_log_erase << "<br>"
             "<button onclick='twingoTxReset()'>Reset</button> Own send intervals 0x426 / 0x435 / 0x436 (time of the call "
             "to the send function in microseconds, not of the wire; target 100 ms):<pre id='twingoTxStat'>-</pre>"
             "<span id='twingoRx155'>" << rx_155_text() << "</span><br>";
  // clang-format on
  {
    // Bau 10: the block of the sleep run is a collapsible section: open while a run is going, closed afterwards, so
    // that the buttons below it stay in sight
    String quiet_block;
    append_quiet_html(quiet_block);
    const bool run_going =
        UserRequestNVROLReset || NVROLstateMachine == 5 || NVROLstateMachine == 7 || NVROLstateMachine == 8;
    if (!run_going && !nvrol_silence_done) {
      content << quiet_block;  // before the first run: only the one line "not run yet", nothing to fold
    } else {
      content << (run_going ? "<details id='twingoQuiet' open>" : "<details id='twingoQuiet'>")
              << "<summary><b>Sleep run details</b></summary>" << quiet_block << "</details>";
    }
  }
  content << "</div>";
  append_refresh_script_html(content, nvrol_busy);
#endif

  return content;
}

void RenaultTwingoGen1Battery::setup(void) {  // Performs one time setup at startup
  // Suppress CAN_NATIVE_BUS_ERROR/CAN_NATIVE_BUFFER_FULL for this interface during the boot window (same
  // duration as the 0x425/0x424 boot filters above) - covers the brief CAN-error burst right after
  // power-on/reset, same framework function MG-GEN1-BATTERY.cpp and MEB-BATTERY.cpp already use around
  // their own BMS-reset moments (02.10. phantom BATTERY_OVERVOLTAGE investigation).
  ignore_can_errors_for(can_interface, EXT_425_BOOT_FILTER_TIMEOUT_MS);
  age_load_from_nvm();
  apply_vehicle_id();  // Bau 18: default ID of the car or the typed own ID
  if (time_436_active) {
    time_436_set(time_436_value);  // the typed time of 0x436 carries on from the stored value
  }

  // UDS: send requests/flow control to 0x79B, accept replies from the BMS on 0x7BB.
  setup_uds(0x79B, 0x7BB);

  // The Twngo Gen1 BMS only speaks KWP2000-style one-byte local identifiers.
  set_pid_scan_mode(PidScanMode::OneByteLocalId);

  // No PID scan list (12.10., Bau 6): the standard path 0x79B/0x7BB is never answered by this battery and 0x79B does
  // not occur on the bus between battery and EVC; with a list the driver sent about 5 requests per second on 0x79B
  // although no row of the simulator switches them. The extended path 0x18DADBF1 does all the polling.

  strncpy(datalayer.system.info.battery_protocol, Name, 63);
  datalayer.system.info.battery_protocol[63] = '\0';
  datalayer.system.status.battery_allows_contactor_closing = true;
  datalayer_battery->info.number_of_cells = 96;
  datalayer_battery->info.max_design_voltage_dV = MAX_PACK_VOLTAGE_DV;
  datalayer_battery->info.min_design_voltage_dV = MIN_PACK_VOLTAGE_DV;
  datalayer_battery->info.max_cell_voltage_mV = MAX_CELL_VOLTAGE_MV;
  datalayer_battery->info.min_cell_voltage_mV = MIN_CELL_VOLTAGE_MV;
  datalayer_battery->info.max_cell_voltage_deviation_mV = MAX_CELL_DEVIATION_MV;
#ifdef TWINGO_EXTENDED_CELL_POLLING
  // This battery provides 8 single pack temperature sensors (shown on the Cellmonitor page).
  datalayer_battery->status.temperature_sensors_count = EXT_TEMP_SENSOR_COUNT;
#endif
}

// The generic UdsCanBattery sequence machinery (start_sequence/send_sequence_message/
// on_uds_sequence_step) is bound to setup_uds(0x79B, 0x7BB) - the standard KWP2000 path, which
// RenoLink confirmed (01.10.) never answers on this battery (same dead path as the generic
// supports_read_DTC()/supports_reset_DTC() buttons' default implementation would have used). read_DTC()
// and reset_DTC() below are therefore NOT routed through that machinery at all; they drive the separate
// dtc_ext_* state machine instead, which talks to the extended 29-bit protocol (0x18DADBF1/0x18DAF1DB)
// that this battery actually responds to for everything else (NVROL, 0x9281, cell polling).
void RenaultTwingoGen1Battery::on_uds_sequence_step(uint16_t state, uint8_t sid, const uint8_t* data, uint16_t len) {
  // Intentionally empty - see comment above.
}

void RenaultTwingoGen1Battery::read_DTC() {
  if (dtc_ext_state != DTC_EXT_IDLE || UserRequestNVROLReset) {
    return;  // already busy with a DTC exchange, or a Sleep/NVROL sequence is running
  }
  ZOE_POLL_18DADBF1.data = {0x02, 0x10, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};  // open extended session
  transmit_can_frame(&ZOE_POLL_18DADBF1);
  strncpy(dtc_ext_log_read, "requested", sizeof(dtc_ext_log_read) - 1);
  dtc_ext_log_read[sizeof(dtc_ext_log_read) - 1] = '\0';
  dtc_ext_state = DTC_EXT_READ_SESSION_SENT;
  dtc_ext_step_start_ms = millis();
}

void RenaultTwingoGen1Battery::read_DTC_scpu() {
  char hex[16];
  snprintf(hex, sizeof(hex), "19 02 %02X", (unsigned)datalayer_extended.twingoGen1.dtc_ext_read_mask);
  // The answer is the field "Answer" of this page and fills the list "SCPU" of the DTC section
  uq_scpu_row_pending = true;
  const char* msg = start_user_query(hex, 1);
  if (strcmp(msg, "OK") != 0) {
    uq_scpu_row_pending = false;
    dtc_scpu_data.dtc_read_failed = true;  // not started (e.g. a request is running)
    dtc_scpu_data.dtc_last_read_millis = millis();
  }
}

void RenaultTwingoGen1Battery::reset_DTC_scpu() {
  start_user_query("14 FF FF FF", 1);
}

void RenaultTwingoGen1Battery::reset_DTC() {
  if (dtc_ext_state != DTC_EXT_IDLE || UserRequestNVROLReset) {
    return;
  }
  ZOE_POLL_18DADBF1.data = {0x02, 0x10, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};  // open extended session
  transmit_can_frame(&ZOE_POLL_18DADBF1);
  strncpy(dtc_ext_log_erase, "requested", sizeof(dtc_ext_log_erase) - 1);
  dtc_ext_log_erase[sizeof(dtc_ext_log_erase) - 1] = '\0';
  dtc_ext_state = DTC_EXT_ERASE_SESSION_SENT;
  dtc_ext_step_start_ms = millis();
}

// ---------------------------------------------------------------------------
// Free read request, fault counters (19 14) and DTC details (19 06): one request on the extended 29-bit
// protocol, one reply collector (03.10.). Only read services are accepted for the free request.
// ---------------------------------------------------------------------------
uint8_t RenaultTwingoGen1Battery::uq_target = 0;
bool RenaultTwingoGen1Battery::diag_in_silence = false;

void RenaultTwingoGen1Battery::uq_restore_poll_template() {
  ZOE_POLL_18DADBF1.ID = UQ_ID_REQ_DB;
  ZOE_POLL_FLOW_CONTROL.ID = UQ_ID_REQ_DB;
  uq_active_dc = false;
  uq_post_read = false;
  ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}

// Identifiers the free request may write (0x2E): the two time counters, the two mileages and the temporisation.
const uint16_t RenaultTwingoGen1Battery::UQ_WRITE_DIDS[UQ_WRITE_DID_COUNT] = {0x9261, 0x9264, 0x926B, 0x91C1,
                                                                              0x91CF, 0x925F, 0x9281, 0x925E};

// Short meaning of the negative response codes (ISO 14229) that matter for a write; "" for any other code.
const char* RenaultTwingoGen1Battery::uq_nrc_text(uint8_t nrc) {
  switch (nrc) {
    case 0x12:
      return "sub-function not supported";
    case 0x13:
      return "incorrect message length or format";
    case 0x22:
      return "conditions not correct";
    case 0x24:
      return "request sequence error";
    case 0x31:
      return "request out of range";
    case 0x33:
      return "security access denied";
    case 0x72:
      return "general programming failure";
    case 0x7E:
      return "sub-function not supported in the active session";
    case 0x7F:
      return "service not supported in the active session";
    default:
      return "";
  }
}

// Read of the identifier a write (0x2E) is about to change: 22 <DID hi> <DID lo>, answered by 62 <DID> <data>.
void RenaultTwingoGen1Battery::uq_send_pre_read() {
  for (uint8_t i = 0; i < 8; i++) {
    ZOE_POLL_18DADBF1.data.u8[i] = 0xAA;
  }
  ZOE_POLL_18DADBF1.data.u8[0] = 0x03;
  ZOE_POLL_18DADBF1.data.u8[1] = 0x22;
  ZOE_POLL_18DADBF1.data.u8[2] = uq_req[1];
  ZOE_POLL_18DADBF1.data.u8[3] = uq_req[2];
  transmit_can_frame(&ZOE_POLL_18DADBF1);
  uq_buf_len = 0;
  uq_expected_len = 0;
  uq_received_len = 0;
  uq_in_progress = false;
  uq_pre_read = true;
  uq_before_len = 0;
}

// Sends uq_req (uq_req_len bytes) as a single frame on the shared poll frame object, padded with 0xAA like the
// other sequences of this driver, and clears the collector.
void RenaultTwingoGen1Battery::uq_send_request() {
  for (uint8_t i = 0; i < 8; i++) {
    ZOE_POLL_18DADBF1.data.u8[i] = 0xAA;
  }
  ZOE_POLL_18DADBF1.data.u8[0] = uq_req_len;
  for (uint8_t i = 0; i < uq_req_len && i < 7; i++) {
    ZOE_POLL_18DADBF1.data.u8[1 + i] = uq_req[i];
  }
  transmit_can_frame(&ZOE_POLL_18DADBF1);
  uq_buf_len = 0;
  uq_expected_len = 0;
  uq_received_len = 0;
  uq_in_progress = false;
}

bool RenaultTwingoGen1Battery::uq_begin(uint8_t mode, const uint8_t* req, uint8_t len, bool needs_session) {
  const bool silence_ok = diag_in_silence && NVROLstateMachine == 5 && req[0] != 0x2E;
  if (dtc_ext_state != DTC_EXT_IDLE || (UserRequestNVROLReset && !silence_ok) || ext_isotp_in_progress || len == 0 ||
      len > 7) {
    return false;
  }
  uq_mode = mode;
  uq_req_len = len;
  uq_to_scpu_row = uq_scpu_row_pending && mode == UQ_FREE;  // the request of the button "Read DTC SCPU"
  uq_scpu_row_pending = false;
  for (uint8_t i = 0; i < len; i++) {
    uq_req[i] = req[i];
  }
  uq_needs_session = needs_session;
  uq_post_read = false;
  uq_active_dc = (mode == UQ_FREE && uq_req_target == 1);
  ZOE_POLL_18DADBF1.ID = uq_active_dc ? UQ_ID_REQ_DC : UQ_ID_REQ_DB;
  ZOE_POLL_FLOW_CONTROL.ID = ZOE_POLL_18DADBF1.ID;
  uq_pre_read = false;
  uq_before_len = 0;
  snprintf(uq_result, sizeof(uq_result), "requested");
  if (needs_session) {
    ZOE_POLL_18DADBF1.data = {0x02, 0x10, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};  // open extended session first
    transmit_can_frame(&ZOE_POLL_18DADBF1);
    dtc_ext_state = DTC_EXT_USER_SESSION_SENT;
  } else {
    uq_send_request();
    dtc_ext_state = DTC_EXT_USER_CMD_SENT;
  }
  dtc_ext_step_start_ms = millis();
  return true;
}

const char* RenaultTwingoGen1Battery::start_user_query(const char* hex, int8_t target) {
  const uint8_t tgt = target < 0 ? uq_target : (target == 1 ? 1 : 0);
  uint8_t req[8];
  uint8_t n = 0;
  int hi = -1;
  for (const char* c = hex; c != nullptr && *c != '\0'; c++) {
    char ch = *c;
    int v;
    if (ch == ' ' || ch == '\t' || ch == ',' || ch == ':') {
      continue;  // separators are allowed
    } else if (ch >= '0' && ch <= '9') {
      v = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
      v = ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
      v = ch - 'A' + 10;
    } else {
      return "invalid character (hex digits only)";
    }
    if (hi < 0) {
      hi = v;
    } else {
      if (n >= 7) {
        return "too long (at most 7 bytes)";
      }
      req[n++] = (uint8_t)((hi << 4) | v);
      hi = -1;
    }
  }
  if (hi >= 0) {
    return "odd number of hex digits";
  }
  if (n == 0) {
    return "empty request";
  }
  if (req[0] == 0x22) {
    if (n < 3 || ((n - 1) % 2) != 0) {
      return "0x22 needs one or more 2-byte identifiers";
    }
  } else if (req[0] == 0x19) {
    if (n < 2) {
      return "0x19 needs a sub-function";
    }
  } else if (req[0] == 0x14) {
    // ClearDiagnosticInformation, only "all DTCs" (14 FF FF FF), point 18 (09.10.)
    if (n != 4 || req[1] != 0xFF || req[2] != 0xFF || req[3] != 0xFF) {
      return "0x14 is only allowed as 14 FF FF FF (clear all DTCs)";
    }
  } else if (req[0] == 0x10) {
    // DiagnosticSessionControl, only the extended session (10 03) that the writes and 0x19 use anyway (point 10)
    if (n != 2 || req[1] != 0x03) {
      return "0x10 is only allowed as 10 03 (extended session)";
    }
  } else if (req[0] == 0x2E) {
    if (tgt == 1) {
      return "0x2E is not allowed for DC (safety CPU)";
    }
    // WriteDataByIdentifier: 2E <DID hi> <DID lo> <1..4 data bytes> (a single frame carries 7 bytes at most),
    // only for the identifiers of UQ_WRITE_DIDS.
    if (n < 4) {
      return "0x2E needs a 2-byte identifier and 1 to 4 data bytes";
    }
    uint16_t did = (uint16_t)((req[1] << 8) | req[2]);
    bool allowed = false;
    for (uint8_t i = 0; i < UQ_WRITE_DID_COUNT; i++) {
      if (UQ_WRITE_DIDS[i] == did) {
        allowed = true;
      }
    }
    if (!allowed) {
      return "0x2E is only allowed for 9261, 9264, 926B, 91C1, 91CF, 925F, 9281 and 925E";
    }
  } else {
    return "only 0x22 and 0x19 (read), 0x10 03 (session), 14 FF FF FF (clear DTCs) and 0x2E (write, "
           "9261/9264/926B/91C1/91CF/925F/9281/925E only) are allowed";
  }
  uq_req_target = tgt;
  if (!uq_begin(UQ_FREE, req, n, req[0] == 0x19 || req[0] == 0x2E || req[0] == 0x14)) {
    return "busy (another diagnostic exchange or a Sleep/NVROL run is active)";
  }
  return "OK";
}

const char* RenaultTwingoGen1Battery::write_time_now() {
  uint32_t age = 0;
  if (!vehicle_age_available(millis(), age)) {
    return "no vehicle age known yet (no clock and no manual age)";
  }
  char hex[24];
  snprintf(hex, sizeof(hex), "2E 92 61 %02X %02X %02X", (unsigned)((age >> 16) & 0xFF), (unsigned)((age >> 8) & 0xFF),
           (unsigned)(age & 0xFF));
  return start_user_query(hex);
}

// printf into out at pos without ever running past the buffer (pos stays below outsz).
static void uq_printf(char* out, size_t outsz, size_t& pos, const char* fmt, ...) {
  if (outsz == 0 || pos + 1 >= outsz) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(out + pos, outsz - pos, fmt, ap);
  va_end(ap);
  if (n < 0) {
    return;
  }
  size_t room = outsz - pos - 1;
  pos += ((size_t)n < room) ? (size_t)n : room;
}

// Appends n bytes as "XX XX ..." to out at pos; ends with "..." when the text buffer is full.
static void uq_append_hex(char* out, size_t outsz, size_t& pos, const uint8_t* d, uint16_t n) {
  for (uint16_t i = 0; i < n; i++) {
    if (pos + 8 >= outsz) {
      uq_printf(out, outsz, pos, "...");
      return;
    }
    uq_printf(out, outsz, pos, "%s%02X", i == 0 ? "" : " ", d[i]);
  }
}

// A complete reply is in uq_buf (first byte = SID of the answer, 0x7F for a negative one).
void RenaultTwingoGen1Battery::uq_reply_complete() {
  char* out = uq_result;
  const size_t outsz = sizeof(uq_result);
  out[0] = '\0';
  size_t pos = 0;
  if (uq_mode == UQ_FREE) {
    // The request first, then the answer: "22 92 5E: OK 62 92 5E 13 88 6F" ("[DC] " in front for the safety CPU)
    if (uq_active_dc) {
      uq_printf(out, outsz, pos, "[DC] ");
    }
    uq_append_hex(out, outsz, pos, uq_req, uq_req_len);
    uq_printf(out, outsz, pos, ": ");
    if (uq_req[0] == 0x2E && uq_before_len > 0) {
      uq_printf(out, outsz, pos, "before ");  // the value read in front of the write
      uq_append_hex(out, outsz, pos, uq_before, uq_before_len);
      uq_printf(out, outsz, pos, " -> ");
    }
  }
  if (uq_buf_len >= 3 && uq_buf[0] == 0x7F) {
    uq_printf(out, outsz, pos, "NEGATIVE %02X %02X %02X (SID 0x%02X, NRC 0x%02X)", uq_buf[0], uq_buf[1], uq_buf[2],
              uq_buf[1], uq_buf[2]);
    if (uq_mode == UQ_FREE && uq_req[0] == 0x2E && uq_nrc_text(uq_buf[2])[0] != '\0') {
      uq_printf(out, outsz, pos, " - %s", uq_nrc_text(uq_buf[2]));
    }
  } else {
    uq_printf(out, outsz, pos, "OK ");
    uq_append_hex(out, outsz, pos, uq_buf, uq_buf_len);
    if (uq_expected_len > uq_buf_len) {
      uq_printf(out, outsz, pos, " (cut, %u bytes in total)", (unsigned)uq_expected_len);
    }
    if (uq_mode == UQ_FREE && uq_req[0] == 0x22 && uq_req_len == 3 && uq_buf_len >= 4 && uq_buf[0] == 0x62 &&
        uq_buf_len == uq_expected_len) {
      // Known identifiers shown in plain words (point 19): the times as days / hours / minutes, the mileages in km
      const uint16_t did = (uint16_t)((uq_buf[1] << 8) | uq_buf[2]);
      const uint8_t n = (uint8_t)(uq_buf_len - 3);
      const uint8_t* d = &uq_buf[3];
      if ((did == 0x9261 || did == 0x91C1) && n == 3) {
        const uint32_t m = ((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2];
        uq_printf(out, outsz, pos, " | %lu min = %lu d %lu h %lu min", (unsigned long)m, (unsigned long)(m / 1440UL),
                  (unsigned long)((m % 1440UL) / 60UL), (unsigned long)(m % 60UL));
      } else if (did == 0x925F && n == 4) {
        const uint32_t raw = ((uint32_t)d[0] << 24) | ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | d[3];
        uq_printf(out, outsz, pos, " | %lu.%02lu km", (unsigned long)(raw / 100UL), (unsigned long)(raw % 100UL));
      } else if ((did == 0x91CD || did == 0x921C) && n == 2) {
        // Bau 16: thermistors T1 ("Cooling temperature") and T2 ("Skin temperature") in degrees. Scale like the cell
        // sensors (raw / 16 - 40); NOT confirmed for these two identifiers (bench 14.10.: 3.3 kOhm -> raw 940 = 18.75 C).
        const uint16_t raw = (uint16_t)((d[0] << 8) | d[1]);
        const int32_t c100 = (int32_t)raw * 625 / 100 - 4000;  // hundredths of a degree
        if (raw != 0 && raw != 0xFFFF && c100 <= 10000) {
          const int32_t a = c100 < 0 ? -c100 : c100;
          uq_printf(out, outsz, pos, " | %s%ld.%02ld C (scale like the cell sensors, unconfirmed)", c100 < 0 ? "-" : "",
                    (long)(a / 100), (long)(a % 100));
        }
      } else if (did == 0x9262 && n == 4 && uq_active_dc) {
        // SCPU vehicle distance totalizer: raw minus 0x80000000, 0.01 km
        const uint32_t raw = ((uint32_t)d[0] << 24) | ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | d[3];
        const uint32_t v = raw - 0x80000000UL;
        if (raw >= 0x80000000UL) {
          uq_printf(out, outsz, pos, " | %lu.%02lu km", (unsigned long)(v / 100UL), (unsigned long)(v % 100UL));
        }
      }
    }
    if (uq_mode == UQ_FREE && uq_req[0] == 0x19 && uq_req[1] == 0x02 && uq_buf_len >= 3 && uq_buf[0] == 0x59 &&
        uq_buf[1] == 0x02 && uq_buf_len == uq_expected_len && ((uq_buf_len - 3) % 4) == 0) {
      // ReadDTCInformation 02: 59 02 <availability mask>, then per DTC 3 bytes code + 1 byte status
      const uint16_t entries = (uint16_t)((uq_buf_len - 3) / 4);
      if (entries == 0) {
        uq_printf(out, outsz, pos, " | no DTC");
      } else {
        uq_printf(out, outsz, pos, " | DTC:");
        for (uint16_t e = 0; e < entries; e++) {
          const uint8_t* d = &uq_buf[3 + e * 4];
          if (pos + 14 >= outsz) {
            uq_printf(out, outsz, pos, " ...");
            break;
          }
          uq_printf(out, outsz, pos, " %02X%02X%02X=%02X", d[0], d[1], d[2], d[3]);
        }
      }
    }
  }
  uq_in_progress = false;
  if (uq_to_scpu_row) {
    // the field "SCPU": positive answer 59 02 <mask> then the 4-byte entries; anything else counts as failed
    if (uq_buf_len >= 3 && uq_buf[0] == 0x59 && uq_buf[1] == 0x02 && uq_buf_len == uq_expected_len) {
      fill_dtc_list(dtc_scpu_data, &uq_buf[3], (uint16_t)(uq_buf_len - 3), (uint16_t)((uq_buf_len - 3) / 4));
    } else {
      dtc_scpu_data.dtc_read_failed = true;
      dtc_scpu_data.dtc_last_read_millis = millis();
    }
    uq_to_scpu_row = false;
  }
  if (uq_mode == UQ_FREE && uq_req[0] == 0x2E && uq_buf_len >= 1 && uq_buf[0] == 0x6E && !uq_post_read) {
    // Positive answer to the write: read the identifier back at once (point 10), the answer is appended to the line
    ZOE_POLL_18DADBF1.data = {0x03, 0x22, uq_req[1], uq_req[2], 0xAA, 0xAA, 0xAA, 0xAA};
    transmit_can_frame(&ZOE_POLL_18DADBF1);
    uq_post_read = true;
    memcpy(uq_write_line, uq_result, sizeof(uq_write_line));
    snprintf(uq_result, sizeof(uq_result), "requested");  // the page keeps waiting for the read back
    dtc_ext_step_start_ms = millis();
    return;
  }
  uq_restore_poll_template();
  dtc_ext_state = DTC_EXT_IDLE;
}

// Does this reply belong to the request of uq_begin()? p = payload starting with the SID, n = number of payload
// bytes available (single frame: all, First Frame: the first 6). Positive: SID = request SID + 0x40 and, for 0x22,
// the first requested DID / for 0x19 the sub-function is echoed. Negative: 7F and the requested SID is ours.
// Everything else - the "50 03 ..." confirmation of the session we opened, a late reply of the cell polling
// ("62 90 72 ...") - does not belong to the request (03.10.: both were shown as the answer before).
bool RenaultTwingoGen1Battery::uq_reply_matches(const uint8_t* p, uint8_t n) const {
  if (uq_req_len >= 1 && uq_req[0] == 0x14 && n >= 1 && p[0] == 0x54) {
    return true;  // positive answer to ClearDiagnosticInformation is the single byte 54
  }
  if (n < 2 || uq_req_len < 1) {
    return false;
  }
  if (p[0] == 0x7F) {
    return n >= 3 && p[1] == uq_req[0];
  }
  if (p[0] != (uint8_t)(uq_req[0] + 0x40)) {
    return false;
  }
  if (uq_req[0] == 0x22) {
    return n >= 3 && uq_req_len >= 3 && p[1] == uq_req[1] && p[2] == uq_req[2];
  }
  if (uq_req[0] == 0x19) {
    return uq_req_len >= 2 && p[1] == uq_req[1];
  }
  if (uq_req[0] == 0x2E) {
    return n >= 3 && uq_req_len >= 3 && p[1] == uq_req[1] && p[2] == uq_req[2];  // 6E <DID>
  }
  if (uq_req[0] == 0x10) {
    return uq_req_len >= 2 && p[1] == uq_req[1];  // 50 03
  }
  return false;
}

// Reply to the read in front of a write (uq_pre_read): single frame 62 <DID> <1..4 data bytes>, or 7F 22 <NRC>.
// Positive: the data bytes are kept as the "before" value; when their number is the number of the entered data
// bytes the write is sent now, otherwise nothing is written. Anything else is left to the normal handling.
bool RenaultTwingoGen1Battery::handle_pre_read_reply(const CAN_frame& f) {
  uint8_t pci = f.data.u8[0];
  if (pci < 3 || pci > 7) {
    return false;  // only single frames are expected here
  }
  const uint8_t* p = &f.data.u8[1];
  char head[48];
  size_t hpos = 0;
  head[0] = '\0';
  uq_append_hex(head, sizeof(head), hpos, uq_req, uq_req_len);
  if (p[0] == 0x7F) {
    if (p[1] != 0x22) {
      return false;
    }
    if (pci == 3 && p[2] == 0x78) {
      dtc_ext_step_start_ms = millis();  // the final answer follows later
      return true;
    }
    snprintf(uq_result, sizeof(uq_result), "%s: read before the write refused (NRC 0x%02X) - nothing was written", head,
             p[2]);
  } else if (p[0] == 0x62 && p[1] == uq_req[1] && p[2] == uq_req[2]) {
    uint8_t n = (uint8_t)(pci - 3);
    uint8_t entered = (uint8_t)(uq_req_len - 3);
    if (n == 0 || n > sizeof(uq_before)) {
      snprintf(uq_result, sizeof(uq_result), "%s: read before the write gave %u data bytes - nothing was written", head,
               (unsigned)n);
    } else {
      for (uint8_t i = 0; i < n; i++) {
        uq_before[i] = p[3 + i];
      }
      uq_before_len = n;
      if (n != entered) {
        snprintf(uq_result, sizeof(uq_result), "%s: the value is %u bytes long, %u entered - nothing was written", head,
                 (unsigned)n, (unsigned)entered);
      } else {
        uq_pre_read = false;
        uq_send_request();  // the write itself
        dtc_ext_step_start_ms = millis();
        return true;
      }
    }
  } else {
    return false;
  }
  uq_pre_read = false;
  uq_in_progress = false;
  uq_restore_poll_template();
  dtc_ext_state = DTC_EXT_IDLE;
  return true;
}

// Reply to the read back after a write (uq_post_read): the data bytes are appended to the result line.
bool RenaultTwingoGen1Battery::handle_post_read_reply(const CAN_frame& f) {
  uint8_t pci = f.data.u8[0];
  if (pci < 3 || pci > 7) {
    return false;
  }
  const uint8_t* p = &f.data.u8[1];
  memcpy(uq_result, uq_write_line, sizeof(uq_result));
  size_t pos = strlen(uq_result);
  if (p[0] == 0x7F && p[1] == 0x22) {
    if (pci == 3 && p[2] == 0x78) {
      dtc_ext_step_start_ms = millis();
      return true;
    }
    uq_printf(uq_result, sizeof(uq_result), pos, " | read back refused (NRC 0x%02X)", p[2]);
  } else if (p[0] == 0x62 && p[1] == uq_req[1] && p[2] == uq_req[2]) {
    uq_printf(uq_result, sizeof(uq_result), pos, " | read back ");
    uq_append_hex(uq_result, sizeof(uq_result), pos, &p[3], (uint8_t)(pci - 3));
  } else {
    return false;
  }
  uq_post_read = false;
  uq_restore_poll_template();
  dtc_ext_state = DTC_EXT_IDLE;
  return true;
}

// Collects the reply to the request of uq_begin(): single frame, or First Frame (flow control sent here) plus
// Consecutive Frames. "Response pending" (NRC 0x78) only restarts the timeout. Returns false for a frame that is
// not the reply to this request; the caller then handles it like any other frame (cell polling etc.).
bool RenaultTwingoGen1Battery::handle_user_query_reply(const CAN_frame& f) {
  if (uq_post_read) {
    return handle_post_read_reply(f);
  }
  if (uq_pre_read) {
    return handle_pre_read_reply(f);
  }
  uint8_t pci = f.data.u8[0];
  if (pci < 0x10) {
    uint8_t len = pci;
    if (len == 0 || len > 7 || !uq_reply_matches(&f.data.u8[1], len)) {
      return false;
    }
    for (uint8_t i = 0; i < len; i++) {
      uq_buf[i] = f.data.u8[1 + i];
    }
    uq_buf_len = len;
    uq_expected_len = len;
    uq_received_len = len;
    if (len == 3 && uq_buf[0] == 0x7F && uq_buf[2] == 0x78) {
      dtc_ext_step_start_ms = millis();  // the final answer follows later
      return true;
    }
    uq_reply_complete();
    return true;
  }
  if ((pci & 0xF0) == 0x10) {
    uint16_t expected = (uint16_t)(((pci & 0x0F) << 8) | f.data.u8[1]);
    if (expected < 7 || !uq_reply_matches(&f.data.u8[2], 6)) {
      return false;  // not a valid First Frame, or the start of another reply (late cell polling reply)
    }
    uq_expected_len = expected;
    uq_buf_len = 0;
    for (uint8_t i = 0; i < 6; i++) {
      uq_buf[uq_buf_len++] = f.data.u8[2 + i];
    }
    uq_received_len = 6;
    uq_in_progress = true;
    transmit_can_frame(&ZOE_POLL_FLOW_CONTROL);
    return true;
  }
  if ((pci & 0xF0) == 0x20) {
    if (!uq_in_progress) {
      return false;  // a Consecutive Frame of some other reply
    }
    uint16_t remaining = (uint16_t)(uq_expected_len - uq_received_len);
    uint8_t n = remaining < 7 ? (uint8_t)remaining : 7;
    for (uint8_t i = 0; i < n; i++) {
      if (uq_buf_len < sizeof(uq_buf)) {
        uq_buf[uq_buf_len++] = f.data.u8[1 + i];  // bytes beyond the buffer are counted but not stored
      }
    }
    uq_received_len = (uint16_t)(uq_received_len + n);
    if (uq_received_len >= uq_expected_len) {
      uq_reply_complete();
    }
    return true;
  }
  return false;
}

// Drives the DTC Read/Erase probe: open an extended diagnostic session (0x10 0x03, same subfunction
// used successfully for the 0x9281 sequence - NOT the 0xC0 session the old, broken read_DTC() used to
// send), wait DTC_EXT_SESSION_GAP_MS (mirrors the NVROL sequence's own 100ms pacing between steps, not
// a measured requirement of this BMS), then send the actual service. The raw reply (or its absence) is
// logged verbatim via handle_dtc_ext_reply() / the timeout branch below - nothing about the response is
// assumed or parsed into a DTC list, since none has ever been seen on this battery.
void RenaultTwingoGen1Battery::handle_dtc_ext(unsigned long currentMillis) {
  switch (dtc_ext_state) {
    case DTC_EXT_IDLE:
      break;

    case DTC_EXT_READ_SESSION_SENT:
      if (currentMillis - dtc_ext_step_start_ms >= DTC_EXT_SESSION_GAP_MS) {
        // Gap elapsed: send ReadDTCInformation, status mask (0x09 default, or 0xFF if the "all statuses"
        // checkbox was ticked before pressing Read DTC).
        ZOE_POLL_18DADBF1.data = {0x03, 0x19, 0x02, datalayer_extended.twingoGen1.dtc_ext_read_mask,
                                  0xAA, 0xAA, 0xAA, 0xAA};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        dtc_ext_step_start_ms = currentMillis;
        dtc_ext_state = DTC_EXT_READ_CMD_SENT;
      }
      break;
    case DTC_EXT_READ_CMD_SENT:
      if (currentMillis - dtc_ext_step_start_ms >= DTC_EXT_REPLY_TIMEOUT_MS) {
        strncpy(dtc_ext_log_read, "no response", sizeof(dtc_ext_log_read) - 1);
        dtc_ext_log_read[sizeof(dtc_ext_log_read) - 1] = '\0';
        ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // restore poll template
        dtc_ext_state = DTC_EXT_IDLE;
      }
      break;

    case DTC_EXT_ERASE_SESSION_SENT:
      if (currentMillis - dtc_ext_step_start_ms >= DTC_EXT_SESSION_GAP_MS) {
        // ClearDiagnosticInformation, group 0xFFFFFF (all groups) - same group Zoe Gen2's existing,
        // upstream Erase DTC button uses on this same protocol.
        ZOE_POLL_18DADBF1.data = {0x04, 0x14, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA};
        transmit_can_frame(&ZOE_POLL_18DADBF1);
        dtc_ext_step_start_ms = currentMillis;
        dtc_ext_state = DTC_EXT_ERASE_CMD_SENT;
      }
      break;
    case DTC_EXT_ERASE_CMD_SENT:
      if (currentMillis - dtc_ext_step_start_ms >= DTC_EXT_REPLY_TIMEOUT_MS) {
        strncpy(dtc_ext_log_erase, "no response", sizeof(dtc_ext_log_erase) - 1);
        dtc_ext_log_erase[sizeof(dtc_ext_log_erase) - 1] = '\0';
        ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // restore poll template
        dtc_ext_state = DTC_EXT_IDLE;
      }
      break;

    case DTC_EXT_USER_SESSION_SENT:
      if (currentMillis - dtc_ext_step_start_ms >= DTC_EXT_SESSION_GAP_MS) {
        if (uq_mode == UQ_FREE && uq_req[0] == 0x2E) {
          uq_send_pre_read();  // write: read the identifier first, the write follows the answer
        } else {
          uq_send_request();
        }
        dtc_ext_step_start_ms = currentMillis;
        dtc_ext_state = DTC_EXT_USER_CMD_SENT;
      }
      break;
    case DTC_EXT_USER_CMD_SENT:
      if (currentMillis - dtc_ext_step_start_ms >= DTC_EXT_REPLY_TIMEOUT_MS) {
        if (uq_post_read) {
          memcpy(uq_result, uq_write_line, sizeof(uq_result));
          size_t pos = strlen(uq_result);
          uq_printf(uq_result, sizeof(uq_result), pos, " | read back: no response");
          uq_post_read = false;
          uq_restore_poll_template();
          dtc_ext_state = DTC_EXT_IDLE;
          break;
        }
        if (uq_mode == UQ_FREE && uq_req[0] == 0x2E) {
          snprintf(uq_result, sizeof(uq_result),
                   uq_pre_read ? "no response to the read before the write - nothing was written"
                               : "no response to the write - not known whether it was written, read the value again");
        } else {
          snprintf(uq_result, sizeof(uq_result), "%sno response", (uq_mode == UQ_FREE && uq_active_dc) ? "[DC] " : "");
          if (uq_to_scpu_row) {
            dtc_scpu_data.dtc_read_failed = true;
            dtc_scpu_data.dtc_last_read_millis = millis();
            uq_to_scpu_row = false;
          }
        }
        uq_pre_read = false;
        uq_in_progress = false;
        uq_restore_poll_template();
        dtc_ext_state = DTC_EXT_IDLE;
      }
      break;
  }
}

// Called once a positive ReadDTCInformation reply has been fully received (single- or multi-frame,
// see the two call sites above) - data/len already point past SID+subfunction+mask, i.e. straight at
// the repeated 4-byte DTC entries (3-byte code + 1-byte status), same layout UdsCanBattery's own
// (private, so not reusable from here) handle_dtc_response() expects for the non-KWP2000 case. Feeds
// the SAME `dtc` structure the existing "Diagnostic Trouble Codes" table on /advanced already renders
// (DTC/Status columns; Description stays "Unknown" - no renault_zoe_gen1_dtc.json exists, see 01.10.
// research) - this is the first time that table gets real data instead of "Not read yet".
void RenaultTwingoGen1Battery::handle_dtc_read_response(const uint8_t* data, uint16_t len) {
  if (dtc != nullptr) {
    fill_dtc_list(*dtc, data, len, (uint16_t)(len / 4));  // may exceed dtc_count if truncated at MAX_DTC_COUNT
  }
  snprintf(dtc_ext_log_read, sizeof(dtc_ext_log_read), "OK, %u DTC(s), %u bytes raw", (unsigned)(len / 4),
           (unsigned)len);
  ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // restore poll template
  dtc_ext_state = DTC_EXT_IDLE;
}

// Logs whatever came back, verbatim - same format as handle_nvrol_reply() (OK raw=.. / NEGATIVE SID=../
// short reply / unexpected multi-frame), reused here since the two state machines never run at once.
// Only used for Erase replies now (always tiny: "54" positive / "7F 14 NRC" negative, both single-frame
// - Read replies are handled by handle_dtc_read_response() above via the generic reassembly path).
void RenaultTwingoGen1Battery::handle_dtc_ext_reply(CAN_frame rx_frame) {
  char* log = dtc_ext_log_erase;  // only Erase routes here now, see the dispatch comment above
  uint8_t pci = rx_frame.data.u8[0];
  if (pci >= 0x10) {
    snprintf(log, 48, "unexpected multi-frame (PCI=0x%02X)", pci);
  } else if (pci < 3) {
    snprintf(log, 48, "short reply (%u bytes)", pci);
  } else if (rx_frame.data.u8[1] == 0x7F) {
    snprintf(log, 48, "NEGATIVE SID=0x%02X NRC=0x%02X", rx_frame.data.u8[2], rx_frame.data.u8[3]);
  } else {
    snprintf(log, 48, "OK raw=%02X %02X %02X %02X %02X %02X %02X", rx_frame.data.u8[1], rx_frame.data.u8[2],
             rx_frame.data.u8[3], rx_frame.data.u8[4], rx_frame.data.u8[5], rx_frame.data.u8[6], rx_frame.data.u8[7]);
  }
  ZOE_POLL_18DADBF1.data = {0x03, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // restore poll template
  dtc_ext_state = DTC_EXT_IDLE;  // reply seen - no need to wait out the timeout
}
