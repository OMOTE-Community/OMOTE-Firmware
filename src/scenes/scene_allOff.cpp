#include <map>
#include "scenes/scene_allOff.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
// devices
#include "devices/TV/device_samsungTV/device_samsungTV.h"
#include "devices/AVreceiver/device_yamahaAmp/device_yamahaAmp.h"
#include "applicationInternal/commandHandler.h"

uint16_t SCENE_ALLOFF      ; //"Scene_allOff"
uint16_t SCENE_ALLOFF_FORCE; //"Scene_allOff_force"

std::map<char, repeatModes> key_repeatModes_allOff;
std::map<char, uint16_t> key_commands_short_allOff;
std::map<char, uint16_t> key_commands_long_allOff;

void scene_setKeys_allOff() {
  key_repeatModes_allOff = {
  
  
  
  
  
  
  
  
  
  
  };
  
  key_commands_short_allOff = {
  
  
  
  
  
  
  
  
  
  
  };
  
  key_commands_long_allOff = {
  
  
  };

}

void scene_start_sequence_allOff(void) {
  executeCommand(SAMSUNG_POWER_OFF);
  delay(500);
  executeCommand(YAMAHA_POWER_OFF);
  delay(500);
  // repeat IR to be sure
  executeCommand(SAMSUNG_POWER_OFF);
  delay(500);
  executeCommand(YAMAHA_POWER_OFF);
  delay(500);
  // repeat IR to be sure
  executeCommand(SAMSUNG_POWER_OFF);
  delay(500);
  executeCommand(YAMAHA_POWER_OFF);
  delay(500);
  // you cannot power off FireTV, but at least you can stop the currently running app
  executeCommand(KEYBOARD_HOME);
  delay(500);
  executeCommand(KEYBOARD_HOME);

}

void scene_end_sequence_allOff(void) {

}

std::string scene_name_allOff = "Off";

void register_scene_allOff(void) {
  register_command(&SCENE_ALLOFF      , makeCommandData(SCENE, {scene_name_allOff}));
  register_command(&SCENE_ALLOFF_FORCE, makeCommandData(SCENE, {scene_name_allOff, "FORCE"}));

  register_scene(
    scene_name_allOff,
    & scene_setKeys_allOff,
    & scene_start_sequence_allOff,
    & scene_end_sequence_allOff,
    & key_repeatModes_allOff,
    & key_commands_short_allOff,
    & key_commands_long_allOff,
    NULL,
    SCENE_ALLOFF);
}
