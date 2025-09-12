#include <lvgl.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/omote_log.h"
#include "devices/mediaPlayer/device_appleTV/gui_appleTV.h"

#include "applicationInternal/commandHandler.h"
#include "devices/mediaPlayer/device_appleTV/device_appleTV.h"

// LVGL declarations
LV_IMG_DECLARE(appleTvIcon);
LV_IMG_DECLARE(appleDisplayIcon);
LV_IMG_DECLARE(appleBackIcon);

// Apple Key Event handler
static void appleKey_event_cb(lv_event_t* e) {
  // Send IR command based on the event user data  
  int user_data = *((int*)(&(e->user_data)));
  omote_log_v("appleKey_event_cb: Event Id: '%d'.\r\n", user_data);

  if (user_data == 2)
  {
    executeCommand(APPLETV_HOME);
  }
  else if (user_data == 1)
  {
    executeCommand(APPLETV_MENU);
  }
}

// Global metadata display objects
static lv_obj_t* metadata_container = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* artist_label = nullptr;
static lv_obj_t* state_label = nullptr;

void create_tab_content_appleTV(lv_obj_t* tab) {
  // Add content to the Apple TV tab
  lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE); // Disable scrollbars on the tab
  
  // Add a nice apple tv logo
  lv_obj_t* appleImg = lv_img_create(tab);
  lv_img_set_src(appleImg, &appleTvIcon);
  lv_obj_align(appleImg, LV_ALIGN_CENTER, 0, -80);
  
  // Create metadata display container
  metadata_container = lv_obj_create(tab);
  lv_obj_set_size(metadata_container, SCR_WIDTH - 20, 120); // 20px margin (10px on each side)
  lv_obj_align(metadata_container, LV_ALIGN_CENTER, 0, 20);
  lv_obj_clear_flag(metadata_container, LV_OBJ_FLAG_SCROLLABLE); // Disable scrollbars
  lv_obj_set_style_bg_color(metadata_container, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(metadata_container, LV_OPA_90, LV_PART_MAIN);
  lv_obj_set_style_border_width(metadata_container, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(metadata_container, lv_color_hex(0x404040), LV_PART_MAIN);
  lv_obj_set_style_radius(metadata_container, 8, LV_PART_MAIN);
  lv_obj_set_style_pad_all(metadata_container, 12, LV_PART_MAIN);
  
  // Title label
  title_label = lv_label_create(metadata_container);
  lv_label_set_text(title_label, "No media playing");
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_set_style_text_color(title_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(title_label, SCR_WIDTH - 44); // Container width minus padding (20 + 24)
  
  // Artist label
  artist_label = lv_label_create(metadata_container);
  lv_label_set_text(artist_label, "");
  lv_obj_set_style_text_font(artist_label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_color(artist_label, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
  lv_obj_align(artist_label, LV_ALIGN_TOP_LEFT, 0, 25);
  lv_label_set_long_mode(artist_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(artist_label, SCR_WIDTH - 44); // Container width minus padding (20 + 24)
  
  // State label
  state_label = lv_label_create(metadata_container);
  lv_label_set_text(state_label, "");
  lv_obj_set_style_text_font(state_label, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_text_color(state_label, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_align(state_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
}

void update_appleTV_metadata(const std::string& title, const std::string& artist, 
                             const std::string& album, const std::string& state) {
  // Update metadata display if the GUI is active
  if (title_label != nullptr && artist_label != nullptr && state_label != nullptr) {
    // Update title
    if (!title.empty()) {
      lv_label_set_text(title_label, title.c_str());
    } else {
      lv_label_set_text(title_label, "No media playing");
    }
    
    // Update artist/album
    std::string artist_text = "";
    if (!artist.empty()) {
      artist_text = artist;
      if (!album.empty()) {
        artist_text += " • " + album;
      }
    }
    lv_label_set_text(artist_label, artist_text.c_str());
    
    // Update state with icon
    std::string state_text = "";
    if (state == "playing") {
      state_text = LV_SYMBOL_PLAY " Playing";
    } else if (state == "paused") {
      state_text = LV_SYMBOL_PAUSE " Paused";
    } else if (state == "stopped") {
      state_text = LV_SYMBOL_STOP " Stopped";
    } else if (!state.empty()) {
      state_text = state;
    }
    lv_label_set_text(state_label, state_text.c_str());
  }
}

void notify_tab_before_delete_appleTV(void) {
  // remember to set all pointers to lvgl objects to NULL if they might be accessed from outside.
  // They must check if object is NULL and must not use it if so
  metadata_container = nullptr;
  title_label = nullptr;
  artist_label = nullptr;
  state_label = nullptr;
}

void register_gui_appleTV(void){
  register_gui(std::string(tabName_appleTV), & create_tab_content_appleTV, & notify_tab_before_delete_appleTV);
}
