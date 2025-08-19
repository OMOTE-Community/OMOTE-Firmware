#pragma once

#include <lvgl.h>
#include <string>

const char * const tabName_irReceiver = "IR Receiver";
void register_gui_irReceiver(void);

// used by commandHandler to show IR messages
void showNewIRmessage(std::string);
void showMQTTmessage(std::string topic, std::string payload);

#if (ENABLE_HUB_COMMUNICATION == 1)
// used by commandHandler to show ESP-NOW messages
void showEspNowMessage(std::string payload);
#endif // ENABLE_HUB_COMMUNICATION

#if (ENABLE_WIFI_AND_MQTT == 1)
// used by commandHandler to show WiFi status
void showMQTTmessage(std::string topic, std::string payload);
#endif // ENABLE_WIFI_AND_MQTT
