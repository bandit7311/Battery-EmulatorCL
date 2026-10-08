#include "simulator_html.h"
#include <Arduino.h>
#include "../../battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../datalayer/datalayer_extended.h"

// Groups rows by interval_ms for display, in signal-table order within each group - no sorting needed
// since the table itself is already laid out 10/20/100/1000ms-grouped (see the header comment there).
static void append_signal_row(String& content, uint8_t i) {
  const RenaultTwingoGen1Battery::SimSignal& s = RenaultTwingoGen1Battery::sim_signals[i];
  bool checked = RenaultTwingoGen1Battery::sim_row_enabled(i);

  content += "<tr><td><input type='checkbox' id='sim" + String(i) + "' " + (checked ? "checked " : "") +
             "onchange=\"fetch('/editTwingoSimSignal?index=" + String(i) + "&value='+(this.checked?1:0))\"></td>";

  content += "<td>0x" + String(s.id, HEX) + "</td>";

  const char* tagClass = (s.tag == 'I') ? "tag-i" : (s.tag == 'P') ? "tag-p" : "tag-a";
  content += "<td><span class='" + String(tagClass) + "'>" + String(s.tag) + "</span></td>";

  content += "<td>" + String(s.interval_ms) + " ms</td>";

  // "X" = this ID never occurs in the real vehicle log (canmitlog.log, 02.10.)
  content += s.not_in_vehicle_log ? "<td><span class='tag-x' title='never seen in the real vehicle log'>X</span></td>"
                                  : "<td></td>";

  content += "<td>" + String(s.label);
  if (s.bms_origin) {
    content +=
        " <span class='note'>(normally sent by the BMS itself, not the EVC - no technical lock, just a "
        "note)</span>";
  }
  if (s.tag == 'I') {
    content += " <span class='note'>(sent for real by this driver - this checkbox switches that sending)</span>";
  }
  content += "</td><td>" + String(s.sender) + "</td><td class='info'>" + String(s.info) + "</td></tr>";
}

String simulator_processor(const String& var) {
  if (var == "X") {
    String content = "";
    content += "<style>";
    content += "body { background-color: black; color: white; font-family: sans-serif; }";
    content += "table { border-collapse: collapse; width: 1200px; max-width: 97vw; }";
    content += "td, th { border: 1px solid #444; padding: 4px 8px; text-align: left; }";
    content += "th { text-align: center; }";
    content += ".tag-i { color: #6fcf6f; font-weight: bold; }";
    content += ".tag-p { color: #ffd479; font-weight: bold; }";
    content += ".tag-a { color: #ff9b9b; font-weight: bold; }";
    content += ".note { color: #999; font-size: 0.85em; }";
    content += ".tag-x { color: #ff5c5c; font-weight: bold; }";
    content += ".info { color: #bbb; font-size: 0.9em; }";
    content += "h3 { margin-top: 24px; margin-bottom: 6px; }";
    content += "</style>";

    content += "<h2>CAN Signal Simulator</h2>";
    content +=
        "<p>35 cyclic signals. Each checkbox is independent and switches exactly that signal on or off, "
        "including the 10 signals this driver sends by itself (I). Content comes from real captures "
        "(Log_Twingo_Ladung.log, canmitlog.log), not invented. The sender is the ECU named in the CanZE table "
        "(not verified for the Twingo), \"meaning unknown\" means exactly that.</p>";
    content +=
        "<p><b>Legend:</b> <span class='tag-i'>I</span> = Installed, already sent for real by this "
        "driver &nbsp; <span class='tag-p'>P</span> = Planned, content/meaning from the real log, not yet "
        "sent &nbsp; <span class='tag-a'>A</span> = Assumed, content from the real log but meaning "
        "unconfirmed &nbsp; <span class='tag-x'>X</span> = this ID never occurs in the real Twingo vehicle log "
        "(canmitlog.log, 02.10.)</p>";

    content +=
        "<p>&#9745; = on by default (these are the 10 signals this driver already sent before the simulator existed) "
        "&nbsp; &#9744; = off by default (needs a deliberate click)</p>";

    // Steady 0x350 frame: C7 (like the vehicle while ready to drive) or C3 (old value), runtime only, not saved.
    content += "<p><b>0x350 steady frame:</b> <label><input type='radio' name='steady350' id='steady350c7' ";
    content += RenaultTwingoGen1Battery::steady_350_use_c3 ? "" : "checked ";
    content += "onclick=\"fetch('/editTwingoSteady350?value=0')\"> C7 (like the car, default)</label> &nbsp; ";
    content += "<label><input type='radio' name='steady350' id='steady350c3' ";
    content += RenaultTwingoGen1Battery::steady_350_use_c3 ? "checked " : "";
    content += "onclick=\"fetch('/editTwingoSteady350?value=1')\"> C3 (old value, for diagnostic tests)</label> ";
    content +=
        "<span class='note'>- runtime only, back to C7 after a restart; the checkbox of row 0x350 below switches the "
        "frame on or off, sleep and wake-up send their own 0x350</span></p>";

    // Switch A (04.10.): shutdown sequence, wake-up, 0x214 and the end of the rows "as before" or "like the car".
    content +=
        "<p><b>Shutdown, wake-up, 0x214 and end of the rows:</b> <label><input type='radio' name='carmode' "
        "id='carmode0' ";
    content += RenaultTwingoGen1Battery::shutdown_like_car ? "" : "checked ";
    content += "onclick=\"fetch('/editTwingoCarMode?value=0')\"> as before (default)</label> &nbsp; ";
    content += "<label><input type='radio' name='carmode' id='carmode1' ";
    content += RenaultTwingoGen1Battery::shutdown_like_car ? "checked " : "";
    content += "onclick=\"fetch('/editTwingoCarMode?value=1')\"> like the car (vehicle log of 04.10.)</label> ";
    content +=
        "<span class='note'>- runtime only, back to \"as before\" after a restart. \"Like the car\" changes the stage "
        "times and 0x350 bytes of the shutdown (C3 63.2 s, 00 0.9 s), the wake-up (12 steps, about 10 s), sends "
        "0x214 also while awake (FB FE, F8 3E, 08 02, every 20 ms) and lets every row end where it ends in the "
        "car (fast frames at C0, others at 00, some until the end of the bus). Every row below stays "
        "individually switchable.</span></p>";

    // Vehicle age (point 17): automatic from the pack reference, or a manual final value.
    {
      const auto& tg = datalayer_extended.twingoGen1;
      content += "<p><b>Vehicle age (0x350 bytes 1-3, 0x523, 0x376):</b> ";
      if (tg.age_source != 0) {
        content += String(tg.age_last_sent) + " min (";
        content +=
            tg.age_source == 3 ? "manual" : (tg.age_source == 2 ? "automatic, raised from the pack" : "automatic");
        content += ")";
      } else {
        content += "none yet (nothing is sent without an age)";
      }
      content +=
          " <span class='note'>- automatic: reference " + String(tg.age_pack_value) + " min at Unix " +
          String(tg.age_pack_unix) +
          " counted +1 per minute by the clock, plus one day safety lead once; raised only when the pack reports "
          "more than is sent. Without a valid clock (NTP) no age is sent. Persisted.</span></p>";
    }

    // Manual vehicle age: own final value in minutes, replaces the automatic mode while set.
    content += "<p><b>Manual vehicle age:</b> <input type='number' id='ageManual' min='0' max='16777215' value='";
    content += RenaultTwingoGen1Battery::age_manual_active ? String(RenaultTwingoGen1Battery::age_manual_start_min)
                                                           : String("1314935");
    content += "' style='width:9em'> minutes <button onclick=\"ageManualSet()\">Set</button> ";
    content += "<button onclick=\"ageManualOff()\">Off</button> <span id='ageManualState'>";
    content += RenaultTwingoGen1Battery::age_manual_active ? "ACTIVE - started at the value shown, +1 per minute"
                                                           : "off - automatic mode";
    content +=
        "</span> <span class='note'>- the typed value is final (no safety lead added), used in all age frames, "
        "persisted. 24 bit: 0 to 16777215.</span></p>";
    content += "<script>function ageManualSet(){var v=document.getElementById('ageManual').value;";
    content += "fetch('/editTwingoAgeManual?value='+encodeURIComponent(v)).then(function(r){return r.text();})";
    content += ".then(function(t){document.getElementById('ageManualState').textContent=";
    content += "(t==='OK')?'ACTIVE - started at '+v+', +1 per minute':t;});}";
    content += "function ageManualOff(){fetch('/editTwingoAgeManual?value=off').then(function(){";
    content += "document.getElementById('ageManualState').textContent='off - automatic mode';});}";
    content += "</script>";

    // Time in the Zoe frame 0x436 (bytes 1-3), the bench SCPU seems to take its $9261 from there.
    content += "<p><b>Time in 0x436 (bytes 1-3):</b> <input type='number' id='t436' min='0' max='16777215' value='";
    {
      const uint32_t shown =
          RenaultTwingoGen1Battery::time_436_active
              ? RenaultTwingoGen1Battery::time_436_value
              : (datalayer_extended.twingoGen1.age_last_sent != 0 ? datalayer_extended.twingoGen1.age_last_sent
                                                                  : 1311344UL);
      content += String(shown);
    }
    content += "' style='width:9em'> minutes <button onclick=\"t436Set()\">Set</button> ";
    content += "<button onclick=\"t436Off()\">Off</button> <span id='t436State'>";
    content += RenaultTwingoGen1Battery::time_436_active ? "ACTIVE - counts +1 per minute" : "off - default 14 00 xx";
    content +=
        "</span> <span class='note'>- runtime only. The row 0x436 must be on. Default: 14 00 xx (xx = minutes since "
        "the start of the emulator).</span></p>";
    content += "<p><label><input type='checkbox' id='t436follow' ";
    content += RenaultTwingoGen1Battery::time_436_follow_age ? "checked " : "";
    content +=
        "onclick=\"fetch('/editTwingoTime436Follow?value='+(this.checked?1:0))\"> 0x436 carries the vehicle age "
        "(stored)</label> <span class='note'>- the same age as 0x350 (bytes 1-3), also after a restart. Without a "
        "valid "
        "age (no clock) no 0x436 is sent. A value set above by hand wins over it.</span></p>";
    content += "<p><button onclick=\"fetch('/twingoSimAllOff')\">All rows off at once</button> ";
    content += "<button onclick=\"fetch('/twingoSimRestore')\">Rows back as before</button> ";
    content +=
        "<span class='note'>- runtime only, not stored. Every frame (also 0x350) ends at once, without the shutdown "
        "sequence. Reload the page to see the checkboxes.</span></p>";
    content += "<script>function t436Set(){var v=document.getElementById('t436').value;";
    content += "fetch('/editTwingoTime436?value='+encodeURIComponent(v)).then(function(r){return r.text();})";
    content += ".then(function(t){document.getElementById('t436State').textContent=(t==='OK')?";
    content += "'ACTIVE - started at '+v+', +1 per minute':t;});}";
    content += "function t436Off(){fetch('/editTwingoTime436?value=off').then(function(){";
    content += "document.getElementById('t436State').textContent='off - default 14 00 xx';});}</script>";

    // Odometer (runtime only): 0x5D7 (row 0x5D7) and the Zoe frame 0x426.
    content += "<p><b>Odometer 0x5D7:</b> <input type='number' id='odo5d7' min='0' max='2684354' value='";
    content += String(RenaultTwingoGen1Battery::odo_5d7_km);
    content += "' style='width:8em'> km <button onclick=\"odoSet('5d7')\">Set</button> &nbsp; ";
    content += "<b>0x426 (Zoe frame):</b> <input type='number' id='odo426' min='0' max='65535' value='";
    content += String(RenaultTwingoGen1Battery::odo_426_km);
    content += "' style='width:6em'> km, byte 7 <input type='number' id='odo426b7' min='0' max='255' value='";
    content += String(RenaultTwingoGen1Battery::odo_426_b7);
    content += "' style='width:4em'> <button onclick=\"odoSet('426')\">Set</button> <span id='odoState'></span> ";
    content +=
        "<span class='note'>- runtime only. 0x426 holds km * 256 in 24 bit, so at most 65,535 km, and it triggers "
        "E14381; 0x5D7 reaches 2,684,354 km. Defaults 19,400 km, byte 7 = 64 (0x40).</span></p>";
    content += "<script>function odoSet(w){var u='/editTwingoOdo?which='+w+'&km='+encodeURIComponent(";
    content += "document.getElementById(w=='5d7'?'odo5d7':'odo426').value);";
    content += "if(w=='426'){u+='&b7='+encodeURIComponent(document.getElementById('odo426b7').value);}";
    content += "fetch(u).then(function(r){return r.text();}).then(function(t){";
    content += "document.getElementById('odoState').textContent=t;});}</script>";

    content +=
        "<table><thead><tr><th>On</th><th>ID</th><th>Tag</th><th>Interval</th><th>In car log</th><th>Signal</th>"
        "<th>Sender</th><th>Meaning</th></tr></thead><tbody>";

    // Display grouped by interval (10/20/100/1000ms), even though the underlying table/NVM bit order is
    // not sorted that way - a simple stable selection sort over interval_ms, operating on original
    // indices only, so checkbox IDs/bit positions (fetch('/editTwingoSimSignal?index=N...')) stay the
    // fixed meaning they have everywhere else (sim_signals[N], bit N of simulator_enabled_mask).
    uint8_t order[RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT];
    for (uint8_t i = 0; i < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; i++) {
      order[i] = i;
    }
    for (uint8_t a = 0; a < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT - 1; a++) {
      for (uint8_t b = 0; b < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT - 1 - a; b++) {
        if (RenaultTwingoGen1Battery::sim_signals[order[b]].interval_ms >
            RenaultTwingoGen1Battery::sim_signals[order[b + 1]].interval_ms) {
          uint8_t t = order[b];
          order[b] = order[b + 1];
          order[b + 1] = t;
        }
      }
    }

    uint16_t lastInterval = 0;
    for (uint8_t k = 0; k < RenaultTwingoGen1Battery::SIM_SIGNAL_COUNT; k++) {
      uint8_t i = order[k];
      uint16_t iv = RenaultTwingoGen1Battery::sim_signals[i].interval_ms;
      if (iv != lastInterval) {
        content += "</tbody></table><h3>" + String(iv) + " ms</h3><table><tbody>";
        lastInterval = iv;
      }
      append_signal_row(content, i);
    }
    content += "</tbody></table>";

    bool driveMode = datalayer_extended.twingoGen1.sim_55d_drive_mode_enabled;
    content += "<h3 style='color:#ff9b9b;'>EXPERIMENTAL - unverified</h3>";
    content +=
        "<p>Only affects the 0x55D row above, and only while its own checkbox is also on. Content is NOT "
        "from a real log capture - driving was never recorded. Use at your own risk.</p>";
    content += "<label><input type='checkbox' id='sim55dDrive' " + String(driveMode ? "checked " : "") +
               "onchange=\"fetch('/editTwingoSim55dDriveMode?value='+(this.checked?1:0))\"> "
               "Send 0x55D as 'drive/discharge active' instead of the normal content</label>";
    content += "<table><tbody>";
    content += "<tr><td>Data</td><td colspan='2'>05 FD F0 01 00 00 00 81</td></tr>";
    content += "<tr><td>Byte 0</td><td>0x05</td><td>EVC_State = Drive Active (claimed)</td></tr>";
    content += "<tr><td>Byte 1</td><td>0xFD</td><td>HV_Enable (claimed)</td></tr>";
    content += "<tr><td>Byte 2</td><td>0xF0</td><td>Contactor_Cmd_Inverter, precharge done (claimed)</td></tr>";
    content += "<tr><td>Byte 3</td><td>0x01</td><td>Discharge_Enable (claimed)</td></tr>";
    content += "<tr><td>Byte 4-6</td><td>00 00 00</td><td>Reserve (claimed)</td></tr>";
    content += "<tr><td>Byte 7</td><td>0x81</td><td>Interlock_OK (claimed)</td></tr>";
    content += "</tbody></table>";

    bool restActive = datalayer_extended.twingoGen1.sim_55d_rest_active_enabled;
    content +=
        "<h3 style='color:#ff9b9b;'>EXPERIMENTAL - rest state &amp; staged relay closing</h3>"
        "<p>Off: 0x55D continuously sends a GUESSED \"rest/idle\" content (byte 0 = 0x01). Ticking this "
        "box starts a staged sequence - byte 0 steps 0x02 (\"Precharge\", GUESSED) &rarr; 0x04 (\"Main "
        "relay closing\", GUESSED) &rarr; settles on the normal/drive content above (REAL), 200ms per "
        "stage (GUESSED timing, no real log data). Unticking reverts to rest immediately. Only works "
        "while the 0x55D row checkbox is also on.</p>";
    content += "<label><input type='checkbox' id='sim55dRestActive' " + String(restActive ? "checked " : "") +
               "onchange=\"fetch('/editTwingoSim55dRestActive?value='+(this.checked?1:0))\"> "
               "Active (precharge &amp; close main relay)</label>";

    content += "<p><a href='/advanced' style='color:#8fd3ff;'>Back to More Battery Info</a></p>";
    return content;
  }
  return String();
}
