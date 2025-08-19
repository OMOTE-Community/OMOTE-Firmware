#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_lgbluray.h"

uint16_t LGBLURAY_POWER;
uint16_t LGBLURAY_OPEN_CLOSE;
//uint16_t LGBLURAY_1;
//uint16_t LGBLURAY_2;
//uint16_t LGBLURAY_3;
//uint16_t LGBLURAY_4;
//uint16_t LGBLURAY_5;
//uint16_t LGBLURAY_6;
//uint16_t LGBLURAY_7;
//uint16_t LGBLURAY_8;
//uint16_t LGBLURAY_9;
//uint16_t LGBLURAY_0;
//uint16_t LGBLURAY_TITLE_POPUP;
//uint16_t LGBLURAY_REPEAT;
uint16_t LGBLURAY_REW;
uint16_t LGBLURAY_FF;
uint16_t LGBLURAY_PREVIOUS;
uint16_t LGBLURAY_SKIP;
uint16_t LGBLURAY_PAUSE;
uint16_t LGBLURAY_PLAY;
uint16_t LGBLURAY_STOP;
uint16_t LGBLURAY_HOME;
uint16_t LGBLURAY_INFO;
uint16_t LGBLURAY_CURSOR_UP;
uint16_t LGBLURAY_LEFT;
uint16_t LGBLURAY_ENTER;
uint16_t LGBLURAY_RIGHT;
uint16_t LGBLURAY_CURSOR_DOWN;
uint16_t LGBLURAY_BACK;
uint16_t LGBLURAY_DISC_MENU;
uint16_t LGBLURAY_KEY_A;
uint16_t LGBLURAY_KEY_B;
uint16_t LGBLURAY_KEY_C;
uint16_t LGBLURAY_KEY_D;

void register_device_lgbluray() {
  // LG BLURAY Remote AKB73896401
  register_command(&LGBLURAY_POWER,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B40CF3"}}));
  register_command(&LGBLURAY_OPEN_CLOSE,    makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B46C93"}}));
  //register_command(&LGBLURAY_1,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4DC23"}}));
  //register_command(&LGBLURAY_2,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B43CC3"}}));
  //register_command(&LGBLURAY_3,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4BC43"}}));
  //register_command(&LGBLURAY_4,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B47C83"}}));
  //register_command(&LGBLURAY_5,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4FC03"}}));
  //register_command(&LGBLURAY_6,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B402FD"}}));
  //register_command(&LGBLURAY_7,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4827D"}}));
  //register_command(&LGBLURAY_8,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B442BD"}}));
  //register_command(&LGBLURAY_9,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4C23D"}}));
  //register_command(&LGBLURAY_0,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B422DD"}}));
  //register_command(&LGBLURAY_TITLE_POPUP,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B452AD"}}));
  //register_command(&LGBLURAY_REPEAT,        makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4CA35"}}));
  register_command(&LGBLURAY_REW,           makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B44CB3"}}));
  register_command(&LGBLURAY_FF,            makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4CC33"}}));
  register_command(&LGBLURAY_PREVIOUS,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4AC53"}}));
  register_command(&LGBLURAY_SKIP,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B42CD3"}}));
  register_command(&LGBLURAY_PAUSE,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B41CE3"}}));
  register_command(&LGBLURAY_PLAY,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B48C73"}}));
  register_command(&LGBLURAY_STOP,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B49C63"}}));
  register_command(&LGBLURAY_HOME,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4E619"}}));
  register_command(&LGBLURAY_INFO,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B45CA3"}}));
  register_command(&LGBLURAY_CURSOR_UP,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4E21D"}}));
  register_command(&LGBLURAY_LEFT,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B49A65"}}));
  register_command(&LGBLURAY_ENTER,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B41AE5"}}));
  register_command(&LGBLURAY_RIGHT,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B45AA5"}}));
  register_command(&LGBLURAY_CURSOR_DOWN,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B412ED"}}));
  register_command(&LGBLURAY_BACK,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4A25D"}}));
  register_command(&LGBLURAY_DISC_MENU,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4D22D"}}));
  register_command(&LGBLURAY_KEY_A,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B43EC1"}}));
  register_command(&LGBLURAY_KEY_B,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4BE41"}}));
  register_command(&LGBLURAY_KEY_C,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B47E81"}}));
  register_command(&LGBLURAY_KEY_D,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0xB4B4FE01"}}));
}