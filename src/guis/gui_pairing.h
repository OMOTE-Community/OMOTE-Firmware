#pragma once

#include <lvgl.h>

const char * const tabName_pairing = "Pairing";
void register_gui_pairing(void);
void gui_pairing_update_status(const char* message);
void gui_pairing_clear_pin();
