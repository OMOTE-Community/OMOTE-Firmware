#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_lgTV.h"

#if (ENABLE_HUB_COMMUNICATION == 1)
#include "devices/misc/device_hub_helper.h"
#endif

// Only activate the commands that are used. Every command takes 100 bytes, wether used or not
uint16_t LGTV_POWER_ON;
uint16_t LGTV_POWER_OFF;
uint16_t LGTV_POWER_TOGGLE;
uint16_t LGTV_SOURCE;
uint16_t LGTV_NUM_1;
uint16_t LGTV_NUM_2;
uint16_t LGTV_NUM_3;
uint16_t LGTV_NUM_4;
uint16_t LGTV_NUM_5;
uint16_t LGTV_NUM_6;
uint16_t LGTV_NUM_7;
uint16_t LGTV_NUM_8;
uint16_t LGTV_NUM_9;
uint16_t LGTV_NUM_0;
uint16_t LGTV_PRECH;
uint16_t LGTV_VOL_MINUS;
uint16_t LGTV_VOL_PLUS;
uint16_t LGTV_MUTE_TOGGLE;
//uint16_t LGTV_CHLIST;
uint16_t LGTV_CHANNEL_UP;
uint16_t LGTV_CHANNEL_DOWN;
uint16_t LGTV_MENU; // Q-Menu
//uint16_t LGTV_APPS;
uint16_t LGTV_GUIDE;
uint16_t LGTV_SETUP;
uint16_t LGTV_INFO;
uint16_t LGTV_UP;
uint16_t LGTV_DOWN;
uint16_t LGTV_LEFT;
uint16_t LGTV_RIGHT;
uint16_t LGTV_OK;
uint16_t LGTV_RETURN;
uint16_t LGTV_EXIT;
uint16_t LGTV_KEY_A;
uint16_t LGTV_KEY_B;
uint16_t LGTV_KEY_C;
uint16_t LGTV_KEY_D;
//uint16_t LGTV_PSIZE;
//uint16_t LGTV_ADSUBT;
//uint16_t LGTV_REWIND;
//uint16_t LGTV_PAUSE;
//uint16_t LGTV_FASTFORWARD;
//uint16_t LGTV_RECORD;
//uint16_t LGTV_PLAY;
//uint16_t LGTV_STOP;
//uint16_t LGTV_POWER_OFF;
//uint16_t LGTV_POWER_ON;
uint16_t LGTV_INPUT_HDMI_1;
uint16_t LGTV_INPUT_HDMI_2;
uint16_t LGTV_INPUT_HDMI_3;
uint16_t LGTV_INPUT_HDMI_4;
//uint16_t LGTV_INPUT_COMPONENT;
uint16_t LGTV_INPUT_TV;
//uint16_t LGTV_SLEEP;
uint16_t LGTV_3D;
uint16_t LGTV_HOME;

void register_device_lgTV()
{
  // https://github.com/natcl/studioimaginaire/blob/master/arduino_remote/ircodes.py
  // https://tasmota.github.io/docs/Codes-for-IR-Remotes/#lg-55uh8509-tv
  // Tested on LG 42LA6208-ZA

  register_command(&LGTV_POWER_OFF      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFA35C"}));
  register_command(&LGTV_POWER_ON       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF23DC"}));
  register_command(&LGTV_POWER_TOGGLE   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF10EF"}));
  register_command(&LGTV_SOURCE         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFD02F"}));
  register_command(&LGTV_NUM_1          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF8877"}));
  register_command(&LGTV_NUM_2          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF48B7"}));
  register_command(&LGTV_NUM_3          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFC837"}));
  register_command(&LGTV_NUM_4          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF28D7"}));
  register_command(&LGTV_NUM_5          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFA857"}));
  register_command(&LGTV_NUM_6          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF6897"}));
  register_command(&LGTV_NUM_7          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFE817"}));
  register_command(&LGTV_NUM_8          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF18E7"}));
  register_command(&LGTV_NUM_9          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF9867"}));
  register_command(&LGTV_NUM_0          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF08F7"}));
  register_command(&LGTV_PRECH          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF5BA7"}));
  register_command(&LGTV_VOL_MINUS      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFC03F"}));
  register_command(&LGTV_VOL_PLUS       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF40BF"}));
  register_command(&LGTV_MUTE_TOGGLE    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF906F"}));
  //register_command(&LGTV_CHLIST         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFCA35"}));
  register_command(&LGTV_CHANNEL_UP     , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF00FF"}));
  register_command(&LGTV_CHANNEL_DOWN   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF807F"}));
  register_command(&LGTV_MENU           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFA25D"}));
  register_command(&LGTV_GUIDE          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFD52A"}));
  register_command(&LGTV_SETUP          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFC23D"}));
  register_command(&LGTV_INFO           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF55AA"}));
  register_command(&LGTV_UP             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF02FD"}));
  register_command(&LGTV_DOWN           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF827D"}));
  register_command(&LGTV_LEFT           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFE01F"}));
  register_command(&LGTV_RIGHT          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF609F"}));
  register_command(&LGTV_OK             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF22DD"}));
  register_command(&LGTV_RETURN         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF14EB"}));
  register_command(&LGTV_EXIT           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFDA25"}));
  register_command(&LGTV_KEY_A          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF4EB1"})); // Red
  register_command(&LGTV_KEY_B          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF8E71"})); // Green
  register_command(&LGTV_KEY_C          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFC639"})); // Yellow
  register_command(&LGTV_KEY_D          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF8679"})); // Blue
  //register_command(&LGTV_PSIZE          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF0EF1"}));
  //register_command(&LGTV_ADSUBT         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF8976"}));
  //register_command(&LGTV_REWIND         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFF10E"}));
  //register_command(&LGTV_PAUSE          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF5DA2"}));
  //register_command(&LGTV_FASTFORWARD    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF718E"}));
  //register_command(&LGTV_RECORD         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFBD42"}));
  //register_command(&LGTV_PLAY           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF0DF2"}));
  //register_command(&LGTV_STOP           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF8D72"}));
  //register_command(&LGTV_POWER_OFF      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFA35C"}));
  //register_command(&LGTV_POWER_ON       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF23DC"}));
  register_command(&LGTV_INPUT_HDMI_1   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF738C"}));
  register_command(&LGTV_INPUT_HDMI_2   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF33CC"}));
  register_command(&LGTV_INPUT_HDMI_3   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF9768"}));
  register_command(&LGTV_INPUT_HDMI_4   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF5BA4"}));
  //register_command(&LGTV_INPUT_COMPONENT, makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DFFD02"}));
  register_command(&LGTV_INPUT_TV       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF6B94"}));
  //register_command(&LGTV_SLEEP          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF58A7"}));
  register_command(&LGTV_3D             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF3BC4"}));
  register_command(&LGTV_HOME           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC), "0x20DF3EC1"}));

  #if (ENABLE_HUB_COMMUNICATION == 1)
    register_hub_command(&LGTV_POWER_OFF, "LG_TV", "POWER_OFF");
    register_hub_command(&LGTV_POWER_ON, "LG_TV", "POWER_ON");
    register_hub_command(&LGTV_SOURCE, "LG_TV", "SOURCE");
    register_hub_command(&LGTV_NUM_1, "LG_TV", "NUM_1");
    register_hub_command(&LGTV_NUM_2, "LG_TV", "NUM_2");
    register_hub_command(&LGTV_NUM_3, "LG_TV", "NUM_3");
    register_hub_command(&LGTV_NUM_4, "LG_TV", "NUM_4");
    register_hub_command(&LGTV_NUM_5, "LG_TV", "NUM_5");
    register_hub_command(&LGTV_NUM_6, "LG_TV", "NUM_6");
    register_hub_command(&LGTV_NUM_7, "LG_TV", "NUM_7");
    register_hub_command(&LGTV_NUM_8, "LG_TV", "NUM_8");
    register_hub_command(&LGTV_NUM_9, "LG_TV", "NUM_9");
    register_hub_command(&LGTV_NUM_0, "LG_TV", "NUM_0");
    register_hub_command(&LGTV_PRECH, "LG_TV", "PRECH");
    register_hub_command(&LGTV_VOL_MINUS, "LG_TV", "VOL_MINUS");
    register_hub_command(&LGTV_VOL_PLUS, "LG_TV", "VOL_PLUS");
    register_hub_command(&LGTV_MUTE_TOGGLE, "LG_TV", "MUTE_TOGGLE");
    register_hub_command(&LGTV_CHANNEL_UP, "LG_TV", "CHANNEL_UP");
    register_hub_command(&LGTV_CHANNEL_DOWN, "LG_TV", "CHANNEL_DOWN");
    register_hub_command(&LGTV_MENU, "LG_TV", "MENU");
    register_hub_command(&LGTV_GUIDE, "LG_TV", "GUIDE");
    register_hub_command(&LGTV_SETUP, "LG_TV", "SETUP");
    register_hub_command(&LGTV_INFO, "LG_TV", "INFO");
    register_hub_command(&LGTV_UP, "LG_TV", "UP");
    register_hub_command(&LGTV_DOWN, "LG_TV", "DOWN");
    register_hub_command(&LGTV_LEFT, "LG_TV", "LEFT");
    register_hub_command(&LGTV_RIGHT, "LG_TV", "RIGHT"); 
    register_hub_command(&LGTV_OK, "LG_TV", "OK");
    register_hub_command(&LGTV_RETURN, "LG_TV", "RETURN");
    register_hub_command(&LGTV_EXIT, "LG_TV", "EXIT");
    register_hub_command(&LGTV_KEY_A, "LG_TV", "KEY_A");
    register_hub_command(&LGTV_KEY_B, "LG_TV", "KEY_B");
    register_hub_command(&LGTV_KEY_C, "LG_TV", "KEY_C");
    register_hub_command(&LGTV_KEY_D, "LG_TV", "KEY_D");
    register_hub_command(&LGTV_INPUT_HDMI_1, "LG_TV", "INPUT_HDMI_1");
    register_hub_command(&LGTV_INPUT_HDMI_2, "LG_TV", "INPUT_HDMI_2");
    register_hub_command(&LGTV_INPUT_HDMI_3, "LG_TV", "INPUT_HDMI_3");
    register_hub_command(&LGTV_INPUT_HDMI_4, "LG_TV", "INPUT_HDMI_4");
    register_hub_command(&LGTV_INPUT_TV, "LG_TV", "INPUT_TV"); 
    register_hub_command(&LGTV_3D, "LG_TV", "3D");
    register_hub_command(&LGTV_HOME, "LG_TV", "HOME"); 
  #endif
}