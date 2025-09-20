#pragma once

#include <lvgl.h>
#include <string>

const char * const tabName_irReceiver = "IR Receiver";
void register_gui_irReceiver(void);

// used by commandHandler to show IR messages
void showNewIRmessage(std::string);

#if (ENABLE_WIFI_AND_MQTT == 1)
// used by commandHandler to show WiFi status
void showMQTTmessage(std::string topic, std::string payload);
#endif // ENABLE_WIFI_AND_MQTT