#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Callback type definition
typedef void (*EspNowMessageCallback)(json);

// Function declarations
void set_announceEspNowMessage_cb_HAL(EspNowMessageCallback callback);
void init_espnow_HAL();
void espnow_loop_HAL();
bool publishEspNowMessage_HAL(json payload);
void espnow_shutdown_HAL();