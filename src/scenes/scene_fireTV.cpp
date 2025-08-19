#include <map>
#include "scenes/scene_fireTV.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
// devices
#include "devices/TV/device_samsungTV/device_samsungTV.h"
#include "devices/AVreceiver/device_yamahaAmp/device_yamahaAmp.h"
#include "applicationInternal/commandHandler.h"
// guis
#include "guis/gui_numpad.h"

uint16_t SCENE_FIRETV      ; //"Scene_firetv"
uint16_t SCENE_FIRETV_FORCE; //"Scene_firetv_force"

std::map<char, repeatModes> key_repeatModes_fireTV;
std::map<char, uint16_t> key_commands_short_fireTV;
std::map<char, uint16_t> key_commands_long_fireTV;

void scene_setKeys_fireTV() {
  key_repeatModes_fireTV = {
  
                                       {KEY_REWI,  SHORTorLONG      },    {KEY_PLAY,  SHORT            },    {KEY_FORW,  SHORTorLONG      },
    {KEY_CONF,  SHORT            },                                                                          {KEY_INFO,  SHORT            },
                                                         {KEY_UP,    SHORT            },
                      {KEY_LEFT,  SHORT            },    {KEY_OK,    SHORT            },    {KEY_RIGHT, SHORT           },
                                                         {KEY_DOWN,  SHORT            },
                                                                                                             {KEY_SRC,   SHORT            },
  
  
  
  };
  
  key_commands_short_fireTV = {
  
                                       {KEY_REWI,  KEYBOARD_REWIND  },    {KEY_PLAY, KEYBOARD_PLAYPAUSE},    {KEY_FORW,  KEYBOARD_FASTFORWARD},
    {KEY_CONF,  KEYBOARD_HOME    },                                                                          {KEY_INFO,  KEYBOARD_MENU    },
                                                         {KEY_UP,    KEYBOARD_UP      },
                      {KEY_LEFT,  KEYBOARD_LEFT   },     {KEY_OK,    KEYBOARD_SELECT  },    {KEY_RIGHT, KEYBOARD_RIGHT   },
                                                         {KEY_DOWN,  KEYBOARD_DOWN    },
                                                                                                             {KEY_SRC,   KEYBOARD_BACK    },
  
  
  
  };
  
  key_commands_long_fireTV = {
    {KEY_REWI,  KEYBOARD_REWIND_LONG},
    {KEY_FORW,  KEYBOARD_FASTFORWARD_LONG},
  };

}

void scene_start_sequence_fireTV(void) {
  executeCommand(SAMSUNG_POWER_ON);
  delay(500);
  executeCommand(YAMAHA_POWER_ON);
  delay(1500);
  executeCommand(YAMAHA_INPUT_DTV);
  delay(3000);
  executeCommand(SAMSUNG_INPUT_HDMI_2);
  delay(100);
  
  executeCommand(KEYBOARD_HOME);
  delay(500);
  executeCommand(KEYBOARD_HOME);

}

void scene_end_sequence_fireTV(void) {
  // you cannot power off FireTV, but at least you can stop the currently running app
  executeCommand(KEYBOARD_HOME);
  delay(500);
  executeCommand(KEYBOARD_HOME);

}

std::string scene_name_fireTV = "Fire TV";
t_gui_list scene_fireTV_gui_list = {tabName_numpad};

void register_scene_fireTV(void) {
  register_command(&SCENE_FIRETV,       makeCommandData(SCENE, {scene_name_fireTV}));
  register_command(&SCENE_FIRETV_FORCE, makeCommandData(SCENE, {scene_name_fireTV, "FORCE"}));

  register_scene(
    scene_name_fireTV,
    & scene_setKeys_fireTV,
    & scene_start_sequence_fireTV,
    & scene_end_sequence_fireTV,
    & key_repeatModes_fireTV,
    & key_commands_short_fireTV,
    & key_commands_long_fireTV,
    & scene_fireTV_gui_list,
    SCENE_FIRETV);
}
