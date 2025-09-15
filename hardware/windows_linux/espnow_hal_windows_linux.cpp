#include "espnow_hal_windows_linux.h"
#include "mock_hub_simulator.h"
#include <iostream>

// Callback function pointer
static EspNowMessageCallback espNowMessageCallback = nullptr;

void set_announceEspNowMessage_cb_HAL(EspNowMessageCallback callback) {
  espNowMessageCallback = callback;
  std::cout << "ESP-NOW callback registered (simulator with mock hub)" << std::endl;
}

void init_espnow_HAL() {
  std::cout << "ESP-NOW initialized (simulator with mock hub)" << std::endl;
  
  // Start the mock hub simulator
  if (espNowMessageCallback) {
    startMockHubSimulator(espNowMessageCallback);
  }
}

void espnow_loop_HAL() {
  // Nothing to do in the simulator - mock hub runs in background thread
}

bool publishEspNowMessage_HAL(json payload) {
  std::cout << "ESP-NOW message sent to mock hub: " << payload.dump() << std::endl;
  
  // Forward the command to the mock hub for processing
  handleMockHubCommand(payload);
  
  return true; // Always return success in the simulator
}

void espnow_shutdown_HAL() {
  std::cout << "ESP-NOW shutdown (stopping mock hub)" << std::endl;
  stopMockHubSimulator();
}