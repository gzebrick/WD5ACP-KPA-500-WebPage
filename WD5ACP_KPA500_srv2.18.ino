String CodeVersion = "2.18";

// Web server interface for Elecraft KPA500 amateur radio amplifier. Communicates to "PC" serial port on amplifer
//Version 2.2 - Stable release
//Version 2.3 - Tweak colors
// Version 2.4 -
// Version 2.5 Added peak hold
// Version 2.6 - Built for ESP8266 WeMos D1R1 board. Use file preferences > Additional boards > http://arduino.esp8266.com/stable/package_esp8266com_index.json to add to IDE
// Version 2.7 - tweaked power on and amp mode colors
// Version 2.9 - trying to keep webpage client connected
// Version 2.10 Added input watts / peak input watts / Efficiency
// Version 2.11 Fixed display for Efficiency calc. Teaked font/colors on display. Added Firmware and serial number to display
// Version 2.12 Finally fixed HTML code to auto-update web page every 5 seconds.
// Version 2.13 Fixed bug in retaining Peak Input Watts. Added Peak Clear button
// Version 2.14  tweak to web page  layout. Tweaked delays in serial code (hope to reduce serial drop outs).
// Version 2.15 Fixed mhz to MHZ on display. Tweaked LCD color
// Version 2.16 Added Clear Last Command button. Added WiFi SSID and signal strength to web page
// Version 2.17 Changed timing on serial port - still sometimes loose connection - moved serial rec to right after tx. Commented out extra serial messages
// Version 2.18 Added color coding to temperature display to alert high temperatures

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h> // Allows a 2nd serial port on the UNO style Arduino - this port is for the KPA500 RS232
SoftwareSerial mySerial(13, 15); // RX, TX  - Requires a TTL<>232 converter. Do NOT connect TTL level to your amp directly.
#include <ArduinoOTA.h>  // Allows over-the-air OTA programming of Arduino via WiFi (once it is loaded with the correct files - see the internet for details

// Replace with your network credentials  - maybe one day I'll make this via the serial port.
const char* ssid     = "zhome";
const char* password = "";

// Set web server port number to 80
WiFiServer server(80);

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
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;
String inString = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
String BootMode = "null";
String AmpConnect = "null";
String OnMode = "null";
String OpMode = "null";
String BandCode = "null";
String FaultCode = "null";
String PaTemp = "null";
int TempC = 0;
int TempF = 0;
String FanMin = "null";
float PaVolts = 0;
float PaAmps = 0;
float PoWatts = 0;
float PoSwr = 0;
float PeakWatts = 0;
float PeakSwr = 0;
float InputWatts = 0;
float PeakInputWatts = 0;
float RevNumber = 0.0;
float SerialNumber = 0.0;

void setup() {
  delay(200);
  Serial.begin(115200);
  // set the data rate for the SoftwareSerial port
  delay(200);
  mySerial.begin(38400);
  delay(200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  delay (100);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

}  // End of setup----------------------------------------------------------------------------------------------------------------



void loop() { //--------------------------------------------- Main Loop -----------------------------------------------------------

  ArduinoOTA.handle();

  // ------ This is where we talk to the KPA500

  // See if the amp is talking via the serial port...

  // Query Mains on/ff Status of Amp
  mySerial.print("^ON;");
  recvWithEndMarker();
  if (inString == "^ON1") {
    OnMode = "KPA500 Power is ON";
  }
  else {
    OnMode = "null";
  }


  AmpConnect = "Serial Communication OK"; // Here we assume the amp is ON and communicating...
  if (OnMode == "null") { // Here there's no response from the amp on the serial port...
    AmpConnect = "Serial Communication NOT updating. Power amp ON?";
    OnMode = "null";
    OpMode = "null";
    BandCode = "null";
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
  }

  // Query Firmware revision number
  mySerial.print("^RVM;");
  recvWithEndMarker();
  if (inString.startsWith("^RVM")) {
    RevNumber = (inString.substring(4, 9).toFloat());
  }

  // Query Firmware revision number
  mySerial.print("^SN;");
  recvWithEndMarker();
  if (inString.startsWith("^SN")) {
    SerialNumber = (inString.substring(3, 8).toFloat());
  }

  // Query Mains on/ff Status of Amp
  mySerial.print("^ON;");
  recvWithEndMarker();
  if (inString == "^ON1") {
    OnMode = "Power is ON";
  }
  else {
    OnMode = "null";
  }

  // Query Opertional Mode of Amp
  mySerial.print("^OS;");
  recvWithEndMarker();
  if (inString == "^OS0") {
    OpMode = "STBY";
  }
  if (inString == "^OS1") {
    OpMode = "OPER";
  }


  // Query operating band of Amp
  mySerial.print("^BN;");
  recvWithEndMarker();
  BandCode = "null";
  if (inString == "^BN00") {
    BandCode = "1.8 MHZ * 160 meters";
  }
  if (inString == "^BN01") {
    BandCode = "  3.5 MHZ * 80 meters  ";
  }
  if (inString == "^BN02") {
    BandCode = "  5.3 MHZ * 60 meters  ";
  }
  if (inString == "^BN03") {
    BandCode = "  7.0 MHZ * 40 meters  ";
  }
  if (inString == "^BN04") {
    BandCode = "  10.1 MHZ * 30 meters  ";
  }
  if (inString == "^BN05") {
    BandCode = "  14.0 MHZ * 20 meters  ";
  }
  if (inString == "^BN06") {
    BandCode = "  18.1 MHZ * 17 meters  ";
  }
  if (inString == "^BN07") {
    BandCode = "  21.0 MHZ * 15 meters  ";
  }
  if (inString == "^BN08") {
    BandCode = "  24.9 MHZ * 12 meters  ";
  }
  if (inString == "^BN09") {
    BandCode = "  28.0 MHZ * 10 meters  ";
  }
  if (inString == "^BN10") {
    BandCode = "  50.0 MHZ * 6 meters  ";
  }

  // Query Watts and SWR of PA
  mySerial.print("^WS;");
  recvWithEndMarker();
  PoWatts = 0;
  PoSwr = 0;
  if (inString.startsWith("^WS")) {
    PoWatts = (inString.substring(3, 6).toFloat());
    PoWatts = PoWatts / 1;
    PoSwr = (inString.substring(7, 9).toFloat());
    PoSwr = PoSwr / 10;
  }
  if (PoWatts > PeakWatts) {
    PeakWatts = PoWatts;
  }
  if (PoSwr > PeakSwr) {
    PeakSwr = PoSwr;
  }

  // Query Temperature of Amp
  mySerial.print("^TM;");
  recvWithEndMarker();
  PaTemp = "No Temp";

  if (inString.startsWith("^TM")) {
    PaTemp = inString.substring(3);
    TempC = (inString.substring(3).toInt());
    TempF = (TempC * 1.8) + 32;
  }

  // Query Minimum Fan Speed
  mySerial.print("^FC;");
  recvWithEndMarker();
  FanMin = "Off";

  if (inString.startsWith("^FC")) {
    FanMin = inString.substring(3);
  }

  // Query Volts and Amps of PA
  mySerial.print("^VI;");
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
  }


  // Query Fault of Amp
  mySerial.print("^FL;");
  recvWithEndMarker();
  if (inString == "^FL00") {
    FaultCode = "00 = No Faults";
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

  // Here we check the WiFi connection for activity..................

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
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
              mySerial.print("P");
              recvWithEndMarker();
              // Serial.println("Power UP command Issued");
            }
            if (header.indexOf("GET /Power/off") >= 0) {
              mySerial.print("^ON0;");
              recvWithEndMarker();
              // Serial.println("Power DOWN command Issued");
            }

            // Issue commands to KPA500 - Operate mode
            if (header.indexOf("GET /OpMode/Operate") >= 0) {
              mySerial.print("^OS1;");
              recvWithEndMarker();
              // Serial.println("Operate command Issued");
            }

            // Issue commands to KPA500 - Standby Mode
            if (header.indexOf("GET /OpMode/Standby") >= 0) {
              mySerial.print("^OS0;");
              recvWithEndMarker();
              // Serial.println("Standby command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin00") >= 0) {
              mySerial.print("^FC0;");
              recvWithEndMarker();
              // Serial.println("FanMin 00 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin00") >= 0) {
              mySerial.print("^FC0;");
              recvWithEndMarker();
              // Serial.println("FanMin 00 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin01") >= 0) {
              mySerial.print("^FC1;");
              recvWithEndMarker();
              // Serial.println("FanMin 01 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin02") >= 0) {
              mySerial.print("^FC2;");
              recvWithEndMarker();
              // Serial.println("FanMin 02 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin03") >= 0) {
              mySerial.print("^FC3;");
              recvWithEndMarker();
              // Serial.println("FanMin 03 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin04") >= 0) {
              mySerial.print("^FC4;");
              recvWithEndMarker();
              // Serial.println("FanMin 04 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin05") >= 0) {
              mySerial.print("^FC5;");
              recvWithEndMarker();
              // Serial.println("FanMin 05 Command Issued");
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin06") >= 0) {
              mySerial.print("^FC6;");
              recvWithEndMarker();
              // Serial.println("FanMin 06 Command Issued");
            }

            // Issue commands to KPA500 - Fault clear
            if (header.indexOf("GET /FaultCode/FaultClear") >= 0) {
              mySerial.print("^FLC;");
              recvWithEndMarker();
              // Serial.println("Fault CLEAR Command Issued");
            }

            // Clear Peaks
            if (header.indexOf("GET /PeakClear/ClearPeak") >= 0) {
              // ---------------------------------- Peak Values are zeroed out.
              PeakWatts = 0;
              PeakSwr = 0;
              PeakInputWatts = 0;
            }

            // -------------------------------------------------------------------------------------------------------------------  Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\"  http-equiv=\"refresh\" content=\"5\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.print("<a href=""> </a>" );

            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            // client.println(".button { background-color: #7787A; border: none; color: black; padding: 8px 20px;");  // Was #195B6A
            client.println(".button { background-color: #7787A; color: black; padding: 8px 20px;");  // Was #195B6A
            // client.println("text-decoration: none; font-size:18px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #7787A; font-size:12px}</style></head>"); //#7787A

            // Web Page Heading
            client.println("<body><h1>WD5ACP Elecraft KPA500 <> WebServer Ver " + CodeVersion + "</h1>");  // Here's the version number ........................................

            if (AmpConnect.indexOf("NOT") > 0) {
              client.print("<span style=color:red>");
            }
            client.println("Connection Status:  " + AmpConnect + "</span> </p>");

            if (OnMode.indexOf("ON") > 0) {
              client.print("<span style=color:black;background-color:white> " );  //LCD orange was #FF9900 font-size:24px
            }
            else {
              client.print("<span style=color:black;background-color:lightgrey> " );
            }
            client.println(OnMode);
            client.print (" // Amplifier Firmware Version: ");
            client.print (RevNumber);
            client.print (" // Amplifier Serial Number: ");
            client.print (SerialNumber);
            client.print ( "</span> </p>");

            client.print("<p><a href=\"/Power/on\"><button class=\"button\">POWER  .ON.</button></a>");
            client.println("<a href=\"/Power/off\"><button class=\"button\">POWER OFF.</button></a></p>");

            if (OpMode.startsWith("OPER")) {
              client.print("<span style=background-color:lime;padding:2px;font-size:26px;border:solid>");
              client.print("OPERATE");
              client.print("</span>");
              client.print("<span style=background-color:white;color:white;padding:2px;font-size:26px>");
              client.print("-----");
              client.print("</span>");
              client.print("<span style=background-color:grey;padding:2px;font-size:26px;border:solid>");
              client.print("STANDBY");
              client.print("</span>");
            }
            else {
              client.print("<span style=background-color:grey;padding:2px;font-size:26px;border:solid>");
              client.print("OPERATE");
              client.print("</span>");
              client.print("<span style=background-color:white;color:white;padding:2px;font-size:26px>");
              client.print("-----");
              client.print("</span>");
              client.print("<span style=background-color:yellow;padding:2px;font-size:26px;border:solid>"); //
              client.print("STANDBY");
              client.print("</span>");
            }

            client.println("</p>");

            client.print("<p><a href=\"/OpMode/Operate\"><button class=\"button\">OPERATE</button></a>");
            client.println("<a href=\"/OpMode/Standby\"><button class=\"button\">STANDBY</button></a></p>");

            client.print("<p> <span style=color:black;background-color:#FF9900;padding:8px;border:double;font-size:24px>" );  //LCD orange was #FF9900
            client.println(BandCode + "</span> </p>");
            client.print("<p>");
            client.print("<span style=font-size:24px> " );
            if ((PeakWatts > 550) || (PeakSwr > 1.5)) {
              client.print("<span style=background-color:LightYellow;font-size:24px> " );
            }
            if ((PeakWatts > 599) || (PeakSwr > 1.99)) {
              client.print("<span style=color:white;background-color:Red;font-size:24px> " );
            }
            client.print(PoWatts);
            client.print("W // ");
            client.print(PeakWatts);
            client.print("W Peak Watts OUT at ");
            client.print(PoSwr);
            client.print(" SWR // ");
            client.print(PeakSwr);
            client.print(" Peak SWR");
            client.println("</span></p>");

            client.print("Minimum Fan Speed:  " + FanMin + "</p>");
            client.print("<a href=\"/FanMin/FanMin00\"><button class=\"button.button2\">FAN NORMAL</button></a>");
            client.print("<a href=\"/FanMin/FanMin01\"><button class=\"button.button2\">01</button></a>");
            client.print("<a href=\"/FanMin/FanMin02\"><button class=\"button.button2\">02</button></a>");
            client.print("<a href=\"/FanMin/FanMin03\"><button class=\"button.button2\">03</button></a>");
            client.print("<a href=\"/FanMin/FanMin04\"><button class=\"button.button2\">04</button></a>");
            client.print("<a href=\"/FanMin/FanMin05\"><button class=\"button.button2\">05</button></a>");
            client.print("<a href=\"/FanMin/FanMin06\"><button class=\"button.button2\">06</button></a> </p>" );

            client.print("<p>");
            if (TempF > 120) {
              client.print("<span style=color:brown>");
            }
            if (TempF > 150) {
              client.print("<span style=color:crimson>");
            }
            client.println("PA Temperature:  " + PaTemp + "'C / " + TempF + "'F </span> </p>");


            client.print("<p>PA Volts: " );
            client.print(PaVolts);
            client.print("V   *   PA Amps: ");
            client.print(PaAmps);
            client.print("A   ");
            client.print("  =  ");
            client.print(InputWatts);
            client.print("W Input Watts // ");
            client.print(PeakInputWatts);
            client.print("W Peak Input Watts</p>");
            client.println("<p> Efficiency (best measured with steady tune) =  ");
            client.print(PeakWatts);
            client.print("W Peak Watts OUT / ");
            client.print(PeakInputWatts);
            client.print("W Peak Input Watts ");
            client.print(" = ");
            client.print((PeakWatts / PeakInputWatts) * 100.0);
            client.println(" % </p>");

            if (FaultCode.startsWith("00")) {
              client.print("<span style=font-size:24px> " );
            }
            else {
              client.print("<span style=color:white;background-color:Red;font-size:24px> " );
            }
            client.println("FaultCode = "  + FaultCode + "</span> </p>");
            client.print("<a href=\"/FaultCode\FaultClear\"><button class=\"button.button2\">CLEAR FAULT CODES</button></a>" );
            client.print("<a href=\"/PeakClear/ClearPeak\"><button class=\"button.button2\">CLEAR PEAK HOLDS</button></a>" );
            client.print("<a href=\"/\"><button class=\"button.button2\">CLEAR LAST COMMAND</button></a> </p>" );




            client.println("<p>WebServer updates every 5 seconds. Connected to network:  ");
            client.println(ssid);
            long rssiLong = WiFi.RSSI();
            String WiFiSig = "signal strength";

            if (rssiLong < 0) {
              WiFiSig = "Excellent WiFi Signal Strength";
            }
            if (rssiLong < -60) {
              WiFiSig = "Very Good WiFi Signal Strength";
            }
            if (rssiLong < -70) {
              WiFiSig = "Good WiFi Signal Strength";
            }
            if (rssiLong < -80) {
              WiFiSig = "Low WiFi Signal Strength";
            }
            if (rssiLong < -90) {
              WiFiSig = "Very Low WiFi Signal Strength";
            }
            if (rssiLong < -99) {
              ;
              WiFiSig = "Extremely Low Signal Strength";
            }

            client.println(rssiLong);
            client.println("dBm = " + WiFiSig);
            client.println("</p>");

            // The HTTP response ends with another blank line
            client.println();

            client.println("</body></html>");
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
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
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}// ------------------------------------------------------------------------- End of main loop --------------------------------


void recvWithEndMarker() {  // ----------------------- receive data -----------------------------------
  static byte ndx = 0;
  char endMarker = ';';
  char rc;
  delay(15); //was 50

  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
    // delay(10); // was 10
    inString = String(receivedChars);
  }
  newData = false;

}
