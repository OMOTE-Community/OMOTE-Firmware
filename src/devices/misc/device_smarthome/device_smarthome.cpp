#include "applicationInternal/commandHandler.h"
#include "device_smarthome.h"

uint16_t SMARTHOME_MQTT_BULB1_SET            ; //"Smarthome_mqtt_bulb1_set";
uint16_t SMARTHOME_MQTT_BULB2_SET            ; //"Smarthome_mqtt_bulb2_set";
uint16_t SMARTHOME_MQTT_BULB1_BRIGHTNESS_SET ; //"Smarthome_mqtt_bulb1_brightness_set";
uint16_t SMARTHOME_MQTT_BULB2_BRIGHTNESS_SET ; //"Smarthome_mqtt_bulb2_brightness_set";

void register_device_smarthome() {
  #if (ENABLE_WIFI_AND_MQTT == 1)
  register_command(&SMARTHOME_MQTT_BULB1_SET            , makeCommandData(MQTT, {"bulb1_set"             })); // payload must be set when calling commandHandler
  register_command(&SMARTHOME_MQTT_BULB2_SET            , makeCommandData(MQTT, {"bulb2_set"             })); // payload must be set when calling commandHandler
  register_command(&SMARTHOME_MQTT_BULB1_BRIGHTNESS_SET , makeCommandData(MQTT, {"bulb1_setbrightness"   })); // payload must be set when calling commandHandler
  register_command(&SMARTHOME_MQTT_BULB2_BRIGHTNESS_SET , makeCommandData(MQTT, {"bulb2_setbrightness"   })); // payload must be set when calling commandHandler
  #endif
}
