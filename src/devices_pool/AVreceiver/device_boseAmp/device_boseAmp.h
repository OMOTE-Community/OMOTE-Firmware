#pragma once

// Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
extern uint16_t BOSE_POWER_ON;
extern uint16_t BOSE_POWER_OFF;
extern uint16_t BOSE_CBL_SAT;
extern uint16_t BOSE_BD_DVD;
extern uint16_t BOSE_GAME;
extern uint16_t BOSE_TV;
extern uint16_t BOSE_BLUETOOTH;
extern uint16_t BOSE_AUX;
extern uint16_t BOSE_INPUT;
extern uint16_t BOSE_AUDIO;
extern uint16_t BOSE_SOURCE;
extern uint16_t BOSE_MENU;
extern uint16_t BOSE_GUIDE;
extern uint16_t BOSE_INFO;
extern uint16_t BOSE_EXIT;
extern uint16_t BOSE_UP;
extern uint16_t BOSE_DOWN;
extern uint16_t BOSE_LEFT;
extern uint16_t BOSE_RIGHT;
extern uint16_t BOSE_OK;
extern uint16_t BOSE_VOL_UP;
extern uint16_t BOSE_VOL_DOWN;
extern uint16_t BOSE_DVR;
extern uint16_t BOSE_WORLD;
extern uint16_t BOSE_SYSTEM;
extern uint16_t BOSE_MUTE;
extern uint16_t BOSE_BACK;

void register_device_boseAmp();
