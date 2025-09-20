#include <lvgl.h>
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiMemoryOptimizer.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/omote_log.h"

struct t_gui_on_tab {
  lv_obj_t* tab;
  std::string GUIname;
  int gui_list_index;
  int gui_list_index_previous;
};
struct t_gui_state {
  // the next three and the last are saved in the preferenceStorage every time they change
  std::string activeScene_internalDontUse;
  std::string activeGUIname_internalDontUse;
  GUIlists activeGUIlist_internalDontUse;
  // ---
  int activeTabID = -1;      // id of the active tab (one of 0,1,2)
  int oldTabID = -1;         // id of the tab before swiping (one of 0,1,2)
  t_gui_on_tab gui_on_tab[3] = {{NULL, "", -1, -1}, {NULL, "", -1, -1}, {NULL, "", -1, -1}};
  // the last active gui of scene. Will be stored to easily navigate back to it with guis_doTabCreationForNavigateToLastActiveGUIofPreviousGUIlist()
  GUIlists last_active_gui_list = (GUIlists)-1;
  int last_active_gui_list_index_internalDontUse = -1;
};
t_gui_state gui_state;

// Both the gui_state and the preferenceStorage should know at any time the current state (scene, GUIname, and GUIlist)
// preferenceStorage should know it because when going to sleep, it should persist the state in NVM.
// So whenever values change, it should be done through these functions.
// On startup, the gui_state is set by gui_memoryOptimizer_onStartup()
std::string gui_memoryOptimizer_getActiveSceneName() {
  gui_state.activeScene_internalDontUse = get_activeScene();
  return gui_state.activeScene_internalDontUse;
}
void gui_memoryOptimizer_setActiveSceneName(std::string aSceneName) {
  gui_state.activeScene_internalDontUse = aSceneName;
  set_activeScene(aSceneName);
}
std::string gui_memoryOptimizer_getActiveGUIname() {
  gui_state.activeGUIname_internalDontUse = get_activeGUIname();
  return gui_state.activeGUIname_internalDontUse;
}
void gui_memoryOptimizer_setActiveGUIname(std::string aGUIname) {
  gui_state.activeGUIname_internalDontUse = aGUIname;
  set_activeGUIname(aGUIname);
}
GUIlists gui_memoryOptimizer_getActiveGUIlist() {
  gui_state.activeGUIlist_internalDontUse = (GUIlists)get_activeGUIlist();
  return gui_state.activeGUIlist_internalDontUse;
}
void gui_memoryOptimizer_setActiveGUIlist(GUIlists aGUIlist) {
  gui_state.activeGUIlist_internalDontUse = aGUIlist;
  set_activeGUIlist(aGUIlist);
}
int gui_memoryOptimizer_getLastActiveGUIlistIndex() {
  gui_state.last_active_gui_list_index_internalDontUse = get_lastActiveGUIlistIndex();
  return gui_state.last_active_gui_list_index_internalDontUse;
}
void gui_memoryOptimizer_setLastActiveGUIlistIndex(int aGUIlistIndex) {
  gui_state.last_active_gui_list_index_internalDontUse = aGUIlistIndex;
  set_lastActiveGUIlistIndex(aGUIlistIndex);
}

int gui_memoryOptimizer_getActiveTabID() {
  return gui_state.activeTabID;
}

bool gui_memoryOptimizer_isTabIDInMemory(int tabID) {
  // range check
  if ((tabID < 0) || (tabID >= 3)) {
    return false;
  }
  return (gui_state.gui_on_tab[tabID].gui_list_index != -1);
}

bool gui_memoryOptimizer_isGUInameInMemory(std::string GUIname) {
  for (uint8_t index=0; index <= 2; index++) {
    if (gui_state.gui_on_tab[index].GUIname == GUIname) {
      return true;
    }
  }
  return false;
}

void notify_active_tabs_before_delete(t_gui_state *gui_state) {
  omote_log_d("  Will notify tabs about deletion\r\n");
  std::string nameOfTab;
  for (int index=0; index <= 2; index++) {
    if (gui_state->gui_on_tab[index].gui_list_index == -1) {
      omote_log_d("    Will not notify tab %d about deletion because it does not exist\r\n", index);
      continue;
    }

    // For deletion, do not use the gui_list_index, but the name of the gui.
    // The gui_list might have changed (when switching from a scene specific list to the main list or vice versa), so index could have changed as well.
    nameOfTab = gui_state->gui_on_tab[index].GUIname;
    if (nameOfTab == "") {
      omote_log_w("    Will not notify tab %d about deletion because it is not set\r\n", index);
    } else if (registered_guis_byName_map.count(nameOfTab) == 0) {
      omote_log_w("    Can not notify tab %d about deletion because name \"%s\" was not found in registry\r\n", index, nameOfTab.c_str());
    } else {
      omote_log_d("    Will notify tab %d with name \"%s\" about deletion\r\n", index, nameOfTab.c_str());
      registered_guis_byName_map.at(nameOfTab).this_notify_tab_before_delete();
    }
  }
}

void clear_tabview(lv_obj_t* tabview, t_gui_state *gui_state) {
  if (tabview != NULL) {
    // first remove events for the tabview
    lv_obj_remove_event_cb(tabview, tabview_tab_changed_event_cb);
    lv_obj_remove_event_cb(tabview, tabview_content_is_scrolling_event_cb);
    // delete tabview
    lv_obj_del(tabview);
    tabview = NULL;
  }

  // the gui_list_index_previous is needed for setGUIlistIndicesToBeShown_afterSlide();
  gui_state->gui_on_tab[0] = {NULL, "", -1, gui_state->gui_on_tab[0].gui_list_index};
  gui_state->gui_on_tab[1] = {NULL, "", -1, gui_state->gui_on_tab[1].gui_list_index};
  gui_state->gui_on_tab[2] = {NULL, "", -1, gui_state->gui_on_tab[2].gui_list_index};

}

void clear_panel(lv_obj_t* panel, lv_obj_t* img1, lv_obj_t* img2) {
  if (panel != NULL) {
    lv_obj_del(panel);
    panel = NULL;
  }
  if (img1 != NULL) {
    lv_obj_del(img1);
    img1 = NULL;
  } 
  if (img2 != NULL) {
    lv_obj_del(img2);
    img2 = NULL;
  } 

}

lv_obj_t* create_tabview() {
  // Setup a scrollable tabview for devices and settings ----------------------------------------------------
  lv_obj_t* tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 0); // Hide tab labels by setting their height to 0
  #ifdef drawRedBorderAroundMainWidgets
  lv_obj_add_style(tabview, &style_red_border, LV_PART_MAIN);
  #endif
  lv_obj_set_style_bg_color(tabview, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_size(tabview, SCR_WIDTH, tabviewHeight);
  lv_obj_align(tabview, LV_ALIGN_TOP_MID, 0, tabviewTop);
  return tabview;
}

lv_obj_t* create_panel() {
  // Create a page indicator at the bottom ------------------------------------------------------------------
  lv_obj_t* panel = lv_obj_create(lv_scr_act());
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_CLICKABLE);  // This indicator will not be clickable
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE); // This indicator will not be scrollable
  lv_obj_set_size(panel, SCR_WIDTH, panelHeight);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
  lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);

  return panel;
}

std::string get_name_of_gui_to_be_shown(int index) {
  if (index == -1) {
    return "";
  } else if (index <= get_gui_list_active_withFallback()->size() -1) {
    return get_gui_list_active_withFallback()->at(index);
  } else {
    return "";
  }
}

void create_new_tab(lv_obj_t* tabview, t_gui_on_tab *gui_on_tab) {
  std::string nameOfTab = get_name_of_gui_to_be_shown(gui_on_tab->gui_list_index);

  if (nameOfTab == "") {
    omote_log_d("    Will not create new tab because no name was provided\r\n");
  } else if (registered_guis_byName_map.count(nameOfTab) == 0) {
    omote_log_w("    Will not create new tab because name %s was not found in registry\r\n", nameOfTab.c_str());
  } else {
    omote_log_d("    Will create tab with name \"%s\" \r\n", nameOfTab.c_str());
    // save name of tab for deletion later
    gui_on_tab->GUIname = nameOfTab;
    // create tab and save pointer to tab in gui_on_tab
    gui_on_tab->tab = lv_tabview_add_tab(tabview, nameOfTab.c_str());
    // let the gui create it's content
    registered_guis_byName_map.at(nameOfTab).this_create_tab_content(gui_on_tab->tab);
  }
}

// create up to three tabs and the content of the tabs
/*
example: gui_list: 0 1 2 3 4
in memory  active
0 1 -1     0 <- first state, special case - also the initial state
0 1 2      1
1 2 3      1
2 3 4      1
3 4 -1     1 <- last state, special case
*/
void setGUIlistIndicesToBeShown_forSpecificGUIlistIndex(int gui_list_index, t_gui_state *gui_state) {
  // Set the gui_list_indeces to be shown for a specific gui_list_index
  if (gui_list_index == 0) {
    // first state
    omote_log_d("  GUIlistIndices: will resume at specific index with \"first state\"\r\n");
    gui_state->gui_on_tab[0] = {NULL, "", 0};
    // take care if there is only one gui in list
    gui_state->gui_on_tab[1] = {NULL, "", get_gui_list_active_withFallback()->size() >= 2 ? 1 : -1};
    gui_state->gui_on_tab[2] = {NULL, "", -1};
    gui_state->activeTabID = 0;
  } else if (gui_list_index == get_gui_list_active_withFallback()->size() -1) {
    // last state
    omote_log_d("  GUIlistIndices: will resume at specific index with \"last state\"\r\n");
    gui_state->gui_on_tab[0] = {NULL, "", gui_list_index -1};
    gui_state->gui_on_tab[1] = {NULL, "", gui_list_index};
    gui_state->gui_on_tab[2] = {NULL, "", -1};
    gui_state->activeTabID = 1;
  } else {
    // any other state
    omote_log_d("  GUIlistIndices: will resume at specific index with \"state between\"\r\n");
    gui_state->gui_on_tab[0] = {NULL, "", gui_list_index -1};
    gui_state->gui_on_tab[1] = {NULL, "", gui_list_index};
    gui_state->gui_on_tab[2] = {NULL, "", gui_list_index +1};
    gui_state->activeTabID = 1;
  }
}

void setGUIlistIndicesToBeShown_forFirstGUIinGUIlist(t_gui_state *gui_state) {
  omote_log_d("  GUIlistIndices: will show the first gui from \"gui_list\" as initial state\r\n");
  // take care if there is no gui in list
  gui_state->gui_on_tab[0] = {NULL, "", get_gui_list_active_withFallback()->size() != 0 ? 0 : -1};
  // take care if there is only one gui in list
  gui_state->gui_on_tab[1] = {NULL, "", get_gui_list_active_withFallback()->size() >= 2 ? 1 : -1};
  gui_state->gui_on_tab[2] = {NULL, "", -1};
  gui_state->activeTabID = 0;
}

void setGUIlistIndicesToBeShown_afterSlide(t_gui_state *gui_state) {
  int oldListIndex = -1;
 
  if (gui_state->oldTabID > gui_state->activeTabID) {
    // swipe to previous item in list
    omote_log_d("  Will swipe to previous item in list\r\n");
    oldListIndex = gui_state->gui_on_tab[1].gui_list_index_previous;
    if ((oldListIndex == 1)) {
      // next state is the "first state"
      gui_state->gui_on_tab[0] = {NULL, "", 0};
      gui_state->gui_on_tab[1] = {NULL, "", 1};
      gui_state->gui_on_tab[2] = {NULL, "", -1};
      gui_state->activeTabID = 0;
    } else {
      gui_state->gui_on_tab[0] = {NULL, "", oldListIndex -2};
      gui_state->gui_on_tab[1] = {NULL, "", oldListIndex -1};
      gui_state->gui_on_tab[2] = {NULL, "", oldListIndex};
      gui_state->activeTabID = 1;
    }
  } else {
    // swipe to next item in list
    omote_log_d("  Will swipe to next item in list\r\n");
    if (gui_state->gui_on_tab[2].gui_list_index_previous == -1) {
      // last state was the first state
      oldListIndex = gui_state->gui_on_tab[0].gui_list_index_previous; // is always 0
    } else {
      oldListIndex = gui_state->gui_on_tab[1].gui_list_index_previous;
    }
    if (oldListIndex == get_gui_list_active_withFallback()->size() -2) {
      // next state is the "last state"
      gui_state->gui_on_tab[0] = {NULL, "", oldListIndex};
      gui_state->gui_on_tab[1] = {NULL, "", oldListIndex +1};
      gui_state->gui_on_tab[2] = {NULL, "", -1};
      gui_state->activeTabID = 1;
    } else {
      gui_state->gui_on_tab[0] = {NULL, "", oldListIndex};
      gui_state->gui_on_tab[1] = {NULL, "", oldListIndex +1};
      gui_state->gui_on_tab[2] = {NULL, "", oldListIndex +2};
      gui_state->activeTabID = 1;
    }
  }
 
}

void doTabCreation_strategyMax3(lv_obj_t* tabview, t_gui_state *gui_state) {
  
  // create the tabs
  omote_log_d("  Will create tabs. List indices of the three tabs are %d, %d, %d, tab nr %d will be activated\r\n", gui_state->gui_on_tab[0].gui_list_index, gui_state->gui_on_tab[1].gui_list_index, gui_state->gui_on_tab[2].gui_list_index, gui_state->activeTabID);
  for (int i=0; i<3; i++) {
    create_new_tab(tabview, &gui_state->gui_on_tab[i]);
  }

  if (get_gui_list_active_withFallback()->size() > 0) {
    std::string nameOfNewActiveTab = get_gui_list_active_withFallback()->at(gui_state->gui_on_tab[gui_state->activeTabID].gui_list_index);
    omote_log_d("  New visible tab is \"%s\"\r\n", nameOfNewActiveTab.c_str());

    // set active tab
    setActiveTab(gui_state->activeTabID, LV_ANIM_OFF);
    gui_memoryOptimizer_setActiveGUIname(nameOfNewActiveTab);
  }
}

LV_IMG_DECLARE(gradientLeft);
LV_IMG_DECLARE(gradientRight);

void fillPanelWithPageIndicator_strategyMax3(lv_obj_t* panel, lv_obj_t* img1, lv_obj_t* img2, t_gui_state *gui_state) {
  omote_log_d("  Will fill panel with page indicators\r\n");

  if (get_gui_list_active_withFallback()->size() == 0) {
    omote_log_d("    no tab available, so no page indicators\r\n");
    // at least add the style
    lv_obj_add_style(panel, &panel_style, 0);
    #ifdef drawRedBorderAroundMainWidgets
    lv_obj_add_style(panel, &style_red_border, LV_PART_MAIN);
    #endif
    return;
  }

  // This small hidden button enables the page indicator to scroll further
  lv_obj_t* btn = lv_btn_create(panel);
  lv_obj_set_size(btn, 50, lv_pct(100));
  lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
  lv_obj_set_style_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);

/*
  There needs to be two more screen indicators because of their different size (158 for page indicator, 240 for whole tab)
  In some cases they need to have color black, if they are before the first tab or after the last tab.
  In all other cases, they have color "color_primary". See this list:
  example: gui_list: 0 1 2 3 4
  in memory  color         active
  0 1 -1     b p p p       0 <- first state, special case - also the initial state
  0 1 2      b p p p p     1
  1 2 3      p p p p p     1
  2 3 4      p p p p b     1
  3 4 -1     p p p b       1 <- last state, special case
*/
  // first page indicator before the first tab
  btn = lv_btn_create(panel);
  lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(btn, 150, lv_pct(100));
  if (gui_state->gui_on_tab[0].gui_list_index == 0) {
    lv_obj_set_style_bg_color(btn, lv_color_black(), LV_PART_MAIN);
  } else {
    lv_obj_set_style_bg_color(btn, color_primary,    LV_PART_MAIN);
  }

  uint8_t breadcrumpDotSize     = 8; // should be an even number
  uint8_t breadcrumpDotDistance = 2; // should be an even number
  uint8_t breadcrumpMainGuiListLength = get_gui_list_withFallback(MAIN_GUI_LIST)->size();
  int8_t  breadcrumpMainGuiListStartPositionX = (-1) * (breadcrumpMainGuiListLength -1) * (breadcrumpDotSize + breadcrumpDotDistance) / 2;
  uint8_t breadcrumpSceneGuiListLength = get_gui_list_withFallback(SCENE_GUI_LIST)->size();
  int8_t  breadcrumpSceneGuiListStartPositionX = (-1) * (breadcrumpSceneGuiListLength -1) * (breadcrumpDotSize + breadcrumpDotDistance) / 2;
  #if (USE_SCENE_SPECIFIC_GUI_LIST != 0)
  bool show_scene_gui_list = get_scene_has_gui_list(gui_memoryOptimizer_getActiveSceneName());
  #else
  bool show_scene_gui_list = false;
  #endif
  int8_t breadcrumpMainGuiList_yPos;
  int8_t breadcrumpSceneGuiList_yPos;
  int8_t nameOfGUI_yPos;
  if (!show_scene_gui_list) {
    breadcrumpMainGuiList_yPos = -6;
    nameOfGUI_yPos = 6;
  } else {
    breadcrumpMainGuiList_yPos = -8;
    breadcrumpSceneGuiList_yPos = -1;
    nameOfGUI_yPos = 8;
  }

  // create the panel content for the three guis (or less) which are currently in memory
  std::string nameOfGUI;
  uint8_t breadcrumpPosition;
  for (int i=0; i<3; i++) {
    if (gui_state->gui_on_tab[i].gui_list_index != -1) {
      nameOfGUI = gui_state->gui_on_tab[i].GUIname;
      breadcrumpPosition = gui_state->gui_on_tab[i].gui_list_index +1;

      // Create actual buttons for every tab
      lv_obj_t* btn = lv_btn_create(panel);
      if (i == gui_state->activeTabID) {
        // only if this is the button for the currently active tab, make it clickable to get to scene selection gui
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(btn,(void *)(intptr_t)2);
        lv_obj_add_event_cb(btn, sceneLabel_or_pageIndicator_event_cb, LV_EVENT_CLICKED, NULL);

      } else if ((i==0 || i==1) && (gui_state->gui_on_tab[i+1].gui_list_index != -1)) {
        // this is the button on the previous tab, which can be seen on the active tab
        // activate click to prev tab
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(btn,(void *)(intptr_t)0);
        lv_obj_add_event_cb(btn, pageIndicator_navigate_event_cb, LV_EVENT_CLICKED, NULL);

      } else if (i==1 || i==2) {
        // this is the button on the next tab, which can be seen on the active tab
        // activate click to next tab
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(btn,(void *)(intptr_t)1);
        lv_obj_add_event_cb(btn, pageIndicator_navigate_event_cb, LV_EVENT_CLICKED, NULL);

      }
      lv_obj_set_size(btn, 150, lv_pct(100));
      lv_obj_remove_style(btn, NULL, LV_STATE_PRESSED);
      lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
      lv_obj_set_style_bg_color(btn, color_primary, LV_PART_MAIN);

      // create a breadcrump dot for each gui in the main_gui_list
      for (int j=0; j<breadcrumpMainGuiListLength; j++) {
        lv_obj_t* dot = lv_obj_create(btn);
        lv_obj_set_size(dot, breadcrumpDotSize, breadcrumpDotSize);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        // hightlight dot if it is the one for the currently active tab
        #if (USE_SCENE_SPECIFIC_GUI_LIST != 0)
        if ( ((gui_memoryOptimizer_getActiveGUIlist() == MAIN_GUI_LIST) || !get_scene_has_gui_list(gui_memoryOptimizer_getActiveSceneName()))
        #else
        if ( true
        #endif
             && (j == (breadcrumpPosition-1))) {
          lv_obj_set_style_bg_color(dot, lv_color_lighten(color_primary, 255), LV_PART_MAIN);
        } else if ((gui_memoryOptimizer_getActiveGUIlist() == SCENE_GUI_LIST) && get_scene_has_gui_list(gui_memoryOptimizer_getActiveSceneName()) && (j == gui_memoryOptimizer_getLastActiveGUIlistIndex())) {
          // hightlight dot a little bit if it is at least the one which was last active in the other gui list
          lv_obj_set_style_bg_color(dot, lv_color_lighten(color_primary, 140), LV_PART_MAIN);
        } else {
          lv_obj_set_style_bg_color(dot, lv_color_lighten(color_primary, 30), LV_PART_MAIN);
        }
        
        lv_obj_align(dot, LV_ALIGN_TOP_MID, breadcrumpMainGuiListStartPositionX +j*(breadcrumpDotSize + breadcrumpDotDistance), breadcrumpMainGuiList_yPos);
        // this dot needs to get clickable again
        lv_obj_set_user_data(dot,(void *)(intptr_t)1);
        lv_obj_add_flag(dot, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(dot, LV_OBJ_FLAG_EVENT_BUBBLE);
      }

      // create a breadcrump dot for each gui in the scene gui list, if there is one
      if (show_scene_gui_list) {
      for (int j=0; j<breadcrumpSceneGuiListLength; j++) {
        lv_obj_t* dot = lv_obj_create(btn);
        lv_obj_set_size(dot, breadcrumpDotSize, breadcrumpDotSize);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        if ((gui_memoryOptimizer_getActiveGUIlist() == SCENE_GUI_LIST) && (j == (breadcrumpPosition-1))) {
          // hightlight dot if it is the one for the currently active tab
          lv_obj_set_style_bg_color(dot, lv_color_lighten(color_primary, 255), LV_PART_MAIN);
        } else if ((gui_memoryOptimizer_getActiveGUIlist() == MAIN_GUI_LIST) && (j == gui_memoryOptimizer_getLastActiveGUIlistIndex())) {
          // hightlight dot a little bit if it is at least the one which was last active in the other gui list
          lv_obj_set_style_bg_color(dot, lv_color_lighten(color_primary, 140), LV_PART_MAIN);
        } else {
          lv_obj_set_style_bg_color(dot, lv_color_lighten(color_primary, 30), LV_PART_MAIN);
        }
        
        lv_obj_align(dot, LV_ALIGN_TOP_MID, breadcrumpSceneGuiListStartPositionX +j*(breadcrumpDotSize + breadcrumpDotDistance), breadcrumpSceneGuiList_yPos);
        // this dot needs to get clickable again
        lv_obj_set_user_data(dot,(void *)(intptr_t)1);
        lv_obj_add_flag(dot, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(dot, LV_OBJ_FLAG_EVENT_BUBBLE);
      }
      }


      // create a label for nameOfGUI
      lv_obj_t* label = lv_label_create(btn);
      lv_obj_set_style_text_font(label, &lv_font_montserrat_10, LV_PART_MAIN);
      lv_label_set_text_fmt(label, "%s", nameOfGUI.c_str());
      lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, nameOfGUI_yPos);

    }
  }

  // last page indicator after the last tab
  btn = lv_btn_create(panel);
  lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(btn, 150, lv_pct(100));
  //   4 at last position                                                                         4 at middle position                                                                          only one tab available overall
  if ((gui_state->gui_on_tab[2].gui_list_index == get_gui_list_active_withFallback()->size()-1) || (gui_state->gui_on_tab[1].gui_list_index == get_gui_list_active_withFallback()->size()-1) || (gui_state->gui_on_tab[1].gui_list_index == -1)) {
    lv_obj_set_style_bg_color(btn, lv_color_black(), LV_PART_MAIN);
  } else {
    lv_obj_set_style_bg_color(btn, color_primary,    LV_PART_MAIN);
  }

  // This small hidden button enables the page indicator to scroll further
  btn = lv_btn_create(panel);
  lv_obj_set_size(btn, 50, lv_pct(100));
  lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
  lv_obj_set_style_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);

  // creation of style was moved to init_gui(void)
  // otherwise repeated calls of lv_style_init will lead to a memory leak of about 46 bytes each time
  // https://docs.lvgl.io/8.3/overview/style.html?highlight=lv_style_t#initialize-styles-and-set-get-properties
  lv_obj_add_style(panel, &panel_style, 0);
  #ifdef drawRedBorderAroundMainWidgets
  lv_obj_add_style(panel, &style_red_border, LV_PART_MAIN);
  #endif

  // Make the indicator fade out at the sides using gradient bitmaps
  // Bitmaps are above the buttons and labels
  // don't create it here
  // img1 = lv_img_create(lv_scr_act());
  lv_img_set_src(img1, &gradientLeft);
  lv_obj_align(img1, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_size(img1, panelHeight, panelHeight); // stretch the 1-pixel high image to 30px
  #ifdef drawRedBorderAroundMainWidgets
  lv_obj_add_style(img1, &style_red_border, LV_PART_MAIN);
  #endif
  // don't create it here
  // img2 = lv_img_create(lv_scr_act());
  lv_img_set_src(img2, &gradientRight);
  lv_obj_align(img2, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_size(img2, panelHeight, panelHeight);
  #ifdef drawRedBorderAroundMainWidgets
  lv_obj_add_style(img2, &style_red_border, LV_PART_MAIN);
  #endif

}

void gui_memoryOptimizer_notifyAndClear(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2, t_gui_state *gui_state) {

  // 1. notify old guis that they will be deleted so that they can persist their state if needed
  notify_active_tabs_before_delete(gui_state);
  // 2. clear current tabview and save gui_list_index_previous (needed for swipe)
  clear_tabview(*tabview, gui_state);
  // 3. clear current panel for page indicator
  clear_panel(*panel, *img1, *img2);

}

void gui_memoryOptimizer_doContentCreation(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2, t_gui_state *gui_state);

// 1. tab creation on startup (called by init_gui())
// find the position of the current GUI in the gui list which was active last (both were automatically saved in the preferences) 
void gui_memoryOptimizer_onStartup(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2) {

  omote_log_i("Startup: try to resume at scene \"%s\" with GUI \"%s\"\r\n", gui_memoryOptimizer_getActiveSceneName().c_str(), gui_memoryOptimizer_getActiveGUIname().c_str());

  // Get last state from preferences and save it in gui_state
  // So it is ok to call them without using the return values.
  gui_memoryOptimizer_getActiveSceneName();
  gui_memoryOptimizer_getActiveGUIname();
  gui_memoryOptimizer_getActiveGUIlist();
  gui_memoryOptimizer_getLastActiveGUIlistIndex();

  // 1. find last used gui
  int gui_list_index = -1;
  // find index of gui_memoryOptimizer_getActiveGUIname() in gui_list_active
  for (int i=0; i<get_gui_list_active_withFallback()->size(); i++) {
    if (get_gui_list_active_withFallback()->at(i) == gui_memoryOptimizer_getActiveGUIname()) {
      omote_log_i("Startup: found GUI with name \"%s\" in \"gui_list_active\" at position %d\r\n", gui_memoryOptimizer_getActiveGUIname().c_str(), i);
      gui_list_index = i;
      break;
    }
  }
  
  // 2. set gui_list_indices and the tab to be activated
  if ((gui_list_index >= 0) && (gui_list_index < get_gui_list_active_withFallback()->size())) {
    // gui was found
    setGUIlistIndicesToBeShown_forSpecificGUIlistIndex(gui_list_index, &gui_state);

  } else {
    // gui was not found
    omote_log_w("Startup: GUI with name \"%s\" was not found. Will start with first GUI of main_gui_list\r\n", gui_memoryOptimizer_getActiveGUIname().c_str());
    gui_memoryOptimizer_setActiveGUIlist(MAIN_GUI_LIST);
    setGUIlistIndicesToBeShown_forFirstGUIinGUIlist(&gui_state);

  }  

  // 3. create content
  gui_memoryOptimizer_doContentCreation(tabview, panel, img1, img2, &gui_state);

}

// 2. tab creation after sliding (called by tabview_tab_changed_event_cb())
void gui_memoryOptimizer_afterSliding(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2, int newTabID) {

  // Here the magic for dynamic creation and deletion of lvgl objects happens to keep memory usage low.
  // The next and previous tab must always be available in the tabview, because they can already been seen during the animation.
  // And you always need 3 tabs, otherwise you even could not slide to the next or previous tab.
  // So we always have 3 tabs.
  // After the animation, the tabview and hence all tabs are deleted and recreated.

  omote_log_d("--- Start of tab deletion and creation\r\n");

  gui_state.oldTabID = gui_state.activeTabID;
  gui_state.activeTabID = newTabID;

  omote_log_d("Changing from oldTabID %d \"%s\" to newTabID %d \"%s\"\r\n",
    gui_state.oldTabID,    gui_state.gui_on_tab[gui_state.oldTabID].GUIname.c_str(),
    gui_state.activeTabID, gui_state.gui_on_tab[gui_state.activeTabID].GUIname.c_str());

  // 1. notify old guis and clear tabview and panel
  gui_memoryOptimizer_notifyAndClear(tabview, panel, img1, img2, &gui_state);

  // only optional: delete and create the whole screen. Not necessary.
  // Only used for a test. init_gui_status_bar() would need to be called again at a suitable place, because the status bar would also be deleted.
  // lv_obj_t* oldscr = lv_scr_act();
  // // create new screen
  // lv_obj_t* newscr = lv_obj_create(NULL);
  // // load this new screen
  // lv_scr_load(newscr);
  // lv_obj_del(oldscr);

  // 2. set gui_list_indices and the tab to be activated
  setGUIlistIndicesToBeShown_afterSlide(&gui_state);

  // 3. create content
  gui_memoryOptimizer_doContentCreation(tabview, panel, img1, img2, &gui_state);

}

// 3. after gui list has changed (called by handleScene()), when switching between main_gui_list and scene specific list. Will show first GUi in list
void gui_memoryOptimizer_afterGUIlistChanged(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2, GUIlists newGUIlist) {

  omote_log_d("--- Will change to new gui_list\r\n");

  if (gui_state.last_active_gui_list != newGUIlist) {
    // we are changing the gui_list, so save the last_active_gui_list_index
    gui_memoryOptimizer_setLastActiveGUIlistIndex(gui_state.gui_on_tab[gui_state.activeTabID].gui_list_index);
  }
  
  // 1. notify old guis and clear tabview and panel
  gui_memoryOptimizer_notifyAndClear(tabview, panel, img1, img2, &gui_state);

  // 2. set gui_list_indices and the tab to be activated
  gui_memoryOptimizer_setActiveGUIlist(newGUIlist);
  setGUIlistIndicesToBeShown_forFirstGUIinGUIlist(&gui_state);

  // 3. create content
  gui_memoryOptimizer_doContentCreation(tabview, panel, img1, img2, &gui_state);

}

// 4. navigate to a specific GUI in gui_list
void gui_memoryOptimizer_navigateToGUI(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2, GUIlists GUIlist, int gui_list_index) {

  omote_log_d("--- Will navigate to specific GUI\r\n");

  if ( !((gui_list_index >= 0) && (gui_list_index < get_gui_list_withFallback(GUIlist)->size()))) {
    omote_log_w("  cannot navigate to GUI because gui_list_index \"%d\" is out of range\r\n", gui_list_index);
    return;
  }  

  if (gui_state.last_active_gui_list != GUIlist) {
    // we are changing the gui_list, so save the last_active_gui_list_index
    gui_memoryOptimizer_setLastActiveGUIlistIndex(gui_state.gui_on_tab[gui_state.activeTabID].gui_list_index);
  }

  // 1. notify old guis and clear tabview and panel
  gui_memoryOptimizer_notifyAndClear(tabview, panel, img1, img2, &gui_state);

  // 2. set gui_list_indices and the tab to be activated
  gui_memoryOptimizer_setActiveGUIlist(GUIlist);
  setGUIlistIndicesToBeShown_forSpecificGUIlistIndex(gui_list_index, &gui_state);

  // 3. create content
  gui_memoryOptimizer_doContentCreation(tabview, panel, img1, img2, &gui_state);

}

// 5. navigate back to last gui in scene
void gui_memoryOptimizer_navigateToLastActiveGUIofPreviousGUIlist(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2) {

  #if (USE_SCENE_SPECIFIC_GUI_LIST == 0)
    omote_log_w("--- Cannot navigate to last GUI from scene, because scene specific gui lists are not enabled\r\n");
    return;
  #endif

  if (gui_memoryOptimizer_getLastActiveGUIlistIndex() == -1) {
    omote_log_w("--- Cannot navigate to last GUI from scene, because it is not set\r\n");
    return;
  } else {
    omote_log_d("--- Will navigate to last GUI from scene\r\n");
  }

  // navigate to the other gui_list
  if (gui_memoryOptimizer_getActiveGUIlist() == MAIN_GUI_LIST) {
    gui_memoryOptimizer_navigateToGUI(tabview, panel, img1, img2, SCENE_GUI_LIST, gui_memoryOptimizer_getLastActiveGUIlistIndex());
  } else {
    gui_memoryOptimizer_navigateToGUI(tabview, panel, img1, img2, MAIN_GUI_LIST, gui_memoryOptimizer_getLastActiveGUIlistIndex());
  }

}


void gui_memoryOptimizer_doContentCreation(lv_obj_t** tabview, lv_obj_t** panel, lv_obj_t** img1, lv_obj_t** img2, t_gui_state *gui_state) {
  // recreate the tabview
  lv_obj_t* newTabview = create_tabview();
  *tabview = newTabview;
  
  // Create the tabs. Use strategy "3 tabs at maximum" to keep memory usage low.
  // Set the tab we swiped to as active
  doTabCreation_strategyMax3(*tabview, gui_state);

  // Create the panel for the page indicator. Panel itself takes about 2136 bytes for three tabs.
  lv_obj_t* newPanel = create_panel();
  *panel = newPanel;
  *img1 = lv_img_create(lv_scr_act());
  *img2 = lv_img_create(lv_scr_act());
  fillPanelWithPageIndicator_strategyMax3(*panel, *img1, *img2, gui_state);

  // now, as the correct tab is active, register again the events for the tabview
  lv_obj_add_event_cb(*tabview, tabview_tab_changed_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(lv_tabview_get_content(*tabview), tabview_content_is_scrolling_event_cb, LV_EVENT_SCROLL, NULL);
  // Initialize scroll position of the page indicator
  lv_event_send(lv_tabview_get_content(*tabview), LV_EVENT_SCROLL, NULL);

  // gui_memoryOptimizer_doContentCreation() is called as last step every time the 3 tabs are recreated.
  // Save here the last_active_gui_list. If the used list changes in a future navigation, save the last_active_gui_list_index
  // so that we can use SCENE_BACK_TO_PREVIOUS_GUI_LIST
  gui_state->last_active_gui_list = gui_memoryOptimizer_getActiveGUIlist();

  omote_log_d("------------ End of tab deletion and creation\r\n");

}
