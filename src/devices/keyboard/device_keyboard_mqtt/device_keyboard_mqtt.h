#pragma once

#include <stdint.h>

// The "MQTT keyboard" simply sends MQTT commands to a remote keyboard, which is connected via USB to a device
// https://github.com/KlausMu/esp32-mqtt-keyboard
// if you activate the MQTT keyboard, consider changing the mapping of the keyboard commands to the MQTT keyboard in file "commandHandler.h"

#if (ENABLE_KEYBOARD_MQTT == 1)

#if (ENABLE_KEYBOARD_MQTT == 1) && !(ENABLE_WIFI_AND_MQTT == 1)
static_assert(false, "You have to use \"-D ENABLE_WIFI_AND_MQTT=1\" in \"platformio.ini\" when having \"-D ENABLE_KEYBOARD_MQTT=1\"");
#endif

extern uint16_t KEYBOARD_MQTT_UP;
extern uint16_t KEYBOARD_MQTT_DOWN;
extern uint16_t KEYBOARD_MQTT_RIGHT;
extern uint16_t KEYBOARD_MQTT_LEFT;
extern uint16_t KEYBOARD_MQTT_SELECT;
extern uint16_t KEYBOARD_MQTT_SENDSTRING;
extern uint16_t KEYBOARD_MQTT_BACK;
extern uint16_t KEYBOARD_MQTT_HOME;
extern uint16_t KEYBOARD_MQTT_MENU;
extern uint16_t KEYBOARD_MQTT_SCAN_PREVIOUS_TRACK;
extern uint16_t KEYBOARD_MQTT_REWIND_LONG;
extern uint16_t KEYBOARD_MQTT_REWIND;
extern uint16_t KEYBOARD_MQTT_PLAYPAUSE;
extern uint16_t KEYBOARD_MQTT_FASTFORWARD;
extern uint16_t KEYBOARD_MQTT_FASTFORWARD_LONG;
extern uint16_t KEYBOARD_MQTT_SCAN_NEXT_TRACK;
extern uint16_t KEYBOARD_MQTT_MUTE;
extern uint16_t KEYBOARD_MQTT_VOLUME_INCREMENT;
extern uint16_t KEYBOARD_MQTT_VOLUME_DECREMENT;

void register_device_keyboard_mqtt();

#endif
