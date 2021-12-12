void reset_oled(){
  Display.setTextSize(1); 
  Display.setCursor(0, 0);
  Display.dim(false);
  Display.clearDisplay();
  Display.display();
  display_is_on = false;
}

void displayOnOLED(){
  Display.display();
  display_is_on = true;
}

void display_startup_infoscreen(){
  reset_oled();

  // Display Startup Message (Artnet Device ID Name) 
  Display.setCursor(0, 0);
  Display.setTextSize(2);
  Display.cp437(true);
  Display.println(String(artnet_device_name));
  displayOnOLED();
}

void display_initialization_infoscreen(){
  reset_oled();

  // Display current WiFi configuration
  Display.println("Current WiFi SSID:");
  Display.println(String(ssid));
  Display.println("Password:");
  Display.println(String(password));
  Display.println("");
  Display.println("Press TB for (WiFi) Setup via HotSpot.");
  displayOnOLED();
}

void display_configuration_infoscreen(){
  reset_oled();

  // Display current Device configuration
  Display.println("ARTNET DEVICE ONLINE");
  Display.println("");
  Display.println(String(artnet_device_name));
  Display.println(WiFi.localIP());
  Display.println("");
  Display.println("Connected to:");
  Display.println(String(ssid));
  displayOnOLED();
}

void display_setup_infoscreen_start(){
  reset_oled();

  Display.println("SETUP - HOTSPOT MODE");
  Display.println("");
  displayOnOLED();
}

void display_setup_infoscreen_update(){
  // Display current WiFi configuration
  Display.println("WiFi HotSpot Enabled:");
  Display.print("SSID: ");
  Display.println(artnet_device_name);
  Display.print("PASS: ");
  Display.println(hotspot_password);
  Display.println(WiFi.softAPIP());
  displayOnOLED();
}