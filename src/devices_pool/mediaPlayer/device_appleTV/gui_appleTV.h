#pragma once

#include <lvgl.h>
#include <string>

const char * const tabName_appleTV = "Apple TV";
void register_gui_appleTV(void);
void update_appleTV_metadata(const std::string& title, const std::string& artist, 
                             const std::string& album, const std::string& state,
                             int duration = 0, int position = 0);
