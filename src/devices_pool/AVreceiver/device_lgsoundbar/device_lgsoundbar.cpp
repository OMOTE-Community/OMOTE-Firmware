#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_lgsoundbar.h"

uint16_t LGSOUNDBAR_POWER;
//uint16_t LGSOUNDBAR_FUNCTION;
//uint16_t LGSOUNDBAR_FOLDER_UP;
//uint16_t LGSOUNDBAR_FOLDER_DOWN;
//uint16_t LGSOUNDBAR_OPTICAL;
//uint16_t LGSOUNDBAR_MUTE;
//uint16_t LGSOUNDBAR_VOLUME_UP;
//uint16_t LGSOUNDBAR_VOLUME_DOWN;
//uint16_t LGSOUNDBAR_AUTOPOWER;
//uint16_t LGSOUNDBAR_AV_SYNC;
//uint16_t LGSOUNDBAR_INFO;
//uint16_t LGSOUNDBAR_CURSOR_UP;
//uint16_t LGSOUNDBAR_LEFT;
//uint16_t LGSOUNDBAR_ENTER;
//uint16_t LGSOUNDBAR_RIGHT;
//uint16_t LGSOUNDBAR_CURSOR_DOWN;
//uint16_t LGSOUNDBAR_SOUND_EFFECT;
//uint16_t LGSOUNDBAR_AUTOVOLUME;
uint16_t LGSOUNDBAR_WOOFER_LEVEL;
//uint16_t LGSOUNDBAR_1;
//uint16_t LGSOUNDBAR_2;
//uint16_t LGSOUNDBAR_3;
//uint16_t LGSOUNDBAR_4;
//uint16_t LGSOUNDBAR_5;
//uint16_t LGSOUNDBAR_6;
//uint16_t LGSOUNDBAR_7;
//uint16_t LGSOUNDBAR_8;
//uint16_t LGSOUNDBAR_9;
//uint16_t LGSOUNDBAR_0;
//uint16_t LGSOUNDBAR_DOBLY_DRC;
//uint16_t LGSOUNDBAR_SLEEP;

void register_device_lgsoundbar() {
  // LG SOUNDBAR Remote AKB73575421
  register_command(&LGSOUNDBAR_POWER,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x34347887"}}));
  //register_command(&LGSOUNDBAR_FUNCTION,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343451AE"}}));
  //register_command(&LGSOUNDBAR_FOLDER_UP,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343430CF"}}));
  //register_command(&LGSOUNDBAR_FOLDER_DOWN,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434B04F"}}));
  //register_command(&LGSOUNDBAR_OPTICAL,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x34346D92"}}));
  //register_command(&LGSOUNDBAR_MUTE,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434F807"}}));
  //register_command(&LGSOUNDBAR_VOLUME_UP,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434E817"}}));
  //register_command(&LGSOUNDBAR_VOLUME_DOWN,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x34346897"}}));
  //register_command(&LGSOUNDBAR_AUTOPOWER,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434C936"}}));
  //register_command(&LGSOUNDBAR_AV_SYNC,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x34349B64"}}));
  //register_command(&LGSOUNDBAR_INFO,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434C53A"}}));
  //register_command(&LGSOUNDBAR_CURSOR_UP,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434728D"}}));
  //register_command(&LGSOUNDBAR_LEFT,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434609F"}}));
  //register_command(&LGSOUNDBAR_ENTER,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434A05F"}}));
  //register_command(&LGSOUNDBAR_RIGHT,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434E01F"}}));
  //register_command(&LGSOUNDBAR_CURSOR_DOWN,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434F20D"}}));
  //register_command(&LGSOUNDBAR_SOUND_EFFECT,  makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434F40B"}}));
  //register_command(&LGSOUNDBAR_AUTOVOLUME,    makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343438C7"}}));
  register_command(&LGSOUNDBAR_WOOFER_LEVEL,  makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x34346699"}}));
  //register_command(&LGSOUNDBAR_1,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434827D"}}));
  //register_command(&LGSOUNDBAR_2,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343442BD"}}));
  //register_command(&LGSOUNDBAR_3,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434C23D"}}));
  //register_command(&LGSOUNDBAR_4,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343422DD"}}));
  //register_command(&LGSOUNDBAR_5,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434A25D"}}));
  //register_command(&LGSOUNDBAR_6,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434629D"}}));
  //register_command(&LGSOUNDBAR_7,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434E21D"}}));
  //register_command(&LGSOUNDBAR_8,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343412ED"}}));
  //register_command(&LGSOUNDBAR_9,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434926D"}}));
  //register_command(&LGSOUNDBAR_0,             makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x3434D22D"}}));
  //register_command(&LGSOUNDBAR_DOBLY_DRC,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343446B9"}}));
  //register_command(&LGSOUNDBAR_SLEEP,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), {"0x343443BC"}}));
}