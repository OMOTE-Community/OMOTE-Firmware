#include <string>
#include <lvgl.h>
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiMemoryOptimizer.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"
#include "guis/gui_sceneSelection.h"
#include "scenes/scene__default.h"

void setLabelActiveScene() {
  if ((SceneLabel != NULL) && sceneExists(gui_memoryOptimizer_getActiveSceneName())) {
    lv_label_set_text(SceneLabel, gui_memoryOptimizer_getActiveSceneName().c_str());
  }
}

void showSpecificGUI(GUIlists GUIlist, std::string GUIname);

void handleScene(uint16_t command, commandData commandData, std::string additionalPayload = "") {

  auto current = commandData.commandPayloads.begin();
  std::string scene_name = *current;

  // --- do not switch scene, but show scene selection gui. From that on, we are in the main_gui_list. ----------------
  if (scene_name == scene_name_selection) {
    omote_log_d("scene: will show scene selection gui\r\n");
    showSpecificGUI(MAIN_GUI_LIST, tabName_sceneSelection);
    return;
  }

  // --- do not switch scene, but navigate to the prev or next gui in the currently active list of guis ---------------
  if ((scene_name == scene_gui_next) || (scene_name == scene_gui_prev)) {
    if (scene_name == scene_gui_prev) {
      if (gui_memoryOptimizer_getActiveTabID() == 0) {
        omote_log_d("scene: cannot navigate to prev gui, because there is none\r\n");
      } else {
        omote_log_d("scene: will navigate to prev gui\r\n");
        setActiveTab(gui_memoryOptimizer_getActiveTabID() -1, LV_ANIM_ON, true);
      }

    } else if (scene_name == scene_gui_next) {
      if (!gui_memoryOptimizer_isTabIDInMemory(gui_memoryOptimizer_getActiveTabID() +1)) {
        omote_log_d("scene: cannot navigate to next gui, because there is none\r\n");
      } else {
        omote_log_d("scene: will navigate to next gui\r\n");
        setActiveTab(gui_memoryOptimizer_getActiveTabID() +1, LV_ANIM_ON, true);
      }

    }
    return;
  }

  // --- do not switch scene. Switch to the other gui list ------------------------------------------------------------
  if (scene_name == scene_back_to_previous_gui_list) {
    
    if (get_scene_has_gui_list(gui_memoryOptimizer_getActiveSceneName())) {
      omote_log_d("scene: will navigate back to last active gui from previous gui list\r\n");
      guis_doTabCreationForNavigateToLastActiveGUIofPreviousGUIlist();
    } else {
      omote_log_d("scene: cannot navigate back to last active gui from previous gui list, because no scene specific gui list was defined\r\n");
    }

    return;
  }

  // --- This is the normal case. We navigate to a new scene ----------------------------------------------------------

  // FORCE can be either as second payload in commandData
  // e.g. register_command(&SCENE_TV_FORCE  , makeCommandData(SCENE, {scene_name_TV, "FORCE"}));
  // or as additionalPayload, used by gui_sceneSelection.cpp
  // e.g. executeCommand(activate_scene_command, "FORCE");

  // we can have a second payload
  std::string isForcePayload = "";
  ++current;
  if (current != commandData.commandPayloads.end()) {
    isForcePayload = *current;
  }

  // check if we know the new scene
  if (!sceneExists(scene_name)) {
    omote_log_w("scene: cannot start scene %s, because it is unknown\r\n", scene_name.c_str());
    return;
  } else {
    omote_log_d("scene: will switch from old scene %s to new scene %s\r\n", gui_memoryOptimizer_getActiveSceneName().c_str(), scene_name.c_str());
  }

  // do not activate the same scene again, only when forced to do so (e.g. by long press on the gui or when selected by hardware key)
  bool callEndAndStartSequences;
  if ((scene_name == gui_memoryOptimizer_getActiveSceneName()) && ((isForcePayload != "FORCE") && (additionalPayload != "FORCE"))) {
    omote_log_d("scene: will not start scene again, because it is already active\r\n");
    callEndAndStartSequences = false;
  } else if ((scene_name == gui_memoryOptimizer_getActiveSceneName()) && ((isForcePayload == "FORCE") || (additionalPayload == "FORCE"))) {
    omote_log_d("scene: scene is already active, but FORCE was set, so start scene again\r\n");
    callEndAndStartSequences = true;
  } else {
    // this is the default when switching to a different scene
    callEndAndStartSequences = true;
  }

  if (SceneLabel != NULL) {lv_label_set_text(SceneLabel, "changing...");}
  gui_loop();

  if (callEndAndStartSequences) {
    // end old scene
    if (!sceneExists(gui_memoryOptimizer_getActiveSceneName()) && (gui_memoryOptimizer_getActiveSceneName() != "")) {
      omote_log_w("scene: WARNING: cannot end scene %s, because it is unknown\r\n", gui_memoryOptimizer_getActiveSceneName().c_str());
  
    } else {
      if (gui_memoryOptimizer_getActiveSceneName() != "") {
        omote_log_d("scene: will call end sequence for scene %s\r\n", gui_memoryOptimizer_getActiveSceneName().c_str());
        scene_end_sequence_from_registry(gui_memoryOptimizer_getActiveSceneName());
      }
  
    }

    // start new scene
    omote_log_d("scene: will call start sequence for scene %s\r\n", scene_name.c_str());
    scene_start_sequence_from_registry(scene_name);
  }

  gui_memoryOptimizer_setActiveSceneName(scene_name);

  if (SceneLabel != NULL) {lv_label_set_text(SceneLabel, gui_memoryOptimizer_getActiveSceneName().c_str());}

  omote_log_d("scene: scene handling finished, new scene %s is active\r\n", gui_memoryOptimizer_getActiveSceneName().c_str());

  guis_doTabCreationAfterGUIlistChanged(SCENE_GUI_LIST);
}

void showSpecificGUI(GUIlists GUIlist, std::string GUIname) {
  gui_list gui_list_for_search = get_gui_list_withFallback(GUIlist);

  // 1. search for gui in the gui list
  int gui_list_index = -1;
  for (int i=0; i < gui_list_for_search->size(); i++) {
    if (gui_list_for_search->at(i) == GUIname) {
      omote_log_d("showSpecificGUI: found GUI with name \"%s\" in %s at position %d\r\n", GUIname.c_str(), GUIlist == MAIN_GUI_LIST ? "\"main_gui_list\"" : "\"scene gui list\"", i);
      gui_list_index = i;
      break;
    }
  }
  
  // 2. call guiBase.cpp
  if ((gui_list_index >= 0) && (gui_list_index < gui_list_for_search->size())) {
    guis_doTabCreationForSpecificGUI(GUIlist, gui_list_index);

  } else {
    // gui was not found
    omote_log_w("showSpecificGUI: GUI with name \"%s\" was not found in gui list %s. Cannot navigate to that GUI\r\n", GUIname.c_str(), GUIlist == MAIN_GUI_LIST ? "\"main_gui_list\"" : "\"scene gui list\"");
    return;
  }  
}

void handleGUI(uint16_t command, commandData commandData, std::string additionalPayload = "") {

  auto current = commandData.commandPayloads.begin();
  GUIlists GUIlist = (GUIlists)std::stoi(*current);

  current = std::next(current, 1);
  std::string GUIname = *current;

  showSpecificGUI(GUIlist, GUIname);

}
