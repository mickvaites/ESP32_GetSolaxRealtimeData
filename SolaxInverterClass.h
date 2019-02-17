#include <HTTPClient.h>
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SolaxInverterClasses.h
//
// Classes used to make connections to Solax and Power Switches
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#define SOLAX_REALTIME_DATA_URL             "http://solaxinverter/api/realTimeData.htm"

#define POWER_Q_SIZE      120
#define MAX_SOLVALS       18

#define PV1_CURRENT       0
#define PV2_CURRENT       1
#define PV1_VOLTAGE       2
#define PV2_VOLTAGE       3
#define GRID_CURRENT      4
#define GRID_VOLTAGE      5
#define GRID_POWER        6
#define INNER_TEMP        7
#define SOLAR_TODAY       8
#define SOLAR_TOTAL       9
#define FEED_IN_POWER     10
#define PV1_POWER         11
#define PV2_POWER         12
#define BATTERY_VOLTAGE   13
#define BATTERY_CURRENT   14
#define BATTERY_POWER     15
#define BATTERY_TEMP      16
#define BATTERY_CAPACITY  17

const char  *solax_field_names[] = { "PV1 Current", "PV2 Current", "PV1 Voltage", "PV2 Voltage",
                                     "Grid Current", "Grid Voltage", "Grid Power",
                                     "Inner Temporature", "Solar Today", "Solar Total", "Feed in Power",
                                     "PV1 Power", "PV2 Power",
                                     "Battery Voltage", "Battery Current", "Battery Power", "Battery Temporature", "Battery Capacity", 0 };
////////////////////////////////////////////////////////////////////////////////////////////////////

class SolaxInverter {

  HTTPClient    http;
  
  int           update_interval;
  unsigned long last_update;
  int           collect_failures;

  struct  _powerq {
    float     load_power;
    float     solar_power;
    float     solar_power_gradient;
  }powerq[POWER_Q_SIZE];

public:

  float   solax_vals[MAX_SOLVALS] = {0};
  float   solar_power = 0;
  float   solar_power_sma = 0;
  float   solar_power_gradient = 0;
  float   load_power = 0;
  float   load_power_sma = 0;
  float   battery_capacity = 0;

  bool    oled_update_required;
  bool    network_problem_detected;
   
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  SolaxInverter( int interval ) {
    
    update_interval = interval;
    last_update = millis();
    collect_failures = 0;
    network_problem_detected = false;
    oled_update_required = false;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  update() {
    if((millis() - last_update) > update_interval) {
      last_update = millis();
      collectData();                          // Resets collect_failures as soon as a good one is in.
      oled_update_required = true;
      if( collect_failures > 10 ) {
        collect_failures = 5;                // If not reset by collectData(), allow another 5 before it resets wifi again
        network_problem_detected = true;
      } else {
        network_problem_detected = false;
      }
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  calculateSMA() {

    float sp_avg = 0, lp_avg = 0, sp_gradient = 0;

    for( int a = POWER_Q_SIZE - 1; a > 0; --a ) {
      powerq[a].solar_power = powerq[a-1].solar_power;
      sp_avg += powerq[a].solar_power;
      powerq[a].solar_power_gradient = powerq[a-1].solar_power_gradient;
      sp_gradient += powerq[a].solar_power_gradient;
      powerq[a].load_power = powerq[a-1].load_power;
      lp_avg += powerq[a].load_power;
    }
    powerq[0].solar_power = solar_power;
    sp_avg += powerq[0].solar_power;
    powerq[0].solar_power_gradient = powerq[0].solar_power - powerq[1].solar_power;
    sp_gradient += powerq[0].solar_power_gradient;
    powerq[0].load_power = load_power;
    lp_avg += powerq[0].load_power;

    if( powerq_count < POWER_Q_SIZE ) {
      powerq_count ++;
    }
    solar_power_sma = sp_avg / powerq_count;
    solar_power_gradient = sp_gradient / powerq_count;
    load_power_sma = lp_avg / powerq_count;  
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  collectData() {
  
    char  tmpbuf1[100];
    char  buf[600];
    float current_solvals[MAX_SOLVALS] = {0};
    char  *cp, *cp2, *pp;
    int   c;
    int   http_code;

    if ( conf.console_logging_enabled == true ) {
      Serial.println("CollectSolaxData() ...");
    }

    http.begin(SOLAX_REALTIME_DATA_URL);
        
    http_code = http.GET();
  
    if(http_code == HTTP_CODE_OK) {
      strcpy(buf, http.getString().c_str());
      if( cp = strchr(buf,'[')) {
        cp++;
        if( cp2 = strchr(cp, ']')) {
          *cp2 = '\0';
          c = 0;
          pp = strtok(cp,",");
          do {
            current_solvals[c++] = atof(pp);
          }while(( pp = strtok(NULL, ",")) && (c < MAX_SOLVALS ));
          
          if( c == MAX_SOLVALS ) {
            for( int a = 0; a < MAX_SOLVALS; a++ ) {
              solax_vals[a] = current_solvals[a];
              if ( conf.console_logging_enabled == true ) {
                Serial.printf( " Arg(%02d) %30s = %8.2f\n", a, solax_field_names[a], current_solvals[a] );
              }
            }
            solar_power = solax_vals[PV1_POWER] + solax_vals[PV2_POWER];
            battery_capacity = solax_vals[BATTERY_CAPACITY];
            load_power = solar_power;
            load_power -= solax_vals[FEED_IN_POWER];
            load_power -= solax_vals[BATTERY_POWER];
            calculateSMA();
          } else {
            sprintf( tmpbuf1, "CollectSolaxData - Insufficient arguments c = %d\n", c );
            writeSDAuditLog( tmpbuf1 );
            for( int a = 0; a < c; a++ ) {
              sprintf( tmpbuf1, "Arg(%d) = %8.2f", a, current_solvals[a] );
              writeSDAuditLog( tmpbuf1 );
            }
          }
        }
      }
      if( collect_failures ) {
        sprintf( tmpbuf1, "collectSolaxData - [HTTP] GET success" );
        writeSDAuditLog( tmpbuf1 );
        collect_failures = 0;
      }
    } else {
      collect_failures ++;
      sprintf( tmpbuf1, "collectSolaxData - [HTTP] GET failed ... code: %d - qty(%d)", http_code, collect_failures );
      writeSDAuditLog( tmpbuf1 );
    }
    http.end();
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  dumpSolaxData() {
    Serial.println("");
    Serial.println("** Debug dump of solax data **" );
    for( int a = 0; a < MAX_SOLVALS; a++ ) {
      Serial.printf( "Arg(%d) = %8.2f\n", a, solax_vals[a] );
    }  
    Serial.println("******************************" );
    Serial.println("");
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  void  dataTimeStamp() {
    
    Serial.printf("============= SOLAX =============\n");
    Serial.printf("pv1 current          = %8.2fA\n", solax_vals[PV1_CURRENT] );
    Serial.printf("pv2 current          = %8.2fA\n", solax_vals[PV2_CURRENT] );
    Serial.printf("pv1 voltage          = %8.2fV\n", solax_vals[PV1_VOLTAGE] );
    Serial.printf("pv2 voltage          = %8.2fV\n", solax_vals[PV2_VOLTAGE] );
    Serial.printf("grid current         = %8.2fA\n", solax_vals[GRID_CURRENT] );
    Serial.printf("grid voltage         = %8.2fV\n", solax_vals[GRID_VOLTAGE] );
    Serial.printf("grid power           = %8.2fW\n", solax_vals[GRID_POWER] );
    Serial.printf("inner temp           = %8.2f\n", solax_vals[INNER_TEMP] );
    Serial.printf("solar today          = %8.2fkWh\n", solax_vals[SOLAR_TODAY] );
    Serial.printf("solar total          = %8.2fkWh\n", solax_vals[SOLAR_TOTAL] );
    Serial.printf("feed in power        = %8.2fW\n", solax_vals[FEED_IN_POWER] );
    Serial.printf("pv1 power            = %8.2fW\n", solax_vals[PV1_POWER] );
    Serial.printf("pv2 power            = %8.2fW\n", solax_vals[PV2_POWER] );
    Serial.printf("battery voltage      = %8.2fV\n", solax_vals[BATTERY_VOLTAGE] );
    Serial.printf("battery current      = %8.2fA\n", solax_vals[BATTERY_CURRENT] );
    Serial.printf("battery power        = %8.2fW\n", solax_vals[BATTERY_POWER] );
    Serial.printf("battery temp         = %8.2f°C\n", solax_vals[BATTERY_TEMP] );
    Serial.printf("battery capacity     = %8.2f%%\n", solax_vals[BATTERY_CAPACITY] );
    Serial.printf("=========== Calculated ==========\n");  
    Serial.printf("Current Solar Power  = %8.2fW\n", solar_power );      
    Serial.printf("Current Load Power   = %8.2fW\n", load_power );
    Serial.printf("======== Moving Averages ========\n");  
    Serial.printf("Solar Power          = %8.2fW\n", solar_power_sma );
    Serial.printf("Solar Gradient       = %8.2f\n", solar_power_gradient);
    Serial.printf("Load Power           = %8.2fW\n", load_power_sma );
  }
};
