#include <map>
#include <stdexcept>
#include "applicationInternal/gui/guiMemoryOptimizer.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"
// scenes
#include "scenes/scene__default.h"

std::map<std::string, scene_definition> registered_scenes;
t_scene_list scenes_on_sceneSelectionGUI;

void register_scene(
  std::string a_scene_name,
  scene_setKeys a_scene_setKeys,
  scene_start_sequence a_scene_start_sequence,
  scene_end_sequence a_scene_end_sequence,
  key_repeatModes a_key_repeatModes,
  key_commands_short a_key_commands_short,
  key_commands_long a_key_commands_long,
  gui_list a_gui_list,
  uint16_t a_activate_scene_command) {

  // put the scene_definition in a map that can be accessed by name
  registered_scenes[a_scene_name] = scene_definition{
    a_scene_setKeys,
    a_scene_start_sequence,
    a_scene_end_sequence,
    a_key_repeatModes,
    a_key_commands_short,
    a_key_commands_long,
    a_gui_list,
    a_activate_scene_command
  };

  // Additionally, put all registered scenes in a sequence of scenes to be shown in the sceneSelection gui.
  // Exactly in the order they have been registered.
  // Can be overwritten in main.cpp
  scenes_on_sceneSelectionGUI.insert(scenes_on_sceneSelectionGUI.end(), {std::string(a_scene_name)});

  setKeysForAllRegisteredGUIsAndScenes();

}

bool sceneExists(std::string sceneName) {
  return (registered_scenes.count(sceneName) > 0);
}

void scene_start_sequence_from_registry(std::string sceneName) {
  try {
    registered_scenes.at(sceneName).this_scene_start_sequence();
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("scene_start_sequence_from_registry: internal error, sceneName not registered\r\n");
  }
}

void scene_end_sequence_from_registry(std::string sceneName) {
  try {
    registered_scenes.at(sceneName).this_scene_end_sequence();
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("scene_end_sequence_from_registry: internal error, sceneName not registered\r\n");
  }
}

repeatModes get_key_repeatMode(std::string sceneName, char keyChar) {
  try {
    // look if the map of the active gui has a definition for it
    std::string GUIname = gui_memoryOptimizer_getActiveGUIname();
    if ((registered_guis_byName_map.count(GUIname) > 0) && (registered_guis_byName_map.at(GUIname).this_key_repeatModes != NULL) && (registered_guis_byName_map.at(GUIname).this_key_repeatModes->count(keyChar) > 0)) {
      omote_log_v("get_key_repeatMode: will use key from gui %s\r\n", GUIname.c_str());
      return registered_guis_byName_map.at(GUIname).this_key_repeatModes->at(keyChar);

    // look if the map of the active scene has a definition for it
    } else if ((registered_scenes.count(sceneName) > 0) && (registered_scenes.at(sceneName).this_key_repeatModes->count(keyChar) > 0)) {
      omote_log_v("get_key_repeatMode: will use key from scene %s\r\n", sceneName.c_str());
      return registered_scenes.at(sceneName).this_key_repeatModes->at(keyChar);

    // look if there is a default definition
    } else if (key_repeatModes_default.count(keyChar) > 0) {
      omote_log_v("get_key_repeatMode: will use default key\r\n");
      return key_repeatModes_default.at(keyChar);

    // no key definition found
    } else {
      omote_log_v("get_key_repeatMode: WARNING no key definition found\r\n");
      return REPEAT_MODE_UNKNOWN;
    }
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("get_key_repeatMode: internal error, sceneName not registered\r\n");
    return REPEAT_MODE_UNKNOWN;
  }
}

uint16_t get_command_short(std::string sceneName, char keyChar) {
  try {
    // look if the map of the active gui has a definition for it
    std::string GUIname = gui_memoryOptimizer_getActiveGUIname();
    if ((registered_guis_byName_map.count(GUIname) > 0) && (registered_guis_byName_map.at(GUIname).this_key_commands_short != NULL) && (registered_guis_byName_map.at(GUIname).this_key_commands_short->count(keyChar) > 0)) {
      omote_log_v("get_command_short: will use key from gui %s\r\n", GUIname.c_str());
      return registered_guis_byName_map.at(GUIname).this_key_commands_short->at(keyChar);

    // look if the map of the active scene has a definition for it
    } else if ((registered_scenes.count(sceneName) > 0) && (registered_scenes.at(sceneName).this_key_commands_short->count(keyChar) > 0)) {
      omote_log_v("get_command_short: will use key from scene %s\r\n", sceneName.c_str());
      return registered_scenes.at(sceneName).this_key_commands_short->at(keyChar);
    
    // look if there is a default definition
    } else if (key_commands_short_default.count(keyChar) > 0) {
      omote_log_v("get_command_short: will use default key\r\n");
      return key_commands_short_default.at(keyChar);
    
    // no key definition found
    } else {
      omote_log_v("get_command_short: WARNING no key definition found\r\n");
      return COMMAND_UNKNOWN;
    }
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("get_command_short: internal error, sceneName not registered\r\n");
    return COMMAND_UNKNOWN;
  }

}

uint16_t get_command_long(std::string sceneName, char keyChar) {
  try {
    // look if the map of the active gui has a definition for it
    std::string GUIname = gui_memoryOptimizer_getActiveGUIname();
    if ((registered_guis_byName_map.count(GUIname) > 0) && (registered_guis_byName_map.at(GUIname).this_key_commands_long != NULL) && (registered_guis_byName_map.at(GUIname).this_key_commands_long->count(keyChar) > 0)) {
      omote_log_v("get_command_long: will use key from gui %s\r\n", GUIname.c_str());
      return registered_guis_byName_map.at(GUIname).this_key_commands_long->at(keyChar);

    // look if the map of the active scene has a definition for it
    } else if ((registered_scenes.count(sceneName) > 0) && (registered_scenes.at(sceneName).this_key_commands_long->count(keyChar) > 0)) {
      omote_log_v("get_command_long: will use key from scene %s\r\n", sceneName.c_str());
      return registered_scenes.at(sceneName).this_key_commands_long->at(keyChar);
    
    // look if there is a default definition
    } else if (key_commands_long_default.count(keyChar) > 0) {
      omote_log_v("get_command_long: will use default key\r\n");
      return key_commands_long_default.at(keyChar);
    
    // no key definition found
    } else {
      omote_log_v("get_command_long: WARNING no key definition found\r\n");
      return COMMAND_UNKNOWN;
    }
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("get_command_long: internal error, sceneName not registered\r\n");
    return COMMAND_UNKNOWN;
  }

}

gui_list get_gui_list_withFallback(GUIlists gui_list) {
  try {
    // If gui_list == MAIN_GUI_LIST, then we want the main_gui_list, either if a scene is active or not.
    // If gui_list == SCENE_GUI_LIST, then we want the scene gui list. If none is defined, return main_gui_list as fallback.

    if (gui_list == MAIN_GUI_LIST) {
      return &main_gui_list;

    } else {
      #if (USE_SCENE_SPECIFIC_GUI_LIST != 0)
        // look if the active scene has a definition for a gui list
        if ((registered_scenes.count(gui_memoryOptimizer_getActiveSceneName()) > 0) && (registered_scenes.at(gui_memoryOptimizer_getActiveSceneName()).this_gui_list != NULL)) {
          omote_log_v("get_gui_list: will use gui_list from scene %s\r\n", gui_memoryOptimizer_getActiveSceneName().c_str());
          return registered_scenes.at(gui_memoryOptimizer_getActiveSceneName()).this_gui_list;
        } else {
          // no scene specific gui list was defined
          return &main_gui_list;
        }
      #else
        // never use scene specific gui list
        return &main_gui_list;
      #endif
    }  
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("get_gui_list: internal error, sceneName not registered\r\n");
    return NULL;
  }
}

gui_list get_gui_list_active_withFallback() {
  return get_gui_list_withFallback(gui_memoryOptimizer_getActiveGUIlist());
}

bool get_scene_has_gui_list(std::string sceneName) {
  try {
    // look if the scene is known
    if ((registered_scenes.count(sceneName) > 0)) {
      return (registered_scenes.at(sceneName).this_gui_list != NULL);
    } else {
      return false;
    }
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("get_scene_has_gui_list: internal error, sceneName not registered\r\n");
    return false;
  }
}

uint16_t get_activate_scene_command(std::string sceneName) {
  try {
    // look if the scene is known
    if ((registered_scenes.count(sceneName) > 0)) {
      omote_log_v("get_activate_scene_command: will use activate_scene_command from scene %s\r\n", sceneName.c_str());
      return registered_scenes.at(sceneName).this_activate_scene_command;
    
    // if the scene is not know, simply return 0
    } else {
      omote_log_v("get_activate_scene_command: will return 0\r\n");
      return 0;
    
    }
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("get_activate_scene_command: internal error, sceneName not registered\r\n");
    return 0;
  }

}

scene_list get_scenes_on_sceneSelectionGUI() {
  return &scenes_on_sceneSelectionGUI;
}

void set_scenes_on_sceneSelectionGUI(t_scene_list a_scene_list) {
  scenes_on_sceneSelectionGUI.clear();
  for (int i=0; i<a_scene_list.size(); i++) {
    scenes_on_sceneSelectionGUI.insert(scenes_on_sceneSelectionGUI.end(), a_scene_list.at(i));
  }

}


char KEY_OFF    = 'o';
char KEY_STOP   = '=';
char KEY_REWI   = '<';
char KEY_PLAY   = 'p';
char KEY_FORW   = '>';
char KEY_CONF   = 'c';
char KEY_INFO   = 'i';
char KEY_UP     = 'u';
char KEY_DOWN   = 'd';
char KEY_LEFT   = 'l';
char KEY_RIGHT  = 'r';
char KEY_OK     = 'k';
char KEY_BACK   = 'b';
char KEY_SRC    = 's';
char KEY_VOLUP  = '+';
char KEY_VOLDO  = '-';
char KEY_MUTE   = 'm';
char KEY_REC    = 'e';
char KEY_CHUP   = '^';
char KEY_CHDOW  = 'v';
char KEY_RED    = '1';
char KEY_GREEN  = '2';
char KEY_YELLO  = '3';
char KEY_BLUE   = '4';
