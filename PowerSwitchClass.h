////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#define DATETIME_STR_LEN            22
#define PSW_URL_MAX_LEN             60
#define PSW_RELAY_STATE_STR_LEN     4
#define PSW_CONTROL_ENABLED_STR_LEN 10

class PowerSwitch {

  HTTPClient    http;
  
  int           update_interval;
  unsigned long last_update;
  int           psw_id;
  unsigned int  psw_toggle_count;

public:

  char          psw_url[PSW_URL_MAX_LEN] = {0};
  char          psw_control_enabled_str[PSW_CONTROL_ENABLED_STR_LEN] = {0};
  char          psw_relay_state_str[PSW_RELAY_STATE_STR_LEN] = {0};
  char          psw_last_on_time_str[DATETIME_STR_LEN] = {0};
  char          psw_last_off_time_str[DATETIME_STR_LEN] = {0};

  bool          oled_update_required;
  
  PowerSwitch( int interval, char *url, int id ) {
    update_interval = interval;
    last_update = millis();
    psw_id = id;
    psw_toggle_count = 0;
    strcpy( psw_url, url );
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  begin() {
    char  tmpbuf[40];
    
    sprintf(tmpbuf, "conf.psw[%d].relay_state = %d", psw_id, conf.psw[psw_id].relay_state );
    writeSDAuditLog(tmpbuf);
    setEnableControl( conf.psw[psw_id].control_enabled );
    setPSWRelayOnLED( psw_id, conf.psw[psw_id].relay_state );
    setRelayState( conf.psw[psw_id].relay_state );
    setDateTimeString( conf.psw[psw_id].last_on_time, psw_last_on_time_str, conf.psw[psw_id].last_on_triggered );
    setDateTimeString( conf.psw[psw_id].last_off_time, psw_last_off_time_str, conf.psw[psw_id].last_off_triggered );
    oled_update_required = true;

  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  update() {
    if((millis() - last_update) > update_interval) {
      last_update = millis();
      collectPSWData();
      oled_update_required = true;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  setEnableControl( bool state ) {
    char  tmpbuf1[50];
    
    if( state == true ) {
      strcpy( psw_control_enabled_str, "Live" );
      sprintf( tmpbuf1, "Control Live for psw %d", psw_id );
      writeSDAuditLog( tmpbuf1 );
    } else {
      strcpy( psw_control_enabled_str, "Passive" );
      sprintf( tmpbuf1, "Control Passive for psw %d", psw_id );
      writeSDAuditLog( tmpbuf1 );
    }
    setPSWControlEnabledLED( psw_id, state );
    oled_update_required = true;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  setRelayState( bool state ) {
    if( state == true ) {
      strcpy( psw_relay_state_str, "On" );
    } else {
      strcpy( psw_relay_state_str, "Off" );
    }
    setPSWRelayOnLED( psw_id, state );
    oled_update_required = true;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  turnPSWOn() {
    char    tmpbuf1[60];
    int     http_code = 0;
  
    http.begin( String(psw_url) + "/on" );
    http.setAuthorization( TIMER_PSW_USERNAME, TIMER_PSW_PASSWORD );

    http_code = http.GET();

    if(http_code == HTTP_CODE_OK || http_code == HTTP_CODE_FOUND ) {
      sprintf( tmpbuf1, "Turned psw %d On", psw_id );
      writeSDAuditLog( tmpbuf1 );

      conf.psw[psw_id].relay_state = 1;
      conf.psw[psw_id].last_on_time = now();
      conf.psw[psw_id].last_on_triggered = true;
      saveConfiguration();
      setRelayState( conf.psw[psw_id].relay_state );
      setDateTimeString( conf.psw[psw_id].last_on_time, psw_last_on_time_str, conf.psw[psw_id].last_on_triggered );
      oled_update_required = true;
      
    } else {
      sprintf( tmpbuf1, "Turn Psw[%d] On - [HTTP] GET failed ... code: %d", psw_id, http_code);
      writeSDAuditLog( tmpbuf1 );
    }
    http.end();  
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  turnPSWOff() {
    char    tmpbuf1[60];
    int     http_code = 0;

    http.begin( String(psw_url) + "/off" );
    http.setAuthorization( TIMER_PSW_USERNAME, TIMER_PSW_PASSWORD );

    http_code = http.GET();

    if(http_code == HTTP_CODE_OK || http_code == HTTP_CODE_FOUND ) {
      sprintf( tmpbuf1, "Turned psw %d Off", psw_id );
      writeSDAuditLog( tmpbuf1 );
      
      conf.psw[psw_id].relay_state = 0;
      conf.psw[psw_id].last_off_time = now();
      conf.psw[psw_id].last_off_triggered = true;
      saveConfiguration();
      setRelayState( conf.psw[psw_id].relay_state );
      setDateTimeString( conf.psw[psw_id].last_off_time, psw_last_off_time_str, conf.psw[psw_id].last_off_triggered );
      oled_update_required = true;

    } else {
      sprintf( tmpbuf1, "Turn Psw[%d] Off - [HTTP] GET failed ... code: %d", psw_id, http_code);
      writeSDAuditLog( tmpbuf1 );
    }
    http.end();  
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  byte  parseJSON_for_current_relay_state( char *json ) {
  
    byte  relay_state = -1;
    char  tmpbuf1[200];
    char  *cp, *cp2, *fp, *pp, *tp, *ep;
    int   a;

    a = 0;
    if( cp = strchr(json,'{')) {
      cp++;

      if( cp2 = strchr(cp, '}')) {
        *cp2 = '\0';

        fp = strtok(cp, ",");
        do {
          tp = fp;
          while( *tp != '\0' && (*tp == ' ' || *tp == '"' )) {
            tp ++;
          }
          if( *tp ) {
            if( pp = strchr( tp, (char)':')) {
              if( ep = strchr( tp, (char)'"')) {
                *ep = '\0';
              }
              pp++;         // Step over the ':'
              while( *pp != '\0' && (*pp == ' ' || *pp == '"' )) {
                pp ++;
              }
              if( ep = strchr( pp, (char)'"')) {
                *ep = '\0';
              }
              if( strncasecmp( tp, "current_relay_state", 19 ) == 0 ) {
                relay_state = atoi(pp);
                if(  conf.console_logging_enabled == true ) {
                  Serial.printf(" Arg(%02d) %30s = [%s]\n", a, tp, relay_state == 1 ? "On" : "Off" );                  
                }
              } else if ( conf.console_logging_enabled == true ) {
                Serial.printf(" Arg(%02d) %30s = %s\n", a, tp, pp );
              }
            }
          }
          a ++;
        }while( fp = strtok( NULL, "," ));    
        if( relay_state != -1 ) {
          return( relay_state );
        }
      } else {
        if( strlen( json ) > 130 ) {
          json[130] = '\0';
        }
        sprintf(tmpbuf1, "parseJSON psw[%d] unrecognised %s no trailing }", psw_id, json );
        writeSDAuditLog( tmpbuf1 );
      }
    } else {
      if( strlen( json ) > 130 ) {
        json[130] = '\0';
      }
      sprintf(tmpbuf1, "parseJSON psw[%d] unrecognised %s no leading }", psw_id, json );
      writeSDAuditLog( tmpbuf1 );
    }
    return( -1 );
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  collectPSWData() {
    char  buf[500];
    char  tmpbuf1[100];
    char  *cp, *cp2, *fp, *pp, *tp;
    byte  psw_state = -1;
    int   http_code = 0;
  
    if ( conf.console_logging_enabled == true ) {
      Serial.printf("CollectPSWData [%d] ...\n", psw_id );
    }
    http.begin(String(psw_url) + "/json/status");
        
    http_code = http.GET();

    if(http_code == HTTP_CODE_OK) {
      strcpy(buf, http.getString().c_str());

      psw_state = parseJSON_for_current_relay_state( buf );
      if( psw_state == -1 ) {
        sprintf( tmpbuf1, "Error collecting current_relay_state for psw [%d]", psw_id );
        writeSDAuditLog( tmpbuf1 );
      } else if( psw_state != conf.psw[psw_id].relay_state ) {
        conf.psw[psw_id].relay_state = psw_state;
        if( conf.psw[psw_id].relay_state == 0 ) {
          conf.psw[psw_id].last_off_time = now();
          conf.psw[psw_id].last_off_triggered = false;
          setDateTimeString( conf.psw[psw_id].last_off_time, psw_last_off_time_str, conf.psw[psw_id].last_off_triggered );
          strcpy( psw_relay_state_str, "Off" );
          sprintf( tmpbuf1, "Detected psw %d changed to Off", psw_id );
          writeSDAuditLog( tmpbuf1 );
          setPSWRelayOnLED( psw_id, false );
        } else {
          conf.psw[psw_id].last_on_time = now();
          conf.psw[psw_id].last_on_triggered = false;
          setDateTimeString( conf.psw[psw_id].last_on_time, psw_last_on_time_str, conf.psw[psw_id].last_on_triggered );
          strcpy( psw_relay_state_str, "On" );
          sprintf( tmpbuf1, "Detected psw %d changed to On", psw_id );
          writeSDAuditLog( tmpbuf1 );
          setPSWRelayOnLED( psw_id, true );
        }
        saveConfiguration();
        oled_update_required = true;
      }
    } else {
      sprintf( tmpbuf1, "collectPSWData [%d] - [HTTP] GET failed ... code: %d", psw_id, http_code);
      writeSDAuditLog( tmpbuf1 );
    }
    http.end();
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  checkPSWState( float solar_power_sma, float load_power_sma, float solax_battery_capacity ) {

    char  tmpbuf1[120];
    
    if ( conf.console_logging_enabled == true ) {
      Serial.printf("checkPSWState(%d) ...\n", psw_id );
    }

    if( conf.psw[psw_id].relay_state == 0 ) {
      if(( solar_power_sma > conf.psw[psw_id].solar_poweron_level ) &&
         ( solax_battery_capacity > conf.psw[psw_id].battery_charge_poweron_level )) {
        sprintf( tmpbuf1, "PSW%d is OFF, Solar Power SMA %8.2f > %d and Battery Charge %8.2f > %d, need to power it ON",
                          psw_id,
                          solar_power_sma, conf.psw[psw_id].solar_poweron_level,
                          solax_battery_capacity, conf.psw[psw_id].battery_charge_poweron_level );
        writeSDAuditLog( tmpbuf1 );
        turnPSWOn();
      }
    } else {  // conf.psw[psw_id].relay_state == 1
      if(( solar_power_sma < conf.psw[psw_id].solar_poweroff_level ) &&
         ( solax_battery_capacity < conf.psw[psw_id].battery_charge_poweroff_level )) {
        sprintf( tmpbuf1, "PSW%d is ON, Solar Power SMA %8.2f < %d and Battery Charge %8.2f < %d, need to power it OFF",
                          psw_id,
                          solar_power_sma, conf.psw[psw_id].solar_poweroff_level,
                          solax_battery_capacity, conf.psw[psw_id].battery_charge_poweroff_level );
        writeSDAuditLog( tmpbuf1 );
        turnPSWOff();
        psw_toggle_count = 0;
      } else if( load_power_sma < conf.psw[psw_id].solar_poweroff_level ) {
        if( psw_toggle_count > 4 ) {
          sprintf( tmpbuf1, "PSW%d is ON, but Load Power SMA %8.2f < %d, NOT TOGGLING THIS HAS BEEN TRIED 4 TIMES ALREADY !!!!!",
                            psw_id, load_power_sma, conf.psw[psw_id].solar_poweroff_level );
          writeSDAuditLog( tmpbuf1 );
          
        } else {
          psw_toggle_count ++;
          sprintf( tmpbuf1, "PSW%d is ON, but Load Power SMA %8.2f < %d, need to toggle power as power supply not kicked in, turning power off",
                            psw_id, load_power_sma, conf.psw[psw_id].solar_poweroff_level );
          writeSDAuditLog( tmpbuf1 );
          turnPSWOff();  
        }
      } else {
        psw_toggle_count = 0;
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  dataTimeStamp() {

    Serial.printf("Psw%d Control Enabled = %d (%s)\n", psw_id, conf.psw[psw_id].control_enabled, psw_control_enabled_str );
    Serial.printf("Psw%d Relay State     = %d (%s)\n", psw_id, conf.psw[psw_id].relay_state, psw_relay_state_str );
    Serial.printf("Psw%d Last on time    = %s\n", psw_id, psw_last_on_time_str );
    Serial.printf("Psw%d Last off time   = %s\n", psw_id, psw_last_off_time_str );
  }
};
