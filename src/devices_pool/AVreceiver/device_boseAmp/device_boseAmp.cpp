#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_boseAmp.h"

// Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
  uint16_t BOSE_POWER_ON;
  uint16_t BOSE_POWER_OFF;
  uint16_t BOSE_CBL_SAT;
  uint16_t BOSE_BD_DVD;
  uint16_t BOSE_GAME;
  uint16_t BOSE_TV;
  uint16_t BOSE_BLUETOOTH;
  uint16_t BOSE_AUX;
  uint16_t BOSE_INPUT;
  uint16_t BOSE_AUDIO;
  uint16_t BOSE_SOURCE;
  uint16_t BOSE_MENU;
  uint16_t BOSE_GUIDE;
  uint16_t BOSE_INFO;
  uint16_t BOSE_EXIT;
  uint16_t BOSE_UP;
  uint16_t BOSE_DOWN;
  uint16_t BOSE_LEFT;
  uint16_t BOSE_RIGHT;
  uint16_t BOSE_OK;
  uint16_t BOSE_VOL_UP;
  uint16_t BOSE_VOL_DOWN;
  uint16_t BOSE_DVR;
  uint16_t BOSE_WORLD;
  uint16_t BOSE_SYSTEM;
  uint16_t BOSE_MUTE;
  uint16_t BOSE_BACK;

void register_device_boseAmp() {
  register_command(&BOSE_POWER_ON       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2A05F", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_POWER_OFF      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2A45B", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_CBL_SAT        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2A857", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_BD_DVD         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2CA35", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_GAME           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2609F", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_TV             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2708F", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_BLUETOOTH      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2CD32", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_AUX            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD2B04F", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_INPUT          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD23CC3", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_AUDIO          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD232CD", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_SOURCE         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD234CB", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_MENU           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD209F6", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_GUIDE          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD20BF4", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_INFO           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD226D9", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_EXIT           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD20CF3", kNECBits,   kNoRepeat),          concatenateIRsendParams("0x5DD28C73", kNECBits, kNoRepeat)}));
  register_command(&BOSE_UP             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD203FC", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_DOWN           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD204FB", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_LEFT           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD205FA", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_RIGHT          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD206F9", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_OK             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD207F8", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_VOL_UP         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD2C03F", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_VOL_DOWN       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD240BF", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_DVR            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_EPSON), concatenateIRsendParams("0x5DD26D92", kEpsonBits, kEpsonMinRepeat)}));
  register_command(&BOSE_WORLD          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD2837C", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_SYSTEM         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD201FE", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_MUTE           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD2807F", kNECBits,   kNoRepeat)}));
  register_command(&BOSE_BACK           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC),   concatenateIRsendParams("0x5DD228D7", kNECBits,   kNoRepeat)}));
}
