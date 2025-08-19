#include <lvgl.h>
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/omote_log.h"
#include "devices/misc/device_airconditioner/gui_airconditioner.h"
#include "devices/misc/device_airconditioner/device_airconditioner.h"
#include "scenes/scene__default.h"

uint16_t GUI_AIRCONDITIONER_ACTIVATE;

std::map<char, repeatModes> key_repeatModes_airconditioner = {};
std::map<char, uint16_t> key_commands_short_airconditioner = {};
std::map<char, uint16_t> key_commands_long_airconditioner = {};

lv_obj_t* onoffLabel;
lv_obj_t* tempLabel;
lv_obj_t* modeLabel;
lv_obj_t* speedLabel;
lv_obj_t* timerLabel;
lv_obj_t* CFLabel;

void update_onoff_label();
void update_temp_label();
void update_mode_label();
void update_speed_label();
void update_timer_label();
void update_CF_label();

static void airConditioner_event_cb(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  lv_obj_t* cont = lv_event_get_current_target(e);
  if (target == cont) return; // stop if container was clicked
  
  int user_data = (intptr_t)(target->user_data);

  switch (user_data) {
    case 0: {
      if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED)) {
        omote_log_i("Air conditioner: on\r\n");
        airConditionerPAC_N81.set_onoff(true);
      } else {
        omote_log_i("Air conditioner: off\r\n");
        airConditionerPAC_N81.set_onoff(false);
      }
      update_onoff_label();
      airConditionerPAC_N81.sendIRcommand();
      break;
    }
    case 1: {
      omote_log_i("Air conditioner: minus\r\n");
      airConditionerPAC_N81.set_temp_down();
      update_temp_label();
      airConditionerPAC_N81.sendIRcommand();
      break;
    }
    case 2: {
      omote_log_i("Air conditioner: plus\r\n");
      airConditionerPAC_N81.set_temp_up();
      update_temp_label(),
      airConditionerPAC_N81.sendIRcommand();
      break;
    }
    case 3: {
      omote_log_i("Air conditioner: mode\r\n");
      airConditionerPAC_N81.set_mode_next();
      update_mode_label();
      airConditionerPAC_N81.sendIRcommand();
      break;
      }
    case 4: {
      omote_log_i("Air conditioner: speed\r\n"); 
      airConditionerPAC_N81.set_fan_next();
      update_speed_label();
      airConditionerPAC_N81.sendIRcommand();
      break;
    }
    case 5: {
      if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED)) {
        omote_log_i("Air conditioner: timer on\r\n");
        airConditionerPAC_N81.set_timer(true);
      } else {
        omote_log_i("Air conditioner: timer off\r\n");
        airConditionerPAC_N81.set_timer(false);
      }
      update_timer_label();
      airConditionerPAC_N81.sendIRcommand();
      break;
    }
    case 6: {
      // This event does not help, because it is not fired when a value in the dropdown was selected. Use airConditioner_dropdown_cb()    
      omote_log_i("Air conditioner: timer dropdown opened or closed\r\n");
      break;
    }
    case 7: {
      omote_log_i("Air conditioner: C/F\r\n");
      airConditionerPAC_N81.set_CF_next();
      update_temp_label();
      update_CF_label();
      airConditionerPAC_N81.sendIRcommand();
      break;
      }
  }

}

static void airConditioner_dropdown_cb(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  uint16_t selected_index = lv_dropdown_get_selected(target);
      
  omote_log_i("Air conditioner: timer dropdown selected %d\r\n", selected_index);
  airConditionerPAC_N81.set_timer_value(selected_index);
  airConditionerPAC_N81.sendIRcommand();
}

void update_onoff_label() {
  lv_label_set_text(onoffLabel, airConditionerPAC_N81.get_onoff_str().c_str());
}
void update_temp_label() {
  lv_label_set_text(tempLabel, (airConditionerPAC_N81.get_temp_str() + "°" + airConditionerPAC_N81.get_CF_str()).c_str());
}
void update_mode_label() {
  lv_label_set_text(modeLabel, airConditionerPAC_N81.get_mode_str().c_str());
}
void update_speed_label() {
  lv_label_set_text(speedLabel, airConditionerPAC_N81.get_fan_str().c_str());
}
void update_timer_label() {
  lv_label_set_text(timerLabel, airConditionerPAC_N81.get_timer_str().c_str());
}
void update_CF_label() {
  lv_label_set_text(CFLabel, airConditionerPAC_N81.get_CF_str().c_str());
}

void create_label(lv_obj_t** label, lv_obj_t* tab, lv_coord_t x_ofs, lv_coord_t y_ofs) {
  *label = lv_label_create(tab);
  lv_obj_set_style_text_font(*label, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
}

void create_button(lv_obj_t* tab, lv_coord_t width, lv_coord_t height, lv_coord_t x_ofs, lv_coord_t y_ofs, std::string text, uint8_t user_data) {
  lv_obj_t* obj;
  lv_obj_t* buttonLabel;

  obj = lv_btn_create(tab);
  lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
  lv_obj_set_size(obj, width, height);
  lv_obj_set_style_bg_color(obj, color_primary, LV_PART_MAIN);
  lv_obj_set_style_radius(obj, 14, LV_PART_MAIN);
  lv_obj_set_style_shadow_color(obj, lv_color_hex(0x404040), LV_PART_MAIN);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);        // Clicking a button causes an event in its container
  lv_obj_set_user_data(obj,(void *)(intptr_t)user_data); // Add user data so we can identify which button caused the container event
  buttonLabel = lv_label_create(obj);
  lv_label_set_text(buttonLabel, text.c_str());
  lv_obj_set_style_text_font(buttonLabel, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_center(buttonLabel);
}

void create_switch(lv_obj_t* tab, lv_coord_t width, lv_coord_t height, lv_coord_t x_ofs, lv_coord_t y_ofs, uint8_t user_data) {
  lv_obj_t* obj = lv_switch_create(tab);
  lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
  lv_obj_set_size(obj, width, height);
  lv_obj_set_style_bg_color(obj, lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);        // Clicking a switch causes an event in its container
  lv_obj_set_user_data(obj,(void *)(intptr_t)user_data); // Add user data so we can identify which button caused the container event
}

void create_dropdownTimer(lv_obj_t* tab, lv_coord_t width, lv_coord_t height, lv_coord_t x_ofs, lv_coord_t y_ofs, uint8_t user_data) {
  lv_obj_t * dropdownTimer = lv_dropdown_create(tab);
  lv_dropdown_set_options(dropdownTimer, "0 h\n" "1 h\n" "2 h\n" "3 h\n" "4 h\n" "5 h\n" "6 h\n" "7 h\n" "8 h\n" "9 h\n" "10 h\n" "11 h\n" "12 h");
  lv_obj_align(dropdownTimer, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
  lv_obj_set_size(dropdownTimer, width, height);
  lv_obj_set_style_pad_top(dropdownTimer, 3, LV_PART_MAIN);
  lv_obj_add_event_cb(dropdownTimer, airConditioner_dropdown_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)user_data);
  // This event does not help, because it is not fired when a value in the dropdown was selected. Use airConditioner_dropdown_cb()    
  // lv_obj_add_flag(dropdownTimer, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a dropdown causes a event in its container
  // lv_obj_set_user_data(dropdownTimer,(void *)(intptr_t)user_data); // Add user data so we can identify which button caused the container event
}

void create_tab_content_airconditioner(lv_obj_t* tab) {

  lv_obj_t* menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Air Conditioner");
  lv_obj_set_style_text_font(menuLabel, &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_align(menuLabel, LV_ALIGN_TOP_MID, 0, 0);

  uint8_t buttons_width  = 60;
  uint8_t buttons_height = 25;
  uint8_t buttons_startx = 10;
  uint8_t buttons_starty = 30;
  uint8_t buttons_space  = 35;
  create_switch       (tab, buttons_width, buttons_height, buttons_startx,      buttons_starty                  ,           0);
  create_button       (tab, buttons_width, buttons_height, buttons_startx,      buttons_starty + 1*buttons_space, "-"     , 1);
  create_button       (tab, buttons_width, buttons_height, buttons_startx +140, buttons_starty + 1*buttons_space,  "+"    , 2);
  create_button       (tab, buttons_width, buttons_height, buttons_startx,      buttons_starty + 2*buttons_space, "mode"  , 3);
  create_button       (tab, buttons_width, buttons_height, buttons_startx,      buttons_starty + 3*buttons_space, "speed" , 4);
  create_switch       (tab, buttons_width, buttons_height, buttons_startx,      buttons_starty + 4*buttons_space,           5);
  create_dropdownTimer(tab, buttons_width, buttons_height, buttons_startx +140, buttons_starty + 4*buttons_space,           6);
  create_button       (tab, buttons_width, buttons_height, buttons_startx,      buttons_starty + 5*buttons_space, "C/F"   , 7);

  uint8_t labels_startx = 83;
  uint8_t labels_starty = 35;
  uint8_t labels_space  = 35;
  create_label(&onoffLabel, tab, labels_startx, labels_starty                 ); update_onoff_label();
  create_label(&tempLabel,  tab, labels_startx, labels_starty + 1*labels_space); update_temp_label();
  create_label(&CFLabel,    tab, labels_startx, labels_starty + 5*labels_space); update_CF_label();
  create_label(&modeLabel,  tab, labels_startx, labels_starty + 2*labels_space); update_mode_label();
  create_label(&speedLabel, tab, labels_startx, labels_starty + 3*labels_space); update_speed_label();
  create_label(&timerLabel, tab, labels_startx, labels_starty + 4*labels_space); update_timer_label();

  // Create a shared event for all button inside container
  lv_obj_add_event_cb(tab, airConditioner_event_cb, LV_EVENT_CLICKED, NULL);

}

void notify_tab_before_delete_airconditioner(void) {
  // remember to set all pointers to lvgl objects to NULL if they might be accessed from outside.
  // They must check if object is NULL and must not use it if so

}

void gui_setKeys_airconditioner() {
  key_commands_short_airconditioner = {
    {KEY_STOP, SCENE_SELECTION},
  };
}

void register_gui_airconditioner(void){

  register_gui(
    std::string(tabName_airconditioner),
    & create_tab_content_airconditioner,
    & notify_tab_before_delete_airconditioner,
    & gui_setKeys_airconditioner,
    & key_repeatModes_airconditioner,
    & key_commands_short_airconditioner,
    & key_commands_long_airconditioner
    );

  register_command(&GUI_AIRCONDITIONER_ACTIVATE, makeCommandData(GUI, {std::to_string(MAIN_GUI_LIST), std::string(tabName_airconditioner)}));
}
