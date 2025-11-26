#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_shield.h"

#if (ENABLE_HUB_COMMUNICATION > 0)
#include "devices/misc/device_hub_helper.h"
#endif

uint16_t SHIELD_POWER_TOGGLE;
uint16_t SHIELD_POWER_ON;       // mapped to POWER_TOGGLE in the hub
uint16_t SHIELD_POWER_OFF;      // mapped to POWER_TOGGLE in the hub
uint16_t SHIELD_MENU;           //
uint16_t SHIELD_EXIT;           // ESC
uint16_t SHIELD_STOP;           // x
uint16_t SHIELD_PLAY;           // space
uint16_t SHIELD_FORWARD;        // f
uint16_t SHIELD_REVERSE;        // r
uint16_t SHIELD_UP;             // up
uint16_t SHIELD_DOWN;           // down
uint16_t SHIELD_LEFT;           // left
uint16_t SHIELD_RIGHT;          // right
uint16_t SHIELD_OK;             // return
uint16_t SHIELD_SHIELD;         // ?

void register_device_shield() {

    #if (ENABLE_HUB_COMMUNICATION > 0)
    // Register hub commands for Nvidia Shield TV when Hub is enabled
    register_hub_command(&SHIELD_POWER_ON, "ANDROID_TV", "POWER_ON");
    register_hub_command(&SHIELD_POWER_OFF, "ANDROID_TV", "POWER_OFF");
    register_hub_command(&SHIELD_MENU, "ANDROID_TV", "MENU");
    register_hub_command(&SHIELD_EXIT, "ANDROID_TV", "HOME");
    register_hub_command(&SHIELD_STOP, "ANDROID_TV", "STOP");
    register_hub_command(&SHIELD_PLAY, "ANDROID_TV", "PLAY_PAUSE");
    register_hub_command(&SHIELD_FORWARD, "ANDROID_TV", "SKIP_FORWARD");
    register_hub_command(&SHIELD_REVERSE, "ANDROID_TV", "SKIP_BACKWARD");
    register_hub_command(&SHIELD_UP, "ANDROID_TV", "UP");
    register_hub_command(&SHIELD_DOWN, "ANDROID_TV", "DOWN");
    register_hub_command(&SHIELD_LEFT, "ANDROID_TV", "LEFT");
    register_hub_command(&SHIELD_RIGHT, "ANDROID_TV", "RIGHT");
    register_hub_command(&SHIELD_OK, "ANDROID_TV", "SELECT");
  #else
    // http://www.hifi-remote.com/wiki/index.php/Infrared_Protocol_Primer
    // https://www.avsforum.com/threads/discrete-ir-code-list-for-nvidia-shield-tv.2126570/
    register_command(&SHIELD_POWER_TOGGLE,  makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E609F"}));
    register_command(&SHIELD_MENU,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E906F"}));
    register_command(&SHIELD_EXIT,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E40BF"}));
    register_command(&SHIELD_STOP,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017EE01F"}));
    register_command(&SHIELD_PLAY,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E8877"}));
    register_command(&SHIELD_FORWARD,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017EF00F"}));
    register_command(&SHIELD_REVERSE,       makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E50AF"}));
    register_command(&SHIELD_UP,            makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E30CF"}));
    register_command(&SHIELD_DOWN,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E708F"}));
    register_command(&SHIELD_LEFT,          makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017ED02F"}));
    register_command(&SHIELD_RIGHT,         makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017EB04F"}));
    register_command(&SHIELD_OK,            makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017EC03F"}));
    register_command(&SHIELD_SHIELD,        makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x017E847B"}));  
  #endif
}