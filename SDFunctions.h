////////////////////////////////////////////////////////////////////////////////////////////////////
#define           SD_MISO         19
#define           SD_SCLK         18
#define           SD_MOSI         23
#define           SD_CS           5

#define           AUDIT_FILENAME      "/audit.log"

#define   FILENAME_LEN    30

static  bool      SD_ok;
static  uint8_t   SD_card_type;
static  uint64_t  SD_card_size;

////////////////////////////////////////////////////////////////////////////////////////////////////
bool initSDCard() {

  SD_ok = true;
  pinMode(SD_MISO, INPUT_PULLUP);
  
  if(!SD.begin()){
    if ( conf.console_logging_enabled == true ) {
      Serial.println("Card Mount Failed");
    }
    SD_ok = false;
  } else {
    SD_card_type = SD.cardType();
      
    if(SD_card_type == CARD_NONE){
      if ( conf.console_logging_enabled == true ) {
        Serial.println("No SD card attached");
      }
      SD_ok = false;
    } else {
      if ( conf.console_logging_enabled == true ) {
        Serial.print("SD Card Type: ");
      }
      if(SD_card_type == CARD_MMC){
        if ( conf.console_logging_enabled == true ) {
          Serial.println("MMC");
        }
      } else if(SD_card_type == CARD_SD){
        if ( conf.console_logging_enabled == true ) {
          Serial.println("SDSC");
        }
      } else if(SD_card_type == CARD_SDHC){
        if ( conf.console_logging_enabled == true ) {
          Serial.println("SDHC");
        }
      } else {
        if ( conf.console_logging_enabled == true ) {
          Serial.println("UNKNOWN");
        }
      }
      SD_card_size = SD.cardSize() / (1024 * 1024);
      if ( conf.console_logging_enabled == true ) {
        Serial.printf("SD Card Size: %lluMB\n", SD_card_size);
      }
    }
  }
  return SD_ok;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool  writeSDAuditLog( char *message ) {

  File f;
  char  filename[FILENAME_LEN];
  bool  writeStatus;

  if ( conf.console_logging_enabled == true ) {
    Serial.println( message );
  }
  if( SD_ok == false ) {
    return false;
  }
  while( SDCardAccessSemaphore );
  SDCardAccessSemaphore = true;

  sprintf( filename, "/audit_%04d%02d%02d.log", year(), month(), day() ); 
  f = SD.open( filename, FILE_APPEND );
  if( f ) {
    f.printf("%02d/%02d/%04d %02d:%02d:%02d :: %s\n", day(), month(), year(), hour(), minute(), second(), message );
    f.close();
    writeStatus =  true;
  } else {
    if ( conf.console_logging_enabled == true ) {
      Serial.printf( "Failed to open audit file %s\n", filename );
    }
    writeStatus =  false;
  }
  SDCardAccessSemaphore = false;
  return  writeStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool  writeSDTimeStamp( float lp, float lp_ma, float sp, float sp_ma, float grid, float batt, float batt_cap, bool psw1, bool psw2 ) {

  File  f;
  char  filename[FILENAME_LEN];
  bool  writeStatus;
    
  if( SD_ok == false ) {
    return false;
  }

  while( SDCardAccessSemaphore );
  SDCardAccessSemaphore = true;
  
  sprintf( filename, "/timestamp_%04d%02d%02d.csv", year(), month(), day() ); 
  
  if( SD.exists( filename ) == false ) {
    f = SD.open( filename, FILE_WRITE );
    if( f ) {
      f.printf("Year,Time,Load Power,Load Power SMA,Solar Power,Solar Power SMA,Grid Power,Battery Power,Battery Capacity,Psw1 State,Psw2 State\n" );            
    }
  } else {
    f = SD.open( filename, FILE_APPEND );
  }
  if( f ) {
    f.printf("%04d/%02d/%02d,%02d:%02d:%02d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d\n",
              year(), month(), day(), hour(), minute(), second(),
              lp, lp_ma, sp, sp_ma, grid, batt, batt_cap, psw1, psw2 );    
    f.close();
    writeStatus = true;
  } else {
    if ( conf.console_logging_enabled == true ) {
      Serial.printf( "Failed to open timestamp %s\n", filename );
    }
    writeStatus = false;
  }
  SDCardAccessSemaphore = false;
  return  writeStatus;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
