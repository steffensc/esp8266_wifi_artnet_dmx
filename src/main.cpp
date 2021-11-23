#include <ArtnetnodeWifi.h>
#include "espDMX.h"

#include "config.h"

#define DEBUG_PRINT false

//Wifi settings
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WiFiUDP UdpSend;
ArtnetnodeWifi artnetnode;


// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);

  #if (DEBUG_PRINT) 
  Serial.println("");
  Serial.println("Connecting to WiFi");
  Serial.print("Connecting");
  #endif

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if (DEBUG_PRINT) 
    Serial.print(".");
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
  
  return state;
}

/*
static volatile uint8_t prev_data[512] = {0};
static volatile uint8_t framecount = 0;
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{

  if(prev_data != data || framecount == 0) {
    // Send "break" as a "slow" zero.
    
    Serial.begin(56700);
    Serial.write((uint8_t)0);
    delayMicroseconds(220);
    Serial.begin(250000, SERIAL_8N2);
    
    Serial.write((uint8_t)0); // Start-Byte

    // send out the buffer
    for (int i = 0; i < length; i++)
    {
      Serial.write(data[i]);
      prev_data[i] = data[i];
    }
    
  }

  framecount++;
  if(framecount >= 30){
    framecount = 0;
  }
}
*/

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universe == 0){
    dmxA.setChans(data, length, 1);
  }
}

void setup()
{
  // set-up serial for initial info output, hopefully DMX gear will not be confused.
  #if (DEBUG_PRINT) 
  Serial.begin(250000);
  #endif

  ConnectWifi();
  artnetnode.setName("ESP8266 - Art-Net");
  artnetnode.setNumPorts(1);
  artnetnode.enableDMXOutput(0);
  artnetnode.begin();

  // this will be called for each packet received
  artnetnode.setArtDmxCallback(onDmxFrame);

  // Start dmxA, status LED on pin 12 with full intensity
  dmxA.begin(12);
}


void loop()
{
  // we call the read function inside the loop
  artnetnode.read();
}