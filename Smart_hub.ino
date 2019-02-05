#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>
#include <RCSwitch.h>
#include <fauxmoESP.h>
#include <FastLED.h>
#include <WebSocketsServer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include "config.h";
//NRF24L01 - ESP82 12
//1 GND
//2 VCC
//3 CE - GPI05
//4 CSN - GPI016
//5 SCK - GPI014
//6 MOSI - GPI013
//7 MISO - GPI012
//8 IRQ - NC

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdater;

File fsUploadFile;
RCSwitch mySwitch = RCSwitch();
fauxmoESP fauxmo;
/*
  const char *ssid = "ESP8266 Access Point"; // The name of the Wi-Fi network that will be created
  const char *password = "thereisnospoon";   // The password required to connect to it, leave blank for an open network
*/

#define radioPin 4

//NRF024
RF24 radio(5, 16);

//FAST LED
#define DATA_PIN 3
#define NUM_LEDS 7
#define BRIGHTNESS 0
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define delayTimeBetweenSendRC 400

boolean light1 = false;
boolean light2 = false;
boolean light3 = false;
boolean light4 = false;
boolean light5 = false;
boolean light6 = false;
boolean bedroom = false;

long timeLastAction = 0;

boolean actionLight1 = false;
boolean actionLight2 = false;
boolean actionLight3 = false;
boolean actionLight4 = false;
boolean actionLight5 = false;
boolean actionLight6 = false;
boolean actionBedroom = false;

const byte address[][6] = {READING_PIPE, WRITING_PIPE};
char message[80];
boolean send = false;

struct package {
  char device[6];
  char command[7];
};

struct packagePosition {
  char device[6];
  byte left;
  byte right;
};

struct packageSavedPosition {
  char device[6];
  byte closeSun;
  byte closeNight;
  byte openLow;
  byte openMiddle;
  byte openHight;
};

/*----------------------------------SETUP--------------------------*/

void setup()
{
  Serial.begin(115200);
  delay(10);
  EEPROM.begin(512);
  startWiFi();
  startMDNS();
  startSPIFFS();
  startServer();
  startHTTPUpdate();
  //startWebSocket();
  fauxmoSetup();
  startRadio433Mhz(); //433 Mhz
  startNRF24L01();
  //startFastLed();
}

void loop()
{
  fauxmo.handle();
  handleRemoteCommand();
  //webSocket.loop();
  server.handleClient();
}

/*------------------------------SETUP_FUNCTIONS-------------------------*/
void startWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  Serial.println("Connect");
  Serial.println("ssid: " + (String) + WIFI_SSID + ":");
  Serial.println("IP address: " + WiFi.localIP().toString());
  Serial.println();
}

void startServer()
{
  server.begin();
  server.onNotFound(handleNotFound);

  server.on("/upload", HTTP_GET, []() {
    if (!handleFileRead("/upload.html"))
      server.send(404, "text/plain", "404: Not Found");
  });

  server.on("/upload", HTTP_POST, []() {
    server.send(200);
  }, handleFileUpload
           );

  server.on("/ledlamp", HTTP_GET, []() {
    if (!handleFileRead("/ledlamp.html"))
      server.send(404, "text/plain", "404: Not Found");
  });

  server.on("/setlights", HTTP_GET, []() {
    if (!handleFileRead("/setlights.html"))
      server.send(404, "text/plain", "404: Not Found");
    if (server.argName(0) == "radio_code") {
      if (server.arg(0).toInt()) {
        int code = server.arg(0).toInt();
        mySwitch.send(code, 24);
        Serial.println("Posielam radio kod: " + String(code));
      }
    }
  });

  server.on("/jsonBlindPosition", HTTP_GET, []() {
    server.send(200, "application/json", sendFromNRF2401("GBP"));
  });

  server.on("/jsonEeepromValues", HTTP_GET, [](){
    server.send(200, "application/json", sendFromNRF2401("GEV"));
  });

  server.on("/setblinds", HTTP_GET, []() {
    if (!handleFileRead("/setblinds.html")) {
      server.send(404, "text/plain", "404: Not Found");
    }
    if (server.argName(0) == "Code" && server.arg(0) != "") {
        String value = server.arg(0);
        sendFromNRF2401(value);
      }
  });

  server.on("/", HTTP_GET, []() {
    if (!handleFileRead("/"))
      server.send(404, "text/plain", "404: Not Found");
    if (server.args() != 0) {
      if (server.argName(0) == "Light1") {
        if (server.arg(0) == "ON")
          turnOnLight(1);
        else if (server.arg(0) == "OFF")
          turnOffLight(1);
      }
      if (server.argName(0) == "Light2") {
        if (server.arg(0) == "ON")
          turnOnLight(2);
        else if (server.arg(0) == "OFF")
          turnOffLight(2);
      }
      if (server.argName(0) == "AllBedroomLights") {
        if (server.arg(0) == "ON")
          mySwitch.send(ALL_LIGHTS_BEDROOM_ON, 24); // ON
        else if (server.arg(0) == "OFF")
          mySwitch.send(ALL_LIGHTS_BEDROOM_OFF, 24); //OFF
      }
      if (server.argName(0) == "Light3") {
        if (server.arg(0) == "ON")
          turnOnLight(3);
        else if (server.arg(0) == "OFF")
          turnOffLight(3);
      }
      if (server.argName(0) == "Light4") {
        if (server.arg(0) == "ON")
          turnOnLight(4);
        else if (server.arg(0) == "OFF")
          turnOffLight(4);
      }
      if (server.argName(0) == "Light5") {
        if (server.arg(0) == "ON")
          turnOnLight(5);
        else if (server.arg(0) == "OFF")
          turnOffLight(5);
      }
      if (server.argName(0) == "Light6") {
        if (server.arg(0) == "ON")
          turnOnLight(6);
        else if (server.arg(0) == "OFF")
          turnOffLight(6);
      }
      if (server.argName(0) == "AllLivingRoomLights") {
        if (server.arg(0) == "ON") {
          Serial.println("All LIVING ROOM ON");
          turnOnLight(3);
          turnOnLight(4);
          turnOnLight(5);
          turnOnLight(6);
        } else if (server.arg(0) == "OFF") {
          Serial.println("ALL LIVING ROOM OFF");
          turnOffLight(3);
          turnOffLight(4);
          turnOffLight(5);
          turnOffLight(6);
        }
      }
      if (server.argName(0) == "LeftCloseSun") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("L0");
        }
      }
      if (server.argName(0) == "RightCloseSun") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("R0");
        }
      }
      if (server.argName(0) == "AllCloseSun") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("A0");
        }
      }
      if (server.argName(0) == "LeftCloseNight") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("L1");
        }
      }
      if (server.argName(0) == "RightCloseNight") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("R1");
        }
      }
      if (server.argName(0) == "AllCloseNight") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("A1");
        }
      }
      if (server.argName(0) == "LeftOpenLow") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("L2");
        }
      }
      if (server.argName(0) == "RightOpenLow") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("R2");
        }
      }
      if (server.argName(0) == "AllOpenLow") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("A2");
        }
      }
      if (server.argName(0) == "LeftOpenMiddle") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("L3");
        }
      }
      if (server.argName(0) == "RightOpenMiddle") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("R3");
        }
      }
      if (server.argName(0) == "AllOpenMiddle") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("A3");
        }
      }
      if (server.argName(0) == "LeftOpenHight") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("L4");
        }
      }
      if (server.argName(0) == "RightOpenHight") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("R4");
        }
      }
      if (server.argName(0) == "AllOpenHight") {
        if (server.arg(0) == "ON") {
          sendFromNRF2401("A4");
        }
      }
      if (server.argName(0) == "LeftBlind") {
        int value = server.arg(0).toInt();
        if (value >= 0 && value <= 180) {
          sendFromNRF2401("L-"+server.arg(0));
        }
      }
      if (server.argName(0) == "RightBlind") {
        int value = server.arg(0).toInt();
        if (value >= 0 && value <= 180) {
          sendFromNRF2401("R-"+server.arg(0));
        }
      }
    }
  });
  Serial.println("HTTP server started");
}

/********************************************START FUNCTIONS***************************/
void startMDNS()
{
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
}

void startWebSocket()
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started.");
}

void startFastLed()
{
  delay(1000);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 250);
  FastLED.addLeds<WS2812B, DATA_PIN, BRG>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
}

void startRadio433Mhz()
{
  mySwitch.enableTransmit(radioPin);
  mySwitch.setPulseLength(321);
  mySwitch.setProtocol(1);
  mySwitch.setRepeatTransmit(15);
}

void startHTTPUpdate()
{
  httpUpdater.setup(&server, UPDATE_PATH, UPDATE_USERNAME, UPDATE_PASSWORD);
  Serial.print("HTTPUpdateServer ready! Open http://" + WiFi.localIP().toString() + "/update in your browser\n");
}

void startSPIFFS()
{
  SPIFFS.begin();
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.println("\tFS File: " + (String)fileName.c_str() + ", size: " + (String)formatBytes(fileSize).c_str());
      //Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.println("\n");
  }
}
/*-----------------------------LIGHTS FUNCTION FOR ON/OFF---------------*/
void turnOnLight(byte numberLight)
{
  switch (numberLight) {
    case 1:
      actionLight1 = true;
      light1 = true;
      break;
    case 2:
      actionLight2 = true;
      light2 = true;
      break;
    case 3:
      actionLight3 = true;
      light3 = true;
      break;
    case 4:
      actionLight4 = true;
      light4 = true;
      break;
    case 5:
      actionLight5 = true;
      light5 = true;
      break;
    case 6:
      actionLight6 = true;
      light6 = true;
      break;
  }
}

void turnOffLight(byte numberLight)
{
  switch (numberLight) {
    case 1:
      actionLight1 = true;
      light1 = false;
      break;
    case 2:
      actionLight2 = true;
      light2 = false;
    case 3:
      actionLight3 = true;
      light3 = false;
      break;
    case 4:
      actionLight4 = true;
      light4 = false;
      break;
    case 5:
      actionLight5 = true;
      light5 = false;
      break;
    case 6:
      actionLight6 = true;
      light6 = false;
      break;
  }
}
/*------------------------------SERVER_HANDLERS------------------------*/
void handleNotFound()
{
  if (!handleFileRead(server.uri())) {  //check of the file exists in the flash memory, if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
}

boolean handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r"); //Open the file
    size_t send = server.streamFile(file, contentType);
    file.close();
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;
}

void handleFileUpload()
{
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {
      String pathWithGz = path + ".gz";
      if (SPIFFS.exists(pathWithGz)) {
        SPIFFS.remove(pathWithGz);
      }
    }
    Serial.println("handleFileUpload Name: " + path);
    fsUploadFile = SPIFFS.open(path, "w");
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      Serial.println("handleFileUpload Size: " + upload.totalSize);
      server.sendHeader("Location", "/success.html");
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}
/*-------------------------WebSocket_Funtion---------------------------*/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght)
{
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[" + (String)num + "]" + " Disconnected!");
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.println("[" + (String)num + "]" + "Connected");
      }
      break;
    case WStype_TEXT: {
        Serial.println("[" + (String)num + "] Text : " + (int)payload);
        if (payload[0] == '#') {
          uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
          int r = ((rgb >> 16) & 0xFF); // R: bits 16-24
          int g = ((rgb >> 8) & 0xFF); // G: bits 8-16
          int b = rgb & 0xFF; // B: bits 0-8
          Serial.println("R: " + (String)r);
          Serial.println("G: " + (String)g);
          Serial.println("B: " + (String)b);
          leds[0] = CRGB(r, b, g);
          leds[1] = CRGB(r, b, g);
          leds[2] = CRGB(r, b, g);
          leds[3] = CRGB(r, b, g);
          leds[4] = CRGB(r, b, g);
          leds[5] = CRGB(r, b, g);
          leds[6] = CRGB(r, b, g);
          FastLED.setBrightness(200);
          FastLED.show();
        }
      }
      break;
  }
}

/*------------------------HELPER_FUNCTIONS-----------------------------*/
String getContentType(String filename)
{
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

String formatBytes(size_t bytes)
{
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}
/*-----------------------------CONTROL_LED_FUNCTIONS---------------------------*/
void runLeds()
{
  Serial.println("Turn On Leds");
  leds[0] = CRGB(255, 0, 0);
  leds[1] = CRGB(0, 255, 0);
  leds[2] = CRGB(0, 0, 255);
  leds[3] = CRGB(255, 255, 255);
  FastLED.show();
}

void stopFastLed()
{
  Serial.println("Turn off Leds");
  FastLED.setBrightness(0);
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();
}
/*--------------------------EEPROM_FUNCTION-------------------------*/

void saveTimeUp(unsigned int time)
{
  if (loadTimeUp() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(0, b1);
    EEPROM.write(1, b2);
    EEPROM.commit();
  }
}

unsigned loadTimeUp()
{
  unsigned int time = (EEPROM.read(0) & 0xFF) << 8 | (EEPROM.read(1) & 0xFF);
  return time;
}

void saveTimeDown(unsigned int time)
{
  if (loadTimeDown() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(2, b1);
    EEPROM.write(3, b2);
    EEPROM.commit();
  }
}

unsigned int loadTimeDown()
{
  unsigned int time = (EEPROM.read(2) & 0xFF) << 8 | (EEPROM.read(3) & 0xFF);
  return time;
}

void saveTime90(unsigned int time)
{
  if (loadTime90() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(4, b1);
    EEPROM.write(5, b2);
    EEPROM.commit();
  }
}

unsigned int loadTime90()
{
  unsigned int time = (EEPROM.read(4) & 0xFF) << 8 | (EEPROM.read(5) & 0xFF);
  return time;
}

void saveTime180(unsigned int time)
{
  if (loadTime180() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(6, b1);
    EEPROM.write(7, b2);
    EEPROM.commit();
  }
}

unsigned int loadTime180()
{
  unsigned int time = (EEPROM.read(6) & 0xFF) << 8 | (EEPROM.read(7) & 0xFF);
  return time;
}

void saveTimeOpen(unsigned int time)
{
  if (loadTimeOpen() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(8, b1);
    EEPROM.write(9, b2);
    EEPROM.commit();
  }
}

unsigned int loadTimeOpen()
{
  unsigned int time = (EEPROM.read(8) & 0xFF) << 8 | (EEPROM.read(9) & 0xFF);
  return time;
}

void saveTimeLamelActual(unsigned int time)
{
  if (loadTimeLamel() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(10, b1);
    EEPROM.write(11, b2);
    EEPROM.commit();
  }
}

unsigned int loadTimeLamel()
{
  unsigned int time = (EEPROM.read(10) & 0xFF) << 8 | (EEPROM.read(11) & 0xFF);
  return time;
}

void saveTimeBlindRollActual(unsigned int time)
{
  if (loadTimeBlindRoll() != time) {
    uint8_t b1 = ((time >> 8) & 0xFF);
    uint8_t b2 = (time & 0xFF);
    EEPROM.write(12, b1);
    EEPROM.write(13, b2);
    EEPROM.commit();
  }
}

unsigned int loadTimeBlindRoll()
{
  unsigned int time = (EEPROM.read(12) & 0xFF) << 8 | (EEPROM.read(13) & 0xFF);
  return time;
}
