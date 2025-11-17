#pragma once

#include <cstdint>
#include <cstddef>

// Callback type definition for protobuf messages
typedef void (*EspNowMessageProtoCallback)(const uint8_t* data, size_t len);

// Mock Hub Simulator functions
void startMockHubSimulatorProto(EspNowMessageProtoCallback callback);
void stopMockHubSimulator();
void handleMockHubCommandProto(const uint8_t* data, size_t len);
