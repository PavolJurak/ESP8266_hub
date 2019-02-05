/*-------------------SETUP------------------------*/
void startNRF24L01()
{
  SPI.setHwCs(true);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  radio.begin();
  radio.setCRCLength(RF24_CRC_8);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(5, 10); // delay, count
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.stopListening();
}

/*-----------------------SEND COMMAND FROM RADIO----------------*/
String sendFromNRF2401(String com)
{
  setRadioToSend();
  struct package data;
  String device = "NRF01";
  String command = com;
  device.toCharArray(data.device, sizeof(data.device));
  command.toCharArray(data.command, sizeof(data.command));
  radio.write(&data, sizeof(data));
  Serial.println("Send from NRF2401 command:" + com);
  
  if (com == "GBP") {
    return getBlindPosition();
  }
  if (com = "GEV") {
    return getEepromValues();
  }
  return "";
}

String getBlindPosition()
{
  String json = "{";
  setRadioToRead();
  delay(10);
  long timeSend = millis();
  while ((timeSend + 50) >= millis())
  {
    if (radio.available()){
      struct packagePosition dataPosition;
      radio.read(&dataPosition, sizeof(dataPosition));
      if ((String)dataPosition.device == "NRF01") {
        json += "\"left\":\"" + (String)dataPosition.left +"\",";
        json += "\"right\":\"" + (String)dataPosition.right +"\"";
        //Serial.println(json);
      }
    }
  }
  delay(10);
  json += "}";
  return json;
}

String getEepromValues()
{
  String json = "{";
  setRadioToRead();
  delay(10);
  long timeSend = millis();
  while ((timeSend + 50) >= millis())
  {
    if (radio.available()){
      struct packageSavedPosition dataEepromPosition;
      radio.read(&dataEepromPosition, sizeof(dataEepromPosition));
      if ((String)dataEepromPosition.device == "NRF01") {
        json += "\"P0\":\"" + (String)dataEepromPosition.closeSun +"\",";
        json += "\"P1\":\"" + (String)dataEepromPosition.closeNight +"\",";
        json += "\"P2\":\"" + (String)dataEepromPosition.openLow +"\",";
        json += "\"P3\":\"" + (String)dataEepromPosition.openMiddle +"\",";
        json += "\"P4\":\"" + (String)dataEepromPosition.openHight +"\"";
      }
    }
  }
  delay(10);
  json += "}";
  return json;
}

/******************HELPER FUNCTION***************************/
void setRadioToSend()
{
  radio.stopListening();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(5, 10); // delay, count
  radio.openWritingPipe(address[1]);
  Serial.println("Radio nrf024 was setted to SEND MODE");
}

void setRadioToRead()
{
  radio.openReadingPipe(1, address[0]);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
  Serial.println("Radio nrf024 was setted to READING MODE");
}
