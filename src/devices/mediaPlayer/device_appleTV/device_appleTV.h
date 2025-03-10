#pragma once

#include <stdint.h>


extern uint16_t APPLETV_GUI_EVENT_USER_DATA;
extern uint16_t APPLETV_POWER_ON;
extern uint16_t APPLETV_POWER_OFF;
extern uint16_t APPLETV_PLAY_PAUSE;
extern uint16_t APPLETV_SKIP_FORWARD;
extern uint16_t APPLETV_SKIP_BACKWARD;
extern uint16_t APPLETV_STOP;
extern uint16_t APPLETV_UP;
extern uint16_t APPLETV_DOWN;
extern uint16_t APPLETV_RIGHT;
extern uint16_t APPLETV_LEFT;
extern uint16_t APPLETV_SELECT;
extern uint16_t APPLETV_MENU;
extern uint16_t APPLETV_HOME;

void register_device_appleTV();
