#pragma once

#include <string>
#include <lvgl.h>

#if (ENABLE_KEYBOARD_BLE == 1)
const char * const tabName_blepairing = "BLE pairing";
extern uint16_t GUI_BLEPAIRING_ACTIVATE;
void register_gui_blepairing(void);

void addBLEmessage(std::string message);
void flushBLEMessages();
#endif
