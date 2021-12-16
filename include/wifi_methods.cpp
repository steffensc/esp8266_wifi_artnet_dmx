
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
  displayOnOLED(false);
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
    displayOnOLED(false);
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
  displayOnOLED(false);
  delay(2500);
  reset_oled();
  #endif

  return state;
}
