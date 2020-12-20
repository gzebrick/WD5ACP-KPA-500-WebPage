// Web server interface for Elecraft KPA500 amateur radio amplifier. Communicates to "PC" serial port on amplifer
//Version 2.2 - Stable release
//Version 2.3 - Tweak colors
// Version 2.4 -
// Version 2.5 Added peak hold
// Version 2.6 - Built for ESP8266 WeMos D1R1 board. Use file preferences > Additional boards > http://arduino.esp8266.com/stable/package_esp8266com_index.json to add to IDE
// Version 2.7 - tweaked power on and amp mode colors
// Version 2.9 - trying to keep webpage client connected
// Version 2.10 Added input watts / peak input watts / Efficiency

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
    OnMode = "Power is ON";
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
    PeakInputWatts = 0;
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
    OpMode = "STANDBY";
  }
  if (inString == "^OS1") {
    OpMode = "OPERATE";
  }


  // Query operating band of Amp
  mySerial.print("^BN;");
  recvWithEndMarker();
  BandCode = "null";
  if (inString == "^BN00") {
    BandCode = "1.8 MHZ / 160 meters";
  }
  if (inString == "^BN01") {
    BandCode = "3.5 MHZ / 80 meters";
  }
  if (inString == "^BN02") {
    BandCode = "5.3 MHZ / 60 meters";
  }
  if (inString == "^BN03") {
    BandCode = "7.0 MHZ / 40 meters";
  }
  if (inString == "^BN04") {
    BandCode = "10.1 MHZ / 30 meters";
  }
  if (inString == "^BN05") {
    BandCode = "14.0 MHZ / 20 meters";
  }
  if (inString == "^BN06") {
    BandCode = "18.1 mhz / 17 meters";
  }
  if (inString == "^BN07") {
    BandCode = "21.0 mhz / 15 meters";
  }
  if (inString == "^BN08") {
    BandCode = "24.9 mhz / 12 meters";
  }
  if (inString == "^BN09") {
    BandCode = "28.0 mhz / 10 meters";
  }
  if (inString == "^BN10") {
    BandCode = "50.0 mhz / 6 meters";
  }

  // Query Watts and SWR of PA
  mySerial.print("^WS;");
  PoWatts = 0;
  PoSwr = 0;
  recvWithEndMarker();
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
  PaTemp = "No Temp";
  recvWithEndMarker();
  if (inString.startsWith("^TM")) {
    PaTemp = inString.substring(3);
    TempC = (inString.substring(3).toInt());
    TempF = (TempC * 1.8) + 32;
  }

  // Query Minimum Fan Speed
  mySerial.print("^FC;");
  FanMin = "Off";
  recvWithEndMarker();
  if (inString.startsWith("^FC")) {
    FanMin = inString.substring(3);
  }

  // Query Volts and Amps of PA
  mySerial.print("^VI;");
  PaVolts = 0;
  PaAmps = 0;
  recvWithEndMarker();
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
              Serial.println("Power UP command Issued");
              recvWithEndMarker();
            }
            if (header.indexOf("GET /Power/off") >= 0) {
              mySerial.print("^ON0;");
              Serial.println("Power DOWN command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Operate mode
            if (header.indexOf("GET /OpMode/Operate") >= 0) {
              mySerial.print("^OS1;");
              Serial.println("Operate command Issued");
              recvWithEndMarker();
              PeakWatts = 0;
              PeakSwr = 0;
              PeakInputWatts = 0;
            }

            // Issue commands to KPA500 - Standby Mode
            if (header.indexOf("GET /OpMode/Standby") >= 0) {
              mySerial.print("^OS0;");
              Serial.println("Standby command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin00") >= 0) {
              mySerial.print("^FC0;");
              Serial.println("FanMin 00 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin00") >= 0) {
              mySerial.print("^FC0;");
              Serial.println("FanMin 00 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin01") >= 0) {
              mySerial.print("^FC1;");
              Serial.println("FanMin 01 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin02") >= 0) {
              mySerial.print("^FC2;");
              Serial.println("FanMin 02 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin03") >= 0) {
              mySerial.print("^FC3;");
              Serial.println("FanMin 03 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin04") >= 0) {
              mySerial.print("^FC4;");
              Serial.println("FanMin 04 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin05") >= 0) {
              mySerial.print("^FC5;");
              Serial.println("FanMin 05 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fan Minimum
            if (header.indexOf("GET /FanMin/FanMin06") >= 0) {
              mySerial.print("^FC6;");
              Serial.println("FanMin 06 Command Issued");
              recvWithEndMarker();
            }

            // Issue commands to KPA500 - Fault clear
            if (header.indexOf("GET /FaultCode/FaultClear") >= 0) {
              mySerial.print("^FLC;");
              Serial.println("Fault CLEAR Command Issued");
              recvWithEndMarker();
            }


            // -------------------------------------------------------------------------------------------------------------------  Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<Refresh: 10\r\n>"); // refresh every 10 seconds
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            // client.println(".button { background-color: #7787A; border: none; color: black; padding: 8px 20px;");  // Was #195B6A
            client.println(".button { background-color: #7787A; color: black; padding: 8px 20px;");  // Was #195B6A
            // client.println("text-decoration: none; font-size:18px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #7787A; font-size:12px}</style></head>"); //#7787A

            // Web Page Heading
            client.println("<body><h1>WD5ACP Elecraft KPA500 <> WebServer Ver 2.9</h1>");
            client.println("<p> Select a command or use browser refresh to update data. Operate command clears peak hold </p>");

            if (AmpConnect.indexOf("NOT") > 0) {
              client.print("<span style=background-color:yellow>");
            }
            client.println("Connection Status:  " + AmpConnect + "</span> </p>");

            if (OnMode.indexOf("ON") > 0) {
              client.print("<span style=color:black;background-color:white;font-size:24px> " );  //LCD orange was #FF9900
            }
            else {
              client.print("<span style=color:black;background-color:lightgrey;font-size:24px> " );
            }
            client.println("Power Status:  " + OnMode + "</span> </p>");

            client.print("<p><a href=\"/Power/on\"><button class=\"button\">POWER  ON</button></a>");
            client.println("<a href=\"/Power/off\"><button class=\"button\">POWER OFF</button></a></p>");

            if (OpMode.startsWith("OPERATE")) {
              client.print("<span style=background-color:lightgreen;font-size:24px>");
            }
            else {
              client.print("<span style=background-color:gold;font-size:24px>"); // color was #FF9900
            }

            if (OnMode.indexOf("ON") < 1) {
              client.print("</span>");
              client.print("<span style=color:black;background-color:lightgrey;font-size:24px> " );  //LCD orange was #FF9900
            }

            client.println("Operation Mode: " + OpMode + "</span> </p>");
            client.print("<p><a href=\"/OpMode/Operate\"><button class=\"button\">OPERATE</button></a>");
            client.println("<a href=\"/OpMode/Standby\"><button class=\"button\">STANDBY</button></a></p>");

            client.println("<p>Frequency Band Selected:  " + BandCode + "</p>");
            client.print("<p>Output Power / Peak Power:  ");

            if (PeakWatts > 550) {
              client.print("<span style=background-color:gold;> " );
            }
            if (PeakWatts > 599) {
              client.print("<span style=background-color:salmon> " );
            }

            client.print(PoWatts);
            client.print(" / ");
            client.print(PeakWatts);
            client.print("</span>");

            client.print(" WATTs out at SWR / Peak SWR: ");

            if (PeakSwr > 1.9) {
              client.print("<span style=background-color:gold;> " );
            }
            if (PeakSwr > 2.9) {
              client.print("<span style=background-color:salmon> " );
            }
            client.print(PoSwr);
            client.print(" / ");
            client.print(PeakSwr);
            client.print("</span>");

            client.println("</p>");
            client.print("Minimum Fan Speed:  " + FanMin + "</p>");
            client.print("<a href=\"/FanMin/FanMin00\"><button class=\"button.button2\">FAN NORMAL</button></a>");
            client.print("<a href=\"/FanMin/FanMin01\"><button class=\"button.button2\">01</button></a>");
            client.print("<a href=\"/FanMin/FanMin02\"><button class=\"button.button2\">02</button></a>");
            client.print("<a href=\"/FanMin/FanMin03\"><button class=\"button.button2\">03</button></a>");
            client.print("<a href=\"/FanMin/FanMin04\"><button class=\"button.button2\">04</button></a>");
            client.print("<a href=\"/FanMin/FanMin05\"><button class=\"button.button2\">05</button></a>");
            client.print("<a href=\"/FanMin/FanMin06\"><button class=\"button.button2\">06</button></a> </p>" );

            client.println("<p>PA Temperature:  " + PaTemp + "C / " + TempF + "F </p>");
            client.print("<p>PA Volts: " );
            client.print(PaVolts);
            client.print(" V      PA Amps: ");
            client.print(PaAmps);
            client.print(" A   ");
            client.print("        PA Input Watts / Peak PA Input Watts / Efficiency : ");
            client.print(InputWatts);
            client.print(" W  / ");
            client.print(PeakInputWatts);
            client.print(" W / ");
            client.print((PoWatts / PeakInputWatts)*100.0);
            client.println(" % </p>");

            if (FaultCode.startsWith("00")) {
              client.print("<span font-size:24px> " );
            }
            else {
              client.print("<span style=color:white;background-color:red;font-size:24px> " );
            }
            client.println("FaultCode = "  + FaultCode + "</span> </p>");
            client.print("<a href=\"/FaultCode\FaultClear\"><button class=\"button.button2\">CLEAR FAULT</button></a> </p>" );
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
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
  delay(50);
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
    delay(10);
    inString = String(receivedChars);
  }
  newData = false;

}