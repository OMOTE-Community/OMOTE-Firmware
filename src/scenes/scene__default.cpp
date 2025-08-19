#include <map>
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/commandHandler.h"
// devices
#include "devices/AVreceiver/device_yamahaAmp/device_yamahaAmp.h"
#include "devices/misc/device_smarthome/gui_smarthome.h"
// scenes
#include "scene__default.h"
#include "scenes/scene_allOff.h"
#include "scenes/scene_TV.h"
#include "scenes/scene_fireTV.h"
#include "scenes/scene_chromecast.h"
#include "scenes/scene_appleTV.h"

uint16_t SCENE_SELECTION;
std::string scene_name_selection = "sceneSelection";
uint16_t SCENE_BACK_TO_PREVIOUS_GUI_LIST;
std::string scene_back_to_previous_gui_list = "backToPreviousList";
uint16_t GUI_PREV;
std::string scene_gui_prev = "GUI_prev";
uint16_t GUI_NEXT;
std::string scene_gui_next = "GUI_next";

std::map<char, repeatModes> key_repeatModes_default;
std::map<char, uint16_t> key_commands_short_default;
std::map<char, uint16_t> key_commands_long_default;

// This is the main list of guis we want to be shown when swiping. Need not to be all the guis that have been registered, can be only a subset.
// You can swipe through these guis. Will be in the order you place them here in the vector.
// By default, it is a list of the guis that have been registered in main.cpp
// If a scene defines a scene specific gui list, this will be used instead as long as the scene is active and we don't explicitely navigate back to main_gui_list
t_gui_list main_gui_list;

void register_scene_defaultKeys(void) {
  key_repeatModes_default = {
                                                                                                             {KEY_OFF,   SHORT            },
    {KEY_STOP,  SHORT            },    {KEY_REWI,  SHORTorLONG      },    {KEY_PLAY,  SHORT            },    {KEY_FORW,  SHORTorLONG      },
    {KEY_CONF,  SHORT            },                                                                          {KEY_INFO,  SHORT            },
                                                         {KEY_UP,    SHORT            },
                      {KEY_LEFT,  SHORT            },    {KEY_OK,    SHORT            },    {KEY_RIGHT, SHORT            },
                                                         {KEY_DOWN,  SHORT            },
    {KEY_BACK,  SHORT            },                                                                          {KEY_SRC,   SHORT            },
    {KEY_VOLUP, SHORT_REPEATED   },                      {KEY_MUTE,  SHORT            },                     {KEY_CHUP,  SHORT            },
    {KEY_VOLDO, SHORT_REPEATED   },                      {KEY_REC,   SHORT            },                     {KEY_CHDOW, SHORT            },
    {KEY_RED,   SHORT            },    {KEY_GREEN, SHORT            },    {KEY_YELLO, SHORT            },    {KEY_BLUE,  SHORT            },
  };
  
  key_commands_short_default = {
                                                                                                             {KEY_OFF,   SCENE_ALLOFF_FORCE},
    {KEY_STOP,  GUI_SMARTHOME_ACTIVATE},/*{KEY_REWI,  COMMAND_UNKNOWN  }, {KEY_PLAY,  COMMAND_UNKNOWN  },    {KEY_FORW,  COMMAND_UNKNOWN  },*/
  /*{KEY_CONF,  COMMAND_UNKNOWN  },                                                                          {KEY_INFO,  COMMAND_UNKNOWN  },*/
                                                     /*  {KEY_UP,    COMMAND_UNKNOWN  },*/
                      {KEY_LEFT,  GUI_PREV  },       /*  {KEY_OK,    COMMAND_UNKNOWN  },*/  {KEY_RIGHT, GUI_NEXT  },
                                                     /*  {KEY_DOWN,  COMMAND_UNKNOWN  },*/
    {KEY_BACK,  SCENE_SELECTION  },                                                                        /*{KEY_SRC,   COMMAND_UNKNOWN  },*/
    {KEY_VOLUP, YAMAHA_VOL_PLUS  },                      {KEY_MUTE,  YAMAHA_MUTE_TOGGLE},                  /*{KEY_CHUP,  COMMAND_UNKNOWN  },*/
    {KEY_VOLDO, YAMAHA_VOL_MINUS },                      {KEY_REC,   SCENE_BACK_TO_PREVIOUS_GUI_LIST  },   /*{KEY_CHDOW, COMMAND_UNKNOWN  },*/
    {KEY_RED,   SCENE_TV_FORCE   },    {KEY_GREEN, SCENE_FIRETV_FORCE},  {KEY_YELLO, SCENE_CHROMECAST_FORCE},{KEY_BLUE,  SCENE_APPLETV_FORCE},
  };
  
  key_commands_long_default = {
  
  
  };

  register_command(&SCENE_SELECTION                , makeCommandData(SCENE, {scene_name_selection}));
  register_command(&SCENE_BACK_TO_PREVIOUS_GUI_LIST, makeCommandData(SCENE, {scene_back_to_previous_gui_list}));
  register_command(&GUI_PREV                       , makeCommandData(SCENE, {scene_gui_prev}));
  register_command(&GUI_NEXT                       , makeCommandData(SCENE, {scene_gui_next}));

}
