#include "mock_hub_simulator.h"
#include <iostream>
#include <thread>

static EspNowMessageCallback messageCallback = nullptr;
static bool simulatorRunning = false;
static std::thread simulatorThread;

void startMockHubSimulator(EspNowMessageCallback callback) {
    messageCallback = callback;
    simulatorRunning = false;  // Disable for now

    std::cout << "Mock Hub Simulator (binary message mode):" << std::endl;
    std::cout << "   NOTE: Mock hub simulator not yet implemented for binary hub messages." << std::endl;
    std::cout << "   The simulator requires protocol integration to encode/decode messages." << std::endl;
    std::cout << "   For now, use a real hub instance for testing hub communication." << std::endl;
}

void stopMockHubSimulator() {
    simulatorRunning = false;
    if (simulatorThread.joinable()) {
        simulatorThread.join();
    }
    std::cout << "Mock Hub Simulator stopped!" << std::endl;
}

void handleMockHubCommand(const uint8_t* data, size_t len) {
    std::cout << "Mock Hub: Received binary command (" << len << " bytes)" << std::endl;
    std::cout << "   NOTE: Message decoding not yet implemented in simulator." << std::endl;
    std::cout << "   Use a real hub instance to test hub messages." << std::endl;
}
