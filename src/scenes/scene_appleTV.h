#pragma once

#include <string>
#include <stdint.h>

extern uint16_t SCENE_APPLETV;
// FORCE sends the start sequence again even if scene is already active
extern uint16_t SCENE_APPLETV_FORCE;

extern std::string scene_name_appleTV;
void register_scene_appleTV_commands(void);
void register_scene_appleTV(void);
