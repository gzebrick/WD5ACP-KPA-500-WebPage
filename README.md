 LOOK FOR THE LATEST RELEASE ON THE RIGHT SIDE OF THE SCREEN UNDER 
Releases" >>>>>>>>>>>>>>>>>>>>> 
 
 New! read below for link to 3D case modified for WeMos + 232 board!

This project was started for use in ham shacks where the mains power to the amplifier (120v/240v) is switched OFF and ON. By design, when MAINs power is applied to the amp (by a separate remote command power switch - Alexa?") , Elecraft designed the amp so it still requires a separate power on command from the physical power button to operate. That's hard to do when you're hundreds of miles away. Alternatively the amp does accept a POWER ON command via it's serial port. The initial design goal for this project was  to make a low-cost web/serial interface to allow the power to be turned on remotely.

The webpage is accessed on your LAN by typing either the IP address direct into your browser, or alternatively, the host name followed by .local   
For example (your IP and host name may be different):

http://192.168.1.37
or 
http://kpa500.local 

This project is the Arduino code for a low cost all-in-one Arduino ESP8266 WiFi board. It provides a simple text-based web server interface for the amateur radio Elecraft KPA-500 amplifier. The kpa500  amplifier has an RS-232 DB-9 serial port and accepts simple ASCII strings for monitoring and command. You'll need to purchase a TTL-232 adapter, and the appropriate cables for yoru Arduino board to get a standard RS-232 port.

The web server interface displays amplifier status, band information, fault codes with description and other generic data. The web interface allows power on/off command, minimum fan speed command, and fault reset.

Some notes re: version 4.01...
This version uses the USB Serial port to prompt for new WiFi SSID, then password, then Host Name. Once set it is stored in EEPROM memory and is remembered.

Host name access only works sometimes for my home LAN. You may need to reserved the IP in your LAN DHCP so it doesn't change
and just use the IP address to access the web server.

There's a 12-second countdown once the serial port is plugged in and the board powers up, 
Followed by a  10-second window to start entering a new WiFi SSID. 
If there is a new entry, the data is stored in EEPROM and the board reboots.
If there's no entry within 10 seconds, the boot-up continues, loading the existing infor off the EEPROM.

I had luck using the Serial port built into the Arduino programming tool, but PUTTY won't seem to work right. Not sure why.
Follow the prompts. The serial USB port is 115,200 baud 8,N,1.

The serial poll/response rate is slow, so don't expect real-time data (I do capture max power out and max SWR).

The only technical issue I've run into is upon program reset, the Arduino code transmits garbage out of the serial port. If the amplifier has MAINs power ON, and the garbage string includes the "D" character, the amplifier gets stuck in a firmware Download mode. Electraft makes it WAY TOO EASY for this to happen, so as a standard operating procedure, I recommend you power up this Arduino FIRST, wait 5 seconds, before applying MAINs power to the amp. Reverse on power shutdown....BUT....
Starting with version Release 3.01 the code includes delayed 5v output on an I/O pin that may be used to power up serial port as an option instead of powering the serial 232 board off the standard 5v pin. IF you power up the RS-232 board off this pin, the delay is enough (I hope) to eliminate the issue of the serial startup garbage putting the amp into the Download mode. 

In the spirit of HAM radio- this code is free. 73s. Be nice.

Here's the WeMos Arduino with WiFi I used for this project:
https://smile.amazon.com/WOWOONE-Arduino-ESP8266-Development-Compatible/dp/B0899N647N/ref=sr_1_10?crid=ZNNA6WNVY0KI&dchild=1&keywords=wemos+d1&qid=1614532664&s=electronics&sprefix=WeMos%2Celectronics%2C212&sr=1-10

Here's a link to the TTl-232 board I purchased (A 5-pack) 
https://smile.amazon.com/gp/product/B07Z5Y1WKX/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1

Here's a link to a 3D printed case on Tinkercad. I modified a Arduino case to add room for the 232 board... https://www.tinkercad.com/things/lAU7GRIQzDG-wemos-d1-w-232-v4

Finally, my windows 10 laptop lost the driver for the USB port (driver not found). I had to plugged it in first and installed the driver from this website:
http://www.wch.cn/download/CH341SER_ZIP.html
Then the com port showed up.

