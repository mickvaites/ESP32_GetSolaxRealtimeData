# ESP32_GetSolaxRealtimeData

I currently have a Solax Solar System, with a LiPo battery providing sufficient capacity to provide 1Kw of power for an hour when fully charged. So it seemed to make sense that should be able to monitor the sun, the battery and turn devices on to use the spare capacity. An additional project was completed which was to retrofit the SONOFF Basic B1 (ESP8266) Power Switches to incorporate a simple dual timer, and a JSON interface for feedback and control. This project controls a pair of them.

This project started out life as a very simple Arduino project using ESP8266, and as it required more grunt was first converted to Python running from a RaspberryPi and as realtime code in Python is quite unstable, it was converted to run from dual core ESP32.

The live implementation of the project makes use of the ESP32's onboard Wifi, to communicate with the Inverter, I'm using a HelTech ESP with onboard OLED, and I've attached five external LED's and a light sensor (details below). 

* LED(GPIO25) - Automatic Control Enabled
* LED(GPIO16) - Power Switch One Enabled
* LED(GPIO17) - Power Switch Two Enabled
* LED(GPIO02) - Power Switch One Powered On
* LED(GPIO04) - Power Switch Two Powered On
* CDR(GPIO32) - Light Sensor (the screen and LED's can be turned off if in a dark environment)

In addition to this, there is a DS1307 Realtime clock and an SD Card for data collection.

An appology must be made for my approach to the code - it evolved and I have built is using the Arduino IDE. This means that whilst Classes have been written for realtime stuff, there is a single module with every thing pulled from headers. The power switch project has been written properly, but I haven't got round to doing that for this. So the order and placement of the #include statements is important!

Also it is important to say that both CPU's are used; one for the loop function, which collects data and updates data structures, and the second with keeps the display updated, with things like a clock and a battery charging indicator.

There are loads of credits to sources of information to build this as follows:

* Integrating an SD without a dedicated module: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
* Starting my on being able to map the Solax API: https://community.home-assistant.io/t/solax-hybrid-inverter/22123

Have fun
