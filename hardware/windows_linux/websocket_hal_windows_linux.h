#pragma once

#include <cstdint>
#include <cstddef>

void init_websocket_HAL(const char* hub_url);
void websocket_loop_HAL();
bool publishWebSocketMessageProto_HAL(const uint8_t* data, size_t len);
void websocket_shutdown_HAL();
bool websocket_is_connected_HAL();
const char* get_websocket_hub_url_HAL();
unsigned long get_websocket_reconnect_interval_ms_HAL();

typedef void (*tAnnounceWebSocketMessageProto_cb)(const uint8_t* data, size_t len);

void set_announceWebSocketMessageProto_cb_HAL(tAnnounceWebSocketMessageProto_cb pAnnounceWebSocketMessageProto_cb);
