This project was started for use in ham shacks where the mains power to the amplifier (120v/240v) is switched OFF and ON. By design, when MAINs power is applied to the amp (by a separate remote command power switch - Alexa?") , Elecraft designed the amp so it still requires a separate power on command from the physical power button to operate. That's hard to do when you're hundreds of miles away. Alternatively the amp does accept a POWER ON command via it's serial port. The design goal for this project is to make a low-cost web/serial interface to allow the power to be turned on remotely.

This project is the Arduino code for a low cost all-in-one Arduino ESP8266 WiFi board. It provides a simple text-based web server interface for the amateur radio Elecraft KPA-500 amplifier. The amplifier has an RS-232 DB-9 serial port and accepts simple ASCII strings for monitoring and command. You'll need to purchase a TTL-232 adapter, and the appropriate cables.

The web interface displays amplifier status, band information, fault codes with description and other generic data. The web interface allows power on/off command, minimum fan speed command, and fault reset.

The initial release of this program has the WiFi SSID and password hard-coded in it. There's also NO USER LOGIN or security. I believe that a best practice is to remotely connect via a VPN which has its own security features to this simple program.

The serial poll/response rate is slow, so don't expect real-time data (I do capture max power out and max SWR).

The only technical issue I've run into is upon program reset, the Arduino code transmits garbage out of the serial port. If the amplifier has MAINs power ON, and the garbage string includes the "D" character, the amplifier gets stuck in a firmware Download mode. Electraft makes it WAY TOO EASY for this to happen, so as a standard operating procedure, I recommend you power up this Arduino FIRST, wait 5 seconds, before applying MAINs power to the amp. Reverse on power shutdown.

In the spirit of HAM radio- this code is free. 73s. Be nice.
