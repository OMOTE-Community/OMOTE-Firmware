#include <map>
#include "scenes/scene_appleTV.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
// devices
#include "devices/TV/device_samsungTV/device_samsungTV.h"
#include "devices/AVreceiver/device_yamahaAmp/device_yamahaAmp.h"
#include "applicationInternal/commandHandler.h"
// guis
#include "devices/mediaPlayer/device_appleTV/gui_appleTV.h"

uint16_t SCENE_APPLETV      ; //"Scene_appleTV"
uint16_t SCENE_APPLETV_FORCE; //"Scene_appleTV_force"

std::map<char, repeatModes> key_repeatModes_appleTV;
std::map<char, uint16_t> key_commands_short_appleTV;
std::map<char, uint16_t> key_commands_long_appleTV;

void scene_setKeys_appleTV() {
  key_repeatModes_appleTV = {
  
  
  
  
  
  
  
  
  
  
  };
  
  key_commands_short_appleTV = {
  
  
  
  
  
  
  
  
  
  
  };
  
  key_commands_long_appleTV = {
  
  
  };

}

void scene_start_sequence_appleTV(void) {
  executeCommand(SAMSUNG_POWER_ON);
  delay(500);
  executeCommand(YAMAHA_POWER_ON);
  delay(1500);
  executeCommand(YAMAHA_INPUT_DVD);
  delay(3000);
  executeCommand(SAMSUNG_INPUT_HDMI_3);

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
