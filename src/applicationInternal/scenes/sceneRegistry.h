#pragma once

#include <string>
#include <vector>
#include "applicationInternal/keys.h"
#include "applicationInternal/gui/guiMemoryOptimizer.h"

typedef std::vector<std::string> t_gui_list;
typedef std::vector<std::string> t_scene_list;

typedef void (*scene_setKeys)(void);
typedef void (*scene_start_sequence)(void);
typedef void (*scene_end_sequence)(void);
typedef t_gui_list *gui_list;
typedef t_scene_list *scene_list;

// https://stackoverflow.com/questions/840501/how-do-function-pointers-in-c-work
struct scene_definition {
  scene_setKeys this_scene_setKeys;
  scene_start_sequence this_scene_start_sequence;
  scene_end_sequence this_scene_end_sequence;
  key_repeatModes this_key_repeatModes;
  key_commands_short this_key_commands_short;
  key_commands_long this_key_commands_long;
  gui_list this_gui_list;
  uint16_t this_activate_scene_command;
};

extern std::map<std::string, scene_definition> registered_scenes;

void register_scene(
  std::string a_scene_name,
  scene_setKeys a_scene_setKeys,
  scene_start_sequence a_scene_start_sequence,
  scene_end_sequence a_scene_end_sequence,
  key_repeatModes a_key_repeatModes,
  key_commands_short a_key_commands_short,
  key_commands_long a_key_commands_long,
  gui_list a_gui_list = NULL,
  uint16_t a_activate_scene_command = 0);

bool sceneExists(std::string sceneName);
void scene_start_sequence_from_registry(std::string sceneName);
void scene_end_sequence_from_registry(std::string sceneName);
repeatModes get_key_repeatMode(std::string sceneName, char keyChar);
uint16_t get_command_short(std::string sceneName, char keyChar);
uint16_t get_command_long(std::string sceneName, char keyChar);
gui_list get_gui_list_withFallback(GUIlists gui_list);
gui_list get_gui_list_active_withFallback();
bool get_scene_has_gui_list(std::string sceneName);
uint16_t get_activate_scene_command(std::string sceneName);
scene_list get_scenes_on_sceneSelectionGUI();
void set_scenes_on_sceneSelectionGUI(t_scene_list a_scene_list);

extern char KEY_OFF  ;
extern char KEY_STOP ;
extern char KEY_REWI ;
extern char KEY_PLAY ;
extern char KEY_FORW ;
extern char KEY_CONF ;
extern char KEY_INFO ;
extern char KEY_UP   ;
extern char KEY_DOWN ;
extern char KEY_LEFT ;
extern char KEY_RIGHT;
extern char KEY_OK   ;
extern char KEY_BACK ;
extern char KEY_SRC  ;
extern char KEY_VOLUP;
extern char KEY_VOLDO;
extern char KEY_MUTE ;
extern char KEY_REC  ;
extern char KEY_CHUP ;
extern char KEY_CHDOW;
extern char KEY_RED  ;
extern char KEY_GREEN;
extern char KEY_YELLO;
extern char KEY_BLUE ;
