////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// General Functions
////////////////////////////////////////////////////////////////////////////////////////////////////
void  setDateTimeString( time_t   datetime_t, char  *datetime_str, bool triggered ) {
    
  if( datetime_t == 0 ) {
    sprintf( datetime_str, "Never" ); 
  } else {
    sprintf(  datetime_str,
              "%c %02d/%02d/%02d %02d:%02d:%02d",
              triggered ? '*' : ' ',
              day(datetime_t), month(datetime_t), year(datetime_t),
              hour(datetime_t), minute(datetime_t), second(datetime_t) );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////////////////////////////////////////////

#define   DB_VERSION                                  18    // Database & event log format and version

#ifdef  SH1106_SPI_OLED
#define   DEFAULT_LIGHT_DETECT_LEVEL                  100
#else
#define   DEFAULT_LIGHT_DETECT_LEVEL                  0
#endif
#define   DEFAULT_PSW1_SOLAR_POWERON_LEVEL            1400
#define   DEFAULT_PSW1_BATTERY_CHARGE_POWERON_LEVEL   70
#define   DEFAULT_PSW1_SOLAR_POWEROFF_LEVEL           1000
#define   DEFAULT_PSW1_BATTERY_CHARGE_POWEROFF_LEVEL  30

#define   DEFAULT_PSW2_SOLAR_POWERON_LEVEL            2400
#define   DEFAULT_PSW2_BATTERY_CHARGE_POWERON_LEVEL   70
#define   DEFAULT_PSW2_SOLAR_POWEROFF_LEVEL           1900
#define   DEFAULT_PSW2_BATTERY_CHARGE_POWEROFF_LEVEL  40

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
static bool           save_config_semaphore = false;  // Stop race conditions
static unsigned int   powerq_count = 0;

typedef struct  _psw {
  bool            control_enabled = false;
  bool            relay_state;
  unsigned short  solar_poweron_level;
  unsigned short  solar_poweroff_level;
  unsigned char   battery_charge_poweron_level;
  unsigned char   battery_charge_poweroff_level;
  time_t          last_on_time;
  bool            last_on_triggered;
  time_t          last_off_time;
  bool            last_off_triggered;
}PSW;

static struct {
  time_t          updated;
  unsigned char   db_version;
  char            wifi_ssid[20];
  char            wifi_passwd[20];

  unsigned short  light_detect_level = 0;
  
  bool            psw_control_enabled = false;
  
  PSW             psw[2];

  bool            console_logging_enabled = true;           // This is needed to allow logging before the EEPROM config is loaded
  bool            web_SD_delete_enabled = false;
}conf;
////////////////////////////////////////////////////////////////////////////////////////////////////
void  printConfiguration() {
  if ( conf.console_logging_enabled == true ) {
    Serial.printf("updated                            = %d\n", conf.updated );
    Serial.printf("db_version                         = %d\n", conf.db_version );
    Serial.printf("console_logging_enabled            = %d\n", conf.console_logging_enabled );
    Serial.printf("web_SD_delete_enabled              = %d\n", conf.web_SD_delete_enabled );
    Serial.printf("wifi_ssid                          = %s\n", conf.wifi_ssid );
    Serial.printf("detect_light_level                 = %d\n", conf.light_detect_level );
    Serial.printf("psw_control_enabled                = %d\n", conf.psw_control_enabled );  
    Serial.printf("psw1_control_enabled               = %d\n", conf.psw[0].control_enabled );
    Serial.printf("psw1_solar_poweron_level           = %d\n", conf.psw[0].solar_poweron_level );
    Serial.printf("psw1_battery_charge_poweron_level  = %d\n", conf.psw[0].battery_charge_poweron_level );
    Serial.printf("psw1_solar_poweroff_level          = %d\n", conf.psw[0].solar_poweroff_level );
    Serial.printf("psw1_battery_charge_poweroff_level = %d\n", conf.psw[0].battery_charge_poweroff_level );
    Serial.printf("psw1_relay_state                   = %d\n", conf.psw[0].relay_state );
    Serial.printf("psw1_last_on_time                  = %d\n", conf.psw[0].last_on_time );
    Serial.printf("psw1_last_on_triggered             = %d\n", conf.psw[0].last_on_triggered );
    Serial.printf("psw1_last_off_time                 = %d\n", conf.psw[0].last_off_time );
    Serial.printf("psw1_last_off_triggered            = %d\n", conf.psw[0].last_off_triggered );
    Serial.printf("psw2_control_enabled               = %d\n", conf.psw[1].control_enabled );
    Serial.printf("psw2_solar_poweron_level           = %d\n", conf.psw[1].solar_poweron_level );
    Serial.printf("psw2_battery_charge_poweron_level  = %d\n", conf.psw[1].battery_charge_poweron_level );
    Serial.printf("psw2_solar_poweroff_level          = %d\n", conf.psw[1].solar_poweroff_level );
    Serial.printf("psw2_battery_charge_poweroff_level = %d\n", conf.psw[1].battery_charge_poweroff_level );
    Serial.printf("psw2_relay_state                   = %d\n", conf.psw[1].relay_state );
    Serial.printf("psw2_last_on_time                  = %d\n", conf.psw[1].last_on_time );
    Serial.printf("psw2_last_on_triggered             = %d\n", conf.psw[1].last_on_triggered );
    Serial.printf("psw2_last_off_time                 = %d\n", conf.psw[1].last_off_time );
    Serial.printf("psw2_last_off_triggered            = %d\n", conf.psw[1].last_off_triggered );

    Serial.printf(" Size of buffer = %d\n", sizeof(conf));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  saveConfiguration() {

  while( save_config_semaphore == true );
  save_config_semaphore = true;

  writeSDAuditLog( "Saving configuration " );

  conf.updated = now();
  
  printConfiguration();

  EEPROM.begin(512);
  for( unsigned int t=0; t < sizeof(conf); t++ ) {
    EEPROM.write(t, *((char *)&conf +t));
  }
  EEPROM.end();
  save_config_semaphore = false;  
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  loadConfiguration() {

  writeSDAuditLog( "Loading configuration " );

  EEPROM.begin(512);
  for( unsigned int t=0; t < sizeof(conf); t++ ) {
    *((char *)&conf + t) = EEPROM.read(t);
  }
  EEPROM.end();

  printConfiguration();

  if(( conf.updated == 0) || (conf.db_version != DB_VERSION )) {
    writeSDAuditLog( "EEPROM Configuration Not found or updated version .. setting defaults" );

    conf.db_version = DB_VERSION;
    strcpy(conf.wifi_ssid, DEFAULT_WIFI_SSID );
    strcpy(conf.wifi_passwd, DEFAULT_WIFI_PASSWD );
    
    conf.console_logging_enabled = true;
    conf.web_SD_delete_enabled = false;
    conf.light_detect_level = DEFAULT_LIGHT_DETECT_LEVEL;
    conf.psw_control_enabled = false;
    conf.psw[0].control_enabled = false;
    conf.psw[0].solar_poweron_level = DEFAULT_PSW1_SOLAR_POWERON_LEVEL;
    conf.psw[0].battery_charge_poweron_level = DEFAULT_PSW1_BATTERY_CHARGE_POWERON_LEVEL;
    conf.psw[0].solar_poweroff_level = DEFAULT_PSW1_SOLAR_POWEROFF_LEVEL;
    conf.psw[0].battery_charge_poweroff_level = DEFAULT_PSW1_BATTERY_CHARGE_POWEROFF_LEVEL;
    conf.psw[0].relay_state = 0;
    conf.psw[0].last_on_time = 0;
    conf.psw[0].last_on_triggered = false;
    conf.psw[0].last_off_time = 0;
    conf.psw[0].last_off_triggered = false;
    conf.psw[1].control_enabled = false;
    conf.psw[1].solar_poweron_level = DEFAULT_PSW2_SOLAR_POWERON_LEVEL;
    conf.psw[1].battery_charge_poweron_level = DEFAULT_PSW2_BATTERY_CHARGE_POWERON_LEVEL;
    conf.psw[1].solar_poweroff_level = DEFAULT_PSW2_SOLAR_POWEROFF_LEVEL;
    conf.psw[1].battery_charge_poweroff_level = DEFAULT_PSW2_BATTERY_CHARGE_POWEROFF_LEVEL;
    conf.psw[1].relay_state = 0;
    conf.psw[1].last_on_time = 0;
    conf.psw[1].last_on_triggered = false;
    conf.psw[1].last_off_time = 0;
    conf.psw[1].last_off_triggered = false;
    saveConfiguration();
  }
  writeSDAuditLog( "Loaded configuration " );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// GPIO's
////////////////////////////////////////////////////////////////////////////////////////////////////
#define PSW1_ENABLE_LED     16
#define PSW2_ENABLE_LED     17
#define PSW1_RELAYON_LED    2
#define PSW2_RELAYON_LED    4
#define PSW_CONTROL_LED     25
#define LIGHT_SENSOR        32

static int          ccd_light_level;
static bool         power_save;
static bool         oled_change_power_save;
static bool         power_control_enabled;
static bool         psw_control_enabled[2];
static bool         psw_relayon[2];

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void  initLEDs() {
    
  pinMode( PSW_CONTROL_LED, OUTPUT );
  pinMode( PSW1_ENABLE_LED, OUTPUT );
  pinMode( PSW2_ENABLE_LED, OUTPUT );
  pinMode( PSW1_RELAYON_LED, OUTPUT );
  pinMode( PSW2_RELAYON_LED, OUTPUT );
  
  power_save = false;
  writeSDAuditLog( "LEDs initialised" );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  setControlEnabledLED( bool  state ) {

  power_control_enabled = state;
  if( power_save == false ) {
    if( power_control_enabled == true ) {
      digitalWrite( PSW_CONTROL_LED, HIGH );
    } else {
      digitalWrite( PSW_CONTROL_LED, LOW );    
    }      
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  setPSWControlEnabledLED( int  psw, bool  state ) {

  psw_control_enabled[psw] = state;
  if( power_save == false ) {
    if( psw_control_enabled[psw] == true ) {
      digitalWrite( PSW1_ENABLE_LED + psw, HIGH );
    } else {
      digitalWrite( PSW1_ENABLE_LED + psw, LOW );    
    }      
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  setPSWRelayOnLED( int psw, bool  state ) {

  psw_relayon[psw] = state;
  if( power_save == false ) {
    if( psw_relayon[psw] == true ) {
      digitalWrite( PSW1_RELAYON_LED + (psw * 2), HIGH );
    } else {
      digitalWrite( PSW1_RELAYON_LED + (psw * 2), LOW );    
    }      
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  checkLightLevel() {
  
  ccd_light_level = analogRead( LIGHT_SENSOR );
  if ( conf.console_logging_enabled == true ) {
    Serial.printf( "Light level = %d\n", ccd_light_level );
  }

  if(( ccd_light_level < conf.light_detect_level ) && ( power_save == false )) {
    power_save = true;
    oled_change_power_save = true;
    digitalWrite( PSW_CONTROL_LED, LOW );    
    digitalWrite( PSW1_ENABLE_LED, LOW );    
    digitalWrite( PSW2_ENABLE_LED, LOW );    
    digitalWrite( PSW1_RELAYON_LED, LOW );
    digitalWrite( PSW2_RELAYON_LED, LOW );
    writeSDAuditLog( "Powersave active" );

  } else if(( ccd_light_level > conf.light_detect_level ) && ( power_save == true )) {
    power_save = false;
    oled_change_power_save = true;
    if( power_control_enabled == true ) {
      digitalWrite( PSW_CONTROL_LED, HIGH );
    }      
    for( int psw = 0; psw < 2; psw++ ) {
      if( psw_control_enabled[psw] == true ) {
        digitalWrite( PSW1_ENABLE_LED + psw, HIGH );
      }      
      if( psw_relayon[psw] == true ) {
        digitalWrite( PSW1_RELAYON_LED + (psw * 2), HIGH );
      }       
    }
    writeSDAuditLog( "Powersave not active" );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// WIFI
////////////////////////////////////////////////////////////////////////////////////////////////////
void  startWifi() {

  char  tmpbuf[50] = {0};
  int   len=0;
  int   count = 0;

  WiFi.persistent( false );
  WiFi.disconnect();
  delay(1000);

  WiFi.mode(WIFI_OFF);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESP32_177dc4");
  WiFi.begin(conf.wifi_ssid, conf.wifi_passwd);
  delay(1000);

  writeSDAuditLog( "Connecting to wifi" );

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    if ( conf.console_logging_enabled == true ) {
      Serial.print(".");
    }
    delay(1000);
    count ++;
    if( count > 10 ) {
      delay(5000);
      ESP.restart();
    }
  }
  if ( conf.console_logging_enabled == true ) {
    Serial.println();
  }
  delay(1000);
  sprintf( tmpbuf, "Connected to %s", WiFi.SSID().c_str() );
  writeSDAuditLog( tmpbuf );  
  sprintf( tmpbuf, "IP = %s", WiFi.localIP().toString().c_str() );
  writeSDAuditLog( tmpbuf );
  
}
