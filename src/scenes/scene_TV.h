#pragma once

#include <string>
#include <stdint.h>

extern uint16_t SCENE_TV;
// FORCE sends the start sequence again even if scene is already active
extern uint16_t SCENE_TV_FORCE;

extern std::string scene_name_TV;
void register_scene_TV_commands(void);
void register_scene_TV(void);
