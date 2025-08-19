#pragma once

/*
  If you want to create a new GUI (tab in terms of lvgl) for the touch screen, then
  - copy one of the existing GUIs, e.g. gui_numpad.cpp and gui_numpad.h
  - place the files in the right folder:
    - in folder "gui_general_and_keys" if they are of general purpose, not only for a specific device
    - in the device folder, if the GUI is specific for a certain device
    - in folder "scenes" if the GUI is intended for a specific scene
    It does not really matter where the files are placed, but doing so helps to keep a clean structure
  - rename the functions, they must not have the same name as the ones in the file you copied from
  - call "register_gui_<nameOfGUI>();" in main.cpp

  register_gui(std::string(tabName), & create_tab_content_settings, & notify_tab_before_delete_settings);

  Technically spoken, to register a GUI, you need to provide several function pointers:
  - one for creating the content of the tab
  - one to persist the state of the gui before it will be deleted, if needed
  - and the name of the tab
*/
#include <string>
#include <map>
#include <lvgl.h>
#include "applicationInternal/keys.h"

typedef void (*create_tab_content)(lv_obj_t* tab);
typedef void (*notify_tab_before_delete)(void);
typedef void (*gui_setKeys)(void);

// https://stackoverflow.com/questions/840501/how-do-function-pointers-in-c-work
struct gui_definition {
  std::string this_name;
  create_tab_content this_create_tab_content;
  notify_tab_before_delete this_notify_tab_before_delete;
  gui_setKeys this_gui_setKeys;
  key_repeatModes this_key_repeatModes;
  key_commands_short this_key_commands_short;
  key_commands_long this_key_commands_long;
};

extern std::map<std::string, gui_definition> registered_guis_byName_map;

void register_gui(
  std::string a_name,
  create_tab_content a_create_tab_content,
  notify_tab_before_delete a_notify_tab_before_delete,
  gui_setKeys a_gui_setKeys = NULL,
  key_repeatModes a_key_repeatModes = NULL,
  key_commands_short a_key_commands_short = NULL,
  key_commands_long a_key_commands_long = NULL
  );

void setKeysForAllRegisteredGUIsAndScenes();