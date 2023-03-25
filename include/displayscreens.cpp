void reset_oled(){
  Display.setTextSize(1); 
  Display.setCursor(0, 0);
  Display.dim(false);
  Display.clearDisplay();
  Display.display();
  display_is_on = false;
  
  timer1_disable();
}

void displayOnOLED(bool auto_off){
  Display.display();
  display_is_on = true;
  
  if(auto_off){    
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP); // ESP8266 has 80MHz clock, division by 256 312.5Khz (1 tick = 3.2us - 26843542.4 us max)
  }
}

void display_startup_infoscreen(bool auto_off=false){
  reset_oled();

  // Display Startup Message (Artnet Device ID Name) 
  Display.setCursor(0, 0);
  Display.setTextSize(2);
  Display.cp437(true);
  Display.println(String(artnet_device_name));
  if(enable_artnet_ethernet){
    Display.println("Con: WiFi");
  }
  else{
    Display.println("Con: Ethernet");
  }
  displayOnOLED(auto_off);
}

void display_wifi_initialization_infoscreen(bool auto_off=false){
  reset_oled();

  // Display current WiFi configuration
  Display.println("WiFi SSID & Password:");
  Display.println(String(ssid));
  Display.println(String(password));
  Display.println("");
  Display.println("Press TB for (WiFi) Setup via HotSpot.");
  displayOnOLED(auto_off);
}

void display_configuration_infoscreen(bool auto_off=false){
  reset_oled();

  // Display current Device configuration
  Display.println("ARTNET DEVICE ONLINE");
  Display.println("");
  Display.println(String(artnet_device_name));
  Display.println(WiFi.localIP());
  Display.println("");
  Display.println("Connected to:");
  Display.println(String(ssid));
  displayOnOLED(auto_off);
}

void display_setup_infoscreen_start(bool auto_off=false){
  reset_oled();

  Display.println("SETUP - HOTSPOT MODE");
  Display.println("");
  displayOnOLED(auto_off);
}

void display_setup_infoscreen_update(bool auto_off=false){
  // Display current WiFi configuration
  Display.println("WiFi HotSpot Enabled:");
  Display.print("SSID: ");
  Display.println(artnet_device_name);
  Display.print("PASS: ");
  Display.println(hotspot_password);
  Display.println(WiFi.softAPIP());
  displayOnOLED(auto_off);
}