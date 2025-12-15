/*
  Before changing anything in this file, consider to copy file "secrets_override_example.h" to file "secrets_override.h" and to do your changes there.
  Doing so, you will
  - keep your credentials secret
  - most likely never have conflicts with new versions of this file
*/
#define WIFI_SSID                  "YourWifiSSID"              // override it in file "secrets_override.h"
#define WIFI_PASSWORD              "YourWifiPassword"          // override it in file "secrets_override.h"

#define MQTT_SERVER                "IPAddressOfYourBroker"     // override it in file "secrets_override.h"
#define MQTT_SERVER_PORT           1883                        // override it in file "secrets_override.h"
#define MQTT_USER                  ""                          // override it in file "secrets_override.h"
#define MQTT_PASS                  ""                          // override it in file "secrets_override.h"
#define MQTT_CLIENTNAME            "OMOTE"                     // override it in file "secrets_override.h"

// ESP-NOW Configuration
#define ESPNOW_HUB_MAC             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}  // override it in file "secrets_override.h"

// --- include override settings from seperate file ---------------------------------------------------------------------------------------------------------------
#if __has_include("secrets_override.h")
  #include "secrets_override.h"
#endif
