#include "device_sonyAvr.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include <string>

#if (ENABLE_HUB_COMMUNICATION > 0)
#include "devices/misc/device_hub_helper.h"
#endif

// Only activate the commands that are used. Every command takes 100 bytes.
uint16_t SONY_POWER_ON;
uint16_t SONY_POWER_OFF;
uint16_t SONY_VOL_MINUS;
uint16_t SONY_VOL_PLUS;
uint16_t SONY_VOL_MUTE;

void register_device_sonyAvr() {

  #if (ENABLE_HUB_COMMUNICATION > 0)
    register_hub_command(&SONY_POWER_ON, "SONY_AVR", "POWER_ON");
    register_hub_command(&SONY_POWER_OFF, "SONY_AVR", "POWER_OFF");
    register_hub_command(&SONY_VOL_MINUS, "SONY_AVR", "VOL_MINUS");
    register_hub_command(&SONY_VOL_PLUS, "SONY_AVR", "VOL_PLUS");
    register_hub_command(&SONY_VOL_MUTE, "SONY_AVR", "VOL_MUTE");
  #endif

}
