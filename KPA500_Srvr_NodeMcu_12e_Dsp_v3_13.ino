/**************************************************************************
  This code started as the example ST7735 graphictest code. Modified for use with low cost
  DIGISHUO ESP8266 NodeMCU CH340 ESP-12E Development Board Kit with TFT 1.44" Screen
  Original colde ritten by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution

KPA-500 aserver code from Guy Zebrick WD5ACP
Free for NON COMMERCIAL amateur radio use.

Be sure to add the NodeMCU 1.0 board usign the board manager. Check the libraries required below...

Requires RS-232 DB-9 com port.
Connections
ESP8266 <> RS-232
D6 <> VCC
RX <> RXD
TX <> TXD
G  <> GND

Requires ST7735 TFT 1.44" Screeb
Connections
ESP8266  <> Screen
VCC     <> VCC
G       <> GND
D4 LED  <> LED
D5 CLK  <> CLK
D7 SDI  <> SSI
D3 RS   <> RS
D2 RST  <> RST
D1 CS   <> CS

It's also handy to wire an external momentary normally open push button across pins
RST and G
*/
String CodeVersion = "3.13";
#define PortPowerPin D6  // NO ; needed on definitions

// #include "NetworkSettings.h"

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>  // Multicast DNS
#include <ArduinoOTA.h>   // Allows over-the-air OTA programming of Arduino via WiFi (once it is loaded with the correct files - see the internet for details
// Required for graphics
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <SPI.h>
// TPins specific for the low cost kit. Note TFT_DC is the same as the RS pin.
#define TFT_CS D1
#define TFT_RST D2
#define TFT_DC D3
// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Set web server port number to 80
WiFiServer HttpServer(80);
// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 60000;

// Variables needed for KPA500 code-----------------------------------------------------------------------------------------------------
boolean commStatus = false;

const byte numChars = 32;
char receivedChars[numChars];  // an array to store the received data

boolean newData = false;
String inString = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
String BootMode = "null";
String AmpConnect = "null";
String OnMode = "null";
String OpMode = "null";
String BandCode = "null";
String FaultCode = "null";
String PaTemp = "null";
int TempC = 0;
int TempF = 0;
String FanMin = "fan min null";
float PaVolts = 0;
float VMin = 0;
float VMax = 0;
float PaAmps = 0;
int PoWatts = 0;
float PoSwr = 0;
int PeakWatts = 0;
float PeakSwr = 0;
int InputWatts = 0;
float PeakInputWatts = 0;
int PeakTemp = 0;
int PeakTempC = 0;
float RevNumber = 0.0;
int SerialNumber = 0;

String Tx = " ";

int KeyCheckCount = 0;

boolean AutoFanCommand = false;  // trigger to run fan in low speed
boolean AutoFanButton = false;   // Button to enable auto fan temp control

int PeakPoLED = 0;
int PeakSwrLED = 0;

String TempStartPt = "85";
String TempStopPt = "80";

boolean StartFlag = false;  // flags to prevent repeated commands if not needed...
boolean StopFlag = false;   //


//=========================================================================================================================================================
// These are the routines that run based on opening the wbepage root and/or issuing a commad from the server.
//=========================================================================================================================================================

void recvWithEndMarker() {  //  receive serial port data ........................................................................................................
  static byte ndx = 0;
  char endMarker = ';';
  char rc;
  delay(18);  //was 50 - 18 seems to work well
  while (Serial.available() > 0 && newData == false) {

    rc = Serial.read();

    if (rc != endMarker) {

      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
    inString = String(receivedChars);
    delay(5);  // add small delay to make sure LED is bright enough to see
  }
  newData = false;
}

// This just displays the info on the local screen dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd
void displayIP() {
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.print("WD5ACP<>KPA500 v");
  tft.setTextColor(ST77XX_WHITE);
  tft.println(CodeVersion);
  tft.println("");
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("WiFi connection to:");
  tft.setTextColor(ST77XX_WHITE);
  tft.println(WiFi.SSID());
  tft.println("");
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("Network IP info: ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println(WiFi.localIP());
  tft.setTextColor(ST77XX_YELLOW);
}


// Initial Setup      iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
void setup(void) {
#include "NetworkSettings.h"

  pinMode(PortPowerPin, OUTPUT);    // Prepare external TTL <> RS232 board serial port power up sequence..........................................
  digitalWrite(PortPowerPin, LOW);  // make sure power is off to the RS-232 board since the KPA-500 does not like garbage into the port


  // use this initializer if using a 1.44" TFT:
  tft.initR(INITR_144GREENTAB);  // Init ST7735R chip, green tab
  tft.setRotation(3);            // Rotate to match
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.println("Screen Initialized");

  tft.println("Serial @ 115,200 for 15 secs");
  Serial.begin(115200);  // set the data rate for the single serial port - shared with USB. 38,400 works with the KPA-500 amp but for 5 seconds run high for USB port
  delay(15000);
  tft.println("Serial Port reStarting @ 38,400");
  delay(100);
  pinMode(PortPowerPin, OUTPUT);    // Prepare external TTL <> RS232 board serial port power up sequence..........................................
  digitalWrite(PortPowerPin, LOW);  // make sure power is off to the RS-232 board since the KPA-500 does not like garbage into the port
  delay(1000);
  Serial.begin(38400);  // set the data rate for the single serial port - shared with USB. 38,400 works with the KPA-500 amp
  delay(100);
  digitalWrite(PortPowerPin, HIGH);  // Now, options, power up the TTL-232 board off the PortPowerPin
  delay(100);
  tft.println("Serial Port ready");


  tft.println("Next up... WiFi setup");
  delay(100);
  if (UseStaticIP) {
    if (WiFi.config(staticIP, gateway, subnet, pridns, secdns) == false) {
      tft.println("Configuration failed.");
    }
  } else {
  };


  WiFi.begin(mySSID, myPASS);  // Was commented out prior to 6.11
  WiFi.mode(WIFI_STA);
  // WiFi.hostname(myHostName);  // Was commented out prior tp 6.11
  WiFi.setHostname(myHostName.c_str());  //define hostname

  delay(100);
  tft.println("waiting for a connection");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
  }
  //If connection successful show IP address in serial monitor
  tft.println("");
  tft.print("Connected to ");
  tft.print(WiFi.SSID());
  tft.print(" with IP address: ");
  tft.println(WiFi.localIP());  //IP address assigned to your ESP
  HttpServer.begin();

  tft.print("Using myHostName: ");
  // tft.println(myHostName);

  if (!MDNS.begin(myHostName)) {
    tft.println("Error seting up MDNS responder for hostname");
  } else {
    tft.print("MDNS started with hostname : ");
    tft.print(myHostName);
    tft.print(". Access via a browser with URL of HTTP://");
    tft.print(myHostName);
    tft.println(".local");
    tft.println("");
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);

  ArduinoOTA.onStart([]() {  // The Over The Air programming funtion is enabled ...............................................................
    tft.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    tft.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    tft.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    tft.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) tft.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) tft.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) tft.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) tft.println("Receive Failed");
    else if (error == OTA_END_ERROR) tft.println("End Failed");
  });
  ArduinoOTA.begin();
}

// End of initial Setup -------------------------------------------------------------------------------------------------------------------------------------------------------------


// Here's the MAIN LOOP MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
void loop() {
#include "NetworkSettings.h"
  ArduinoOTA.handle();
  delay(200);

  // tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  displayIP();

  //Serial.println("test");
  //delay(2000);

  Serial.print("^ON;");
  recvWithEndMarker();
  delay(10);
  Serial.print("^ON;");
  recvWithEndMarker();

  tft.setCursor(0, 60);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("last serial msg:");
  tft.setCursor(0, 70);
  tft.fillRect(0, 70, 124, 70, ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(inString);
  if (OnMode == "null") {
    tft.setTextColor(ST77XX_RED);
  } else {
    tft.setTextColor(ST77XX_GREEN);
  }

  if (inString == "^ON1") {
    OnMode = "KPA500 Power is ON";
    // Serial.println("Power is On. inString: " + inString);
  } else {
    OnMode = "null";
    // Serial.println("Power is off. inString: " + inString);
  }
  AmpConnect = "Serial Comm OK";  // Here we assume the amp is ON and communicating...


  if (OnMode == "null") {  // Here there's no response from the amp on the serial port...
    delay(1000);           //poll amp once per second to see if power is on.
    AmpConnect = "Serial Communication NOT updating. Amplifier Power ON?";
    OnMode = "null";
    OpMode = "On mode null";
    BandCode = "...................................";
    FaultCode = "null";
    PaTemp = "null";
    TempC = 0;
    TempF = 0;
    FanMin = "null";
    PaVolts = 0;
    PaAmps = 0;
    PoWatts = 0;
    PoSwr = 0;
    InputWatts = 0;
    SerialNumber = 0;
    RevNumber = 0;
    Tx = "";
    tft.println(AmpConnect);  // Display current AmpConnect mode on local display
  } else {
    tft.println(AmpConnect);  // Display current AmpConnect mode on local display
    // Query Firmware revision number
    if (RevNumber < 1) {
      Serial.print("^RVM;");
      recvWithEndMarker();
      if (inString.startsWith("^RVM")) {
        RevNumber = (inString.substring(4, 9).toFloat());
      }
    }

    // Query Firmware serial number
    if (SerialNumber < 1) {
      Serial.print("^SN;");
      recvWithEndMarker();
      if (inString.startsWith("^SN")) {
        SerialNumber = (inString.substring(3, 8).toFloat());
      }
    }

    // Query Opertional Mode of Amp
    Serial.print("^OS;");
    recvWithEndMarker();
    if (inString == "^OS0") {
      OpMode = "STBY";
    }
    if (inString == "^OS1") {
      OpMode = "OPER";
    }

    // Query operating band of Amp
    Serial.print("^BN;");
    recvWithEndMarker();
    BandCode = "...................................";
    if (inString == "^BN00") {
      BandCode = "   1.8 MHZ - 160 meters";
    }
    if (inString == "^BN01") {
      BandCode = "   3.5 MHZ -  80 meters  ";
    }
    if (inString == "^BN02") {
      BandCode = "   5.3 MHZ -  60 meters  ";
    }
    if (inString == "^BN03") {
      BandCode = "   7.0 MHZ -  40 meters  ";
    }
    if (inString == "^BN04") {
      BandCode = "  10.1 MHZ -  30 meters  ";
    }
    if (inString == "^BN05") {
      BandCode = "  14.0 MHZ -  20 meters  ";
    }
    if (inString == "^BN06") {
      BandCode = "  18.1 MHZ -  17 meters  ";
    }
    if (inString == "^BN07") {
      BandCode = "  21.0 MHZ -  15 meters  ";
    }
    if (inString == "^BN08") {
      BandCode = "  24.9 MHZ -  12 meters  ";
    }
    if (inString == "^BN09") {
      BandCode = "  28.0 MHZ -  10 meters  ";
    }
    if (inString == "^BN10") {
      BandCode = "  50.0 MHZ -  6 meters  ";
    }

    // Query Watts and SWR of PA
    Serial.print("^WS;");
    recvWithEndMarker();
    PoWatts = 0;
    PoSwr = 0;
    if (inString.startsWith("^WS")) {
      PoWatts = (inString.substring(3, 6).toFloat());
      PoWatts = PoWatts / 1;
      PoSwr = (inString.substring(7, 10).toFloat());
      PoSwr = PoSwr / 10;
    }
    if (PoWatts > PeakWatts) {
      PeakWatts = PoWatts;
    }
    if (PoSwr > PeakSwr) {
      PeakSwr = PoSwr;
    }

    // Query Temperature of Amp
    Serial.print("^TM;");
    recvWithEndMarker();
    PaTemp = "No Temp";

    if (inString.startsWith("^TM")) {
      PaTemp = inString.substring(3);
      TempC = (inString.substring(3).toInt());
      TempF = (TempC * 1.8) + 32;
    }

    if (TempF > PeakTemp) {
      PeakTemp = TempF;
    }
    if (TempC > PeakTempC) {
      PeakTempC = TempC;
    }


    // Query Minimum Fan Speed
    Serial.print("^FC;");
    recvWithEndMarker();
    FanMin = "Off";

    if (inString.startsWith("^FC")) {
      FanMin = inString.substring(3);
    }

    // Query Volts and Amps of PA
    Serial.print("^VI;");
    recvWithEndMarker();
    PaAmps = 0;

    if (inString.startsWith("^VI")) {
      PaVolts = (inString.substring(3, 6).toFloat());
      PaVolts = PaVolts / 10;

      PaAmps = (inString.substring(7, 10).toFloat());
      PaAmps = PaAmps / 10;

      InputWatts = PaVolts * PaAmps;
      if (InputWatts > PeakInputWatts) {
        PeakInputWatts = InputWatts;
      }
      if (((PaVolts < VMin) && (PaAmps > 1.0)) || (VMin == 0)) {
        VMin = PaVolts;
      }
      if (PaVolts > VMax) {
        VMax = PaVolts;
      }
    }

    // Query Fault of Amp

    if ((PaAmps < 0.25) & (OpMode == "OPER") & (PoWatts > 20)) {  // KPA500 40 watt MAXIMUM input in OPERATE mode. This trigger requires ~30 watts in to register
      KeyCheckCount = KeyCheckCount + 1;
    } else {
      KeyCheckCount = 0;
    }

    if (KeyCheckCount > 2) {  // Must repeat 3 times a row before fault is triggered. This count is cleared with the clear fault code button on the screen
      FaultCode = "CHECK KEYING CIRCUIT! POWER OUTPUT DETECTED WITHOUT AMP KEYED! SWITCHING TO STANDBY! ";
      Serial.print("^OS0;");  // Command to Standby
      recvWithEndMarker();
      Serial.print("^OS0;");  // Another command to Standby just to make sure
      recvWithEndMarker();
    }

    Serial.print("^FL;");
    recvWithEndMarker();

    if ((inString == "^FL00") & (FaultCode.indexOf("STANDBY") < 1)) {  // Fault code zero if there's no keying circuit message present.
      FaultCode = "00 = No Internal Faults";
    }

    if (inString == "^FL02") {
      FaultCode = "02 = Excessive power amplifier current";
    }
    if (inString == "^FL04") {
      FaultCode = "04 = Power amplifier temperature over limit";
    }
    if (inString == "^FL06") {
      FaultCode = "06 = Excessive driving power";
    }
    if (inString == "^FL08") {
      FaultCode = "08 = 60 volt supply over limit";
    }
    if (inString == "^FL09") {
      FaultCode = "09 = Excessive reflected power (high SWR)";
    }
    if (inString == "^FL11") {
      FaultCode = "Power amplifiers are dissipating excessive power";
    }
    if (inString == "^FL12") {
      FaultCode = "12 = Excessive power output";
    }
    if (inString == "^FL13") {
      FaultCode = "13 = 60 volt supply failure";
    }
    if (inString == "^FL14") {
      FaultCode = "14 = 270 volt supply failure";
    }
    if (inString == "^FL15") {
      FaultCode = "15 = Excessive overall amplifier gain";
    }
  }

  // Here we check the WiFi connection for activity..................

  WiFiClient client = HttpServer.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    // Serial.println("New Client.");  // print a message out in the serial port
    String currentLine = "";  // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        // Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Issue commands to KPA500 - ------------------------------------------------------------------------------
            if (header.indexOf("GET /Power/on") >= 0) {
              Serial.print("P");
              recvWithEndMarker();
              Serial.println("Power UP command Issued");
            }
            if (header.indexOf("GET /Power/off") >= 0) {
              Serial.print("^ON0;");
              recvWithEndMarker();
              // Serial.println("Power DOWN command Issued");
            }

            // Issue commands to KPA500 - Operate mode
            if (header.indexOf("GET /OpMode/Operate") >= 0) {
              Serial.print("^OS1;");
              recvWithEndMarker();
              // Serial.println("Operate command Issued");
            }

            // Issue commands to KPA500 - Standby Mode
            if (header.indexOf("GET /OpMode/Standby") >= 0) {
              Serial.print("^OS0;");
              recvWithEndMarker();
              // Serial.println("Standby command Issued");
            }

            // Issue commands to KPA500 - Fan Min Auto
            if (header.indexOf("GET /FanMin/AutoFanButton") >= 0) {
              AutoFanButton = true;
            }

            // Issue commands to KPA500 - Fan Min Auto
            if (header.indexOf("GET /FanMin/FanAutoOff") >= 0) {
              AutoFanButton = false;
            }

            // Issue commands to KPA500 - Fan Minimum to OFF
            if (((header.indexOf("GET /FanMin/FanMin00") >= 0) || (AutoFanButton && !AutoFanCommand)) && !FanMin.startsWith("00")) {
              Serial.print("^FC0;");
              recvWithEndMarker();
              // Serial.println("FanMin 00 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if ((header.indexOf("GET /FanMin/FanMin01") >= 0) || (AutoFanButton && AutoFanCommand && FanMin.startsWith("00"))) {  // sets minumum fan speed when in auto and temp above setpoint.
              Serial.print("^FC1;");
              recvWithEndMarker();
              // Serial.println("FanMin 01 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin02") >= 0) {
              Serial.print("^FC2;");
              recvWithEndMarker();
              // Serial.println("FanMin 02 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin03") >= 0) {
              Serial.print("^FC3;");
              recvWithEndMarker();
              // Serial.println("FanMin 03 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin04") >= 0) {
              Serial.print("^FC4;");
              recvWithEndMarker();
              // Serial.println("FanMin 04 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin05") >= 0) {
              Serial.print("^FC5;");
              recvWithEndMarker();
              // Serial.println("FanMin 05 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin06") >= 0) {
              Serial.print("^FC6;");
              recvWithEndMarker();
              // Serial.println("FanMin 06 Command Issued");
            }

            // Issue commands to KPA500 - Fault clear
            if (header.indexOf("GET /FaultCode/FaultClear") >= 0) {
              Serial.print("^FLC;");
              recvWithEndMarker();
              KeyCheckCount = 0;  // Clears auto standby reset fault
              FaultCode = "00 = Clearing Faults";
              // Serial.println("Fault CLEAR Command Issued");
            }


            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/1.8") >= 0) {
              Serial.print("^BN00;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/3.5") >= 0) {
              Serial.print("^BN01;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/5.0") >= 0) {
              Serial.print("^BN02;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/7") >= 0) {
              Serial.print("^BN03;");
              recvWithEndMarker();
            }
            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/10") >= 0) {
              Serial.print("^BN04;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/14") >= 0) {
              Serial.print("^BN05;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/18") >= 0) {
              Serial.print("^BN06;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/21") >= 0) {
              Serial.print("^BN07;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/24") >= 0) {
              Serial.print("^BN08;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/28") >= 0) {
              Serial.print("^BN09;");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Manual Band Select
            if (header.indexOf("GET /Band/50") >= 0) {
              Serial.print("^BN10;");
              recvWithEndMarker();
            }

            // Clear Peaks
            if (header.indexOf("GET /PeakClear/ClearPeak") >= 0) {
              // ---------------------------------- Peak Values are zeroed out.
              PeakWatts = 0;
              PeakSwr = 0;
              PeakInputWatts = 0;
              PeakTemp = 0;
              PeakTempC = 0;
              PeakPoLED = 0;
              PeakSwrLED = 0;
              VMax = 0;
              VMin = 0;
            }

            // -------------------------------------------------------------------------------------------------------------------  Display the HTML web page
            // -------------------------------------------------------------------------------------------------------------------  Display the HTML web page
            // -------------------------------------------------------------------------------------------------------------------  Display the HTML web page


            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\"  http-equiv=\"refresh\" content=\"2\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.print("<a href="
                         "> </a>");

            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; color:Ivory; background-color:#303030;}");
            client.println(".button { background-color:whiteSmoke; color: black; padding: 8px 20px; border-color:whiteSmoke; border-radius: 8px; margin: 6px 8px; width:125px;}");
            client.println(".button2 { background-color:whiteSmoke; font-size:14px; border-radius: 8px;margin:4px 2px; width:110px;}");
            client.println(".buttonBand { background-color:whiteSmoke;font-size:14px; border-radius: 8px;margin:4px 10px; width:55px;}");
            client.println(".buttonBandOn { background-color:orange;font-size:14px; border-radius: 8px;margin:4px 10px; width:55px;}");  // #FFAA00 band button light orange
            client.println(".buttonOn { background-color:skyblue;color:black; font-size:14px;border-radius:8px;margin:4px 2px;width:110px;}");
            client.println(".buttonOper { background-color:lime;color:black; padding: 8px 20px;border-color:lime; border-radius:8px;margin:6px 8px; width:125px;}");
            client.println(".buttonStby { background-color:yellow; color: black; padding: 8px 20px;border-color:yellow;border-radius: 8px; margin: 6px 8px; width:125px;}");
            // client.println(".buttonLED {backgroun-color:grey; color:black; padding:1px  1px; border-color:black:; border-radius: 1px; margin: 1px 1px; width:25px;}");
            client.println("</style></head>");

            // Web Page Heading
            client.println("<body><h1><span style=font-family:Times New Roman,serif>");
            client.println("WD5ACP ELECRAFT KPA500 <> WEBSERVER </span></h1>");

            client.print("<p><a href=\"/Power/on\"><button class=\"button\">POWER ON</button></a>");
            client.println("<a href=\"/Power/off\"><button class=\"button\">POWER OFF</button></a>");
            client.println("<br>");

            if (OpMode.startsWith("OPER")) {
              client.print("<a href=\"/OpMode/Operate\"><button class=\"buttonOper\">OPERATE</button></a>");
            } else {
              client.print("<a href=\"/OpMode/Operate\"><button class=\"button\">OPERATE</button></a>");
            }
            if (OpMode.startsWith("ST")) {
              client.println("<a href=\"/OpMode/Standby\"><button class=\"buttonStby\">STANDBY</button></a>");
            } else {
              client.println("<a href=\"/OpMode/Standby\"><button class=\"button\">STANDBY</button></a>");
            }
            client.print("</p>");

            if (PaAmps < 0.25) {  // Amp draws 0.5 amps when keyed even with no modulation.
              Tx = "&#8199";
            } else {
              Tx = "&#x2055";  //2732
            }
            if (AmpConnect.indexOf("NOT") > 0) {
              client.print("<p> <span style=color:black;background-color:grey;padding:8px;border-style:inset;font-size:28px; font-family:'Courier New', monospace; >");
            } else {
              client.print("<p> <span style=color:black;background-color:#FFAA00;padding:8px;border-style:inset;font-size:28px  font-family: 'Courier New',monospace;>");  //LCD orange was #FFAA00
            }
            client.println(Tx + BandCode + "</span> </p>");

            client.print("<p>");
            client.print("<span style= font-size:10px>");
            client.print("&#9484------------------------------------------------------------- BAND -------------------------------------------------------------&#9488");
            client.print("</span><br>");

            //client.print("<p>");
            if (BandCode.indexOf("160 meters") > 0) {
              client.print("<a href=\"/Band/1.8\"><button class=\"buttonBandOn\">1.8</button></a>");
            } else {
              client.print("<a href=\"/Band/1.8\"><button class=\"buttonBand\">1.8</button></a>");
            }

            if (BandCode.indexOf("80") > 0) {
              client.print("<a href=\"/Band/3.5\"><button class=\"buttonBandOn\">3.5</button></a>");
            } else {
              client.print("<a href=\"/Band/3.5\"><button class=\"buttonBand\">3.5</button></a>");
            }

            if (BandCode.indexOf("40") > 0) {
              client.print("<a href=\"/Band/7.0\"><button class=\"buttonBandOn\">7</button></a>");
            } else {
              client.print("<a href=\"/Band/7.0\"><button class=\"buttonBand\">7</button></a>");
            }

            if (BandCode.indexOf("20") > 0) {
              client.print("<a href=\"/Band/14.\"><button class=\"buttonBandOn\">14</button></a>");
            } else {
              client.print("<a href=\"/Band/14\"><button class=\"buttonBand\">14</button></a>");
            }

            if (BandCode.indexOf("15") > 0) {
              client.print("<a href=\"/Band/21\"><button class=\"buttonBandOn\">21</button></a>");
            } else {
              client.print("<a href=\"/Band/21\"><button class=\"buttonBand\">21</button></a>");
            }

            if (BandCode.indexOf("10 meter") > 0) {
              client.print("<a href=\"/Band/28\"><button class=\"buttonBandOn\">28</button></a>");
            } else {
              client.print("<a href=\"/Band/28\"><button class=\"buttonBand\">28</button></a>");
            }

            client.print("</p>");
            client.print("<p>");

            if (BandCode.indexOf("AUX") > 0) {
              client.print("<a href=\"/Band/AUX\"><button class=\"buttonBandOn\">AUX</button></a>");
            } else {
              client.print("<a href=\"/Band/AUX\"><button class=\"buttonBand\">AUX</button></a>");
            }

            if (BandCode.indexOf("-  60 meters") > 0) {
              client.print("<a href=\"/Band/5.0\"><button class=\"buttonBandOn\">5</button></a>");
            } else {
              client.print("<a href=\"/Band/5.0\"><button class=\"buttonBand\">5</button></a>");
            }

            if (BandCode.indexOf("30") > 0) {
              client.print("<a href=\"/Band/10\"><button class=\"buttonBandOn\">10</button></a>");
            } else {
              client.print("<a href=\"/Band/10\"><button class=\"buttonBand\">10</button></a>");
            }

            if (BandCode.indexOf("17") > 0) {
              client.print("<a href=\"/Band/18\"><button class=\"buttonBandOn\">18</button></a>");
            } else {
              client.print("<a href=\"/Band/18\"><button class=\"buttonBand\">18</button></a>");
            }

            if (BandCode.indexOf("12") > 0) {
              client.print("<a href=\"/Band/24\"><button class=\"buttonBandOn\">24</button></a>");
            } else {
              client.print("<a href=\"/Band/24\"><button class=\"buttonBand\">24</button></a>");
            }

            if (BandCode.indexOf("6 meters") > 0) {
              client.print("<a href=\"/Band/50\"><button class=\"buttonBandOn\">50</button></a>");
            } else {
              client.print("<a href=\"/Band/50\"><button class=\"buttonBand\">50</button></a>");
            }
            client.print("</p>");

            client.print("<p>");
            client.print("<span style=color:ivory;font-size:12px>");
            client.println("0------------------100------------------200------------------300------------------400------------------500-------------------600------------------700");
            client.print("<br></span>");
            if (PaAmps < 0.25) {  // Amp draws 0.5 amps when keyed even with no modulation.
              client.print("<span style=color:ivory;font-size:24px> ");
            } else {
              client.print("<span style=color:lime;font-size:24px> ");
            }
            if (PeakWatts > 525) {
              client.print("<span style=color:yellow;font-size:24px> ");
            }
            if (PeakWatts > 600) {
              client.print("<span style=color:OrangeRed;font-size:24px> ");
            }
            client.print("Watts: ");
            client.print(PoWatts);
            client.print(" W  </span>");


            int LitPoLED = (((PoWatts + 25.0) / 700.0) * 30.0) + 0.90;
            for (int PoLED = 1; PoLED < LitPoLED; PoLED++) {
              if (PoLED <= 25) {
                client.print("<span style=color:lime;font-size:24px;>");
              }
              if (PoLED > 25) {
                client.print("<span style=color:yellow;font-size:24px;>");
              }
              if (PoLED > 27) {
                client.print("<span style=color:red;font-size:24px;>");
              }
              client.print("&#9648");  //9603
            }

            PeakPoLED = (((PeakWatts + 25.0) / 700.0) * 30.0) + 0.9;

            for (int PoLED = LitPoLED + 1; PoLED < 30; PoLED++) {
              if (PoLED != PeakPoLED) {
                client.print("<span style=color:grey;font-size:24px;>");
              } else {
                client.print("<span style=color:white;font-size:24px;>");
              }
              client.print("&#9648");
            }

            client.print("<span style=color:ivory;font-size:24px> ");
            if (PeakWatts > 525) {
              client.print("<span style=color:yellow;font-size:24px> ");
            }
            if (PeakWatts > 600) {
              client.print("<span style=color:Red;font-size:24px> ");
            }
            client.print(" Peak: ");
            client.print(PeakWatts);
            client.print(" W ");
            client.print("</span><br>");

            client.print("<span style=color:ivory;font-size:12px>");
            client.print("&nbsp");
            client.print("0--------------1--------------2--------------3--------------4--------------5");
            client.print("<br></span>");

            if (PaAmps < 0.25) {  // Amp draws 0.5 amps when keyed even with no modulation.
              client.print("<span style=color:ivory;font-size:24px> ");
            } else {
              client.print("<span style=color:lime;font-size:24px> ");
            }
            if (PoSwr > 1.60) {
              client.print("<span style=color:yellow;font-size:24px> ");
            }
            if (PoSwr > 2.5) {
              client.print("<span style=color:Red;font-size:24px> ");
            }

            client.print("SWR: ");
            client.print(PoSwr);
            client.print("  ");


            int LitSwrLED = (((PoSwr) / 5.0) * 15.0) + 0.99;

            for (int SwrLED = 1; SwrLED < LitSwrLED; SwrLED++) {
              if (SwrLED <= 4) {
                client.print("<span style=color:lime;>");
              }
              if (SwrLED > 4) {
                client.print("<span style=color:yellow;>");
              }
              if (SwrLED > 7) {
                client.print("<span style=color:red;>");
              }
              client.print("&#9648");
            }

            PeakSwrLED = (((PeakSwr) / 5.0) * 15.0) + 0.99;

            for (int SwrLED = LitSwrLED + 1; SwrLED < 15; SwrLED++) {
              if (SwrLED != PeakSwrLED) {
                client.print("<span style=color:grey;>");
              } else {
                client.print("<span style=color:white;>");
              }
              client.print("&#9648");
            }

            client.print("<span style=color:ivory;font-size:24px> ");
            if (PeakSwr > 1.60) {
              client.print("<span style=color:yellow;font-size:24px> ");
            }
            if (PeakSwr > 2.5) {
              client.print("<span style=color:Red;font-size:24px> ");
            }
            client.print(" Peak: ");
            client.print(PeakSwr);
            client.print("  ");
            client.println("</span></p>");

            client.println("<p> <span style=font-size:24px>");
            if (TempF > 120) {
              client.print("<span style=color:skyblue; font-size:24px>");
            }
            if (TempF > 155) {
              client.print("<span style=color:orange; font-size:24px>");
            }
            // client.println(" Temp // PeakTemp: (" + PaTemp + " 'C // " + PeakTempC + " 'C) = " + TempF + " 'F // " + PeakTemp + " 'F </span> ");
            client.println("Fan Min Speed: " + FanMin + " Temp: (" + PaTemp + " 'C) " + TempF + " 'F  || Peak: (" + PeakTempC + " 'C) " + PeakTemp + " 'F </span> ");

            client.print("<br>");

            if (TempF > TempStartPt.toInt()) {  // ==============================================================    AutoFanCommand triggers min speed fan
              AutoFanCommand = true;
            }
            if (TempF < TempStopPt.toInt()) {
              AutoFanCommand = false;
            }

            client.print("<a href=\"/FanMin/FanMin00\"><button class=\"button2\">FAN MIN 00</button></a>");

            if (AutoFanButton) {
              client.print("<a href=\"/FanMin/FanAutoOff\"><button class=\"buttonOn\">" + TempStopPt + "F <> " + TempStartPt + "F</button></a>");
            }

            if (!AutoFanButton) {
              client.print("<a href=\"/FanMin/AutoFanButton\"><button class=\"button2\">" + TempStopPt + "F <> " + TempStartPt + "F</button></a>");
            }

            if (FanMin.startsWith("01")) {
              client.print("<a href=\"/FanMin/FanMin01\"><button class=\"buttonOn\">FAN MIN 01</button></a>");
            } else {
              client.print("<a href=\"/FanMin/FanMin01\"><button class=\"button2\">FAN MIN 01</button></a>");
            }
            if (FanMin.startsWith("02")) {
              client.print("<a href=\"/FanMin/FanMin02\"><button class=\"buttonOn\">FAN MIN 02</button></a>");
            } else {
              client.print("<a href=\"/FanMin/FanMin02\"><button class=\"button2\">FAN MIN 02</button></a>");
            }
            if (FanMin.startsWith("03")) {
              client.print("<a href=\"/FanMin/FanMin03\"><button class=\"buttonOn\">FAN MIN 03</button></a>");
            } else {
              client.print("<a href=\"/FanMin/FanMin03\"><button class=\"button2\">FAN MIN 03</button></a>");
            }
            if (FanMin.startsWith("04")) {
              client.print("<a href=\"/FanMin/FanMin04\"><button class=\"buttonOn\">FAN MIN 04</button></a>");
            } else {
              client.print("<a href=\"/FanMin/FanMin04\"><button class=\"button2\">FAN MIN 04</button></a>");
            }
            if (FanMin.startsWith("05")) {
              client.print("<a href=\"/FanMin/FanMin05\"><button class=\"buttonOn\">FAN MIN 05</button></a>");
            } else {
              client.print("<a href=\"/FanMin/FanMin05\"><button class=\"button2\">FAN MIN 05</button></a>");
            }
            if (FanMin.startsWith("06")) {
              client.print("<a href=\"/FanMin/FanMin06\"><button class=\"buttonOn\">FAN MIN 06</button></a>");
            } else {
              client.print("<a href=\"/FanMin/FanMin06\"><button class=\"button2\">FAN MIN 06</button></a>");
            }
            client.print("</p>");

            if (FaultCode.startsWith("00")) {
              client.print("<span style=font-size:24px> ");
            } else {
              client.print("<span style=color:white;background-color:Red;font-size:24px> ");
            }
            // client.print("FaultCode = "  + FaultCode + "</span>");
            client.print("FaultCode = " + FaultCode);
            client.print("<br>");
            client.print("<a href=\"/FaultCode/FaultClear\"><button class=\"button2\">CLEAR FAULT CODES</button></a>");
            client.print("<a href=\"/PeakClear/ClearPeak\"><button class=\"button2\">CLEAR PEAK HOLDS</button></a>");
            client.print("<a href=\"/\"><button class=\"button2\">CLEAR LAST COMMAND</button></a>");
            client.print("</p>");
            client.print("<p>");

            client.print("KPA500 Power Amp Section: Volts [Min|Max]: ");
            client.print("<span style=color:lime> ");
            if ((PaVolts < 60.5) || (PaVolts > 82.9)) {
              client.print("<span style=color:yellow> ");
            }
            if ((PaVolts < 58.5) || (PaVolts > 84.5)) {
              client.print("<span style=color:orangered> ");
            }
            client.print(PaVolts);
            client.print("V [");
            client.print(VMin);
            client.print("V | ");
            client.print(VMax);
            client.print("V ]  ");
            client.print("<span style=color:ivory>");
            client.print("    Amps: ");
            client.print("<span style=color:lime> ");
            client.print(PaAmps);
            client.print("A   ");

            client.print("</span>");
            client.print("<span style=color:ivory> ");

            client.print("<br>");

            client.print("Peak Output Efficiency (Valid only with steady-state output): ");
            client.print(PeakWatts);
            client.print("W  ( Output ) / ");
            client.print(PeakInputWatts);
            client.print("W  ( Input ) ");
            client.print(" = ");
            client.print((PeakWatts / PeakInputWatts) * 100.0);
            client.print(" %");
            client.print("</span>");
            client.print("</p>");

            client.print("<p>");

            client.print("<span style=font-size:18px>");
            if (AmpConnect.indexOf("NOT") > 0) {
              client.print("<span style=color:orangered>");
            } else {
              client.print("<span style=color:ivory>");
            }
            client.println("Status:  " + AmpConnect + "</span>");
            client.print(" || ");
            client.print(OnMode);
            client.print(" ||  KPA500 Firmware Version: ");
            client.print(RevNumber);
            client.print(" ||  KPA500 Serial Number: ");
            client.print(SerialNumber);

            client.print("<br>");

            client.print("WebServer Version " + CodeVersion + ".  Refresh: 2 secs. WiFi: ");
            client.print(WiFi.SSID());
            client.print(". Hostname:" + String(myHostName) + " ");
            client.print(" @ ");
            client.print(WiFi.localIP());
            long rssiLong = WiFi.RSSI();
            String WiFiSig = "excellent WiFi signal strength.";
            if (rssiLong < -60) {
              WiFiSig = "very good WiFi signal strength.";
            }
            if (rssiLong < -70) {
              WiFiSig = "good WiFi signal strength.";
            }
            if (rssiLong < -80) {
              WiFiSig = "low WiFi signal strength.";
            }
            if (rssiLong < -90) {
              WiFiSig = "very low WiFi signal strength.";
            }
            if (rssiLong < -99) {
              ;
              WiFiSig = "extremely low signal strength.";
            }
            client.print(" with ");
            client.print(rssiLong);
            client.print("dBm " + WiFiSig);

            client.print("</p>");
            client.print("</span> ");

            // The HTTP response ends with another blank line
            client.println();

            client.println("</body></html>");
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    // Serial.println("Client disconnected.");
    // Serial.println("");
  }
}
