////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t  signal_tile[8] = {

0b10011111,
0b11111111,  
0b10001111,  
0b11111111,
0b10000111,
0b11111111,
0b10000011,
0b11111111, 

};

uint8_t  no_signal_tile[8] = {

0b10011010,
0b11111101,  
0b10001010,  
0b11111111,
0b10000111,
0b11111111,
0b10000011,
0b11111111, 

};

uint8_t  battery_tile[32] = {

0b00000000,       // 0
0b11111110,
0b10000010,
0b10000010,
0b10111010,       // 4 - 11%
0b10000010,
0b10000010,
0b10111010,       // 7 - 22%
0b10000010,
0b10000010,
0b10111010,       // 10 - 33%
0b10000010,
0b10000010,
0b10111010,       // 13 - 44%
0b10000010,
0b10000010,
0b10111010,       // 16 - 55%
0b10000010,
0b10000010,
0b10111010,       // 19 - 66%
0b10000010,
0b10000010,
0b10111010,       // 22 - 77%
0b10000010,
0b10000010,
0b10111010,       // 25 - 88%
0b10000010,
0b10000010,
0b11111110,
0b01111100, 
0b00000000, 
0b00000000, 

};

uint8_t  ip_tile[8] = {

0b00000000,
0b01111110,
0b00000000,
0b01111110,
0b00010010,
0b00011110,
0b00000000,
0b00000000,

};

uint8_t  copyright_tile[8] = {

0b01111100,
0b10000010,
0b10111010,
0b10101010,
0b10101010,
0b10000010,
0b01111100,
0b00000000,

};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef    SH1106_SPI_OLED
////////////////////////////////////////////////////////////////////////////////////////////////////
// OLED - SH1106 - HSPI - On HelTech board PRODUCTION
////////////////////////////////////////////////////////////////////////////////////////////////////
#define OLED_MOSI     13      // D1
#define OLED_CLK      14      // D0
#define OLED_DC       27      // DC
#define OLED_CS       15      // CS
#define OLED_RESET    26      // RES

#define OLED_ROW0     0
#define OLED_ROW1     1
#define OLED_ROW2     2
#define OLED_ROW3     3     // Not normally used
#define OLED_ROW4     4
#define OLED_ROW5     5
#define OLED_ROW6     6
#define OLED_ROW7     7
#define OLED_INVERTED 0
#define OLED_SHOWPSW  0

U8X8_SH1106_128X64_NONAME_4W_SW_SPI u8x8(OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RESET);

// If I2C wanted on HelTech board use Pins 21 & 22
#define SDA           21
#define SCL           22
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#elif    SSD1306_WEMOS
////////////////////////////////////////////////////////////////////////////////////////////////////
// OLED - SSD1306 WeMos
////////////////////////////////////////////////////////////////////////////////////////////////////
#define OLED_MOSI     13      // D1  - TCK
#define OLED_CLK      14      // D0  - TMS
#define OLED_DC       27      // DC  - IO27
#define OLED_CS       15      // CS  - TD0
#define OLED_RESET    26      // RES - IO26

#define OLED_ROW0     1
#define OLED_ROW1     2
#define OLED_ROW2     3
#define OLED_ROW3     4     // Not normally used
#define OLED_ROW4     4
#define OLED_ROW5     5
#define OLED_ROW6     6
#define OLED_ROW7     7
#define OLED_INVERTED 0
#define OLED_SHOWPSW  1

#define SDA           21      // D1
#define SCL           22      // D0

//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
U8X8_SSD1306_128X64_NONAME_4W_SW_SPI u8x8(OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RESET);

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

#ifdef   SSD1306_DIYMORE
////////////////////////////////////////////////////////////////////////////////////////////////////
// OLED - SSD1306 I2C
////////////////////////////////////////////////////////////////////////////////////////////////////
#define OLED_SDA      5
#define OLED_SCL      4

#define OLED_ROW0     0
#define OLED_ROW1     1
#define OLED_ROW2     2
#define OLED_ROW3     3     // Not normally used
#define OLED_ROW4     4
#define OLED_ROW5     5
#define OLED_ROW6     6
#define OLED_ROW7     7
#define OLED_INVERTED 1
#define OLED_SHOWPSW  1

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ OLED_SCL, /* data=*/ OLED_SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
//
#define SDA           21      // D1
#define SCL           22      // D0
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void  initOLED() {
  u8x8.begin();

  u8x8.setFlipMode(OLED_INVERTED);

  u8x8.clearDisplay();
  
  writeSDAuditLog("OLED Initialised");

  u8x8.setFont(u8x8_font_chroma48medium8_r);
//  u8x8.setFont(u8x8_font_artossans8_r);
//  u8x8.setFont(u8x8_font_victoriamedium8_r);
//  u8x8.setFont(u8x8_font_artosserif8_r);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void  writeTextToOLED( int x, int y, char *str, bool inverted ) {

  if( inverted ) {
      u8x8.inverse();
  } else {
      u8x8.noInverse();
  }
  u8x8.drawString(x, y, str );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  clearDisplayOLED() {
  
  u8x8.clearDisplay();
}
