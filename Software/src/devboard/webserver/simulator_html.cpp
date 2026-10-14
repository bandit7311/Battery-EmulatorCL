#include "simulator_html.h"
#include <Arduino.h>
#include "../../battery/RENAULT-TWINGO-GEN1-BATTERY.h"
#include "../../battery/RENAULT-TWINGO-GEN1-BUS.h"
#include "../../datalayer/datalayer_extended.h"

// Page layout (10.10.): the frames of the LBC<>EVC bus (all 50 IDs, grouped by cycle time; since Bau 13, 14.10., the old
// block 2 with the rows of the vehicle CAN is gone), then the other received IDs. LED / "first seen" cells are filled by
// the script at the end of the page from the small JSON of /twingoRxStatus (1 s poll). The page must contain no percent
// sign (template engine), see the tests.
using Bat = RenaultTwingoGen1Battery;

static String interval_name(uint16_t ms) {
  if (ms >= 1000) {
    return String(ms / 1000) + " s";
  }
  return String(ms) + " ms";
}

static String hex3(uint32_t id) {
  char b[12];
  snprintf(b, sizeof(b), "0x%03X", (unsigned)id);
  return String(b);
}

// LED + "first seen" cells; key = "r<row>" or "c<cell frame>", cycle in ms (data-c).
static void append_rx_cells(String& content, const String& key, uint16_t cycle_ms) {
  content += "<td><span class='led' id='L" + key + "' data-c='" + String(cycle_ms) + "'></span></td>";
  content += "<td class='note' id='F" + key + "'>&ndash;</td>";
}

static void append_checkbox(String& content, uint8_t i) {
  content += "<td><input type='checkbox' id='sim" + String(i) + "' " + (Bat::sim_row_enabled(i) ? "checked " : "") +
             "onchange=\"fetch('/editTwingoSimSignal?index=" + String(i) + "&value='+(this.checked?1:0))\"></td>";
}

// Block 1 row: On | LED | first seen | ID | cycle | type | bench measurement | content | source
static void append_bus_row(String& content, uint8_t i) {
  const Bat::SimSignal& s = Bat::sim_signals[i];
  const twingo_bus::FrameDef* def = twingo_bus::find_frame((uint16_t)s.id);
  content += "<tr id='Rr" + String(i) + "'>";
  append_checkbox(content, i);
  append_rx_cells(content, "r" + String(i), s.interval_ms);
  content += "<td>" + hex3(s.id) + "</td><td>" + interval_name(s.interval_ms) + "</td>";
  const char* tagClass = (s.tag == 'I') ? "tag-i" : "tag-r";
  const char tag_text[2] = {s.tag, 0};
  content += "<td><span class='" + String(tagClass) + "'>" + String(tag_text) + "</span></td>";
  content += (def != nullptr && def->from_pack) ? "<td><span class='ja'>Akku</span></td>" : "<td>&ndash;</td>";
  content += "<td>" + String(s.label) + " <span class='info'>" + String(s.info) + "</span></td><td>" +
             String(s.sender) + "</td></tr>";
}

// All rows grouped by cycle time (ascending), stable within a group.
static uint8_t sorted_rows(uint8_t* order) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < Bat::SIM_SIGNAL_COUNT; i++) {
    order[n++] = i;
  }
  for (uint8_t a = 0; a + 1 < n; a++) {
    for (uint8_t b = 0; b + 1 < n - a; b++) {
      if (Bat::sim_signals[order[b]].interval_ms > Bat::sim_signals[order[b + 1]].interval_ms) {
        uint8_t t = order[b];
        order[b] = order[b + 1];
        order[b + 1] = t;
      }
    }
  }
  return n;
}

static void append_group_row(String& content, uint16_t ms, uint8_t cols, const char* extra) {
  content +=
      "<tr><td colspan='" + String(cols) + "' class='g'><b>" + interval_name(ms) + "</b> " + extra + "</td></tr>";
}

// The 20 cell frames: LED only (no sending), 5 cells per frame in the order of the broadcast.
static void append_cell_frame_rows(String& content, uint8_t cols) {
  content += "<tr><td colspan='" + String(cols) +
             "' class='g'><b>Zellframes</b> <span class='note'>nur LED, kein "
             "Senden (5 Zellen je Frame)</span></td></tr>";
  for (uint8_t k = 0; k < Bat::RX_CELL_ID_COUNT; k++) {
    content += "<tr id='Rc" + String(k) + "'><td></td>";
    append_rx_cells(content, "c" + String(k), 3000);  // the cell frames come about every 3 s
    const unsigned first = k * 5 + 1u;
    const unsigned last = (k == 19) ? 96u : first + 4u;
    content += "<td>" + hex3(Bat::RX_CELL_IDS[k]) + "</td><td>&ndash;</td><td><span class='tag-r'>Z</span></td>";
    content += "<td><span class='ja'>Akku</span></td><td>Zellen " + String(first) + "&ndash;" + String(last) +
               "</td><td>Akku</td></tr>";
  }
}

String simulator_processor(const String& var) {
  const bool all = (var == "X");  // the whole page in one piece (tests); the web server asks for the parts A, B and D
  const bool part_a = all || var == "A", part_b = all || var == "B", part_d = all || var == "D";
  if (part_a || part_b || part_d) {
    String content = "";
    content.reserve(all ? 45000 : (part_a ? 10000 : (part_b ? 26000 : 6000)));
    if (part_a) {
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
      content +=
          ".led { display: inline-block; width: 14px; height: 14px; border-radius: 50px; border: 1px solid #888; ";
      content += "background: #666; }";
      content += ".ja { color: #6fcf6f; font-weight: bold; } .nein { color: #ff9b9b; }";
      content += ".tag-r { color: #8fd3ff; font-weight: bold; } .g { background: #1a1a1a; }";
      content += "tr.new td { background: #fff8b0; color: #000; }";
      content += ".bar { position: sticky; top: 0; background: #222; border: 1px solid #888; padding: 6px 12px; ";
      content += "margin: 6px 0; max-width: 1200px; z-index: 5; }";
      content += "</style>";

      content += "<div class='bar'><b>Jetzt: <span id='tNow'>T+?</span></b> &nbsp; ";
      content += "<span class='note'>Uhr (NTP): <span id='tClock'>&ndash;</span></span> &nbsp; | &nbsp; ";
      content += "Letzte Aktion: <b id='tAct'>&ndash;</b> ";
      content += "<span class='note'>(Zeile hellgelb, solange \"Erstmals gesehen\" j&uuml;nger als 15 s)</span></div>";
      content += "<h2>CAN Signal Simulator</h2>";
      content +=
          "<p>Block 1 sendet die Frames des BMS&harr;EVC-Busses wie im Fahrzeug (Format, Takt, Inhalt je Zustand, "
          "Zaehler und Pruefsummen wo bekannt). Jede Checkbox schaltet genau ihre Zeile. Inhalte stammen aus echten "
          "Mitschnitten, nichts erfunden; "
          "\"meaning unknown\" heisst genau das.</p>";
      content +=
          "<p><b>LED:</b> gruen = ein Frame mit dieser ID kam von aussen (Akku), grau = nicht; eigene Sendungen "
          "zaehlen nicht. Schwelle 3 Zyklen, mindestens 2 s, 3-s-Frames 10 s. &nbsp; <b>Typ:</b> "
          "<span class='tag-i'>I</span> = wird vom Treiber gesendet (Zoe-Form), <span class='tag-r'>R</span> = Frame "
          "des "
          "Busses.</p>";

      // Controls for the bus mode (10.10.)
      {
        const auto& tg = datalayer_extended.twingoGen1;
        content += "<div class='box'><button onclick=\"fetch('/twingoSimAllOff')\">Alle Zeilen aus</button> ";
        content +=
            "<span class='note'>T+0 = letzter Druck auf \"Alle Zeilen aus\" (setzt alle \"Erstmals gesehen\" "
            "zur&uuml;ck)</span> ";
        content += "<button onclick=\"fetch('/twingoSimRestore')\">Zeilen wie vorher</button> ";
        content += "<span class='note'>- nur Laufzeit, Seite neu laden fuer die Haken.</span><br>";
        content += "<b>Format der BMS&harr;EVC-Frames (0x423, 0x426, 0x436, 0x19F, 0x69F):</b> ";
        content += "<label><input type='radio' name='busfmt' id='busfmt0' ";
        content += tg.bus_format_zoe_old ? "" : "checked ";
        content += "onclick=\"fetch('/twingoBusFormat?value=0')\"> Fahrzeug (Standard)</label> ";
        content += "<label><input type='radio' name='busfmt' id='busfmt1' ";
        content += tg.bus_format_zoe_old ? "checked " : "";
        content += "onclick=\"fetch('/twingoBusFormat?value=1')\"> Zoe alt</label><br>";
        content += "<b>Betriebsart (Ziel der automatischen Folge):</b> ";
        {
          static const char* const TGT[twingo_bus::TARGET_COUNT] = {"Fahren", "Laden", "Schlafen"};
          for (uint8_t k = 0; k < twingo_bus::TARGET_COUNT; k++) {
            content += "<label><input type='radio' name='bustgt' id='bustgt" + String(k) + "' ";
            content += (tg.bus_wake_target == k) ? "checked " : "";
            content += "onclick=\"fetch('/twingoBusTarget?value=" + String(k) + "')\"> " + TGT[k] + "</label> ";
          }
        }
        content +=
            "<span class='note'>- gespeichert; Fahren und Laden bestimmen, wo das Aufwecken endet (Laden: Aufwecken "
            "&uuml;ber den Stecker mit Zustand A0/41), Schlafen hat noch keine eigene Folge</span><br>";
        content += "<b>Einschlafweg (Sleep, Sleep 0x9281=1, NVROL reset):</b> ";
        {
          static const char* const PTH[twingo_bus::SLEEP_PATH_COUNT] = {
              "lang (Fahrlog Sitzung 1)", "kurz (Fahrlog Sitzung 3)", "Ladelog 2 (vollst&auml;ndig gemessen)"};
          for (uint8_t k = 0; k < twingo_bus::SLEEP_PATH_COUNT; k++) {
            content += "<label><input type='radio' name='slpath' id='slpath" + String(k) + "' ";
            content += (tg.bus_sleep_path == k) ? "checked " : "";
            content += "onclick=\"fetch('/twingoSleepPath?value=" + String(k) + "')\"> " + PTH[k] + "</label> ";
          }
        }
        content +=
            "<span class='note'>- gespeichert; Ladelog 2 endet mit 00/02 f&uuml;r 11,1 s und 0x157/0x0ED als letzten "
            "Frames</span><br>";
        const uint8_t shown = tg.bus_state_manual ? tg.bus_state : Bat::bus_state_shown;
        content += "<b>Zustand:</b> <select id='busstate' ";
        content += tg.bus_state_manual ? "" : "disabled ";
        content += "onchange=\"fetch('/twingoBusState?value='+this.value)\">";
        for (uint8_t k = 0; k < twingo_bus::BUS_STATE_COUNT; k++) {
          content += "<option value='" + String(k) + "'" + (shown == k ? " selected" : "") + ">" +
                     String(twingo_bus::state_name(k)) + "</option>";
        }
        content += "</select> <label><input type='checkbox' id='busmanual' ";
        content += tg.bus_state_manual ? "checked " : "";
        content +=
            "onchange=\"fetch('/twingoBusManual?value='+(this.checked?1:0));"
            "document.getElementById('busstate').disabled=!this.checked;\"> manuell</label> ";
        content +=
            "<span class='note'>- aus: der Zustand l&auml;uft von selbst (nach Start und Aufwecken von zu bis driving, "
            "bei Sleep/NVROL zur&uuml;ck bis zu); an: das Feld bestimmt den Zustand. Bestimmt den Inhalt der "
            "Block-1-Zeilen.</span><br>";
        content += "<b>0x69F gesendet:</b> <span id='tx69f'>" + String((unsigned long)Bat::tx_69f_count) +
                   "</span> Frames (Fahrzeug-ID <code>46 13 88 6F</code>, 1 s)<br>";
        content += "<b>Zellwerte-Quelle:</b> <label><input type='radio' name='cellsrc' id='cellsrc0' ";
        content += tg.cell_source_broadcast ? "" : "checked ";
        content += "onclick=\"fetch('/twingoCellSource?value=0')\"> UDS (heute)</label> ";
        content += "<label><input type='radio' name='cellsrc' id='cellsrc1' ";
        content += tg.cell_source_broadcast ? "checked " : "";
        content += "onclick=\"fetch('/twingoCellSource?value=1')\"> Broadcast (20 Zellframes)</label> ";
        content +=
            "<span class='note'>- die Werte gehen in beiden Faellen unveraendert zum Wechselrichter</span></div>";
      }

      // Switch A (04.10.): stage times of the shutdown sequence and of the wake-up "as before" or "like the car".
      content +=
          "<p><b>Shutdown and wake-up (stage times):</b> <label><input type='radio' name='carmode' "
          "id='carmode0' ";
      content += RenaultTwingoGen1Battery::shutdown_like_car ? "" : "checked ";
      content += "onclick=\"fetch('/editTwingoCarMode?value=0')\"> as before (default)</label> &nbsp; ";
      content += "<label><input type='radio' name='carmode' id='carmode1' ";
      content += RenaultTwingoGen1Battery::shutdown_like_car ? "checked " : "";
      content += "onclick=\"fetch('/editTwingoCarMode?value=1')\"> like the car (vehicle log of 04.10.)</label> ";
      content +=
          "<span class='note'>- kept over a restart. \"Like the car\" changes the stage times of the shutdown (C3 63.2 "
          "s, "
          "00 0.9 s) and the wake-up (12 steps, about 10 s); the frames of the bus have their own times (way of the "
          "sleep above).</span></p>";

      // Vehicle age (point 17): automatic from the pack reference, or a manual final value.
      {
        const auto& tg = datalayer_extended.twingoGen1;
        content += "<p><b>Vehicle age (0x436, bytes 1-3):</b> ";
        if (tg.age_source != 0) {
          content += String(tg.age_last_sent) + " min (";
          content +=
              tg.age_source == 3 ? "manual" : (tg.age_source == 2 ? "automatic, raised from the pack" : "automatic");
          content += ")";
        } else {
          content += "none yet (nothing is sent without an age)";
        }
        content += " <span class='note'>- automatic: reference " + String(tg.age_pack_value) + " min at Unix " +
                   String(tg.age_pack_unix) +
                   " counted +1 per minute by the clock, no safety lead and never raised from the pack (since 10.10.). "
                   "Without a valid clock (NTP) no age is sent. Persisted.</span></p>";
      }

      // Manual vehicle age: own final value in minutes, replaces the automatic mode while set.
      content += "<p><b>Manual vehicle age:</b> <input type='number' id='ageManual' min='0' max='16777215' value='";
      content += RenaultTwingoGen1Battery::age_manual_active ? String(RenaultTwingoGen1Battery::age_manual_start_min)
                                                             : String("1025301");
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
                                                                    : 1025301UL);
        content += String(shown);
      }
      content += "' style='width:9em'> minutes <button onclick=\"t436Set()\">Set</button> ";
      content += "<button onclick=\"t436Off()\">Off</button> <span id='t436State'>";
      content += RenaultTwingoGen1Battery::time_436_active ? "ACTIVE - counts +1 per minute" : "off - default 14 00 xx";
      content +=
          "</span> <span class='note'>- stored, comes back after a restart. The row 0x436 must be on. Default: 14 00 "
          "xx (xx = minutes since "
          "the start of the emulator).</span></p>";
      content += "<p><label><input type='checkbox' id='t436follow' ";
      content += RenaultTwingoGen1Battery::time_436_follow_age ? "checked " : "";
      content +=
          "onclick=\"fetch('/editTwingoTime436Follow?value='+(this.checked?1:0))\"> 0x436 carries the vehicle age "
          "(stored)</label> <span class='note'>- the vehicle age, also after a restart. Without a "
          "valid "
          "age (no clock) no 0x436 is sent. A value set above by hand wins over it.</span></p>";
      content += "<p><button onclick=\"fetch('/twingoSimAllOff')\">All rows off at once</button> ";
      content += "<button onclick=\"fetch('/twingoSimRestore')\">Rows back as before</button> ";
      content +=
          "<span class='note'>- runtime only, not stored. Every frame ends at once, without the shutdown "
          "sequence. Reload the page to see the checkboxes.</span></p>";
      content += "<script>function t436Set(){var v=document.getElementById('t436').value;";
      content += "fetch('/editTwingoTime436?value='+encodeURIComponent(v)).then(function(r){return r.text();})";
      content += ".then(function(t){document.getElementById('t436State').textContent=(t==='OK')?";
      content += "'ACTIVE - started at '+v+', +1 per minute':t;});}";
      content += "function t436Off(){fetch('/editTwingoTime436?value=off').then(function(){";
      content += "document.getElementById('t436State').textContent='off - default 14 00 xx';});}</script>";

      // Odometer (stored since 11.10.): the frame 0x426.
      content += "<p><b>0x426:</b> <input type='number' id='odo426' min='0' max='16777215' value='";
      content += String(RenaultTwingoGen1Battery::odo_426_km);
      content += "' style='width:6em'> km, byte 7 <input type='number' id='odo426b7' min='0' max='255' value='";
      content += String(RenaultTwingoGen1Battery::odo_426_b7);
      content += "' style='width:4em'> <button onclick=\"odoSet('426')\">Set</button> <span id='odoState'></span> ";
      content +=
          "<span class='note'>- stored, comes back after a restart. 0x426 holds the km in 24 bit (bytes 3-5) in the "
          "vehicle form, the old Zoe form only up to 65,535 km and it triggers E14381. Defaults: 6,844 km (pack value "
          "91CF), byte 7 = 64 (0x40).</span></p>";
      content += "<script>function odoSet(w){var u='/editTwingoOdo?which='+w+'&km='+encodeURIComponent(";
      content += "document.getElementById('odo426').value);";
      content += "u+='&b7='+encodeURIComponent(document.getElementById('odo426b7').value);";
      content += "fetch(u).then(function(r){return r.text();}).then(function(t){";
      content += "document.getElementById('odoState').textContent=t;});}</script>";
    }
    if (part_b) {
      // ---- Block 1: the LBC<>EVC bus ----
      content += "<h3>Block 1: ALLE SIMULATIONSWERTE wie im FAHRZEUG &ndash; BMS&harr;EVC-Bus (50 Adressen)</h3>";
      content +=
          "<p class='note'>Spalte \"Bench-Messung\": <span class='ja'>Akku</span> = diese ID sendet der Akku allein "
          "(Log 25.09., unsere fuenf Frames lagen an), \"&ndash;\" = vom Akku nicht gesendet. Auch die "
          "vermeintlichen Akku-Frames lassen sich senden, da wir nichts sicher wissen; Vorsicht bei angeschlossenem "
          "echten Akku.</p>";
      content +=
          "<table><thead><tr><th>An</th><th>LED</th><th>Erstmals gesehen</th><th>ID</th><th>Takt</th><th>Typ</th>"
          "<th>Bench-Messung</th><th>Inhalt</th><th>Quelle</th></tr></thead><tbody>";
      {
        uint8_t order[Bat::SIM_SIGNAL_COUNT];
        const uint8_t n = sorted_rows(order);
        static const uint16_t GROUPS[7] = {10, 20, 100, 200, 500, 1000, 3000};
        for (uint8_t g = 0; g < 7; g++) {
          bool any = false;
          for (uint8_t k = 0; k < n; k++) {
            if (Bat::sim_signals[order[k]].interval_ms == GROUPS[g]) {
              if (!any) {
                append_group_row(content, GROUPS[g], 9, "");
                any = true;
              }
              append_bus_row(content, order[k]);
            }
          }
          if (!any) {
            append_group_row(content, GROUPS[g], 9, "<span class='note'>keine Frames</span>");
          }
        }
        append_cell_frame_rows(content, 9);
      }
      content +=
          "</tbody></table><p class='note'>Typ I = wird heute vom Treiber gesendet (Zoe-Form), R = neu, Z = "
          "Zellframe (nur LED).</p>";
    }
    if (part_d) {
      content += "<h3>Weitere empfangene IDs (nicht in der Liste)</h3>";
      content +=
          "<p class='note'>Alles, was am Bench ankommt und nicht in der Liste oben steht, mit Anzahl und letzter "
          "Zeit.</p>";
      content +=
          "<table><thead><tr><th>ID</th><th>Erstmals gesehen</th><th>Zuletzt vor</th><th>Anzahl</th></tr></thead>"
          "<tbody id='othRows'></tbody></table>";

      // Poll script: no percent sign anywhere (template engine).
      content += "<script>";
      content += "function T(ms){return 'T'+(ms<0?'-':'+')+(Math.abs(ms)/1000).toFixed(2).replace('.',',')+' s';}";
      content += "function clk(u,now,ms){if(!u){return '';}var d=new Date((u-(now-ms)/1000)*1000);";
      content += "return d.toLocaleTimeString();}";
      content += "function led(key,e,now){var l=document.getElementById('L'+key);if(!l){return;}";
      content += "var f=document.getElementById('F'+key);var tr=document.getElementById('R'+key);";
      content += "var c=parseInt(l.getAttribute('data-c'),10);var lim=c>=3000?10000:Math.max(2000,3*c);";
      content += "if(!e){l.style.background='#666';f.textContent='\\u2013';if(tr){tr.className='';}return;}";
      content += "l.style.background=(now-e[2]<=lim)?'#3ecf5a':'#666';";
      content += "f.textContent=T(e[1])+' ('+(window.lastU?clk(window.lastU,now,e[1]):'')+')';";
      content += "if(tr){tr.className=(now-e[1]<15000)?'new':'';}}";
      content += "function poll(){fetch('/twingoRxStatus').then(function(r){return r.json();}).then(function(j){";
      content += "window.lastU=j.u;document.getElementById('tNow').textContent=T(j.now);";
      content += "document.getElementById('tClock').textContent=j.u?new Date(j.u*1000).toLocaleTimeString():'\\u2013';";
      content += "document.getElementById('tAct').textContent=(j.act<0)?'\\u2013':T(j.act)+' \\u2013 '+j.acttxt;";
      content += "var r={},c={},i;for(i=0;i<j.r.length;i++){r[j.r[i][0]]=j.r[i];}";
      content += "for(i=0;i<j.c.length;i++){c[j.c[i][0]]=j.c[i];}";
      content += "var ls=document.getElementsByClassName('led');for(i=0;i<ls.length;i++){";
      content += "var k=ls[i].id.substring(1);var t=k.charAt(0);var n=parseInt(k.substring(1),10);";
      content += "led(k,t=='r'?r[n]:c[n],j.now);}";
      content += "var h='';for(i=0;i<j.o.length;i++){var o=j.o[i];";
      content += "h+='<tr><td>0x'+o[0].toString(16).toUpperCase()+'</td><td>'+T(o[1])+'</td><td>'+";
      content += "((j.now-o[2])/1000).toFixed(1).replace('.',',')+' s</td><td>'+o[3]+'</td></tr>';}";
      content += "var sel=document.getElementById('busstate');if(sel&&j.bm===0){sel.value=j.bs;}";
      content += "var t69=document.getElementById('tx69f');if(t69&&j.t69!==undefined){t69.textContent=j.t69;}";
      content += "document.getElementById('othRows').innerHTML=h;}).catch(function(){});}";
      content += "poll();setInterval(poll,1000);";
      content += "</script>";

      content += "<p><a href='/advanced' style='color:#8fd3ff;'>Back to More Battery Info</a></p>";
    }
    return content;
  }
  return String();
}
