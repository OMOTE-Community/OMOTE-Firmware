#include <lvgl.h>
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/keys.h"
#include "devices/misc/device_smarthome/gui_smarthome.h"
#include "devices/misc/device_smarthome/device_smarthome.h"
#include "scenes/scene__default.h"

// LVGL declarations
LV_IMG_DECLARE(lightbulb);

static lv_obj_t* lightToggleA;
static lv_obj_t* lightToggleB;
static lv_obj_t* sliderA;
static lv_obj_t* sliderB;

static bool lightToggleAstate = false;
static bool lightToggleBstate = false;
static int32_t sliderAvalue = 0;
static int32_t sliderBvalue = 0;

uint16_t GUI_SMARTHOME_ACTIVATE;

std::map<char, repeatModes> key_repeatModes_smarthome = {};
std::map<char, uint16_t> key_commands_short_smarthome = {};
std::map<char, uint16_t> key_commands_long_smarthome = {};

// Smart Home Toggle Event handler
static void smartHomeToggle_event_cb(lv_event_t* e){
  std::string payload;
  if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED)) payload = "true";
  else payload = "false";
  // Publish an MQTT message based on the event user data  
  #if (ENABLE_WIFI_AND_MQTT == 1)
  int user_data = *((int*)(&(e->user_data)));
  if(user_data == 1) executeCommand(SMARTHOME_MQTT_BULB1_SET, payload);
  if(user_data == 2) executeCommand(SMARTHOME_MQTT_BULB2_SET, payload);
  #endif
}

// Smart Home Slider Event handler
static void smartHomeSlider_event_cb(lv_event_t* e){
  lv_obj_t* slider = lv_event_get_target(e);
  char payload[8];
  sprintf(payload, "%.2f", float(lv_slider_get_value(slider)));
  std::string payload_str(payload);
  // Publish an MQTT message based on the event user data
  #if (ENABLE_WIFI_AND_MQTT == 1)
  int user_data = *((int*)(&(e->user_data)));
  if(user_data == 1) executeCommand(SMARTHOME_MQTT_BULB1_BRIGHTNESS_SET, payload_str);
  if(user_data == 2) executeCommand(SMARTHOME_MQTT_BULB2_BRIGHTNESS_SET, payload_str);
  #endif
}

void create_tab_content_smarthome(lv_obj_t* tab) {

  // Add content to the smart home tab
  lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_ACTIVE);

  // Add a label, then a box for the light controls
  lv_obj_t* menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Living Room");

  lv_obj_t* menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 79);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  lv_obj_t* bulbIcon = lv_img_create(menuBox);
  lv_img_set_src(bulbIcon, &lightbulb);
  lv_obj_set_style_img_recolor(bulbIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(bulbIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(bulbIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Floor Lamp");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 22, 3);
  lightToggleA = lv_switch_create(menuBox);
  if (lightToggleAstate) {
    lv_obj_add_state(lightToggleA, LV_STATE_CHECKED);
  } else {
    // lv_obj_clear_state(lightToggleA, LV_STATE_CHECKED);
  }
  lv_obj_set_size(lightToggleA, 40, 22);
  lv_obj_align(lightToggleA, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(lightToggleA, lv_color_lighten(color_primary, 50), LV_PART_MAIN);
  lv_obj_set_style_bg_color(lightToggleA, color_primary, LV_PART_INDICATOR);
  lv_obj_add_event_cb(lightToggleA, smartHomeToggle_event_cb, LV_EVENT_VALUE_CHANGED, (void*)1);

  sliderA = lv_slider_create(menuBox);
  lv_slider_set_range(sliderA, 0, 100);
  lv_obj_set_style_bg_color(sliderA, lv_color_lighten(lv_color_black(), 30), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_color(sliderA, lv_color_lighten(lv_palette_main(LV_PALETTE_AMBER), 180), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_dir(sliderA, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(sliderA, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(sliderA, 255, LV_PART_MAIN);
  lv_obj_set_style_bg_color(sliderA, lv_color_lighten(color_primary, 50), LV_PART_MAIN);
  lv_slider_set_value(sliderA, sliderAvalue, LV_ANIM_OFF);
  lv_obj_set_size(sliderA, lv_pct(90), 10);
  lv_obj_align(sliderA, LV_ALIGN_TOP_MID, 0, 37);
  lv_obj_add_event_cb(sliderA, smartHomeSlider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)1);

  // Add another menu box for a second appliance
  menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 79);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  bulbIcon = lv_img_create(menuBox);
  lv_img_set_src(bulbIcon, &lightbulb);
  lv_obj_set_style_img_recolor(bulbIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(bulbIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(bulbIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Ceiling Light");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 22, 3);
  lightToggleB = lv_switch_create(menuBox);
  if (lightToggleBstate) {
    lv_obj_add_state(lightToggleB, LV_STATE_CHECKED);
  } else {
    // lv_obj_clear_state(lightToggleB, LV_STATE_CHECKED);
  }
  lv_obj_set_size(lightToggleB, 40, 22);
  lv_obj_align(lightToggleB, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(lightToggleB, lv_color_lighten(color_primary, 50), LV_PART_MAIN);
  lv_obj_set_style_bg_color(lightToggleB, color_primary, LV_PART_INDICATOR);
  lv_obj_add_event_cb(lightToggleB, smartHomeToggle_event_cb, LV_EVENT_VALUE_CHANGED, (void*)2);

  sliderB = lv_slider_create(menuBox);
  lv_slider_set_range(sliderB, 0, 100);
  lv_obj_set_style_bg_color(sliderB, lv_color_lighten(lv_color_black(), 30), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_color(sliderB, lv_color_lighten(lv_palette_main(LV_PALETTE_AMBER), 180), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_dir(sliderB, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(sliderB, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(sliderB, 255, LV_PART_MAIN);
  lv_obj_set_style_bg_color(sliderB, lv_color_lighten(color_primary, 50), LV_PART_MAIN);
  lv_slider_set_value(sliderB, sliderBvalue, LV_ANIM_OFF);
  lv_obj_set_size(sliderB, lv_pct(90), 10);
  lv_obj_align(sliderB, LV_ALIGN_TOP_MID, 0, 37);
  lv_obj_add_event_cb(sliderB, smartHomeSlider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)2);


  // Add another room (empty for now)
  menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "Kitchen");

  menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 79);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

}

void notify_tab_before_delete_smarthome(void) {
  // remember to set all pointers to lvgl objects to NULL if they might be accessed from outside.
  // They must check if object is NULL and must not use it if so
  lightToggleAstate = lv_obj_has_state(lightToggleA, LV_STATE_CHECKED);
  lightToggleBstate = lv_obj_has_state(lightToggleB, LV_STATE_CHECKED);
  sliderAvalue = lv_slider_get_value(sliderA);
  sliderBvalue = lv_slider_get_value(sliderB);
}

void gui_setKeys_smarthome() {
  key_commands_short_smarthome = {
    {KEY_STOP, SCENE_SELECTION},
  };
}

void register_gui_smarthome(void){

  register_gui(
    std::string(tabName_smarthome),
    & create_tab_content_smarthome,
    & notify_tab_before_delete_smarthome,
    & gui_setKeys_smarthome,
    & key_repeatModes_smarthome,
    & key_commands_short_smarthome,
    & key_commands_long_smarthome
    );

  register_command(&GUI_SMARTHOME_ACTIVATE, makeCommandData(GUI, {std::to_string(MAIN_GUI_LIST), std::string(tabName_smarthome)}));
}
