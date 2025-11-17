#pragma once

#include <string>
#include <cstdint>
#include <cstddef>

// Function to get MAC address for ESP32
std::string getMACaddress();

void init_espnow_HAL(void);
void espnow_loop_HAL();
bool publishEspNowMessageProto_HAL(const uint8_t* data, size_t len);
void espnow_shutdown_HAL();

typedef void (*tAnnounceEspNowMessageProto_cb)(const uint8_t* data, size_t len);

void set_announceEspNowMessageProto_cb_HAL(tAnnounceEspNowMessageProto_cb pAnnounceEspNowMessageProto_cb); 