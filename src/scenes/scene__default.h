#pragma once

#include <string>
#include <stdint.h>
#include <map>
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"

extern uint16_t SCENE_SELECTION;                    // command
extern std::string scene_name_selection;            // payload: name of this fake default scene
extern uint16_t SCENE_BACK_TO_PREVIOUS_GUI_LIST;    // command
extern std::string scene_back_to_previous_gui_list; // payload: name of this fake scene
extern uint16_t GUI_PREV;                           // command
extern std::string scene_gui_prev;                  // payload: name of this fake scene
extern uint16_t GUI_NEXT;                           // command
extern std::string scene_gui_next;                  // payload: name of this fake scene

extern std::map<char, repeatModes> key_repeatModes_default;
extern std::map<char, uint16_t> key_commands_short_default;
extern std::map<char, uint16_t> key_commands_long_default;

extern t_gui_list main_gui_list;

void register_scene_defaultKeys(void);
