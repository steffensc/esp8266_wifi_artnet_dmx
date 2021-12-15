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

// Interrupt settings
const int interruptPin = 14; // D5 = (GPIO 14)

// Globals
WiFiUDP UdpSend;
String ssid = "default_ssid";
String password = "default_password";
ESP8266WebServer server(80);
String avail_networks_html = "";
ArtnetnodeWifi ArtnetNode;
bool setup_mode = false;
volatile bool INTERRUPT_touchbuttonpressed = false;


#if (USE_OLED)
#include <Adafruit_GFX.h>     // General Graphics Library
#include <Adafruit_SSD1306.h> // 32x128 / 64x128 monochrome OLEDs

// OLED Display Setting
const uint8_t screen_width  = 128;
const uint8_t screen_heigth = 64;
const uint8_t ssd_address   = 0x3C;

Adafruit_SSD1306 Display;

bool display_is_on = false;
#endif



// - - - - - -  METHODS - - - - - -
#include "displayscreens.h"


void createWebServer()
{
  {   
    server.on("/", []() {
      IPAddress hotspot_ip = WiFi.softAPIP();
      String ipStr = String(hotspot_ip[0]) + '.' + String(hotspot_ip[1]) + '.' + String(hotspot_ip[2]) + '.' + String(hotspot_ip[3]);
      
      String html_code = "";
      html_code = "<!DOCTYPE HTML>\r\n<html><h1>";
      html_code += String(artnet_device_name) + " - " + ipStr + "</h1>";
      //html_code += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      html_code += "<p>";
      html_code += avail_networks_html;
      html_code += "</p><br><form method='get' action='setting'><label>SSID:</label><input name='ssid' length=32><br><label>PASS:</label><input name='pass' length=64><br><input type='submit'></form>";
      html_code += "</html>";
      server.send(200, "text/html", html_code);
    });
  
    server.on("/setting", []() {
      String qssid = server.arg("ssid");
      String qpass = server.arg("pass");

      String html_code = "";
      if (qssid.length() > 0 && qpass.length() > 0) {
        
        // Clear EEPROM
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        // Write to EEPROM
        for (unsigned int i = 0; i < qssid.length(); ++i){
          EEPROM.write(i, qssid[i]);
        }
        for (unsigned int i = 0; i < qpass.length(); ++i){
          EEPROM.write(32 + i, qpass[i]);
        }
        EEPROM.commit();
 
        html_code = "<!DOCTYPE HTML>\r\n<html>";
        html_code += "<h1>Success</h1><br>Data saved to EEPROM.<br><br>...resetting for boot into new WiFi.";
        html_code += "</html>";
        server.send(200, "text/html", html_code);
        ESP.reset();
      } 
      else {
        html_code = "<!DOCTYPE HTML>\r\n<html>";
        html_code += "<h1>FAILED</h1><br>Couldnt save EEPROM.";
        html_code += "</html>";
        server.send(200, "text/html", html_code);
      }
 
    });
    
  } 
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  int available_wifi_networks = WiFi.scanNetworks();
  
  avail_networks_html = "<ol>";
  for (int i = 0; i < available_wifi_networks && i < 5; ++i)
  {
    // Print SSID and RSSI for each network found
    avail_networks_html += "<li>";
    avail_networks_html += WiFi.SSID(i);
    avail_networks_html += " (";
    avail_networks_html += WiFi.RSSI(i);
    avail_networks_html += " db)";
    avail_networks_html += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    avail_networks_html += "</li>";
  }
  avail_networks_html += "</ol>";

  delay(100);

  WiFi.softAP(artnet_device_name, hotspot_password);
}


void setupHotSpot(){
  #if (DEBUG_PRINT) 
  Serial.println("HotSpot Mode");
  #endif

  #if (USE_OLED)
  display_setup_infoscreen_start();
  #endif
  
  //pinMode(LED_BUILTIN, OUTPUT);

  setupAP(); // Setup HotSpot

  #if (USE_OLED)
  display_setup_infoscreen_update();
  delay(2000);
  #endif

  Display.dim(true);
  createWebServer();
  server.begin();
  
  delay(8000);
  reset_oled();
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi(void)
{
  #if (DEBUG_PRINT) 
  Serial.println("");
  Serial.println("Connecting to WiFi");
  Serial.print("Connecting");
  #endif

  #if (USE_OLED)
  reset_oled();
  Display.println("Connecting to WiFi:");
  Display.println(String(ssid));
  Display.setCursor(0, 16); // x, y
  displayOnOLED();
  #endif


  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    #if (DEBUG_PRINT) 
    Serial.print(".");
    #endif

    #if (USE_OLED)
    Display.print(".");
    displayOnOLED();
    #endif

    if (i > 20){
      state = false;
      break;
    }
    i++;
  }

  #if (DEBUG_PRINT) 
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  #endif

  #if (USE_OLED)
  Display.setCursor(0, 30);
  if (state) {
    Display.println("CONNECTED!");
    Display.println(WiFi.localIP());
  } else {
     Display.println("Connection failed.");
  }
  displayOnOLED();
  delay(2500);
  reset_oled();
  #endif

  return state;
}

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


IRAM_ATTR void ISR_touchButtonPressed()
{
  INTERRUPT_touchbuttonpressed = true;
}

void setup()
{
  // set-up serial for initial info output, hopefully DMX gear will not be confused.
  #if (DEBUG_PRINT) 
  Serial.begin(250000);
  #endif

  // EXTERNAL INTERRUPT //
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_touchButtonPressed, RISING); 


  delay(500);

  // Get WiFi SSID and Password from EEPROM //
  EEPROM.begin(512);
  delay(10);
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
    delay(2500);
    display_initialization_infoscreen();
    delay(5000);
  }
  #endif

  // Interrupt Handling
  if(INTERRUPT_touchbuttonpressed){
    setupHotSpot();
    setup_mode = true;
    INTERRUPT_touchbuttonpressed = false;
  }

  // WIFI CONNECTION //
  if(!connectWifi() && setup_mode == false){ // When connecting to WiFi is not successful and device not already in SETUP MODE
    setupHotSpot();
    setup_mode = true;
  }

  // ARTNET SETUP / INITIALIZATION //
  if(!setup_mode){ // only procees, when not in setup mode
    ArtnetNode.setName(artnet_device_name);
    ArtnetNode.setNumPorts(1);
    ArtnetNode.enableDMXOutput(0);
    ArtnetNode.begin();

    ArtnetNode.setArtDmxCallback(ISR_onDmxFrame); // this will be called for each packet received
    dmxA.begin(12); // Start dmxA, status LED on pin 12 with full intensity

    display_configuration_infoscreen();
    delay(2500);
  }

  reset_oled();
}


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
  if(INTERRUPT_touchbuttonpressed){

    // DMX MODE
    if(!setup_mode){
      #if (USE_OLED)
      display_configuration_infoscreen();
      delay(2500);
      reset_oled();
      #endif
    }

    // SETIP MODE
    else{
       #if (USE_OLED)
      // Toggle on / off display va touch button
      if(display_is_on){
        reset_oled();
      }
      else{
        Display.dim(true);
        display_setup_infoscreen_start();
        display_setup_infoscreen_update();
      }
      #endif
    }

    INTERRUPT_touchbuttonpressed = false;
  }

}