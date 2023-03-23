// EEPROM Memory indexes / adresses
#define EEPROM_SIZE 256 // max 512 Bytes!
#define EEPROM_SSID_START_IDX 0
#define EEPROM_SSID_BYTES_LEN 32
#define EEPROM_WIFIPASS_START_IDX 32
#define EEPROM_WIFIPASS_BYTES_LEN 64
#define EEPROM_EN_ART_WIFI_START_IDX 96
#define EEPROM_EN_ART_ETH_START_IDX 97

// ArtNet Network connection default configuration
#define USE_WIFI true
#define USE_ETHERNET true

// Wifi settings
#define DEFAUL_WIFI_SSID "default_ssid"
#define DEFAUL_WIFI_PASS "default_password"
const char* hotspot_password = "artnet8266";
const char* artnet_device_name = "ESP8266ArtNet"; // also HotSpot SSID Name

// Pin / Interrupt settings
const uint8_t interruptPin = 14; // D5 = (GPIO 14)
const uint8_t onChipLedPin = LED_BUILTIN; // Blue LED on ESP "Chip"

// DMX Settings
#define DMX_NUM_PORTS 1
#define DMX_SERIAL_OUTPUT_PORT 0

// OLED Display Setting
const uint8_t screen_width  = 128;
const uint8_t screen_heigth = 64;
const uint8_t ssd_address   = 0x3C;
const uint8_t oled_autooff_sec = 10; // MAX on time / auto-off after 26 seconds!