////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
WebServer       server(80);

static  int     html_entry_error = false;
static  String  html_entry_error_msg = "";

const   char    *www_username = WWW_USERNAME;
const   char    *www_password = WWW_PASSWORD;
const   char    *www_realm = "Solar Monitor Authentication";

String          pg_version = String(PG_VERSION);

String authFailResponse = "Authentication Failed";
String html_page_header = "<html><head><title>Solar Monitor</title></head><body>"
                          "<script>\n"
                          "var h,m,s,animate;\n"
                          "function init(){\n"
                          " h=$HOURS$;m=$MINUTES$;s=$SECONDS$;\n"
                          " clock();\n"
                          "};\n"
                          "function clock() {\n "
                          " s++;\n"
                          " if(s==60) {\n"
                          "  s=0;\n"
                          "  m++;\n"
                          "  if(m==60){\n"
                          "   m=0;\n"
                          "   h++;\n"
                          "   if(h==24){\n"
                          "    h=0;\n"
                          "   }\n"
                          "  }\n"
                          " }\n"
                          " $('sec',s);\n"
                          " $('min',m);\n"
                          " $('hr',h);\n"
                          " animate=setTimeout(clock,1000);\n"
                          "};\n "
                          "function $(id,val){\n"
                          " if(val<10){\n"
                          "  val='0'+val;\n"
                          " }\n"
                          " document.getElementById(id).innerHTML=val;\n"
                          "};\n "
                          "window.onload=init;\n "
                          "</script>\n"
                          "<p><center><h1>Solar Monitor (" + String(PG_VERSION) + ")</h1></center></p>"
                          "<p><center><h2><a href=\"/\">Home</a> - <a href=\"/fs/dir\">SD Files</a> - <a href=\"/settings\">Settings</a></h2></center></p>"
                          "<p><center><h2><div class=\"clock\"><span>Time </span><span id=\"hr\">00</span><span> : </span><span id=\"min\">00</span><span> : </span><span id=\"sec\">00</span></div></h2></center></p>";

String html_page_footer = "<p>&nbsp;</p><p><center><h4>&copy; Mick Vaites 2018</h4></center></p>"
                          "</body></html>";

String ajax_homepage  = "<script>\n"
"var y=setInterval(function() {loadData(\"/json/status\", parse_Json )}, 10000);\n"
"\n"
"function loadData(url, cFunction, value) {\n"
"  var xhttp;\n"
"  xhttp=new XMLHttpRequest();\n"
"  xhttp.onreadystatechange = function() {\n"
"    if (this.readyState == 4 && this.status == 200) {\n"
"      cFunction(this);\n"
"    }\n"
"  };\n"
"  xhttp.open(\"GET\", url, true);\n"
"  xhttp.send();\n"
"}\n"
"\n"
"function parse_Json(xhttp) {\n"
"  var obj = JSON.parse(xhttp.responseText );\n"
"\n"
"  document.getElementById(\"solar_power\").innerHTML = obj.solar_power;\n"
"  document.getElementById(\"load_power\").innerHTML = obj.load_power;\n"
"  document.getElementById(\"solar_power_sma\").innerHTML = obj.solar_power_sma;\n"
"  document.getElementById(\"load_power_sma\").innerHTML = obj.load_power_sma;\n"
"  document.getElementById(\"feed_in_power\").innerHTML = obj.feed_in_power;\n"
"  document.getElementById(\"battery_power\").innerHTML = obj.battery_power + \" / \" + obj.battery_capacity;\n"
"  if ( obj.psw_control_enabled === \"0\" ) {\n"
"   document.getElementById(\"psw_control_enabled\").innerHTML = \"Power Switches - (Auto Control Disabled)\";\n"
"  } else {\n"
"   document.getElementById(\"psw_control_enabled\").innerHTML = \"Power Switches - (Auto Control Enabled)\";\n"
"  }\n"
"  document.getElementById(\"psw1_control_enabled\").innerHTML = obj.psw1_control_enabled;\n"
"  document.getElementById(\"psw2_control_enabled\").innerHTML = obj.psw2_control_enabled;\n"
"  document.getElementById(\"psw1_relay_state\").innerHTML = obj.psw1_relay_state;\n"
"  document.getElementById(\"psw2_relay_state\").innerHTML = obj.psw2_relay_state;\n"
"  document.getElementById(\"psw1_last_on_time\").innerHTML = obj.psw1_last_on_time;\n"
"  document.getElementById(\"psw2_last_on_time\").innerHTML = obj.psw2_last_on_time;\n"
"  document.getElementById(\"psw1_last_off_time\").innerHTML = obj.psw1_last_off_time;\n"
"  document.getElementById(\"psw2_last_off_time\").innerHTML = obj.psw2_last_off_time;\n"
"\n"
"}\n"
"\n"
"</script>\n";
////////////////////////////////////////////////////////////////////////////////////////////////////
void handleRoot() {
  String message = html_page_header;
  char  fmtbuf1[80];
  char  fmtbuf2[60];

  sprintf( fmtbuf1, "h=%d;m=%d;s=%d", hour(), minute(), second());
  message.replace( "h=$HOURS$;m=$MINUTES$;s=$SECONDS$", String(fmtbuf1) );

  if( html_entry_error == true ) {
    html_entry_error = false;
    message += "<p><center><h3><font color=\"red\">" + html_entry_error_msg + "</font></h3></center></p>";
  }
  message += "<p><center><table cellspacing=\"10\" cellpadding=\"2\" >";
  
  message += "<tr><td colspan=\"4\" align=\"center\"><h2>Current Power</h2></td></tr>";
  message += "<tr><td align=\"left\"><b>Current Solar Power</b></td><td align=\"right\" id=\"solar_power\">" + String( Solax1.solar_power, 0 ) + "W</td>"
                 "<td align=\"left\"><b>Current Load Power</b></td><td align=\"right\" id=\"load_power\">" + String( Solax1.load_power, 0 ) + "W</td></tr>";
                 
  message += "<tr><td align=\"left\"><b>Moving Average Solar Power</b></td><td align=\"right\" id=\"solar_power_sma\">" + String( Solax1.solar_power_sma, 0 ) + "W</td>"
                 "<td align=\"left\"><b>Moving Average Load Power</b></td><td align=\"right\" id=\"load_power_sma\">" + String( Solax1.load_power_sma, 0 ) + "W</td></tr>";
                 
  message += "<tr><td align=\"left\"><b>Grid</b></td><td align=\"right\" id=\"feed_in_power\">" + String( Solax1.solax_vals[FEED_IN_POWER], 0 ) + "W</td>"
                 "<td align=\"left\"><b>Battery Power/Capacity</b></td><td align=\"right\" id=\"battery_power\">" + String( Solax1.solax_vals[BATTERY_POWER], 0 ) + "W / " + String( Solax1.solax_vals[BATTERY_CAPACITY], 0 ) + "%</td></tr>";

  message += "<tr><td colspan=\"4\" align=\"center\"><br><h2 id=\"psw_control_enabled\">Power Switches - (Auto Control ";
  if( conf.psw_control_enabled == true ) {
    message += "Enabled";
  } else {
    message += "Disabled";    
  }
  message +=  ")</h2></td></tr>";  
  message += "<tr><td align=\"left\"><b>PSW1 Auto Control</b></td><td align=\"right\" id=\"psw1_control_enabled\">" + String( Psw1.psw_control_enabled_str ) + "</td>"
                 "<td align=\"left\"><b>PSW2 Auto Control</b></td><td align=\"right\" id=\"psw2_control_enabled\">" + String( Psw2.psw_control_enabled_str ) + "</td></tr>";

  sprintf( fmtbuf1, "Solar > %dW and Battery > %d%%", conf.psw[0].solar_poweron_level, conf.psw[0].battery_charge_poweron_level  );
  sprintf( fmtbuf2, "Solar > %dW and Battery > %d%%", conf.psw[1].solar_poweron_level, conf.psw[1].battery_charge_poweron_level  );
  message += "<tr><td align=\"left\"><b>PSW1 Power On Thresholds</b></td><td align=\"right\">" + String( fmtbuf1 ) + "</td>"
                 "<td align=\"left\"><b>PSW2 Power On Thresholds</b></td><td align=\"right\">" + String( fmtbuf2 ) + "</td></tr>";

  message += "<tr><td align=\"left\"><b>PSW1 Last Power On Time</b></td><td align=\"right\" id=\"psw1_last_on_time\">" + String( Psw1.psw_last_on_time_str ) + "</td>"
                 "<td align=\"left\"><b>PSW2 Last Power On Time</b></td><td align=\"right\" id=\"psw2_last_on_time\">" + String( Psw2.psw_last_on_time_str ) + "</td></tr>";
  
  sprintf( fmtbuf1, "Solar < %dW and Battery < %d%%", conf.psw[0].solar_poweroff_level, conf.psw[0].battery_charge_poweroff_level  );
  sprintf( fmtbuf2, "Solar < %dW and Battery < %d%%", conf.psw[1].solar_poweroff_level, conf.psw[1].battery_charge_poweroff_level  );
  message += "<tr><td align=\"left\"><b>PSW1 Power Off Thresholds</b></td><td align=\"right\">" + String( fmtbuf1 ) + "</td>"
                 "<td align=\"left\"><b>PSW2 Power Off Thresholds</b></td><td align=\"right\">" + String( fmtbuf2 ) + "</td></tr>";

  message += "<tr><td align=\"left\"><b>PSW1 Last Power Off Time</b></td><td align=\"right\" id=\"psw1_last_off_time\">" + String( Psw1.psw_last_off_time_str ) + "</td>"
                 "<td align=\"left\"><b>PSW2 Last Power Off Time</b></td><td align=\"right\" id=\"psw2_last_off_time\">" + String( Psw2.psw_last_off_time_str ) + "</td></tr>";
        
  message += "<tr><td align=\"left\"><b>PSW1 Relay State</b></td><td align=\"right\" id=\"psw1_relay_state\">" + String( Psw1.psw_relay_state_str ) + "</td>"
                 "<td align=\"left\"><b>PSW2 Relay State</b></td><td align=\"right\" id=\"psw2_relay_state\">" + String( Psw2.psw_relay_state_str ) + "</td></tr>";

  message += "</table></center></p>";

  message += ajax_homepage + html_page_footer;
  server.send(200, "text/html", message );  
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  handleSolaxData() {
  String message = html_page_header;
  char  fmtbuf1[80];
  char  fmtbuf2[60];

  sprintf( fmtbuf1, "h=%d;m=%d;s=%d", hour(), minute(), second());
  message.replace( "h=$HOURS$;m=$MINUTES$;s=$SECONDS$", String(fmtbuf1) );

  if( html_entry_error == true ) {
    html_entry_error = false;
    message += "<p><center><h3><font color=\"red\">" + html_entry_error_msg + "</font></h3></center></p>";
  }

  message += "<p><center><table cellspacing=\"10\" cellpadding=\"2\" >";

  message += "<tr><td colspan=\"6\" align=\"center\"><h2>SOLAX Data</h2></td></tr>";
  message += "<tr><td align=\"left\"><b>PV1 Current</b></td><td align=\"right\">" + String(Solax1.solax_vals[PV1_CURRENT], 2 ) + "A</td>"
                 "<td align=\"left\"><b>PV2 Current</b></td><td align=\"right\">" + String(Solax1.solax_vals[PV2_CURRENT], 2 ) + "A</td>"
                 "<td align=\"left\"><b>Grid Current</b></td><td align=\"right\">" + String(Solax1.solax_vals[GRID_CURRENT], 2 ) + "A</td></tr>";

  message += "<tr><td align=\"left\"><b>PV1 Voltage</b></td><td align=\"right\">" + String(Solax1.solax_vals[PV1_VOLTAGE], 2 ) + "V</td>"
                 "<td align=\"left\"><b>PV2 Voltage</b></td><td align=\"right\">" + String(Solax1.solax_vals[PV2_VOLTAGE], 2 ) + "V</td>"
                 "<td align=\"left\"><b>Grid Voltage</b></td><td align=\"right\">" + String(Solax1.solax_vals[GRID_VOLTAGE], 2 ) + "V</td></tr>";
                 
  message += "<tr><td align=\"left\"><b>PV1 Power</b></td><td align=\"right\">" + String(Solax1.solax_vals[PV1_POWER], 0 ) + "W</td>"
                 "<td align=\"left\"><b>PV2 Power</b></td><td align=\"right\">" + String(Solax1.solax_vals[PV2_POWER], 0 ) + "W</td>"                 
                 "<td align=\"left\"><b>Grid Power</b></td><td align=\"right\">" + String(Solax1.solax_vals[GRID_POWER], 0 ) + "W</td></tr>";


  message += "<tr><td align=\"left\"><b>Current Feed in Power</b></td><td align=\"right\">" + String(Solax1.solax_vals[FEED_IN_POWER], 2 ) + "W</td>"
                 "<td align=\"left\"><b>Total Solar Power Today</b></td><td align=\"right\">" + String(Solax1.solax_vals[SOLAR_TODAY], 2 ) + "KWh</td>"
                 "<td align=\"left\"><b>Total System Solar Power</b></td><td align=\"right\">" + String(Solax1.solax_vals[SOLAR_TOTAL], 2 ) + "KWh</td></tr>";
  
  message += "<tr><td align=\"left\"><b>Battery Voltage</b></td><td align=\"right\">" + String(Solax1.solax_vals[BATTERY_VOLTAGE], 2 ) + "V</td>"
                 "<td align=\"left\"><b>Battery Current</b></td><td align=\"right\">" + String(Solax1.solax_vals[BATTERY_CURRENT], 2 ) + "A</td>"
                 "<td align=\"left\"><b>Battery Power</b></td><td align=\"right\">" + String(Solax1.solax_vals[BATTERY_POWER], 0 ) + "W</td></tr>";

  message += "<tr><td align=\"left\"><b>Battery Temperature</b></td><td align=\"right\">" + String(Solax1.solax_vals[BATTERY_TEMP], 0 ) + "&deg;C</td>"
                 "<td align=\"left\"><b>Battery Capacity</b></td><td align=\"right\">" + String(Solax1.solax_vals[BATTERY_CAPACITY], 0 ) + "%</td>"
                 "<td align=\"left\"><b>Inner Temperature</b></td><td align=\"right\">" + String(Solax1.solax_vals[INNER_TEMP], 0 ) + "&deg;C</td></tr>";

  message += "</table></center></p>";
  message += html_page_footer;
  server.send(200, "text/html", message );  

}


////////////////////////////////////////////////////////////////////////////////////////////////////
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void handleEditSettings() {

  int   a;
  String message = html_page_header;
  String selected = "selected";
  String selected_0, selected_1;
  char  fmtbuf[50];

  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }


  sprintf( fmtbuf, "h=%d;m=%d;s=%d", hour(), minute(), second());
  message.replace( "h=$HOURS$;m=$MINUTES$;s=$SECONDS$", String(fmtbuf) );

  if( html_entry_error == true ) {
    html_entry_error = false;
    message += "<p><center><h3><font color=\"red\">" + html_entry_error_msg + "</font></h3></center></p>";
  }

  message += "<form action=\"/save_settings\">"
             "<center>"
             "<table cellspacing=\"10\" cellpadding=\"2\">"
             "<tr><td colspan=\"3\" align=\"center\"><h2>Change Settings</h2></td></tr>";

  message += "<tr><td colspan=\"3\" ><hr width=100%></td></tr>";
  
  message += "<tr><td align=\"right\" colspan=\"2\">Automatic Control Enabled</td><td align=\"Left\"><input type=\"checkbox\" name=\"psw_control_enabled\" value=\"enabled\" ";
  if( conf.psw_control_enabled == true) {
    message += "checked";
  }
  message += "></td></tr>";

  message += "<tr><td align=\"right\" colspan=\"2\">Power Save Detect Threshold</td><td align=\"Left\"><select name=\"light_detect_level\">";
  for(a = 0; a < 300; a += 10 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.light_detect_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "</option>";
  }
  message += "</select></td></tr>";

  message += "<tr><td align=\"right\" colspan=\"2\">Console Logging Enabled</td><td align=\"Left\"><input type=\"checkbox\" name=\"console_logging_enabled\" value=\"enabled\" ";
  if( conf.console_logging_enabled == true) {
    message += "checked";
  }
  message += "></td></tr>";

  message += "<tr><td align=\"right\" colspan=\"2\">Web File Deletion Enabled</td><td align=\"Left\"><input type=\"checkbox\" name=\"web_sd_delete_enabled\" value=\"enabled\" ";
  if( conf.web_SD_delete_enabled == true) {
    message += "checked";
  }
  message += "></td></tr>";

  message += "<tr><td colspan=\"3\" ><hr width=100%></td></tr>";

  message += "<tr><td align=\"center\" colspan=\"3\"><B>Power Switch 1</B></td></tr>";

  message += "<tr><td align=\"right\" colspan=\"2\">Automatic Control Enabled</td><td align=\"Left\"><input type=\"checkbox\" name=\"psw1_control_enabled\" value=\"enabled\" ";
  if( conf.psw[0].control_enabled == true) {
    message += "checked";
  }
  message += "></td></tr>";
  message += "<tr><td align=\"right\">On Threshold :</td><td align=\"Left\">Solar &gt; <select name=\"psw1_solar_poweron_level\">";
  for(a = 200; a < 4000; a += 100 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[0].solar_poweron_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "W</option>";
  }
  message += "</select></td><td align=\"Left\">Battery &gt; <select name=\"psw1_battery_charge_poweron_level\">";
  for(a = 0; a < 100; a += 10 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[0].battery_charge_poweron_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "%</option>";
  }
  message += "</select></td></tr>";

  message += "<tr><td align=\"right\">Off Threshold :</td><td align=\"Left\">Solar &lt; <select name=\"psw1_solar_poweroff_level\">";
  for(a = 200; a < 4000; a += 100 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[0].solar_poweroff_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "W</option>";
  }
  message += "</select></td><td align=\"Left\">Battery &lt; <select name=\"psw1_battery_charge_poweroff_level\">";
  for(a = 0; a < 100; a += 10 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[0].battery_charge_poweroff_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "%</option>";
  }
  message += "</select></td></tr>";

  message += "<tr><td colspan=\"3\" ><hr width=100%></td></tr>";
  
  message += "<tr><td align=\"center\" colspan=\"3\"><B>Power Switch 2</B></td></tr>";

  message += "<tr><td align=\"right\" colspan=\"2\">Automatic Control Enabled</td><td align=\"Left\"><input type=\"checkbox\" name=\"psw2_control_enabled\" value=\"enabled\" ";
  if( conf.psw[1].control_enabled == true) {
    message += "checked";
  }
  message += "></td></tr>";

  message += "<tr><td align=\"right\">On Threshold :</td><td align=\"Left\">Solar &gt; <select name=\"psw2_solar_poweron_level\">";
  for(a = 1000; a < 4000; a += 100 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[1].solar_poweron_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "W</option>";
  }
  message += "</select></td><td align=\"Left\">Battery &gt; <select name=\"psw2_battery_charge_poweron_level\">";
  for(a = 20; a < 100; a += 10 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[1].battery_charge_poweron_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "%</option>";
  }
  message += "</select></td></tr>";

  message += "<tr><td align=\"right\">Off Threshold :</td><td align=\"Left\">Solar &lt; <select name=\"psw2_solar_poweroff_level\">";
  for(a = 1000; a < 4000; a += 100 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[1].solar_poweroff_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "W</option>";
  }
  message += "</select></td><td align=\"Left\">Battery &lt; <select name=\"psw2_battery_charge_poweroff_level\">";
  for(a = 10; a < 100; a += 10 ) {
    message += "<option value=\"" + String(a) + "\"";
    if( a == conf.psw[1].battery_charge_poweroff_level ) {
      message += " " + selected;
    }
    message += ">" + String(a) + "%</option>";
  }
  message += "</select></td></tr>";
  
  message += "<tr><td colspan=\"3\" ><hr width=100%></td></tr>";
  
  message += "<tr><td colspan=\"3\" align=\"center\"><input type=\"submit\" value=\"Submit Change\"><input type=\"reset\" value=\"Reset\"></td></tr>"
             "</table>"
             "</center>"
             "</form>";

  message += html_page_footer;

  server.send(200, "text/html", message);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void handleCommitSettings() {

  unsigned long req_psw_solar_poweron_level;
  unsigned long req_psw_solar_poweroff_level;
  unsigned long req_psw_battery_charge_poweron_level;
  unsigned long req_psw_battery_charge_poweroff_level;

  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }
    
  if( server.arg("psw_control_enabled") == "enabled" ) {
    conf.psw_control_enabled = true;
    setControlEnabledLED( true );
  } else {
    conf.psw_control_enabled = false;
    setControlEnabledLED( false );
  }

  if( server.arg("psw1_control_enabled") == "enabled" ) {
    conf.psw[0].control_enabled = true;
    Psw1.setEnableControl( true );
  } else {
    conf.psw[0].control_enabled = false;
    Psw1.setEnableControl( false );
  }

  req_psw_solar_poweron_level = server.arg("psw1_solar_poweron_level").toInt();
  req_psw_solar_poweroff_level = server.arg("psw1_solar_poweroff_level").toInt();
  req_psw_battery_charge_poweron_level = server.arg("psw1_battery_charge_poweron_level").toInt();
  req_psw_battery_charge_poweroff_level = server.arg("psw1_battery_charge_poweroff_level").toInt();

  if(( req_psw_solar_poweron_level > req_psw_solar_poweroff_level ) &&
     ( req_psw_battery_charge_poweron_level > req_psw_battery_charge_poweroff_level ) &&
     ( req_psw_battery_charge_poweron_level < 100 ) &&
     ( req_psw_battery_charge_poweroff_level < 100 ) &&
     ( req_psw_solar_poweron_level < 4000 ) &&
     ( req_psw_solar_poweroff_level < 4000 )) {
      
    conf.psw[0].solar_poweron_level = req_psw_solar_poweron_level;
    conf.psw[0].battery_charge_poweron_level = req_psw_battery_charge_poweron_level;
    conf.psw[0].solar_poweroff_level = req_psw_solar_poweroff_level;
    conf.psw[0].battery_charge_poweroff_level = req_psw_battery_charge_poweroff_level;

  }
  
  req_psw_solar_poweron_level = server.arg("psw2_solar_poweron_level").toInt();
  req_psw_solar_poweroff_level = server.arg("psw2_solar_poweroff_level").toInt();
  req_psw_battery_charge_poweron_level = server.arg("psw2_battery_charge_poweron_level").toInt();
  req_psw_battery_charge_poweroff_level = server.arg("psw2_battery_charge_poweroff_level").toInt();

  if(( req_psw_solar_poweron_level > req_psw_solar_poweroff_level ) &&
     ( req_psw_battery_charge_poweron_level > req_psw_battery_charge_poweroff_level ) &&
     ( req_psw_battery_charge_poweron_level < 100 ) &&
     ( req_psw_battery_charge_poweroff_level < 100 ) &&
     ( req_psw_solar_poweron_level < 4000 ) &&
     ( req_psw_solar_poweroff_level < 4000 )) {
      
    conf.psw[1].solar_poweron_level = req_psw_solar_poweron_level;
    conf.psw[1].battery_charge_poweron_level = req_psw_battery_charge_poweron_level;
    conf.psw[1].solar_poweroff_level = req_psw_solar_poweroff_level;
    conf.psw[1].battery_charge_poweroff_level = req_psw_battery_charge_poweroff_level;

  }
    
  if( server.arg("psw2_control_enabled") == "enabled" ) {
    conf.psw[1].control_enabled = true;
    Psw2.setEnableControl( true );
  } else {
    conf.psw[1].control_enabled = false;
    Psw2.setEnableControl( false );
  }
  
  conf.light_detect_level = server.arg("light_detect_level").toInt();

  if( server.arg("console_logging_enabled") == "enabled" ) {
    conf.console_logging_enabled = true;
    writeSDAuditLog("Console logging enabled");
  } else {
    writeSDAuditLog("Console logging disabled");
    conf.console_logging_enabled = false;    
  }

  if( server.arg("web_sd_delete_enabled") == "enabled" ) {
    conf.web_SD_delete_enabled = true;
  } else {
    conf.web_SD_delete_enabled = false;    
  }

  saveConfiguration();

  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void  handleSDDirectoryList() {

  char            fmtbuf[50];
  String          message = html_page_header;
  File            root;
  File            file;
  char            filename[100] = {0};
  unsigned long   bytes;
  int             file_count, len;

  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }

  sprintf( fmtbuf, "h=%d;m=%d;s=%d", hour(), minute(), second());
  message.replace( "h=$HOURS$;m=$MINUTES$;s=$SECONDS$", String(fmtbuf) );

  message += "<form action=\"/\">"
             "<center>"
             "<table cellspacing=\"10\" cellpadding=\"2\">"
             "<tr><td colspan=\"3\" align=\"center\"><h2>Directory Listing of /</h2></td></tr>";
             
  if( SD_ok == false ) {
    message += "<tr><td colspan=\"3\">SD Card not present or not formated</td></tr>";
  } else if( SDCardAccessSemaphore == true ) {
    message += "<tr><td colspan=\"3\">SD Card resource locked &lt;Try Again&gt;</td></tr>";
  } else {
    root = SD.open("/");
    if( !root ){
      message += "<tr><td colspan=\"3\">Failed to open / folder on SD</td></tr>";
    } else {
      file_count = 0;
      root.rewindDirectory();
      while (file = root.openNextFile()) {
        file_count ++;
        if(file.isDirectory()) {
          message += "<tr><td >" + String( file.name() ) + "</td><td>&lt;DIR&gt;</td><td></td></tr>";
        } else {
          len = strlen( file.name() );
          strcpy( filename, file.name() );          
          message += "<tr><td ><a href=/fs/dl?fn=" + String( filename ) + ">" + String( filename ) + "</a></td><td>";
          bytes = file.size();
          if (bytes < 1024) {
            message += String(bytes)+" B";
          } else if(bytes < (1024 * 1024)) {
            message += String(bytes/1024.0,3)+" KB";
          } else if(bytes < (1024 * 1024 * 1024)) {
            message += String(bytes/1024.0/1024.0,3)+" MB";
          } else {
            message += String(bytes/1024.0/1024.0/1024.0,3)+" GB";
          }
          message += "</td><td>";
          if( conf.web_SD_delete_enabled == true ) {
            message += "<a href=/fs/delete?fn=" + String( file.name() ) + ">Del</a>";
          }
          message += "&nbsp;";
          if( strncasecmp( filename, "/timestamp_", 11 ) == 0 ) {
            message += "<a href=/fs/graph?fn=" + String( filename ) + ">Graph</a>";
          }
          message += "</td></tr>";
        }
      }
      message += "<tr><td colspan=\"3\" align=\"center\"><b>Total " + String(file_count) + "</b></td></tr>";
    }
  }
  message += "</table>"
             "</center>"
             "</form>";

  message += ajax_homepage + html_page_footer;

  server.send(200, "text/html", message);  
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void handleSDFileDownload() {

  bool      download_ok = false;
  String    message = html_page_header;
  File      f;

  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }
  
  if(( SD_ok != false ) && ( server.hasArg("fn"))) {
    f = SD.open(server.arg("fn"));
    if( f ) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+server.arg("fn"));
      server.sendHeader("Connection", "close");
      server.streamFile(f, "application/octet-stream");
      f.close();
      download_ok = true;
    } else {
      download_ok = false;
    }
  }
  if( download_ok == false ) {
    message += "<center><h1>Failed to download</h1></centre>" + html_page_footer;
    message += ajax_homepage + html_page_footer;
    server.send(200, "text/html", message);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void  handleSDFileDelete() {
  String        message = html_page_header;
  char          fmtbuf1[80];
  unsigned int  c;

  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }

  sprintf( fmtbuf1, "h=%d;m=%d;s=%d", hour(), minute(), second());
  message.replace( "h=$HOURS$;m=$MINUTES$;s=$SECONDS$", String(fmtbuf1) );
  if( conf.web_SD_delete_enabled == true ) {
    message += "<form action=\"/\">"
               "<center>"
               "<table cellspacing=\"10\" cellpadding=\"2\">"
               "<tr><td colspan=\"3\" align=\"center\"><h4>Are you sure you want to delete " + server.arg("fn") + " ?</h4></td></tr>"
               "<tr><td></td><td align=\"center\" ><a href=\"/fs/confdel?f=" + server.arg("fn") + "\">Yes</a>&nbsp;<a href=\"/fs/dir\">No</a></td><td></td></tr>"
               "</table></center></p>";
  } else {
    message += "<center>"
               "<table cellspacing=\"10\" cellpadding=\"2\">"
               "<tr><td colspan=\"3\" align=\"center\"><h1>Delete Function Disabled</td></tr>"
               "</table></center></p>";
  }
  message += html_page_footer;
  server.send(200, "text/html", message );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void  handleSDFileDeleteConfirmed() {
  String    message = html_page_header;
  File      f;
  
  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }

  if(( SD_ok != false ) && ( server.hasArg("f")) && ( SD.exists( server.arg("f").c_str())) && ( conf.web_SD_delete_enabled == true )) {
    if(SD.remove(server.arg("f"))) {
      if ( conf.console_logging_enabled == true ) {
        Serial.printf("Deleted file [%s] ok\n", server.arg("f").c_str());
      }
    } else {
      if ( conf.console_logging_enabled == true ) {
        Serial.printf("Failed to deleted file [%s]\n", server.arg("f").c_str());
      }
    }
  } else {
    if ( conf.console_logging_enabled == true ) {
      Serial.printf("The deletion of file [%s] is not possible\n", server.arg("f").c_str());    
    }
  }
  server.sendHeader("Location", String("/fs/dir"), true);
  server.send ( 302, "text/plain", "");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void  handleSDShowGraph() {
  String        message = html_page_header;
  char          fmtbuf1[80];
  unsigned int  c;

  if( !server.authenticate(www_username, www_password)) {
    return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  }
  
  sprintf( fmtbuf1, "h=%d;m=%d;s=%d", hour(), minute(), second());
  message.replace( "h=$HOURS$;m=$MINUTES$;s=$SECONDS$", String(fmtbuf1) );

  message += "<p><center><table cellspacing=\"10\" cellpadding=\"2\" >";

  if( server.hasArg("fn")) {
    if( SD.exists( server.arg("fn"))) {
      message += "<tr><td align=\"center\" ><img src=\"/img/solarloadgraph.svg?f=" + server.arg("fn") + "\" height=\"600\" width=\"600\"></td></tr>";
    } else {
      message += "<tr><td align=\"center\" >File not found</td></tr>";      
    }
  }

  message += "</table></center></p>";
  message += html_page_footer;
  server.send(200, "text/html", message );  
  
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Draw /img/solarloadgraph.svg
//
void handleDrawSolarLoadPowerGraph() {

  File    f;
  unsigned int  p, c, x, y, h, ave_lp_y, ave_sp_y, plot_x, plot_lp_y,  plot_sp_y,  lp_i, sp_i;
  String        out = "";
  char          temp[100];
  char          filename[100];
  char          ch;
  char          *dp, *cp, *tp, *lp, *sp;
  char          read_buf1[1024];
  char          start_time[30] = {0};
  char          end_time[30] = {0};

  if( server.hasArg("f")) {
    if( SD.exists( server.arg("f"))) {
      strcpy( filename, server.arg("f").c_str());
      f = SD.open( filename );
      if( f ) {
        p = 0;
        c = 0;
        x = 0;
        y = 0;
        h = 0;
        ave_lp_y = 0;
        ave_sp_y = 0;
        out += "<svg width=\"600\" height=\"600\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n";

        // graph description (200,15)
        sprintf(temp, "<rect x=\"195\" y=\"2\" width=\"%d\" height=\"18\" fill=\"lightgrey\" />\n", 8 * (10 + strlen(filename) ));
        out += temp;
        
        sprintf(temp, "<text x=\"200\" y=\"15\">Chart for %s</text>", filename );
        out += temp;
        
        // x-axis (130,501) to (600,501)
        out += "<line x1=\"129\" y1=\"501\" x2=\"600\" y2=\"501\" stroke=\"black\" stroke-width=\"1\" />";
        // y-axis (129,501) - (129,0)
        out += "<line x1=\"129\" y1=\"501\" x2=\"129\" y2=\"0\" stroke=\"black\" stroke-width=\"1\" />";
        // y-axis labels Power (30,30)
        out += "<text x=\"30\" y=\"30\">Power (W)</text>";
        // y-axis labels 1000 (80,379)
        out += "<text x=\"80\" y=\"379\">1000</text>";
        out += "<line x1=\"120\" y1=\"375\" x2=\"129\" y2=\"375\" stroke=\"black\" stroke-width=\"1\" />";
        // y-axis labels 2000 (80,254)
        out += "<text x=\"80\" y=\"254\">2000</text>";
        out += "<line x1=\"120\" y1=\"250\" x2=\"129\" y2=\"250\" stroke=\"black\" stroke-width=\"1\" />";
        // y-axis labels 3000 (80,129)
        out += "<text x=\"80\" y=\"129\">3000</text>";
        out += "<line x1=\"120\" y1=\"125\" x2=\"129\" y2=\"125\" stroke=\"black\" stroke-width=\"1\" />";
      // x-axis labels Time (330,515)
        out += "<text x=\"330\" y=\"545\">Time (H)</text>";
 
        while( f.read( (uint8_t *)&ch, 1 ) && (x < (470 * 3))) {
          if( ch == '\n' ) {
            read_buf1[p] = '\0';
            if( dp = strtok( read_buf1, "," )) {        // Date
              if( tp = strtok( NULL, "," )) {           // Time
                if( lp = strtok( NULL, "," )) {         // Load Power
                  lp_i = atoi( lp );
                  if( cp = strtok( NULL, "," )) {       // Load Power SMA
                    if( sp = strtok( NULL, "," )) {     // Solar Power
                      sp_i = atoi( sp );
                      if( cp = strtok( NULL, "," )) {   // Solar Power SMA
                        if( x == 0 ) {
                          strcpy( start_time, tp );
                        }
                        if(( lp_i > 0 ) && (lp_i < 4000 )) {
                          x ++;
                          ave_lp_y += lp_i;
                          ave_sp_y += sp_i;
                          if( x % 3 == 0 ) {
                            plot_x = 130 + (x/3);
                            ave_lp_y /= 3;
                            ave_sp_y /= 3;
                            plot_lp_y = 500 - (ave_lp_y / 8);
                            plot_sp_y = 500 - (ave_sp_y / 8);
                            if( x % 6 ) {
                              sprintf(temp, "<line x1=\"%d\" y1=\"500\" x2=\"%d\" y2=\"%d\" stroke=\"orange\" stroke-width=\"1\" />\n", plot_x, plot_x, plot_lp_y );
                            } else {
                              sprintf(temp, "<line x1=\"%d\" y1=\"500\" x2=\"%d\" y2=\"%d\" stroke=\"green\" stroke-width=\"1\" />\n", plot_x, plot_x, plot_sp_y );
                            }
                            out += temp;
                            ave_lp_y = 0;
                            ave_sp_y = 0;
                          }
                          if( x % 117 == 0 ) {
                            plot_x = 130 + (x/3);
                            sprintf( temp, "<line x1=\"%d\" y1=\"501\" x2=\"%d\" y2=\"510\" stroke=\"black\" stroke-width=\"1\" />", plot_x, plot_x );
                            out += temp;
                            h += 2;
                            sprintf( temp, "<text x=\"%d\" y=\"523\">%d</text>", h > 9 ? plot_x - 8 : plot_x - 4, h );
                            out += temp;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            p = 0;
            c ++;
          } else {
            read_buf1[p] = ch;        
            p++;
          }
        }
        f.close();
        if( tp ) {
          strcpy( end_time, tp );
        }
        out += "<text x=\"100\" y=\"580\">Load and solar generated power in Watts between " + String( start_time ) + " and " + String( end_time ) + "</text>";
        out += "\n</svg>\n";
        server.send ( 200, "image/svg+xml", out);
      } else {
        server.send ( 200, "text/plain", "File not found");        
      }
    }
  }
  server.send ( 200, "text/plain", "File not found");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  ajaxJSON_ReturnMonitorStatus() {
  String  json_msg = "{ \"updated\":\"" + String( conf.updated ) + "\","
                     "  \"db_version\":\"" + String( conf.db_version ) + "\","
                     "  \"light_detect_level\":\"" + String( conf.light_detect_level ) + "\","
                     "  \"ccd_light_level\":\"" + String( ccd_light_level ) + "\","
                     "  \"sma\":\"" + String( powerq_count ) + "\","
                     "  \"solar_power\":\"" + String( Solax1.solar_power, 0 ) + "W\","
                     "  \"solar_power_sma\":\"" + String( Solax1.solar_power_sma, 0 ) + "W\","
                     "  \"load_power\":\"" + String( Solax1.load_power, 0 ) + "W\","
                     "  \"load_power_sma\":\"" + String( Solax1.load_power_sma, 0 ) + "W\","
                     "  \"feed_in_power\":\"" + String( Solax1.solax_vals[FEED_IN_POWER], 0 ) + "W\","
                     "  \"battery_power\":\"" + String( Solax1.solax_vals[BATTERY_POWER],0 ) + "W\","
                     "  \"battery_capacity\":\"" + String( Solax1.solax_vals[BATTERY_CAPACITY],0 ) + "%\","
                     "  \"console_logging_enabled\":\"" + String( conf.console_logging_enabled ) + "\","
                     "  \"web_delete_enabled\":\"" + String( conf.web_SD_delete_enabled ) + "\","
                     "  \"psw_control_enabled\":\"" + String( conf.psw_control_enabled ) + "\","
                     "  \"psw1_control_enabled\":\"" + String( Psw1.psw_control_enabled_str ) + "\","
                     "  \"psw1_relay_state\":\"" + String( Psw1.psw_relay_state_str ) + "\","
                     "  \"psw1_last_on_time\":\"" + String( Psw1.psw_last_on_time_str ) + "\","
                     "  \"psw1_last_off_time\":\"" + String( Psw1.psw_last_off_time_str ) + "\","
                     "  \"psw2_control_enabled\":\"" + String( Psw2.psw_control_enabled_str ) + "\","
                     "  \"psw2_relay_state\":\"" + String( Psw2.psw_relay_state_str ) + "\","
                     "  \"psw2_last_on_time\":\"" + String( Psw2.psw_last_on_time_str ) + "\","
                     "  \"psw2_last_off_time\":\"" + String( Psw2.psw_last_off_time_str ) + "\" }";
  server.send(200, "text/html", json_msg );
  if ( conf.console_logging_enabled == true ) {
    Serial.println("ajaxJSON_ReturnMonitorStatus");
    Serial.println(json_msg);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void startWebServer() { // Start a HTTP server with a file read handler and an upload handler

  server.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
  server.on("/", HTTP_GET, handleRoot );  
  server.on("/settings", handleEditSettings );
  server.on("/solax", handleSolaxData );
  server.on("/save_settings", handleCommitSettings );
  server.on("/fs/dir", handleSDDirectoryList );
  server.on("/fs/dl", handleSDFileDownload );
  server.on("/fs/delete", handleSDFileDelete );
  server.on("/fs/confdel", handleSDFileDeleteConfirmed );
  server.on("/fs/graph", handleSDShowGraph );
  server.on("/img/solarloadgraph.svg", handleDrawSolarLoadPowerGraph );
  server.on("/json/status", ajaxJSON_ReturnMonitorStatus );
  server.begin();                             // start the HTTP server
  writeSDAuditLog( "HTTP server Started" );
}
