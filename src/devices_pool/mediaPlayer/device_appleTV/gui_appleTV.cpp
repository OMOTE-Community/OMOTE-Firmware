#include <lvgl.h>
#include <map>
#include <string>
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
static lv_obj_t* progress_bar = nullptr;

void create_tab_content_appleTV(lv_obj_t* tab) {
  lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_t* appleImg = lv_img_create(tab);
  lv_img_set_src(appleImg, &appleTvIcon);
  lv_obj_align(appleImg, LV_ALIGN_CENTER, 0, -100);
  
  metadata_container = lv_obj_create(tab);
  lv_obj_set_size(metadata_container, SCR_WIDTH - 32, 140);
  lv_obj_align(metadata_container, LV_ALIGN_CENTER, 0, 25);
  lv_obj_clear_flag(metadata_container, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_set_style_bg_color(metadata_container, lv_color_hex(0x1C1C1E), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(metadata_container, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_border_width(metadata_container, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(metadata_container, 12, LV_PART_MAIN);
  lv_obj_set_style_pad_all(metadata_container, 16, LV_PART_MAIN);
  
  title_label = lv_label_create(metadata_container);
  lv_label_set_text(title_label, "No media playing");
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_label_set_long_mode(title_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(title_label, SCR_WIDTH - 64);
  
  artist_label = lv_label_create(metadata_container);
  lv_label_set_text(artist_label, "");
  lv_obj_set_style_text_font(artist_label, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_set_style_text_color(artist_label, lv_color_hex(0xEBEBF5), LV_PART_MAIN);
  lv_obj_align(artist_label, LV_ALIGN_TOP_LEFT, 0, 28);
  lv_label_set_long_mode(artist_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(artist_label, SCR_WIDTH - 64);
  
  state_label = lv_label_create(metadata_container);
  lv_label_set_text(state_label, "");
  lv_obj_set_style_text_font(state_label, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_text_color(state_label, lv_color_hex(0x8E8E93), LV_PART_MAIN);
  lv_obj_align(state_label, LV_ALIGN_BOTTOM_LEFT, 0, -24);
  
  progress_bar = lv_bar_create(metadata_container);
  lv_obj_set_size(progress_bar, SCR_WIDTH - 64, 3);
  lv_obj_align(progress_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
  
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
  lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0x007AFF), LV_PART_INDICATOR);
  lv_obj_set_style_radius(progress_bar, 2, LV_PART_MAIN);
  lv_obj_set_style_radius(progress_bar, 2, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(progress_bar, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(progress_bar, LV_OPA_100, LV_PART_INDICATOR);
  
  lv_bar_set_range(progress_bar, 0, 100);
  lv_bar_set_value(progress_bar, 0, LV_ANIM_OFF);
  lv_obj_add_flag(progress_bar, LV_OBJ_FLAG_HIDDEN);
}

static std::string replace_bullet_chars(const std::string& input) {
  std::string result = input;
  size_t pos = 0;
  while ((pos = result.find("·", pos)) != std::string::npos) {
    result.replace(pos, 3, "| ");
    pos += 2;
  }
  return result;
}

static std::string add_marquee_gap(const std::string& input) {
  if (input.empty()) return input;
  return input + "      ";
}

struct MetadataUpdateData {
  std::string title;
  std::string artist;
  std::string album;
  std::string state;
  int duration;
  int position;
};

static void metadata_update_task_cb(lv_timer_t* timer) {
  MetadataUpdateData* data = static_cast<MetadataUpdateData*>(timer->user_data);
  
  if (title_label != nullptr && artist_label != nullptr && state_label != nullptr) {
    std::string title_text, artist_text, state_text;
    
    if (!data->title.empty()) {
      std::string clean_title = replace_bullet_chars(data->title);
      title_text = add_marquee_gap(clean_title);
    } else {
      title_text = "No media playing";
    }
    
    artist_text = data->artist.empty() ? "" : replace_bullet_chars(data->artist);
    if (!artist_text.empty() && !data->album.empty()) {
      artist_text += " • " + replace_bullet_chars(data->album);
    }
    if (!artist_text.empty()) {
      artist_text = add_marquee_gap(artist_text);
    }
    
    static const std::map<std::string, std::string> state_symbols = {
      {"playing", LV_SYMBOL_PLAY " Playing"},
      {"paused", LV_SYMBOL_PAUSE " Paused"},
      {"stopped", LV_SYMBOL_STOP " Stopped"}
    };
    
    auto it = state_symbols.find(data->state);
    if (it != state_symbols.end()) {
      state_text = it->second;
    } else if (!data->state.empty()) {
      state_text = data->state;
    } else {
      state_text = "";
    }
    
    lv_label_set_text(title_label, title_text.c_str());
    lv_label_set_text(artist_label, artist_text.c_str());
    lv_label_set_text(state_label, state_text.c_str());
    
    if (progress_bar != nullptr && data->duration > 0) {
      lv_obj_clear_flag(progress_bar, LV_OBJ_FLAG_HIDDEN);
      
      int progress = (data->position * 100) / data->duration;
      if (progress < 0) progress = 0;
      if (progress > 100) progress = 100;
      
      lv_bar_set_value(progress_bar, progress, LV_ANIM_OFF);
    } else if (progress_bar != nullptr) {
      lv_obj_add_flag(progress_bar, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (metadata_container != nullptr) {
      lv_obj_invalidate(metadata_container);
    }
  }
  
  delete data;
  lv_timer_del(timer);
}

void update_appleTV_metadata(const std::string& title, const std::string& artist, 
                             const std::string& album, const std::string& state,
                             int duration, int position) {
  omote_log_d("Apple TV metadata update: title='%s', artist='%s', album='%s', state='%s'\r\n", 
             title.c_str(), artist.c_str(), album.c_str(), state.c_str());
  
  MetadataUpdateData* data = new MetadataUpdateData();
  data->title = title;
  data->artist = artist;
  data->album = album;
  data->state = state;
  data->duration = duration;
  data->position = position;
  
  lv_timer_t* timer = lv_timer_create(metadata_update_task_cb, 1, data);
  lv_timer_set_repeat_count(timer, 1);
}

void notify_tab_before_delete_appleTV(void) {
  metadata_container = nullptr;
  title_label = nullptr;
  artist_label = nullptr;
  state_label = nullptr;
  progress_bar = nullptr;
}

void register_gui_appleTV(void){
  register_gui(std::string(tabName_appleTV), & create_tab_content_appleTV, & notify_tab_before_delete_appleTV);
}
