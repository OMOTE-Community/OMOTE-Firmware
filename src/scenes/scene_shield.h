#pragma once

#include <string>
#include <stdint.h>

extern uint16_t SCENE_SHIELD;
// FORCE sends the start sequence again even if scene is already active
extern uint16_t SCENE_SHIELD_FORCE;

extern std::string scene_name_shield;
void register_scene_shield(void);

