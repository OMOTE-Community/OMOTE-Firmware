#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_appleTV.h"

uint16_t APPLETV_UP;
uint16_t APPLETV_DOWN;
uint16_t APPLETV_LEFT;
uint16_t APPLETV_RIGHT;
uint16_t APPLETV_SELECT;

uint16_t APPLETV_PLAY;
uint16_t APPLETV_PAUSE;

uint16_t APPLETV_SKIP_FORWARD;
uint16_t APPLETV_SKIP_BACKWARD;

uint16_t APPLETV_NEXT;
uint16_t APPLETV_PREVIOUS;

uint16_t APPLETV_MENU;
uint16_t APPLETV_HOME;

uint16_t APPLETV_POWER_ON;
uint16_t APPLETV_POWER_OFF;
uint16_t APPLETV_PLAY_PAUSE;

uint16_t APPLETV_STOP;

void register_device_appleTV() {
    #if (ENABLE_HUB_COMMUNICATION == 1)
    // Register hub commands for Apple TV when Hub is enabled
    register_hub_command(&APPLETV_GUI_EVENT_USER_DATA, "APPLE_TV", "GUI_EVENT");

    register_hub_command(&APPLETV_POWER_ON, "APPLE_TV", "POWER_ON");
    register_hub_command(&APPLETV_POWER_OFF, "APPLE_TV", "POWER_OFF");
    register_hub_command(&APPLETV_PLAY_PAUSE, "APPLE_TV", "PLAY_PAUSE");
    register_hub_command(&APPLETV_SKIP_FORWARD, "APPLE_TV", "SKIP_FORWARD");
    register_hub_command(&APPLETV_SKIP_BACKWARD, "APPLE_TV", "SKIP_BACKWARD");
    register_hub_command(&APPLETV_STOP, "APPLE_TV", "STOP");
    register_hub_command(&APPLETV_UP, "APPLE_TV", "UP"); 
    register_hub_command(&APPLETV_DOWN, "APPLE_TV", "DOWN");
    register_hub_command(&APPLETV_RIGHT, "APPLE_TV", "RIGHT");
    register_hub_command(&APPLETV_LEFT, "APPLE_TV", "LEFT");
    register_hub_command(&APPLETV_SELECT, "APPLE_TV", "SELECT");
    register_hub_command(&APPLETV_MENU, "APPLE_TV", "MENU");
    register_hub_command(&APPLETV_HOME, "APPLE_TV", "HOME");
  #else
    register_command(&APPLETV_UP                   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E15080"}));
    register_command(&APPLETV_DOWN                 , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E13080"}));
    register_command(&APPLETV_LEFT                 , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E19080"}));
    register_command(&APPLETV_RIGHT                , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E16080"}));
    register_command(&APPLETV_SELECT               , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E13A80"}));

    register_command(&APPLETV_PLAY                 , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E1FA80"})); 
    register_command(&APPLETV_PAUSE                , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0xA7E14C80:32:1"})); // Code + kNECBits + 1 repeat

    register_command(&APPLETV_SKIP_BACKWARD        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0xA7E16480:32:1"})); // Code + kNECBits + 1 repeat
    register_command(&APPLETV_SKIP_FORWARD         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0xA7E11080:32:1"})); // Code + kNECBits + 1 repeat

    register_command(&APPLETV_NEXT                 , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0xA7E1C480:32:1"})); // Code + kNECBits + 1 repeat
    register_command(&APPLETV_PREVIOUS             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0xA7E1A480:32:1"})); // Code + kNECBits + 1 repeat
    
    register_command(&APPLETV_MENU                 , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x77E1C080"}));
    register_command(&APPLETV_HOME                 , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0xA7E10280:32:1"})); // Code + kNECBits + 1 repeat
  #endif
}
