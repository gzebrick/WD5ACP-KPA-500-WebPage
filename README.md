This project was started for use in ham shacks where the mains power to the amplifier (120v/240v) is switched OFF and ON. By design, when MAINs power is applied to the amp (by a separate remote command power switch - Alexa?") , Elecraft designed the amp so it still requires a separate power on command from the physical power button to operate. That's hard to do when you're hundreds of miles away. Alternatively the amp does accept a POWER ON command via it's serial port. The design goal for this project is to make a low-cost web/serial interface to allow the power to be turned on remotely.

This project is the Arduino code for a low cost all-in-one Arduino ESP8266 WiFi board. It provides a simple text-based web server interface for the amateur radio Elecraft KPA-500 amplifier. The amplifier has an RS-232 DB-9 serial port and accepts simple ASCII strings for monitoring and command. You'll need to purchase a TTL-232 adapter, and the appropriate cables.

The web interface displays amplifier status, band information, fault codes with description and other generic data. The web interface allows power on/off command, minimum fan speed command, and fault reset.

Some notes re: version 4.01...
This version uses the USB Serial port to prompt for new WiFi SSID, then password, then Host Name. 

Host name access only works sometimes for my home LAN. You may need to reserved the IP in your LAN DHCP so it doesn't change
and just use the IP address to access the web server.

There's a 12-second countdown once the serial port is plugged in and the board powers up, 
Followed by a  10-second window to start entering a new WiFi SSID. 
If there is a new entry, the data is stored in EEPROM and the board reboots.
If there's no entry within 10 seconds, the boot-up continues, loading the existing infor off the EEPROM.

I had luck using the Serial port built into the Arduino programming tool, but PUTTY won't seem to work right. Not sure why.
Follow the prompts. The serial USB port is 115,200 baud 8,N,1.

The serial poll/response rate is slow, so don't expect real-time data (I do capture max power out and max SWR).

The only technical issue I've run into is upon program reset, the Arduino code transmits garbage out of the serial port. If the amplifier has MAINs power ON, and the garbage string includes the "D" character, the amplifier gets stuck in a firmware Download mode. Electraft makes it WAY TOO EASY for this to happen, so as a standard operating procedure, I recommend you power up this Arduino FIRST, wait 5 seconds, before applying MAINs power to the amp. Reverse on power shutdown.

Note - version Release 3.01 includes delayed output on an I/O pin that may be used to power up serial port as an option instead of powering the serial 232 board off the standard 5v pin.

In the spirit of HAM radio- this code is free. 73s. Be nice.

Here's the WeMos Arduino with WiFi I used for this project:
https://smile.amazon.com/WOWOONE-Arduino-ESP8266-Development-Compatible/dp/B0899N647N/ref=sr_1_10?crid=ZNNA6WNVY0KI&dchild=1&keywords=wemos+d1&qid=1614532664&s=electronics&sprefix=WeMos%2Celectronics%2C212&sr=1-10
Here's a link to the TTl-232 board I purchased (A 5-pack) 
https://smile.amazon.com/gp/product/B07Z5Y1WKX/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1


Finally, my windows 10 laptop lost the driver for the USB port (driver not found). I had to plugged it in first and installed the driver from this website:
http://www.wch.cn/download/CH341SER_ZIP.html
Then the com port showed up.



