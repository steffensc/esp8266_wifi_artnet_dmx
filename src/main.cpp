


#include <Arduino.h>

//#include <string.h>           // memcpy, memcmp

#include <Wire.h>             // I2C
#include <EEPROM.h>

#include <SPI.h>
#include <Ethernet.h>

#include <Artnetnode.h>   // Receive ArtNet
#include "espDMX.h"       // Write DMX data via Serial / UART


#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
  #include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

#include <WiFiUdp.h>



#include "configuration.h"


// Globals
String ssid     = DEFAUL_WIFI_SSID;
String password = DEFAUL_WIFI_PASS;

bool enable_artnet_ethernet = USE_ETHERNET;

ESP8266WebServer server(80);
Artnetnode artnet_node;

bool setup_mode = false;
volatile bool INTERRUPT_ontouchbuttonpressed = false;
volatile bool INTERRUPT_ontimer = false;



#if (USE_OLED)
  #include <Adafruit_GFX.h>     // General Graphics Library
  #include <Adafruit_SSD1306.h> // 32x128 / 64x128 monochrome OLEDs

  Adafruit_SSD1306 Display;

  bool display_is_on = false;
  uint32 oled_autooff_ticks = (oled_autooff_sec * 1000 * 1000) / 3.2; // ESP8266 has 80MHz clock, division by 256 = 312.5Khz (1 tick = 3.2us - 26.8435424 sec max), maximum ticks 8388607 
#endif



// - - - - - -  METHODS - - - - - -
#if (USE_OLED)
  #include "displayscreens.cpp"
#endif
#include "configsite_html.h"
#include "eeprom_methods.cpp"
#include "wifi_methods.cpp"



// - - - - - - INTERRUPTS - - - - - -
/* 
//TODO: Currently ESP crashes when this method is used. Maybe due to global / "external" variables, maybe has to be called with "IRAM_ATTR"
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



// - - - - - - SETUP - - - - - -
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


  // READ DATA FROM EEPROM //
  EEPROM.begin(EEPROM_SIZE);
  delay(10);
  ssid = "";
  eeprom_read_string(EEPROM_SSID_START_IDX, EEPROM_SSID_BYTES_LEN, ssid);
  password = "";
  eeprom_read_string(EEPROM_WIFIPASS_START_IDX, EEPROM_WIFIPASS_BYTES_LEN, password);
  enable_artnet_ethernet = bool(EEPROM.read(EEPROM_EN_ART_ETH_START_IDX));

  enable_artnet_ethernet = false; //DEBUG!!!!!!


  // CONFIGURE DISPLAY & PRINT STARTUP INFOSCRREN //
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
    if (enable_artnet_ethernet) {
      display_ethernet_initialization_infoscreen();
    }
    else{
      display_wifi_initialization_infoscreen();
    }
    delay(4000);
    
    display_enter_setup_infoscreen();
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

    // ARTNET VIA ETHERNET CONNECTION //
    if (enable_artnet_ethernet) {
      display_ethernet_initialization_infoscreen();
      auto Eth_Udp = std::make_shared<EthernetUDP>();
      artnet_node.setUDPConnection(Eth_Udp);
    }

    // ARTNET VIA WIFI CONNECTION //
    else {
      if(!connectWifi()){
        // When connecting to WiFi is not successful:
        setupHotSpot();
        setup_mode = true;
      }

      WiFiUDP WiFi_Udp;
      auto WiFi_Udp_shared = std::make_shared<WiFiUDP>(WiFi_Udp);
      artnet_node.setUDPConnection(WiFi_Udp_shared);
    }

    // ARTNET SETUP / INITIALIZATION //
    artnet_node.setName(artnet_device_name);
    artnet_node.setNumPorts(DMX_NUM_PORTS);
    artnet_node.enableDMXOutput(DMX_SERIAL_OUTPUT_PORT);
    artnet_node.begin();

    artnet_node.setArtDmxCallback(ISR_onDmxFrame); // this will be called for each packet received
    dmxA.begin(onChipLedPin); // Start dmxA, status LED on pin 12 with full intensity

    //display_wifi_online_infoscreen(true);    

  }
}



// - - - - - - MAIN LOOP - - - - - -
void loop()
{
  // DMX MODE
  if(!setup_mode){
    artnet_node.read(); // we call the read function inside the loop
  }

  // SETUP MODE
  else{
    server.handleClient();
  }

  #if (USE_OLED)
    // Touch Button Interrupt Handling
    if(INTERRUPT_ontouchbuttonpressed){

      // DMX MODE
      if(!setup_mode){
        // Toggle on / off display via touch button
        if(display_is_on){
          reset_oled();
        }
        else{
          Display.dim(true);
          if (enable_artnet_ethernet) {
            display_ethernet_online_infoscreen(true);
          }
          else{
            display_wifi_online_infoscreen(true);
          }
        }
      }

      // SETUP MODE
      else{
        // Toggle on / off display via touch button
        if(display_is_on){
          reset_oled();
        }
        else{
          Display.dim(true);
          display_setup_infoscreen_start();
          display_setup_infoscreen_update(true);
        }
      }

      INTERRUPT_ontouchbuttonpressed = false;
    }
      
    // Always Auto-Off OLED after specified time
    if(display_is_on && INTERRUPT_ontimer){
      reset_oled();
      INTERRUPT_ontimer = false;
    }
  #endif

}