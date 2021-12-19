#define DEBUG_PRINT false
#define USE_OLED true

#include <string.h>           // memcpy, memcmp

#include <Wire.h>             // I2C

#include <ArtnetnodeWifi.h>   // Receive ArtNet
#include "espDMX.h"           // Write DMX data via Serial / UART


#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>


// Wifi settings
const char* hotspot_password = "artnet8266";
const char* artnet_device_name = "ESP8266ArtNet"; // also HotSpot SSID Name

// Pin / Interrupt settings
const uint8_t interruptPin = 14; // D5 = (GPIO 14)
const uint8_t onChipLedPin = LED_BUILTIN; // Blue LED on ESP "Chip"

// OLED Display Setting
const uint8_t screen_width  = 128;
const uint8_t screen_heigth = 64;
const uint8_t ssd_address   = 0x3C;
const uint8_t oled_autooff_sec = 10; // MAX on time / auto-off after 26 seconds!


// Globals
WiFiUDP UdpSend;
String ssid = "default_ssid";
String password = "default_password";
ESP8266WebServer server(80);
ArtnetnodeWifi ArtnetNode;
bool setup_mode = false;
volatile bool INTERRUPT_ontouchbuttonpressed = false;
volatile bool INTERRUPT_ontimer = false;

#include "configsite_html.h"



#if (USE_OLED)
#include <Adafruit_GFX.h>     // General Graphics Library
#include <Adafruit_SSD1306.h> // 32x128 / 64x128 monochrome OLEDs

Adafruit_SSD1306 Display;

bool display_is_on = false;
uint32 oled_autooff_ticks = (oled_autooff_sec * 1000 * 1000) / 3.2; // ESP8266 has 80MHz clock, division by 256 = 312.5Khz (1 tick = 3.2us - 26.8435424 sec max), maximum ticks 8388607 
#endif



// - - - - - -  METHODS - - - - - -
#include "displayscreens.cpp"
#include "wifi_methods.cpp"

/* 
//TODO: Currently ESP crashes when this method is used. Maybe due to global / "external§ variables, maybe has to be called with "IRAM_ATTR"
static uint8_t prev_data[512] = {0};
static uint8_t framecount = 0;
void ISR_onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if(universe == 0){

    if(memcmp(&prev_data, &data, length) == -1 || framecount == 0) { // when current dataframe differs from privous
      dmxA.setChans(data, length, 1);    // send out DMX data
      memcpy(&prev_data, &data, length); // retain data of prevous frame
    }

    framecount++;
    if(framecount >= 30){
      framecount = 0;
    }
  }

}
*/

void ISR_onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universe == 0){
    dmxA.setChans(data, length, 1);
  }
}

IRAM_ATTR void ISR_onTouchButtonPressed()
{
  INTERRUPT_ontouchbuttonpressed = true;
}

IRAM_ATTR void ISR_onTimer()
{
 INTERRUPT_ontimer = true; 
}


// - - - - - -  SETUP - - - - - -
void setup()
{
  // set-up serial for initial info output, hopefully DMX gear will not be confused.
  #if (DEBUG_PRINT) 
  Serial.begin(250000);
  #endif

  // EXTERNAL INTERRUPT //
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_onTouchButtonPressed, RISING);

  // TIMER INTERRUPT //
  // USE Timer1, Timer0 already used by WiFi functionality
  timer1_isr_init();
  timer1_attachInterrupt(ISR_onTimer);
  timer1_write(oled_autooff_ticks); // maximum ticks for timer: 8388607


  delay(500);

  // Get WiFi SSID and Password from EEPROM //
  EEPROM.begin(512);
  delay(10);
  //TODO: clearing off/discaring blanks/whitespace at the end of "strings" (currently the whole buffer including unused bytes is loaded to string variable)
  ssid = "";
  for (int i = 0; i < 32; ++i){
    ssid += char(EEPROM.read(i));
  }
  password = "";
  for (int i = 32; i < 96; ++i){
    password += char(EEPROM.read(i));
  }


  // DISPLAY INITIALIZATION & STARTUP INFOSCRRENS //
  #if (USE_OLED)
  Display = Adafruit_SSD1306(screen_width, screen_heigth, &Wire);
  if(!Display.begin(SSD1306_SWITCHCAPVCC, ssd_address)){
    #if (DEBUG_PRINT) 
    Serial.println("Display Init failed");
    #endif
  }
  else{
    Display.setTextColor(SSD1306_WHITE);
    display_startup_infoscreen();
    delay(2000);
    display_initialization_infoscreen();
    delay(4000);
  }
  #endif

  // Handling if Touch Button is pressed during startup
  // ENTER SETUPMODE
  if(INTERRUPT_ontouchbuttonpressed){ 
    setupHotSpot();
    setup_mode = true;
    INTERRUPT_ontouchbuttonpressed = false;
  }

  // PROCEED WITH STARTUP INTO ARTNET MODE
  else{ 
    // WIFI CONNECTION //
    if(!connectWifi()){
      // When connecting to WiFi is not successful 
      setupHotSpot();
      setup_mode = true;
    }

    // ARTNET SETUP / INITIALIZATION //
    ArtnetNode.setName(artnet_device_name);
    ArtnetNode.setNumPorts(1);
    ArtnetNode.enableDMXOutput(0);
    ArtnetNode.begin();

    ArtnetNode.setArtDmxCallback(ISR_onDmxFrame); // this will be called for each packet received
    dmxA.begin(onChipLedPin); // Start dmxA, status LED on pin 12 with full intensity

    display_configuration_infoscreen(true);    
  }
}


// - - - - - -  LOOP - - - - - -
void loop()
{
  // DMX MODE
  if(!setup_mode){
    ArtnetNode.read(); // we call the read function inside the loop
  }

  // SETUP MODE
  else{
    server.handleClient();
  }


  // Touch Button Interrupt Handling
  if(INTERRUPT_ontouchbuttonpressed){

    // DMX MODE
    if(!setup_mode){
      #if (USE_OLED)
      // Toggle on / off display va touch button
      if(display_is_on){
        reset_oled();
      }
      else{
        Display.dim(true);
        display_configuration_infoscreen(true);
      }
      #endif
    }

    // SETUP MODE
    else{
      #if (USE_OLED)
      // Toggle on / off display va touch button
      if(display_is_on){
        reset_oled();
      }
      else{
        Display.dim(true);
        display_setup_infoscreen_start();
        display_setup_infoscreen_update(true);
      }
      #endif
    }

    INTERRUPT_ontouchbuttonpressed = false;
  }

  // Always Auto-Off OLED after specified time
  #if (USE_OLED)
  if(display_is_on && INTERRUPT_ontimer){
    reset_oled();
    INTERRUPT_ontimer = false;
  }
  #endif

}