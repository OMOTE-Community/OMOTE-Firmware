#include "applicationInternal/commandHandler.h"
#include "device_keyboard_mqtt.h"

#if (ENABLE_KEYBOARD_MQTT == 1)

uint16_t KEYBOARD_MQTT_UP                  ; //"Keyboard_mqtt_up";
uint16_t KEYBOARD_MQTT_DOWN                ; //"Keyboard_mqtt_down";
uint16_t KEYBOARD_MQTT_RIGHT               ; //"Keyboard_mqtt_right";
uint16_t KEYBOARD_MQTT_LEFT                ; //"Keyboard_mqtt_left";
uint16_t KEYBOARD_MQTT_SELECT              ; //"Keyboard_mqtt_select";
uint16_t KEYBOARD_MQTT_SENDSTRING          ; //"Keyboard_mqtt_sendstring";
uint16_t KEYBOARD_MQTT_BACK                ; //"Keyboard_mqtt_back";
uint16_t KEYBOARD_MQTT_HOME                ; //"Keyboard_mqtt_home";
uint16_t KEYBOARD_MQTT_MENU                ; //"Keyboard_mqtt_menu";
uint16_t KEYBOARD_MQTT_SCAN_PREVIOUS_TRACK ; //"Keyboard_mqtt_scan_previous_track";
uint16_t KEYBOARD_MQTT_REWIND_LONG         ; //"Keyboard_mqtt_rewind_long";
uint16_t KEYBOARD_MQTT_REWIND              ; //"Keyboard_mqtt_rewind";
uint16_t KEYBOARD_MQTT_PLAYPAUSE           ; //"Keyboard_mqtt_playpause";
uint16_t KEYBOARD_MQTT_FASTFORWARD         ; //"Keyboard_mqtt_fastforward";
uint16_t KEYBOARD_MQTT_FASTFORWARD_LONG    ; //"Keyboard_mqtt_fastforward_long";
uint16_t KEYBOARD_MQTT_SCAN_NEXT_TRACK     ; //"Keyboard_mqtt_scan_next_track";
uint16_t KEYBOARD_MQTT_MUTE                ; //"Keyboard_mqtt_mute";
uint16_t KEYBOARD_MQTT_VOLUME_INCREMENT    ; //"Keyboard_mqtt_volume_increment";
uint16_t KEYBOARD_MQTT_VOLUME_DECREMENT    ; //"Keyboard_mqtt_volume_decrement";

void register_device_keyboard_mqtt() {
  register_command(&KEYBOARD_MQTT_UP                   , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/UP",                  "PRESS"}));
  register_command(&KEYBOARD_MQTT_DOWN                 , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/DOWN",                "PRESS"}));
  register_command(&KEYBOARD_MQTT_RIGHT                , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/RIGHT",               "PRESS"}));
  register_command(&KEYBOARD_MQTT_LEFT                 , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/LEFT",                "PRESS"}));
  register_command(&KEYBOARD_MQTT_SELECT               , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/SELECT",              "PRESS"}));
  register_command(&KEYBOARD_MQTT_SENDSTRING           , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/SENDSTRING"                  })); // payload must be set when calling commandHandler
  register_command(&KEYBOARD_MQTT_BACK                 , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/BACK",                "PRESS"}));
  register_command(&KEYBOARD_MQTT_HOME                 , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/HOME",                "PRESS"}));
  register_command(&KEYBOARD_MQTT_MENU                 , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/MENU",                "PRESS"}));
  register_command(&KEYBOARD_MQTT_SCAN_PREVIOUS_TRACK  , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/SCAN_PREVIOUS_TRACK", "PRESS"}));
  register_command(&KEYBOARD_MQTT_REWIND_LONG          , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/REWIND_LONG",         "PRESS"}));
  register_command(&KEYBOARD_MQTT_REWIND               , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/REWIND",              "PRESS"}));
  register_command(&KEYBOARD_MQTT_PLAYPAUSE            , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/PLAYPAUSE",           "PRESS"}));
  register_command(&KEYBOARD_MQTT_FASTFORWARD          , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/FASTFORWARD",         "PRESS"}));
  register_command(&KEYBOARD_MQTT_FASTFORWARD_LONG     , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/FASTFORWARD_LONG",    "PRESS"}));
  register_command(&KEYBOARD_MQTT_SCAN_NEXT_TRACK      , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/SCAN_NEXT_TRACK",     "PRESS"}));
  register_command(&KEYBOARD_MQTT_MUTE                 , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/MUTE",                "PRESS"}));
  register_command(&KEYBOARD_MQTT_VOLUME_INCREMENT     , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/VOLUME_INCREMENT",    "PRESS"}));
  register_command(&KEYBOARD_MQTT_VOLUME_DECREMENT     , makeCommandData(MQTT, {"esp32_keyboard_firetv/cmnd/VOLUME_DECREMENT",    "PRESS"}));
}

#endif