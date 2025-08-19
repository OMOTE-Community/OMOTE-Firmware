#pragma once

#include <string>
#include <stdint.h>

extern uint16_t SCENE_ALLOFF;
// FORCE sends the start sequence again even if scene is already active
extern uint16_t SCENE_ALLOFF_FORCE;

extern std::string scene_name_allOff;
void register_scene_allOff_commands(void);
void register_scene_allOff(void);
