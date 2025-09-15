#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Callback type definition (same as ESP-NOW)
typedef void (*EspNowMessageCallback)(json);

// Mock Hub Simulator functions
void startMockHubSimulator(EspNowMessageCallback callback);
void stopMockHubSimulator();
void handleMockHubCommand(const json& command);
