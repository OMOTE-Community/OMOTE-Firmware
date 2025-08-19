#include <lvgl.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/memoryUsage.h"
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/omote_log.h"
#include "guis/gui_settings.h"

// LVGL declarations
LV_IMG_DECLARE(high_brightness);
LV_IMG_DECLARE(low_brightness);

lv_obj_t* objBattSettingsVoltage;
lv_obj_t* objBattSettingsPercentage;
//lv_obj_t* objBattSettingsIscharging;

// Display Backlight Slider Event handler
static void bl_slider_event_cb(lv_event_t* e){
  lv_obj_t* slider = lv_event_get_target(e);
  int32_t slider_value = lv_slider_get_value(slider);
  if (slider_value < 60)  {slider_value = 60;}
  if (slider_value > 255) {slider_value = 255;}
  set_backlightBrightness(slider_value);
}

#if(OMOTE_HARDWARE_REV >= 5)
// Keyboard Backlight Slider Event handler
static void kb_slider_event_cb(lv_event_t* e){
  lv_obj_t* slider = lv_event_get_target(e);
  int32_t slider_value = lv_slider_get_value(slider);
  if (slider_value < 0)  {slider_value = 0;}
  if (slider_value > 255) {slider_value = 255;}
  set_keyboardBrightness(slider_value);
}
#endif

// Wakeup by IMU Switch Event handler
static void WakeEnableSetting_event_cb(lv_event_t* e){
  set_wakeupByIMUEnabled(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

// timout event handler
static void timout_event_cb(lv_event_t* e){
  lv_obj_t* drop = lv_event_get_target(e);
  uint16_t selected = lv_dropdown_get_selected(drop);
  switch (selected) {
    case 0: {set_sleepTimeout(  10000); break;}
    case 1: {set_sleepTimeout(  20000); break;}
    case 2: {set_sleepTimeout(  40000); break;}
    case 3: {set_sleepTimeout(  60000); break;}
    case 4: {set_sleepTimeout( 180000); break;}
    case 5: {set_sleepTimeout( 600000); break;}
    case 6: {set_sleepTimeout(3600000); break;}
  }
  omote_log_v("New timeout: %lu ms\r\n", get_sleepTimeout());
  setLastActivityTimestamp();
  // save preferences now, otherwise if you set a very big timeout and upload your firmware again, it never got saved
  save_preferences();
}

// motion threshold event handler
static void motion_threshold_event_cb(lv_event_t* e){
  lv_obj_t* drop = lv_event_get_target(e);
  uint16_t selected = lv_dropdown_get_selected(drop);
  switch (selected) {
    case 0: {set_motionThreshold(120); break;}
    case 1: {set_motionThreshold( 80); break;}
    case 2: {set_motionThreshold( 50); break;}
  }
  omote_log_v("New motion threshold: %lu ms\r\n", get_motionThreshold());
  save_preferences();
}

// show memory usage event handler
static void showMemoryUsage_event_cb(lv_event_t* e) {
  setShowMemoryUsage(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

void create_tab_content_settings(lv_obj_t* tab) {

  // Add content to the settings tab
  // With a flex layout, setting groups/boxes will position themselves automatically
  lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_ACTIVE);

  // Add a label, then a box for the display settings -----------------------------------------
  lv_obj_t* menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Display");

  lv_obj_t* menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 141);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  lv_obj_t* brightnessIcon = lv_img_create(menuBox);
  lv_img_set_src(brightnessIcon, &low_brightness);
  lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(brightnessIcon, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_t* slider = lv_slider_create(menuBox);
  lv_slider_set_range(slider, 60, 255);
  lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(color_primary, 50), LV_PART_MAIN);
  lv_slider_set_value(slider, get_backlightBrightness(), LV_ANIM_OFF);
  lv_obj_set_size(slider, lv_pct(66), 10);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 3);
  brightnessIcon = lv_img_create(menuBox);
  lv_img_set_src(brightnessIcon, &high_brightness);
  lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(brightnessIcon, LV_ALIGN_TOP_RIGHT, 0, -1);
  lv_obj_add_event_cb(slider, bl_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  
  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Lift to Wake");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 32);
  lv_obj_t* wakeToggle = lv_switch_create(menuBox);
  lv_obj_set_size(wakeToggle, 40, 22);
  lv_obj_align(wakeToggle, LV_ALIGN_TOP_RIGHT, 0, 29);
  lv_obj_set_style_bg_color(wakeToggle, lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_event_cb(wakeToggle, WakeEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  if (get_wakeupByIMUEnabled()) {
    lv_obj_add_state(wakeToggle, LV_STATE_CHECKED);
  } else {
    // lv_obj_clear_state(wakeToggle, LV_STATE_CHECKED);
  }

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Sensitivity");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 64);
  lv_obj_t* drop = lv_dropdown_create(menuBox);
  lv_dropdown_set_options(drop, "low\n"
                                "mid\n"
                                "high");
  // if you add more options here, do the same in timout_event_cb()
  switch (get_motionThreshold()) {
    case 120: {lv_dropdown_set_selected(drop, 0); break;}
    case  80: {lv_dropdown_set_selected(drop, 1); break;}
    case  50: {lv_dropdown_set_selected(drop, 2); break;}
  }
  lv_dropdown_set_selected_highlight(drop, true);
  lv_obj_align(drop, LV_ALIGN_TOP_RIGHT, 0, 61);
  lv_obj_set_size(drop, 70, 22);
  //lv_obj_set_style_text_font(drop, &lv_font_montserrat_12, LV_PART_MAIN);
  //lv_obj_set_style_text_font(lv_dropdown_get_list(drop), &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_pad_top(drop, 1, LV_PART_MAIN);
  lv_obj_set_style_bg_color(drop, color_primary, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lv_dropdown_get_list(drop), color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(lv_dropdown_get_list(drop), 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(lv_dropdown_get_list(drop), lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_event_cb(drop, motion_threshold_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Timeout");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 96);
  drop = lv_dropdown_create(menuBox);
  lv_dropdown_set_options(drop, "10s\n"
                                "20s\n"
                                "40s\n"
                                "1m\n"
                                "3m\n"
                                "10m\n"
                                "1h"); // 1h for debug purposes, if you don't want the device to go to slepp
  // if you add more options here, do the same in timout_event_cb()
  switch (get_sleepTimeout()) {
    case   10000: {lv_dropdown_set_selected(drop, 0); break;}
    case   20000: {lv_dropdown_set_selected(drop, 1); break;}
    case   40000: {lv_dropdown_set_selected(drop, 2); break;}
    case   60000: {lv_dropdown_set_selected(drop, 3); break;}
    case  180000: {lv_dropdown_set_selected(drop, 4); break;}
    case  600000: {lv_dropdown_set_selected(drop, 5); break;}
    case 3600000: {lv_dropdown_set_selected(drop, 6); break;}
  }
  lv_dropdown_set_selected_highlight(drop, true);
  lv_obj_align(drop, LV_ALIGN_TOP_RIGHT, 0, 93);
  lv_obj_set_size(drop, 70, 22);
  //lv_obj_set_style_text_font(drop, &lv_font_montserrat_12, LV_PART_MAIN);
  //lv_obj_set_style_text_font(lv_dropdown_get_list(drop), &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_pad_top(drop, 1, LV_PART_MAIN);
  lv_obj_set_style_bg_color(drop, color_primary, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lv_dropdown_get_list(drop), color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(lv_dropdown_get_list(drop), 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(lv_dropdown_get_list(drop), lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_event_cb(drop, timout_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // // Add another label, then a settings box for WiFi
  // menuLabel = lv_label_create(tab);
  // lv_label_set_text(menuLabel, "Wi-Fi");
  // menuBox = lv_obj_create(tab);
  // lv_obj_set_size(menuBox, lv_pct(100), 80);
  // lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  // lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);
  // menuLabel = lv_label_create(menuBox);
  // lv_label_set_text(menuLabel, "Network");
  // menuLabel = lv_label_create(menuBox);
  // lv_label_set_text(menuLabel, LV_SYMBOL_RIGHT);
  // lv_obj_align(menuLabel, LV_ALIGN_TOP_RIGHT, 0, 0);
  // menuLabel = lv_label_create(menuBox);
  // lv_label_set_text(menuLabel, "Password");
  // lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 32);
  // menuLabel = lv_label_create(menuBox);
  // lv_label_set_text(menuLabel, LV_SYMBOL_RIGHT);
  // lv_obj_align(menuLabel, LV_ALIGN_TOP_RIGHT, 0, 32);

  #if(OMOTE_HARDWARE_REV >= 5)
  // Another setting for the keyboard ----------------------------------------------------------
  menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Keyboard");
  menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 44);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);
  
  brightnessIcon = lv_img_create(menuBox);
  lv_img_set_src(brightnessIcon, &low_brightness);
  lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(brightnessIcon, LV_ALIGN_TOP_LEFT, 0, 0);
  slider = lv_slider_create(menuBox);
  lv_slider_set_range(slider, 0, 255);
  lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(color_primary, 50), LV_PART_MAIN);
  lv_slider_set_value(slider, get_keyboardBrightness(), LV_ANIM_OFF);
  lv_obj_set_size(slider, lv_pct(66), 10);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 3);
  brightnessIcon = lv_img_create(menuBox);
  lv_img_set_src(brightnessIcon, &high_brightness);
  lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(brightnessIcon, LV_ALIGN_TOP_RIGHT, 0, -1);
  lv_obj_add_event_cb(slider, kb_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  #endif

  // Another setting for the battery ----------------------------------------------------------
  menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Battery");
  menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 77); // 125
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);
  
  objBattSettingsVoltage = lv_label_create(menuBox);
  lv_label_set_text(objBattSettingsVoltage, "Voltage:");
  lv_obj_align(objBattSettingsVoltage, LV_ALIGN_TOP_LEFT, 0, 0);
  objBattSettingsPercentage = lv_label_create(menuBox);
  lv_label_set_text(objBattSettingsPercentage, "Percentage:");
  lv_obj_align(objBattSettingsPercentage, LV_ALIGN_TOP_LEFT, 0, 32);
  // objBattSettingsIscharging = lv_label_create(menuBox);
  // lv_label_set_text(objBattSettingsIscharging, "Is charging:");
  // lv_obj_align(objBattSettingsIscharging, LV_ALIGN_TOP_LEFT, 0, 64);


  // Memory statistics ------------------------------------------------------------------------
  menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Memory usage");
  menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 48);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);
  
  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Show mem usage");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 3);
  lv_obj_t* memoryUsageToggle = lv_switch_create(menuBox);
  lv_obj_set_size(memoryUsageToggle, 40, 22);
  lv_obj_align(memoryUsageToggle, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(memoryUsageToggle, lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_event_cb(memoryUsageToggle, showMemoryUsage_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  if (getShowMemoryUsage()) {
    lv_obj_add_state(memoryUsageToggle, LV_STATE_CHECKED);
  } else {
    // lv_obj_clear_state(memoryUsageToggle, LV_STATE_CHECKED);
  }
}

void notify_tab_before_delete_settings(void) {
  // remember to set all pointers to lvgl objects to NULL if they might be accessed from outside.
  // They must check if object is NULL and must not use it if so
  objBattSettingsVoltage = NULL;
  objBattSettingsPercentage = NULL;
}

void register_gui_settings(void){
  register_gui(std::string(tabName_settings), & create_tab_content_settings, & notify_tab_before_delete_settings);
}
