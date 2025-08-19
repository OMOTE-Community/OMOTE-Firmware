#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_samsungbluray.h"

uint16_t SAMSUNGBLURAY_POWER;
uint16_t SAMSUNGBLURAY_OPEN_CLOSE;
//uint16_t SAMSUNGBLURAY_1;
//uint16_t SAMSUNGBLURAY_2;
//uint16_t SAMSUNGBLURAY_3;
//uint16_t SAMSUNGBLURAY_4;
//uint16_t SAMSUNGBLURAY_5;
//uint16_t SAMSUNGBLURAY_6;
//uint16_t SAMSUNGBLURAY_7;
//uint16_t SAMSUNGBLURAY_8;
//uint16_t SAMSUNGBLURAY_9;
//uint16_t SAMSUNGBLURAY_0;
uint16_t SAMSUNGBLURAY_DISC_MENU;
//uint16_t SAMSUNGBLURAY_TITLE_POPUP;
uint16_t SAMSUNGBLURAY_PREVIOUS;
uint16_t SAMSUNGBLURAY_SKIP;
uint16_t SAMSUNGBLURAY_REW;
uint16_t SAMSUNGBLURAY_FF;
uint16_t SAMSUNGBLURAY_STOP;
uint16_t SAMSUNGBLURAY_PLAY;
uint16_t SAMSUNGBLURAY_PAUSE;
uint16_t SAMSUNGBLURAY_AUDIO;
uint16_t SAMSUNGBLURAY_HOME;
uint16_t SAMSUNGBLURAY_SUBTITLE;
uint16_t SAMSUNGBLURAY_TOOLS;
uint16_t SAMSUNGBLURAY_INFO;
uint16_t SAMSUNGBLURAY_CURSOR_UP;
uint16_t SAMSUNGBLURAY_LEFT;
uint16_t SAMSUNGBLURAY_ENTER;
uint16_t SAMSUNGBLURAY_RIGHT;
uint16_t SAMSUNGBLURAY_CURSOR_DOWN;
uint16_t SAMSUNGBLURAY_BACK;
uint16_t SAMSUNGBLURAY_EXIT;
uint16_t SAMSUNGBLURAY_KEY_A;
uint16_t SAMSUNGBLURAY_KEY_B;
uint16_t SAMSUNGBLURAY_KEY_C;
uint16_t SAMSUNGBLURAY_KEY_D;
//uint16_t SAMSUNGBLURAY_VIEW;
//uint16_t SAMSUNGBLURAY_A_B;
//uint16_t SAMSUNGBLURAY_REPEAT;
//uint16_t SAMSUNGBLURAY_SCREEN;

void register_device_samsungbluray()
{
    // SAMSUNG BLURAY AKB73896401
    register_command(&SAMSUNGBLURAY_POWER,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E00FF"}}));
    register_command(&SAMSUNGBLURAY_OPEN_CLOSE, makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E807F"}}));
    //register_command(&SAMSUNGBLURAY_1,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E40BF"}}));
    //register_command(&SAMSUNGBLURAY_2,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EC03F"}}));
    //register_command(&SAMSUNGBLURAY_3,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E20DF"}}));
    //register_command(&SAMSUNGBLURAY_4,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EA05F"}}));
    //register_command(&SAMSUNGBLURAY_5,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E609F"}}));
    //register_command(&SAMSUNGBLURAY_6,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EE01F"}}));
    //register_command(&SAMSUNGBLURAY_7,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E10EF"}}));
    //register_command(&SAMSUNGBLURAY_8,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E906F"}}));
    //register_command(&SAMSUNGBLURAY_9,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E50AF"}}));
    //register_command(&SAMSUNGBLURAY_0,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400ED02F"}}));
    register_command(&SAMSUNGBLURAY_DISC_MENU,  makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EB847"}}));
    //register_command(&SAMSUNGBLURAY_TITLE_POPUP, makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E04FB"}}));
    register_command(&SAMSUNGBLURAY_PREVIOUS,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EB04F"}}));
    register_command(&SAMSUNGBLURAY_SKIP,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E8877"}}));
    register_command(&SAMSUNGBLURAY_REW,        makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E48B7"}}));
    register_command(&SAMSUNGBLURAY_FF,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EA857"}}));
    register_command(&SAMSUNGBLURAY_STOP,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EC837"}}));
    register_command(&SAMSUNGBLURAY_PLAY,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E28D7"}}));
    register_command(&SAMSUNGBLURAY_PAUSE,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E4CB3"}}));
    register_command(&SAMSUNGBLURAY_AUDIO,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EA45B"}}));
    register_command(&SAMSUNGBLURAY_HOME,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E6897"}}));
    register_command(&SAMSUNGBLURAY_SUBTITLE,   makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E649B"}}));
    register_command(&SAMSUNGBLURAY_TOOLS,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E5CA3"}}));
    register_command(&SAMSUNGBLURAY_INFO,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E7887"}}));
    register_command(&SAMSUNGBLURAY_CURSOR_UP,  makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E18E7"}}));
    register_command(&SAMSUNGBLURAY_LEFT,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400ED827"}}));
    register_command(&SAMSUNGBLURAY_ENTER,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E38C7"}}));
    register_command(&SAMSUNGBLURAY_RIGHT,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E58A7"}}));
    register_command(&SAMSUNGBLURAY_CURSOR_DOWN, makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E9867"}}));
    register_command(&SAMSUNGBLURAY_BACK,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EE817"}}));
    register_command(&SAMSUNGBLURAY_EXIT,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400ED42B"}}));
    register_command(&SAMSUNGBLURAY_KEY_A,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E847B"}}));
    register_command(&SAMSUNGBLURAY_KEY_B,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E44BB"}}));
    register_command(&SAMSUNGBLURAY_KEY_C,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EC43B"}}));
    register_command(&SAMSUNGBLURAY_KEY_D,      makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E24DB"}}));
    //register_command(&SAMSUNGBLURAY_VIEW,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400ECC33"}}));
    //register_command(&SAMSUNGBLURAY_A_B,        makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E05C8"}}));
    //register_command(&SAMSUNGBLURAY_REPEAT,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400EE41B"}}));
    //register_command(&SAMSUNGBLURAY_SCREEN,     makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG36), {"0x400E9C63"}}));
}