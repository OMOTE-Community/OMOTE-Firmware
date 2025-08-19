#pragma once

#include <string>
#include <stdint.h>

extern uint16_t SCENE_FIRETV;
// FORCE sends the start sequence again even if scene is already active
extern uint16_t SCENE_FIRETV_FORCE;

extern std::string scene_name_fireTV;
void register_scene_fireTV_commands(void);
void register_scene_fireTV(void);
