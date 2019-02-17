////////////////////////////////////////////////////////////////////////////////////////////////////
// MCP23017 Ports
////////////////////////////////////////////////////////////////////////////////////////////////////

void  MCP_init() {
  Wire.beginTransmission(mcp_address);
  Wire.write(IODIRA); // IODIRA register
  Wire.write(0x00); // set all of port A to outputs 00000000 (0=out/1=in)
  Wire.endTransmission();

  Wire.beginTransmission(mcp_address);
  Wire.write(IODIRB); // IODIRA register
  Wire.write(0x00); // set all of port B to outputs 00000000 (0=out/1=in)
  Wire.endTransmission();

}

void  MCP_setGPIO_A( byte data ) {
  Wire.beginTransmission(mcp_address);
  Wire.write(GPIOA);
  Wire.write(data);         
  Wire.endTransmission();

  sprintf( result1, "setGPIO_A (%02x)", data );
  Serial.print( result1 );
  sendStrXY( result1, 5, 0);

}

void  MCP_setGPIO_B( byte data ) {
  Wire.beginTransmission(mcp_address);
  Wire.write(GPIOB);
  Wire.write(data);         
  Wire.endTransmission();  

  sprintf( result1, "setGPIO_B (%02x)", data );
  Serial.print( result1 );
  sendStrXY( result1, 5, 0);

}
