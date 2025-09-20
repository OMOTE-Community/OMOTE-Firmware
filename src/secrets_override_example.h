/*
  If you add additional overrides here, you have to
  1. first add #undef
  2. add new #define
*/
#undef WIFI_SSID
#undef WIFI_PASSWORD
#undef MQTT_SERVER
#undef ESPNOW_HUB_MAC

#define WIFI_SSID                  "YourWifiSSID"              // override here
#define WIFI_PASSWORD              "YourWifiPassword"          // override here
#define MQTT_SERVER                "IPAddressOfYourBroker"     // override here

// MAC address of the hub
// For unicast communication, use the hub's actual MAC address
#define ESPNOW_HUB_MAC {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
