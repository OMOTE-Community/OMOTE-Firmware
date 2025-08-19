#include "applicationInternal/commandHandler.h"
#include "device_specialCommands.h"

uint16_t MY_SPECIAL_COMMAND; //"My_special_command";
// uint16_t TRIGGER_UPDATE_OF_OMOTE_SMART_HOME_DEVICES;

void register_specialCommands() {
  get_uniqueCommandID(&COMMAND_UNKNOWN);

  // command to trigger your home automation software to send the states of the smart home devices known to OMOTE
  // register_command(&TRIGGER_UPDATE_OF_OMOTE_SMART_HOME_DEVICES, makeCommandData(MQTT, {"put_here_your_topic_to_trigger_update_of_omote_smart_home_devices", "PRESS"}));
  
  // put SPECIAL commands here if you want
  register_command(&MY_SPECIAL_COMMAND, makeCommandData(SPECIAL, {""}));

}
