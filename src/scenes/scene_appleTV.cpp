#include <map>
#include "scenes/scene_appleTV.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/commandHandler.h"
// devices
// #include "devices/TV/device_samsungTV/device_samsungTV.h"
// #include "devices/AVreceiver/device_yamahaAmp/device_yamahaAmp.h"
#include "devices/mediaPlayer/device_appleTV/device_appleTV.h"
#include "devices/TV/device_lgTV/device_lgTV.h"
#include "devices/AVreceiver/device_sonyAvr/device_sonyAvr.h"
#include "devices/misc/device_hub_helper.h"
// guis
#include "devices/mediaPlayer/device_appleTV/gui_appleTV.h"

uint16_t SCENE_APPLETV      ; //"Scene_appleTV"
uint16_t SCENE_APPLETV_FORCE; //"Scene_appleTV_force"

std::map<char, repeatModes> key_repeatModes_appleTV;
std::map<char, uint16_t> key_commands_short_appleTV;
std::map<char, uint16_t> key_commands_long_appleTV;

void scene_setKeys_appleTV() {
  key_repeatModes_appleTV = {
      {KEY_UP, SHORT_REPEATED},           {KEY_LEFT, SHORT_REPEATED},
      {KEY_OK, SHORTorLONG},              {KEY_RIGHT, SHORT_REPEATED},
      {KEY_DOWN, SHORT_REPEATED},
  };

  key_commands_short_appleTV = {
      {KEY_STOP, APPLETV_STOP},       {KEY_REWI, APPLETV_SKIP_BACKWARD},
      {KEY_PLAY, APPLETV_PLAY_PAUSE}, {KEY_FORW, APPLETV_SKIP_FORWARD},
      {KEY_CONF, APPLETV_HOME},       {KEY_INFO, COMMAND_UNKNOWN},
      {KEY_UP, APPLETV_UP},           {KEY_LEFT, APPLETV_LEFT},
      {KEY_OK, APPLETV_SELECT},       {KEY_RIGHT, APPLETV_RIGHT},
      {KEY_DOWN, APPLETV_DOWN},       {KEY_BACK, APPLETV_MENU},
      {KEY_CHUP, COMMAND_UNKNOWN},    {KEY_REC, COMMAND_UNKNOWN},
      {KEY_CHDOW, COMMAND_UNKNOWN},
  };

  key_commands_long_appleTV = {
      {KEY_OK, APPLETV_SELECT},
  };

}

void scene_start_sequence_appleTV(void) {
  #if (ENABLE_HUB_COMMUNICATION == 1)
  execute_hub_command(APPLETV_POWER_ON);
  execute_hub_command(SONYAVR_POWER_ON);
  execute_hub_command(LGTV_POWER_ON);
  #else
  // executeCommand(SAMSUNG_POWER_ON);
  // delay(500);
  // executeCommand(YAMAHA_POWER_ON);
  // delay(1500);
  // executeCommand(YAMAHA_INPUT_DVD);
  // delay(3000);
  // executeCommand(SAMSUNG_INPUT_HDMI_3);
  #endif
}

void scene_end_sequence_appleTV(void) {

}

std::string scene_name_appleTV = "Apple TV";
t_gui_list scene_appleTV_gui_list = {tabName_appleTV};

void register_scene_appleTV(void) {
  register_command(&SCENE_APPLETV,       makeCommandData(SCENE, {scene_name_appleTV}));
  register_command(&SCENE_APPLETV_FORCE, makeCommandData(SCENE, {scene_name_appleTV, "FORCE"}));

  register_scene(
    scene_name_appleTV,
    & scene_setKeys_appleTV,
    & scene_start_sequence_appleTV,
    & scene_end_sequence_appleTV,
    & key_repeatModes_appleTV,
    & key_commands_short_appleTV,
    & key_commands_long_appleTV,
    & scene_appleTV_gui_list,
    SCENE_APPLETV);
}
