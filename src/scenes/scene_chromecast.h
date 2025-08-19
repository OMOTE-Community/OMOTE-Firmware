#pragma once

#include <string>
#include <stdint.h>

extern uint16_t SCENE_CHROMECAST;
// FORCE sends the start sequence again even if scene is already active
extern uint16_t SCENE_CHROMECAST_FORCE;

extern std::string scene_name_chromecast;
void register_scene_chromecast_commands(void);
void register_scene_chromecast(void);
