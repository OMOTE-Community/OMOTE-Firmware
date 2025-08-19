#pragma once

#include <lvgl.h>

const char * const tabName_settings = "Settings";
void register_gui_settings(void);

// accessed by "guiStatusUpdate.cpp"
extern lv_obj_t* objBattSettingsVoltage;
extern lv_obj_t* objBattSettingsPercentage;
//extern lv_obj_t* objBattSettingsIscharging;

