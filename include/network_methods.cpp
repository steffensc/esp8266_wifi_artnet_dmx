String avail_networks_html = "";

void scanWiFiNetworks()
{
  // Disconnect WiFi to scan for networks
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  int available_wifi_networks = WiFi.scanNetworks();

  avail_networks_html = "";
  for (int i = 0; i < available_wifi_networks && i < 5; ++i)
  {
    // Print SSID and RSSI for each network found
    avail_networks_html += "<li id='";
    avail_networks_html += WiFi.SSID(i);
    avail_networks_html += "'>";
    avail_networks_html += WiFi.SSID(i);
    avail_networks_html += " (";
    avail_networks_html += WiFi.RSSI(i);
    avail_networks_html += " db) ";
    avail_networks_html += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    avail_networks_html += "</li>";
  }

  delay(100);

  // Re-enable WiFi and soft-AP
  WiFi.softAP(artnet_device_name, hotspot_password);
}


void createWebServer()
{
  {   
    server.on("/", []() {
      IPAddress hotspot_ip = WiFi.softAPIP();
      String ipStr = String(hotspot_ip[0]) + '.' + String(hotspot_ip[1]) + '.' + String(hotspot_ip[2]) + '.' + String(hotspot_ip[3]);
      
      char html_buffer[1500];
      snprintf(html_buffer, 1500, configsite_config_html.c_str(), artnet_device_name, ipStr.c_str(), avail_networks_html.c_str());

      server.send(200, "text/html", html_buffer);
    });
  
    server.on("/setting", []() {
      String qssid = server.arg("ssid");
      String qpass = server.arg("pass");

      String html_code = "";
      if (qssid.length() > 0 && qpass.length() > 0) {
        
        eeprom_write_string(EEPROM_SSID_START_IDX, EEPROM_SSID_BYTES_LEN, qssid); 
        eeprom_write_string(EEPROM_WIFIPASS_START_IDX, EEPROM_WIFIPASS_BYTES_LEN, qpass);
        
        EEPROM.commit();

        server.send(200, "text/html", configsite_success_html);
        ESP.reset();
      } 
      else {
        server.send(200, "text/html", configsite_error_html);
      }
 
    });

    server.on("/rescan", []() {
      scanWiFiNetworks();

      //"Page refresh" by redirecting back to the main / basedir
      server.sendHeader("Location", String("/"), true);
      server.send(302, "text/plain", "");
    });
  } 
}

void setupHotSpot(){
  #if (DEBUG_PRINT) 
  Serial.println("HotSpot Mode");
  #endif

  #if (USE_OLED)
  display_setup_infoscreen_start();
  #endif
  
  //pinMode(LED_BUILTIN, OUTPUT);

  // Automaticallysets up HotSpot after scan
  scanWiFiNetworks();

  #if (USE_OLED)
  display_setup_infoscreen_update(true);
  #endif

  Display.dim(true);
  createWebServer();
  server.begin();
}


boolean connectEtheret(void){
  boolean state = true;

  #if (DEBUG_PRINT) 
  Serial.println("");
  Serial.println("Connecting to Ethernet");
  Serial.print("Connecting");
  #endif

  #if (USE_OLED)
  reset_oled();
  Display.println("Connecting to Ethernet:");
  displayOnOLED(false);
  #endif

  // Enter a MAC address for your controller below.
  // Newer Ethernet shields have a MAC address printed on a sticker on the shield
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

  #if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    Ethernet.init(33); // ESP32 with Adafruit FeatherWing Ethernet
  #elif defined(ARDUINO_ARCH_ESP8266)
    Ethernet.init(15); // ESP8266 with Adafruit FeatherWing Ethernet
  #endif

  // Connect to Ethernet
  if (Ethernet.begin(mac)) { // Dynamic IP setup
    #if (DEBUG_PRINT) 
    Serial.println("DHCP OK!");
    #endif

    #if (USE_OLED)
    Display.println("CONNECTED, DHCP OK!");
    Display.println(Ethernet.localIP());
    #endif
  }
  else{
      /* // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true) {
          delay(1); // do nothing, no point running without Ethernet hardware
        }
      }
      */
      if (Ethernet.linkStatus() == LinkOFF) {
        #if (DEBUG_PRINT) 
        Serial.println("Ethernet Cable error");
        #endif

        #if (USE_OLED)
        Display.println("Eth-Cable error!");
        #endif
        state = false;
      }

      #if (DEBUG_PRINT) 
      Serial.println("Using static configuration");
      #endif

      #if (USE_OLED)
      Display.println("Using static Ethernet configuration");
      #endif

      IPAddress ip(DEFAULT_ETH_IP);
      IPAddress dns(DEFAULT_ETH_DNS);
      IPAddress gw(DEFAULT_ETH_GW);
      IPAddress sn(DEFAULT_ETH_SUBMASK);
      Ethernet.begin(mac, ip, dns, gw, sn);
  }

  return state;
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
  displayOnOLED(false);
  #endif

  WiFi.begin(ssid, password);

  // Wait for connection
  boolean state = true;
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    if (i > WIFI_CONNECTION_TIME_SECS*2){
      state = false;
      break;
    }
    i++;

    #if (DEBUG_PRINT) 
    Serial.print(".");
    #endif

    #if (USE_OLED)
    draw_progressbar(30, int((screen_width/(WIFI_CONNECTION_TIME_SECS*2)*i)));
    displayOnOLED(false);
    #endif
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
  displayOnOLED(true);
  delay(1000);
  #endif

  return state;
}
