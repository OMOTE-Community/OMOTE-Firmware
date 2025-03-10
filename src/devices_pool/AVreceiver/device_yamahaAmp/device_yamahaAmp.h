#pragma once

// Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
extern uint16_t YAMAHA_INPUT_DVD;
extern uint16_t YAMAHA_INPUT_DTV;
// extern uint16_t YAMAHA_INPUT_VCR;
// extern uint16_t YAMAHA_POWER_TOGGLE;
// extern uint16_t YAMAHA_INPUT_CD;
// extern uint16_t YAMAHA_INPUT_MD;
// extern uint16_t YAMAHA_INPUT_VAUX;
// extern uint16_t YAMAHA_MULTICHANNEL;
// extern uint16_t YAMAHA_INPUT_TUNER;
// extern uint16_t YAMAHA_PRESETGROUP;
// extern uint16_t YAMAHA_PRESETSTATION_MINUS;
// extern uint16_t YAMAHA_PRESETSTATION_PLUS;
extern uint16_t YAMAHA_STANDARD;
// extern uint16_t YAMAHA_5CHSTEREO;
// extern uint16_t YAMAHA_NIGHT;
// extern uint16_t YAMAHA_SLEEP;
// extern uint16_t YAMAHA_TEST;
// extern uint16_t YAMAHA_STRAIGHT;
extern uint16_t YAMAHA_VOL_MINUS;
extern uint16_t YAMAHA_VOL_PLUS;
// extern uint16_t YAMAHA_PROG_MINUS;
// extern uint16_t YAMAHA_PROG_PLUS;
extern uint16_t YAMAHA_MUTE_TOGGLE;
// extern uint16_t YAMAHA_LEVEL;
// extern uint16_t YAMAHA_SETMENU;
// extern uint16_t YAMAHA_SETMENU_UP;
// extern uint16_t YAMAHA_SETMENU_DOWN;
// extern uint16_t YAMAHA_SETMENU_MINUS;
// extern uint16_t YAMAHA_SETMENU_PLUS;
extern uint16_t YAMAHA_POWER_OFF;
extern uint16_t YAMAHA_POWER_ON;

void register_device_yamahaAmp();
