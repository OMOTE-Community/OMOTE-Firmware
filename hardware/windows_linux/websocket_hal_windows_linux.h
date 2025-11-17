#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void init_websocket_HAL(const char* hub_url);
void websocket_loop_HAL();
bool publishWebSocketMessage_HAL(json payload);
void websocket_shutdown_HAL();
bool websocket_is_connected_HAL();
const char* get_websocket_hub_url_HAL();

typedef void (*tAnnounceWebSocketMessage_cb)(json payload);
void set_announceWebSocketMessage_cb_HAL(tAnnounceWebSocketMessage_cb pAnnounceWebSocketMessage_cb);

