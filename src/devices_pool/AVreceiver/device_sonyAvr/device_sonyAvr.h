#pragma once

#include <stdint.h>

// Only activate the commands that are used. Every command takes 100 bytes.
extern uint16_t SONY_POWER_ON;
extern uint16_t SONY_POWER_OFF;
extern uint16_t SONY_VOL_MINUS;
extern uint16_t SONY_VOL_PLUS;
extern uint16_t SONY_VOL_MUTE;

void register_device_sonyAvr();