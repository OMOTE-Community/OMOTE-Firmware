#pragma once

#include <cstdint>
#include <cstddef>

// Callback type definition for ESP-NOW binary messages
typedef void (*EspNowMessageCallback)(const uint8_t* data, size_t len);

// Mock Hub Simulator functions
void startMockHubSimulator(EspNowMessageCallback callback);
void stopMockHubSimulator();
void handleMockHubCommand(const uint8_t* data, size_t len);
