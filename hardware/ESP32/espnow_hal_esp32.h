#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void init_espnow_HAL(void);
void espnow_loop_HAL();
bool publishEspNowMessage_HAL(json payload);
void espnow_shutdown_HAL();

typedef void (*tAnnounceEspNowMessage_cb)(json payload);
void set_announceEspNowMessage_cb_HAL(tAnnounceEspNowMessage_cb pAnnounceEspNowMessage_cb); 