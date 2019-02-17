#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <U8x8lib.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "FS.h"
#include "SD.h"
#include "RTC_DS1307.h"
#include <rom/rtc.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#define   PG_VERSION                "0.15c"                           // Program version

#define   DEFAULT_WIFI_SSID          "yourssid"
#define   DEFAULT_WIFI_PASSWD        "yourwifipasswd"

#define SOLAX_REALTIME_DATA_URL     "http://solaxinverter/api/realTimeData.htm"           // URL or IP address of Solax Inverter[

#define   WWW_USERNAME              "mywebusername"                   // Web UI Username
#define   WWW_PASSWORD              "mywebpassword"                   // Web UI Password

#define   TIMER_PSW1_URL            "http://esp_power_switch_1"       // URL or IP of Power Switch One
#define   TIMER_PSW2_URL            "http://esp_power_switch_2"       // URL or IP of Power Switch Two

#define   TIMER_PSW_USERNAME        "powerswusername"                 // Username for Power Switches
#define   TIMER_PSW_PASSWORD        "powerswpassword"                 // Password for Power Switches


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// Three screen types and layouts
////////////////////////////////////////////////////////////////////////////////////////////////////
//#define   SSD1306_DIYMORE   1                               // DIY More - doesn't use external LED's
#define   SH1106_SPI_OLED   1                                 // Production build 
//#define   SSD1306_WEMOS    1                                // Same as above but with broken top line of display

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions called from different modules
////////////////////////////////////////////////////////////////////////////////////////////////////
bool      writeSDTimeStamp( float, float, float, float, float, float, float, bool, bool );
bool      writeSDAuditLog( char * );

bool      SDCardAccessSemaphore = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
RTC_DS1307 rtc;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SharedLibraries.h"              // Must appear before other included classes
#include "SDFunctions.h"
#include "NTPTimeClass.h"
#include "SolaxInverterClass.h"
#include "PowerSwitchClass.h"
#include "OLEDFunctions.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
NTPTime         Ntp1( 3600000, "pool.ntp.org" );
SolaxInverter   Solax1( 12000 );
PowerSwitch     Psw1( 30000, TIMER_PSW1_URL, 0 );
PowerSwitch     Psw2( 30000, TIMER_PSW2_URL, 1 );

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#include  "HTTPFunctions.h"             // Dependent on Psw & Solax Class Definitions
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
Ticker                UpdateTheScreen;

#define               CLOCK_TICK_PERIOD           10
#define               BATTERY_TICK_PERIOD         2
#define               TIMESTAMP_PERIOD            598
#define               PSW_CHECK_PERIOD            (600 * 2) + 7
#define               LIGHT_CHECK_PERIOD          49

static  bool          wifi_network_error = false;
  
static  bool          timestamp_needed = true;
static  bool          psw_check_needed = false;
static  bool          light_check_needed = false;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Update OLED Screen - called 10 times a second (runs on CPU0)
////////////////////////////////////////////////////////////////////////////////////////////////////
static void  UpdateTheScreenCb() {

  static char   *months_str[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }; 
  static unsigned short clock_tick_counter = CLOCK_TICK_PERIOD;
  static unsigned short battery_tick_counter = BATTERY_TICK_PERIOD;
  static unsigned short timestamp_counter = TIMESTAMP_PERIOD;
  static unsigned short psw_check_counter = PSW_CHECK_PERIOD;
  static unsigned short light_check_counter = LIGHT_CHECK_PERIOD;

  char          ip_addr[16];
  char          tmpstr[30];
  int           a;
  float         f;
  static  bool  battery_on_display = false;          
  static  int   charge_bar0 = 0;
  static  int   charge_bar1 = 0;

  clock_tick_counter --;
  timestamp_counter --;
  psw_check_counter --;
  light_check_counter --;
  
  if( clock_tick_counter == 0 ) {
    clock_tick_counter = CLOCK_TICK_PERIOD;
    
    sprintf(tmpstr, "%2d%3s %02d:%02d:%02d ",
                    day(), months_str[month() - 1], hour(), minute(), second() );
    writeTextToOLED(0, OLED_ROW0, tmpstr, 1);
    if( WiFi.status() == WL_CONNECTED ) {
      u8x8.drawTile( 15, OLED_ROW0, 1, signal_tile );
      u8x8.drawTile( 0, OLED_ROW7, 1, ip_tile );
      sprintf(ip_addr, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      sprintf(tmpstr, "%15s", ip_addr );
      writeTextToOLED(1,OLED_ROW7,tmpstr,0);
    } else {
      u8x8.drawTile( 15, OLED_ROW0, 1, no_signal_tile );
      if( wifi_network_error == true ) {
        writeTextToOLED(0,OLED_ROW7," Network  Error ",1);          
      } else {
        writeTextToOLED(0,OLED_ROW7,"Waiting for WiFi",0);
      }
    }
    if( oled_change_power_save == true ) {
      oled_change_power_save = false;
      u8x8.setPowerSave(power_save);        
    }
  }
  
  if( timestamp_counter == 0 ) {
    timestamp_counter = TIMESTAMP_PERIOD;
    timestamp_needed = true;
  }
  
  if( psw_check_counter == 0 ) {
    psw_check_counter = PSW_CHECK_PERIOD;
    psw_check_needed = true;
  }

  if( light_check_counter == 0 ) {
    light_check_counter = LIGHT_CHECK_PERIOD;
    light_check_needed = true;
  }

  if( Solax1.oled_update_required == true ) {

    sprintf(tmpstr, "Load  %4.0f/%4.0fW", Solax1.load_power, Solax1.load_power_sma );
    writeTextToOLED(0,OLED_ROW1,tmpstr,0);

    sprintf(tmpstr, "Solar %4.0f/%4.0fW", Solax1.solar_power, Solax1.solar_power_sma );
    writeTextToOLED(0,OLED_ROW2,tmpstr,0);

    sprintf(tmpstr, "Grid %5.0fW", Solax1.solax_vals[FEED_IN_POWER] );
    writeTextToOLED(0,OLED_ROW4,tmpstr,0);

    sprintf(tmpstr, " %5.0fW %3.0f%%", Solax1.solax_vals[BATTERY_POWER], Solax1.solax_vals[BATTERY_CAPACITY] );
    writeTextToOLED(4,OLED_ROW5,tmpstr,0);

    if( OLED_SHOWPSW ) {
      sprintf(tmpstr, "Sw1 %-3s", Psw1.psw_relay_state_str );
      writeTextToOLED(0,OLED_ROW6,tmpstr,conf.psw[0].control_enabled);

      sprintf(tmpstr, "Sw2 %-3s", Psw2.psw_relay_state_str );
      writeTextToOLED(9,OLED_ROW6,tmpstr,conf.psw[1].control_enabled);
    }

    battery_on_display = true;
    Solax1.oled_update_required = false;
  }
  
  if( battery_on_display == true ) {
    battery_tick_counter --;
    if( battery_tick_counter == 0 ) {
      battery_tick_counter = BATTERY_TICK_PERIOD;

      for( a = 4, f = 11; a <= 25; a += 3, f += 11 ) {
        if( Solax1.solax_vals[BATTERY_CAPACITY] >= f ) {
          battery_tile[a] = 0b10111010;
        } else {
          if( Solax1.solax_vals[BATTERY_POWER] > 0 ) {
            if( charge_bar1 == a ) {
              battery_tile[a] = 0b10111010;
              charge_bar0 = a;
              charge_bar1 = 0;
            } else if( charge_bar0 == a ) {
              battery_tile[a] = 0b10000010;
              charge_bar0 = 0;
              charge_bar1 = 0;
            } else if (charge_bar1 == 0 ) {
              battery_tile[a] = 0b10000010;
              charge_bar0 = 0;
              charge_bar1 = a;
            } else if( a < charge_bar0 || a < charge_bar1 ) {
              battery_tile[a] = 0b10111010;
            } else {
              battery_tile[a] = 0b10000010;                        
            }
          } else {
            battery_tile[a] = 0b10000010;
          }
        }
      }
      u8x8.drawTile( 0, OLED_ROW5, 4, battery_tile );
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_CPU_RESET_REASON_TEXT   16
static  char  *CPU_reset_reason_text[] = {  "UNKNOWN          (?)  : Unknown reason",
                                            "POWERON_RESET    (1)  : Vbat power on reset",
                                            "UNKNOWN          (?)  : Unknown reason",
                                            "SW_RESET         (3)  : Software reset digital core",
                                            "OWDT_RESET       (4)  : Legacy watch dog reset digital core",
                                            "DEEPSLEEP_RESET  (5)  : Deep Sleep reset digital core",
                                            "SDIO_RESET       (6)  : Reset by SLC module, reset digital core",
                                            "TG0WDT_SYS_RESET (7)  : Timer Group0 Watch dog reset digital core",
                                            "TG1WDT_SYS_RESET (8)  : Timer Group1 Watch dog reset digital core",
                                            "RTCWDT_SYS_RESET (9)  : RTC Watch dog Reset digital core",
                                            "INTRUSION_RESET .(10) : Instrusion tested to reset CPU",
                                            "TGWDT_CPU_RESET .(11) : Time Group reset CPU",
                                            "SW_CPU_RESET .   (12) : Software reset CPU",
                                            "RTCWDT_CPU_RESET (13) : RTC Watch dog Reset CPU",
                                            "EXT_CPU_RESET    (14) : for APP CPU, reseted by PRO CPU",
                                            "RTCWDT_BROWN_OUT_RESET (15) : Reset when the vdd voltage is not stable",
                                            "RTCWDT_RTC_RESET (16) : RTC Watch dog reset digital core and rtc module" };

char  *return_CPU_reset_reason( RESET_REASON  reason ) {
  switch( reason ) {
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      return( CPU_reset_reason_text[reason] );
      break;
    default:
      return( CPU_reset_reason_text[0] );
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  char  tmpbuf[100] = {0};
 
  Serial.begin(115200);
  delay(1000);

  Serial.println("Solar Mon starting ...");
  Wire.begin(SDA, SCL );

  Ntp1.begin();
  
  initSDCard();
  writeSDAuditLog( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" );
  writeSDAuditLog( "++++++++++++++++++++++++++ Solar Mon Starting +++++++++++++++++++++++++++" );

  sprintf( tmpbuf, "CPU0 Reset reason : %s", return_CPU_reset_reason(rtc_get_reset_reason(0)));
  writeSDAuditLog( tmpbuf );
  sprintf( tmpbuf, "CPU1 Reset reason : %s", return_CPU_reset_reason(rtc_get_reset_reason(1)));
  writeSDAuditLog( tmpbuf );

  loadConfiguration();
  delay(500);
  // Serial.setDebugOutput(true);
  
  initOLED();
  writeTextToOLED(0,1," Solar  Monitor ",1);
  
  sprintf( tmpbuf, "  %14s", PG_VERSION );
  writeTextToOLED(0,2,tmpbuf,1);

  sprintf( tmpbuf, "Solar Monitor - Version %s", PG_VERSION );
  writeSDAuditLog( tmpbuf );

  u8x8.drawTile( 2, 4, 1, copyright_tile );
  writeTextToOLED(3,4,"Mick Vaites",0);

  delay(5000);
  clearDisplayOLED();
  
  UpdateTheScreen.attach_ms( 100, UpdateTheScreenCb );
  initLEDs();
  setControlEnabledLED( conf.psw_control_enabled );
  
  startWifi();
  delay(1000);

  Psw1.begin();
  Psw2.begin();
  
  delay(1000);
  startWebServer();

  writeSDAuditLog( "++++++++++++++++++++++++++++ Boot Complete ++++++++++++++++++++++++++++++" );
  writeSDAuditLog( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" );

  delay(1000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main loop - (runs on CPU1)
////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    writeSDAuditLog( "Lost Wifi connection ... reconnecting ..." );
    wifi_network_error = true;
    delay(5000);
    startWifi();
    wifi_network_error = false;
  } else if( Solax1.network_problem_detected == true ) {
    writeSDAuditLog( "Solax network problem detected .. restarting Wifi connection ..." );
    wifi_network_error = true;
    delay(5000);
    startWifi();
    Solax1.update();
    wifi_network_error = false;    
  } else {
    if( timestamp_needed == true ) {
      writeSDTimeStamp( Solax1.load_power, Solax1.load_power_sma,
                        Solax1.solar_power, Solax1.solar_power_sma,
                        Solax1.solax_vals[FEED_IN_POWER],
                        Solax1.solax_vals[BATTERY_POWER], Solax1.battery_capacity,
                        conf.psw[0].relay_state, conf.psw[1].relay_state );
      if ( conf.console_logging_enabled == true ) {
        Serial.println("");
        Serial.printf("##### %02d/%02d/%04d  %02d:%02d:%02d #####\n", day(), month(), year(), hour(), minute(), second() );
        Serial.println("");
        Solax1.dataTimeStamp();
        Serial.printf("========== Power Switches =======\n" );
        Psw1.dataTimeStamp();
        Psw2.dataTimeStamp();
        Serial.printf("=================================\n");
        Serial.println("");
      }
      timestamp_needed = false;
    }

    Solax1.update();
    Psw1.update();
    Psw2.update(); 
    Ntp1.update();

    if(( conf.psw_control_enabled == true ) && ( psw_check_needed == true )) {
      psw_check_needed = false;

      if( conf.psw[0].control_enabled == true ) {
        Psw1.checkPSWState( Solax1.solar_power_sma, Solax1.load_power_sma, Solax1.battery_capacity );
      }
      if( conf.psw[1].control_enabled == true ) {
        Psw2.checkPSWState( Solax1.solar_power_sma, Solax1.load_power_sma, Solax1.battery_capacity );
      }
    }
    if( light_check_needed == true ) {
      checkLightLevel();
      light_check_needed = false;
    }
    server.handleClient();                // Service webserver requests    
  }
}
