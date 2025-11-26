#include <map>
#include "scenes/scene_shield.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/commandHandler.h"
#include "devices/mediaPlayer/device_shield/device_shield.h"
#include "devices/AVreceiver/device_denonAvr/device_denonAvr.h"
#include "devices/TV/device_lgTV/device_lgTV.h"
#include "devices/misc/device_hub_helper.h"
#include "guis/gui_numpad.h"

uint16_t SCENE_SHIELD;
uint16_t SCENE_SHIELD_FORCE;

std::map<char, repeatModes> key_repeatModes_shield;
std::map<char, uint16_t> key_commands_short_shield;
std::map<char, uint16_t> key_commands_long_shield;

void scene_setKeys_shield() {
  key_repeatModes_shield = {
      {KEY_UP, SHORT_REPEATED},           {KEY_LEFT, SHORT_REPEATED},
      {KEY_OK, SHORTorLONG},              {KEY_RIGHT, SHORT_REPEATED},
      {KEY_DOWN, SHORT_REPEATED},         {KEY_VOLDO, SHORT_REPEATED},
      {KEY_VOLUP, SHORT_REPEATED},
  };

  key_commands_short_shield = {
      {KEY_STOP, SHIELD_STOP},        {KEY_REWI, SHIELD_REVERSE},
      {KEY_PLAY, SHIELD_PLAY},        {KEY_FORW, SHIELD_FORWARD},
      {KEY_CONF, SHIELD_SHIELD},      {KEY_INFO, COMMAND_UNKNOWN},
      {KEY_UP, SHIELD_UP},            {KEY_LEFT, SHIELD_LEFT},
      {KEY_OK, SHIELD_OK},            {KEY_RIGHT, SHIELD_RIGHT},
      {KEY_DOWN, SHIELD_DOWN},        {KEY_BACK, SHIELD_EXIT},
      {KEY_CHUP, COMMAND_UNKNOWN},    {KEY_REC, COMMAND_UNKNOWN},
      {KEY_CHDOW, COMMAND_UNKNOWN},   {KEY_VOLDO, DENONAVR_VOL_MINUS},
      {KEY_VOLUP, DENONAVR_VOL_PLUS}, {KEY_MUTE, DENONAVR_VOL_MUTE},
  };

  key_commands_long_shield = {
      {KEY_OK, SHIELD_MENU},
  };
}

void scene_start_sequence_shield(void) {
  #if (ENABLE_HUB_COMMUNICATION > 0)
  execute_hub_command(SHIELD_POWER_ON);
  execute_hub_command(DENONAVR_POWER_ON);
  execute_hub_command(LGTV_POWER_ON);
  #endif
}

void scene_end_sequence_shield(void) {
  // Shield doesn't have a direct power off, but we can go to home
  #if (ENABLE_HUB_COMMUNICATION > 0)
  execute_hub_command(SHIELD_SHIELD);
  #endif
}

std::string scene_name_shield = "Shield";
t_gui_list scene_shield_gui_list = {tabName_numpad};

void register_scene_shield(void) {
  register_command(&SCENE_SHIELD,       makeCommandData(SCENE, {scene_name_shield}));
  register_command(&SCENE_SHIELD_FORCE, makeCommandData(SCENE, {scene_name_shield, "FORCE"}));

  register_scene(
    scene_name_shield,
    & scene_setKeys_shield,
    & scene_start_sequence_shield,
    & scene_end_sequence_shield,
    & key_repeatModes_shield,
    & key_commands_short_shield,
    & key_commands_long_shield,
    & scene_shield_gui_list,
    SCENE_SHIELD);
}
