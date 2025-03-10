#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_appleTV.h"

#if (ENABLE_HUB_COMMUNICATION == 1)
#include "devices/misc/device_hub_helper.h"
#endif

uint16_t APPLETV_GUI_EVENT_USER_DATA; //"AppleTV_gui_event_user_data";

// Command IDs for Apple TV
uint16_t APPLETV_POWER_ON;
uint16_t APPLETV_POWER_OFF;
uint16_t APPLETV_PLAY_PAUSE;
uint16_t APPLETV_SKIP_FORWARD;
uint16_t APPLETV_SKIP_BACKWARD;
uint16_t APPLETV_STOP;
uint16_t APPLETV_UP;
uint16_t APPLETV_DOWN;
uint16_t APPLETV_RIGHT;
uint16_t APPLETV_LEFT;
uint16_t APPLETV_SELECT;
uint16_t APPLETV_MENU;
uint16_t APPLETV_HOME;

void register_device_appleTV() {

  #if (ENABLE_HUB_COMMUNICATION == 1)
    // Register hub commands for Apple TV when Hub is enabled
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
    register_hub_command(&APPLETV_GUI_EVENT_USER_DATA, "APPLE_TV", "GUI_EVENT");
  #else
    // Register IR commands for Apple TV when Hub is not enabled
    register_command(&APPLETV_GUI_EVENT_USER_DATA, makeCommandData(IR, {std::to_string(IR_PROTOCOL_SONY)}));  // payload must be set when calling commandHandler
  #endif

}
