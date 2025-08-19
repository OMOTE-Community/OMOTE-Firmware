#include <mutex>
#include <lvgl.h>
#include "guis/gui_BLEpairing.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/memoryUsage.h"
#include "applicationInternal/gui/guiMemoryOptimizer.h"
// for changing to scene Selection gui
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"
#include "scenes/scene__default.h"

lv_color_t color_primary = lv_color_hex(0x303030); // gray
lv_obj_t* MemoryUsageLabel = NULL;
lv_obj_t* WifiLabel = NULL;
lv_obj_t* BluetoothLabel = NULL;
lv_obj_t* BattPercentageLabel = NULL;
lv_obj_t* BattIconLabel = NULL;
lv_obj_t* SceneLabel = NULL;

lv_obj_t* tabview = NULL;
// page indicator
lv_obj_t* panel = NULL;
lv_obj_t* img1 = NULL;
lv_obj_t* img2 = NULL;

static bool global_styles_already_initialized = false;
lv_style_t panel_style;
#ifdef drawRedBorderAroundMainWidgets
lv_style_t style_red_border;
#endif

// to add text threadsafe
std::mutex mutex_guiBase;
bool newWifiLabelStatusAvailable = false;
bool newWiFiLabelStatus;
void flushWiFiConnectedStatus();

void guis_doTabCreationOnStartup();
void guis_doTabCreationAfterSliding(int newTabID);

// Helper Functions -----------------------------------------------------------------------------------------------------------------------

// callback when pageIndicator prev or next was clicked
void pageIndicator_navigate_event_cb(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  
  int user_data = (intptr_t)(target->user_data);
  if (user_data == 0) {
    executeCommand(GUI_PREV);
  } else if (user_data == 1) {
    executeCommand(GUI_NEXT);
  }
}

// callback when sceneLabel or pageIndicator was clicked
void sceneLabel_or_pageIndicator_event_cb(lv_event_t* e) {
  omote_log_d("- Scene selection: sceneLabel or pageIndicator clicked received for navigating to scene selection page\r\n");
  executeCommand(SCENE_SELECTION);
}

// callback for swipe down event to navigate to the scene selection page
void screen_gesture_event_cb(lv_event_t* e) {
  lv_obj_t* screen = lv_event_get_current_target(e);
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
  if (dir == LV_DIR_BOTTOM) {
    omote_log_d("- Scene selection: swipe down received for navigating to scene selection page\r\n");
    executeCommand(SCENE_SELECTION);
  }    
}

// Set the page indicator (panel) scroll position relative to the tabview content scroll position
// this is a callback if the CONTENT of the tabview is scrolled (LV_EVENT_SCROLL)
void tabview_content_is_scrolling_event_cb(lv_event_t* e){
  if (panel == NULL) { return;}

  float bias = float(150.0 + 8.0) / SCR_WIDTH;
  //           screenwidth    indicator    ??    2 small hidden buttons  ??
  int offset = SCR_WIDTH / 2  - 150 / 2    - 8 - 2*50 / 2                -4;
  // get the object to which the event is sent
  lv_obj_t* tabviewContent = lv_event_get_target(e);
  
  // Get the x coordinate of object and scroll panel accordingly
  int16_t tabviewX = lv_obj_get_scroll_x(tabviewContent);
  // the last events we receive are 0, 238 and 476. But should be 0, 240 and 480. Otherwise page indicator jumps a litte bit after recreation of tabs.
  if ((tabviewX >= 237) && (tabviewX <= 240)) {tabviewX = 240;}
  if ((tabviewX >= 475) && (tabviewX <= 480)) {tabviewX = 480;}
  // we need 158 more (the size of one page indicator), because we always have one more page indicator at the beginning and at the end (so normally 5 when having 3 tabs)
  int16_t panelX = tabviewX * bias - offset + 158;
  omote_log_v("scroll %d to %d\r\n", tabviewX, panelX);
  lv_obj_scroll_to_x(panel, panelX, LV_ANIM_OFF);
  // lv_obj_scroll_to_x(panel, lv_obj_get_scroll_x(tabviewContent) * bias - offset, LV_ANIM_OFF);
}

// -----------------------
static bool waitBeforeActionAfterSlidingAnimationEnded = false;
static unsigned long waitBeforeActionAfterSlidingAnimationEnded_timerStart;
static int newTabID_forLateTabCreation;
// This is the callback when the animation of the tab sliding ended
static void tabview_animation_ready_cb(lv_anim_t* a) {
  // Unfortunately, the animation has not completely ended here. We cannot do the recreation of the tabs here.
  // We have to wait some more milliseconds or at least one cycle of lv_timer_handler();
  // calling lv_timer_handler(); here does not help
  // lv_timer_handler();
  // guis_doTabCreationAfterSliding(newTabID_forLateTabCreation);

  waitBeforeActionAfterSlidingAnimationEnded = true;
  waitBeforeActionAfterSlidingAnimationEnded_timerStart = millis();
}

// Update gui when a new tab is selected
// this is a callback if the tabview is changed (LV_EVENT_VALUE_CHANGED)
// Sent when a new tab is selected by sliding or clicking the tab button. lv_tabview_get_tab_act(tabview) returns the zero based index of the current tab.
void tabview_tab_changed_event_cb(lv_event_t* e) {
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
    
    int newTabID = lv_tabview_get_tab_act((lv_obj_t*)lv_event_get_target(e));

    // Wait until the animation ended, then call "guis_doTabCreationAfterSliding(newTabID);"
    // https://forum.lvgl.io/t/delete-a-tab-after-the-tabview-scroll-animation-is-complete/3155/4
    lv_obj_t* myTabview = lv_event_get_target(e);
    lv_obj_t* tabContainer = lv_tabview_get_content(myTabview);
    lv_anim_t* anim = lv_anim_get(tabContainer, NULL);
    if(anim) {
      // Swipe is not yet complete. User released the touch screen, an animation will bring it to the end.
      // That's the normal (and only?) case for the tft touchscreen
      newTabID_forLateTabCreation = newTabID;
      lv_anim_set_ready_cb(anim, tabview_animation_ready_cb);
    } else {
      // Swipe is complete, no additional animation is needed. Most likely only possible in simulator
      omote_log_d("Change of tab detected, without animation at the end. Will directly do my job after sliding.\r\n");
      guis_doTabCreationAfterSliding(newTabID);
    }
  }
}

void setMainWidgetsHeightAndPosition();
void init_gui_status_bar();
void init_gui_memoryUsage_bar();
void init_gui(void) {

  // Setup LVGL ---------------------------------------------------------------------------------------------
  lv_init();

  init_lvgl_hardware();

  // -- draw red border around the main widgets -------------------------------------------------------------
  if (!global_styles_already_initialized) {
    // Style the panel background. Will be initialized only once and reused in fillPanelWithPageIndicator_strategyMax3
    lv_style_init(&panel_style);
    lv_style_set_pad_all(&panel_style, 3);
    lv_style_set_border_width(&panel_style, 0);
    lv_style_set_bg_opa(&panel_style, LV_OPA_TRANSP);
    #ifdef drawRedBorderAroundMainWidgets
    lv_style_init(&style_red_border);
    lv_style_set_border_side(&style_red_border, LV_BORDER_SIDE_FULL);
    lv_style_set_border_color(&style_red_border, lv_color_hex(0xff0000));
    lv_style_set_border_width(&style_red_border, 1);
    global_styles_already_initialized = true;
    #endif
  }
  // Set the background color -------------------------------------------------------------------------------
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
  // set default height and position of main widgets
  setMainWidgetsHeightAndPosition();
  // On startup, set current GUIname and GUIlist according to last state before going to sleep
  guis_doTabCreationOnStartup();
  // memoryUsage bar
  init_gui_memoryUsage_bar();
  // status bar
  init_gui_status_bar();

  // register callback for swipe down event to navigate to the scene selection page
  lv_obj_add_event_cb(lv_scr_act(), screen_gesture_event_cb, LV_EVENT_GESTURE, NULL);

}

int panelHeight;
int memoryUsageBarTop;
int memoryUsageBarHeight;
int statusbarTop;
int statusbarHeight;
int tabviewTop;
int tabviewHeight;
int labelsPositionTop;

void setMainWidgetsHeightAndPosition() {
  panelHeight          = 30;
  memoryUsageBarTop    = 0;
  memoryUsageBarHeight = getShowMemoryUsage() ? 14 : 0;
  statusbarTop         = memoryUsageBarTop + memoryUsageBarHeight;
  statusbarHeight      = 20;
  tabviewTop           = statusbarTop + statusbarHeight;
  tabviewHeight        = SCR_HEIGHT - memoryUsageBarHeight - statusbarHeight - panelHeight;
  labelsPositionTop    = -2;
}

lv_obj_t* memoryUsageBar;
lv_obj_t* statusbar;

void showMemoryUsageBar(bool showBar) {
  setMainWidgetsHeightAndPosition();

  lv_obj_set_size(memoryUsageBar, SCR_WIDTH, memoryUsageBarHeight);
  lv_obj_align(memoryUsageBar, LV_ALIGN_TOP_MID, 0, memoryUsageBarTop);
  lv_obj_set_size(statusbar, SCR_WIDTH, statusbarHeight);
  lv_obj_align(statusbar, LV_ALIGN_TOP_MID, 0, statusbarTop);
  lv_obj_set_size(tabview, SCR_WIDTH, tabviewHeight);
  lv_obj_align(tabview, LV_ALIGN_TOP_MID, 0, tabviewTop);

  return;

  if (showBar) {
    // lv_obj_clear_flag(memoryUsageBar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(memoryUsageBar, SCR_WIDTH, memoryUsageBarHeight);
    lv_obj_align(memoryUsageBar, LV_ALIGN_TOP_MID, 0, memoryUsageBarTop);
    lv_obj_set_size(statusbar, SCR_WIDTH, statusbarHeight);
    lv_obj_align(statusbar, LV_ALIGN_TOP_MID, 0, statusbarTop);
    lv_obj_set_size(tabview, SCR_WIDTH, tabviewHeight);
    lv_obj_align(tabview, LV_ALIGN_TOP_MID, 0, tabviewTop);
  } else {
    // lv_obj_add_flag(memoryUsageBar, LV_OBJ_FLAG_HIDDEN);

  }
}

void init_gui_memoryUsage_bar() {
  // Create a memory status text bar at the top -------------------------------------------------------------
  memoryUsageBar = lv_btn_create(lv_scr_act());
  lv_obj_set_size(memoryUsageBar, SCR_WIDTH, memoryUsageBarHeight);
  lv_obj_set_style_shadow_width(memoryUsageBar, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(memoryUsageBar, lv_color_black(), LV_PART_MAIN);
  #ifdef drawRedBorderAroundMainWidgets
  lv_obj_add_style(memoryUsageBar, &style_red_border, LV_PART_MAIN);
  #endif
  lv_obj_set_style_radius(memoryUsageBar, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(memoryUsageBar, 0, LV_PART_MAIN);
  lv_obj_align(memoryUsageBar, LV_ALIGN_TOP_MID, 0, memoryUsageBarTop);

  MemoryUsageLabel = lv_label_create(memoryUsageBar);
  lv_label_set_text(MemoryUsageLabel, "");
  lv_obj_align(MemoryUsageLabel, LV_ALIGN_TOP_LEFT, 0 +2, labelsPositionTop +2);
  lv_obj_set_style_text_font(MemoryUsageLabel, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_label_set_recolor(MemoryUsageLabel, true);
}

void init_gui_status_bar() {
  // Create a status bar at the top -------------------------------------------------------------------------
  statusbar = lv_btn_create(lv_scr_act());
  lv_obj_clear_flag(statusbar, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(statusbar, SCR_WIDTH, statusbarHeight);
  lv_obj_set_style_shadow_width(statusbar, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(statusbar, lv_color_black(), LV_PART_MAIN);
  #ifdef drawRedBorderAroundMainWidgets
  lv_obj_add_style(statusbar, &style_red_border, LV_PART_MAIN);
  #endif
  lv_obj_set_style_radius(statusbar, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(statusbar, 0, LV_PART_MAIN);
  lv_obj_align(statusbar, LV_ALIGN_TOP_MID, 0, statusbarTop);

  int labelsPositionTopStatusbar = labelsPositionTop + 3;
  // WiFi -------------------------------------------------------------------------
  WifiLabel = lv_label_create(statusbar);
  lv_label_set_text(WifiLabel, "");
  lv_obj_align(WifiLabel, LV_ALIGN_TOP_LEFT, 0, labelsPositionTopStatusbar);
  lv_obj_set_style_text_font(WifiLabel, &lv_font_montserrat_12, LV_PART_MAIN);
  // Bluetooth --------------------------------------------------------------------
  BluetoothLabel = lv_label_create(statusbar);
  lv_label_set_text(BluetoothLabel, "");
  lv_obj_align(BluetoothLabel, LV_ALIGN_TOP_LEFT, 20, labelsPositionTopStatusbar);
  lv_obj_set_style_text_font(BluetoothLabel, &lv_font_montserrat_12, LV_PART_MAIN);
  // Scene ------------------------------------------------------------------------
  SceneLabel = lv_label_create(statusbar);
  lv_label_set_text(SceneLabel, "");
  lv_obj_align(SceneLabel, LV_ALIGN_TOP_MID, 0, labelsPositionTopStatusbar);
  lv_obj_set_style_text_font(SceneLabel, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_add_flag(SceneLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_user_data(SceneLabel,(void *)(intptr_t)0);
  lv_obj_add_event_cb(SceneLabel, sceneLabel_or_pageIndicator_event_cb, LV_EVENT_CLICKED, NULL);

  // Battery ----------------------------------------------------------------------
  BattPercentageLabel = lv_label_create(statusbar);
  lv_label_set_text(BattPercentageLabel, "");
  lv_obj_align(BattPercentageLabel, LV_ALIGN_TOP_RIGHT, -28, labelsPositionTopStatusbar);
  lv_obj_set_style_text_font(BattPercentageLabel, &lv_font_montserrat_12, LV_PART_MAIN);
  BattIconLabel = lv_label_create(statusbar);
  lv_label_set_text(BattIconLabel, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_align(BattIconLabel, LV_ALIGN_TOP_RIGHT, 0, labelsPositionTopStatusbar -1);
  lv_obj_set_style_text_font(BattIconLabel, &lv_font_montserrat_16, LV_PART_MAIN);

}

static bool waitOneLoop = false;
void gui_loop(void) {
  // after the sliding animation ended, we have to wait one cycle of gui_loop() before we can do the recreation of the tabs
  if (waitBeforeActionAfterSlidingAnimationEnded) {
    waitOneLoop = true;
    waitBeforeActionAfterSlidingAnimationEnded = false;
  } else if (waitOneLoop) {
    waitOneLoop = false;
    guis_doTabCreationAfterSliding(newTabID_forLateTabCreation);
  };
  // // as alternative, we could wait some milliseconds. But one cycle of gui_loop() works well.
  // if (waitBeforeActionAfterSlidingAnimationEnded) {
  //   if (millis() - waitBeforeActionAfterSlidingAnimationEnded_timerStart >= 5) {
  //     guis_doTabCreationAfterSliding(newTabID_forLateTabCreation);
  //     waitBeforeActionAfterSlidingAnimationEnded = false;
  //   }
  // }

  lv_timer_handler();

  // flush texts that might have been added from callbacks from other threads
  // has to be done in a thread safe way in the main thread
  #if (ENABLE_WIFI_AND_MQTT ==1)
  flushWiFiConnectedStatus();
  #endif
  #if (ENABLE_KEYBOARD_BLE == 1)
  flushBLEMessages();
  #endif
}

// ------------------------------------------------------------------------------------------------------------
// There are several reasons why the tabs could get recreated. All are going through these functions in "guiBase.cpp", which are calling functions in "guiMemoryOptimizer.cpp"
// 1. tab creation on startup (called by init_gui())
void guis_doTabCreationOnStartup() {
  gui_memoryOptimizer_onStartup(&tabview, &panel, &img1, &img2);
  doLogMemoryUsage();
}
// 2. tab creation after sliding (called by tabview_tab_changed_event_cb())
void guis_doTabCreationAfterSliding(int newTabID) {
  gui_memoryOptimizer_afterSliding(&tabview, &panel, &img1, &img2, newTabID);
  doLogMemoryUsage();
}
// 3. after gui list has changed (called by handleScene()), when switching between main_gui_list and scene specific list. Will show first GUi in list
void guis_doTabCreationAfterGUIlistChanged(GUIlists newGUIlist) {
  gui_memoryOptimizer_afterGUIlistChanged(&tabview, &panel, &img1, &img2, newGUIlist);
  doLogMemoryUsage();
}
// 4. navigate to a specific GUI in gui_list
void guis_doTabCreationForSpecificGUI(GUIlists GUIlist, int gui_list_index) {
  gui_memoryOptimizer_navigateToGUI(&tabview, &panel, &img1, &img2, GUIlist, gui_list_index);
  doLogMemoryUsage();
}
// 5. navigate back to last active gui of previous gui_list
void guis_doTabCreationForNavigateToLastActiveGUIofPreviousGUIlist() {
  gui_memoryOptimizer_navigateToLastActiveGUIofPreviousGUIlist(&tabview, &panel, &img1, &img2);
  doLogMemoryUsage();
}

// ------------------------------------------------------------------------------------------------------------

void setActiveTab(uint32_t index, lv_anim_enable_t anim_en, bool send_tab_changed_event) {
  // unsigned long startTime = millis();
  if (anim_en == LV_ANIM_ON) {
    lv_tabview_set_act(tabview, index, LV_ANIM_ON);
    // startTime = millis();
    // while (millis() - startTime < 1000) {
    //   lv_timer_handler();
    // }
  } else {
    lv_tabview_set_act(tabview, index, LV_ANIM_OFF);
    // lv_timer_handler();
    // log_memory();
  }

  if (send_tab_changed_event) {
    lv_event_send(tabview, LV_EVENT_VALUE_CHANGED, NULL);
  }
}

void flushWiFiConnectedStatus() {
  // this is to flush text which was added by another thread via "void showWiFiConnected(bool connected)"
  // the function "flushWiFiConnectedStatus" is called from the main thread
  std::lock_guard<std::mutex> lck(mutex_guiBase);
  if (newWifiLabelStatusAvailable) {
    if (newWiFiLabelStatus) {
      if (WifiLabel != NULL) {lv_label_set_text(WifiLabel, LV_SYMBOL_WIFI);}
    } else {
      if (WifiLabel != NULL) {lv_label_set_text(WifiLabel, "");}
    }
    newWifiLabelStatusAvailable = false;
  }
}

void showWiFiConnected(bool connected) {
  // this callback is called from another thread from mqtt_hal_esp32.cpp
  std::lock_guard<std::mutex> lck(mutex_guiBase);
  newWifiLabelStatusAvailable = true;
  newWiFiLabelStatus = connected;
}
