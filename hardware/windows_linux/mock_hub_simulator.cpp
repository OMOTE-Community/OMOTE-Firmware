#include "mock_hub_simulator.h"
#include <iostream>
#include <thread>

static EspNowMessageProtoCallback messageCallback = nullptr;
static bool simulatorRunning = false;
static std::thread simulatorThread;

void startMockHubSimulatorProto(EspNowMessageProtoCallback callback) {
    messageCallback = callback;
    simulatorRunning = false;  // Disable for now

    std::cout << "Mock Hub Simulator (Protobuf mode):" << std::endl;
    std::cout << "   NOTE: Mock hub simulator not yet implemented for protobuf." << std::endl;
    std::cout << "   The simulator requires NanoPB integration to encode/decode messages." << std::endl;
    std::cout << "   For now, use a real hub instance for testing protobuf communication." << std::endl;
}

void stopMockHubSimulator() {
    simulatorRunning = false;
    if (simulatorThread.joinable()) {
        simulatorThread.join();
    }
    std::cout << "Mock Hub Simulator stopped!" << std::endl;
}

void handleMockHubCommandProto(const uint8_t* data, size_t len) {
    std::cout << "Mock Hub: Received protobuf command (" << len << " bytes)" << std::endl;
    std::cout << "   NOTE: Protobuf decoding not yet implemented in simulator." << std::endl;
    std::cout << "   Use a real hub instance to test protobuf messages." << std::endl;
}
