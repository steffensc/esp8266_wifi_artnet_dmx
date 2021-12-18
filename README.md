# esp8266_wifi_artnet_dmx
Use ESP8266 to receive ArtNet commands via WiFi and output them with TTL to RS485 converter.
<br>
I had to find out that the sources of (mtongnz/espDMX) won't compile out of box and have some outdated libraries. So I created my own project based on (rstephan/ArtnetnodeWifi).
<br>
<br>
I added a touch button and an OLED screen to my controller, but the programm should also work without touch button and screen.
When starting the ESP and a press of the touch button (rising signal is detected on external interrupt pin D5) it directly enters setup mode, where the ESP goes into soft AP mode (default IP 192.168.4.1) where WiFi SSID and password can be configured. The ESP also goes into setup mode automatically, when connecting to the current configured WiFi isn't succesful.
<br>
When the ESP is in setup or artnet-mode, the touch button is used to toggle on or off the OLED. There is also a timer which automatically toggles off the OLED after configurable time (in code, max 26 seconds). So if you forgot to toggle it off or if it triggers on in someway it won't get worn out so fast.
<br>
<br>
I changed the code for outputing the DMX data within the onDmxFrame callback method from the example (rstephan/ArtnetnodeWifi). It's now done via the espDMX Library (mtongnz/espDMX) which produces more "stable" output and works when youre using a DMX Software (currently using LightKey, which has 24 channels in free version) which continuously sends DMX Data at high frame rates. The code from the "ArtnetnodeWifi" example works when only a few frames are send per second. At higher framerates DMX lights start flickering and behaving weird ;).
\
Currently tested with two Par 56 RGB LED cans working stable for a runtime of 12 hours straight.
<br>
<br>
IMPORTANT NOTE:<br>
Watch out when powering the controller via its USB port with a cable connected to a computer. As soon as DMX data is send out on serial, it's also send over the USB connection and in some cases incoming serial data from the computer leads to crashes of the ESP. I recommend using USB power adapter or powerbank in this case ;).

## This project uses the following Libraries / other Projects:
https://github.com/rstephan/ArtnetnodeWifi \
https://github.com/mtongnz/espDMX

## Hardware

### TTP223 Capacitive Touch Sensor (3,3V)
- https://www.instructables.com/Tutorial-for-TTP223-Touch-Sensor-Module-Capacitive/

### 0,96 Zoll monochrome I2C 128x64 OLED (3,3V)
- https://cdn.shopify.com/s/files/1/1509/1638/files/0_96_Zoll_Display_Datenblatt_AZ-Delivery_Vertriebs_GmbH_241c4223-c03f-4530-a8c0-f9ef2575872f.pdf?v=1622442722

### ARCELI TTL to RS485 Reziproke Converter
- https://www.amazon.de/ARCELI-Adapter-Serielle-Schnittstelle-Converter/dp/B07DN115BZ

### Wiring
D1 - I2C SCL <br>
D2 - I2C SDA <br>
D5 - Interrupt I/O from Touchbutton <br>
TX - TXD of ARCELI TTL to RS485 <br>
VIN - +5V from USB <br>

### Hardware Setup
Note that the green wire connected to the RXD pin of the TTL to RS485 converter board is not conected to the ESP8266. It's plugged in to a free/empty rail of the breadboard. For only sending out DMX connecting only TX is sufficient (so the ESP may not get disturbed when randomly receiving something at RX).
Furthermore note that the TXD pin of this specific TTL to RS485 board has to be connected to TX pin of the ESP8266, RXD has to be connected to RX. When using other TTL to RS485 board the TX of the ESP may has to be connected to the RX pin of the TTL board and vice versa.

![Image of the used hardware](esp_ttl_rs485_xlr_hardwaresetup.jpg)

