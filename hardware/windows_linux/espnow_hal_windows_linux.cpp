#include "espnow_hal_windows_linux.h"

// Callback function pointer
static EspNowMessageCallback espNowMessageCallback = nullptr;

void set_announceEspNowMessage_cb_HAL(EspNowMessageCallback callback) {
  espNowMessageCallback = callback;
  printf("ESP-NOW callback registered (stub implementation)");
}

void init_espnow_HAL() {
  printf("ESP-NOW initialized (stub implementation)");
}

void espnow_loop_HAL() {
  // Nothing to do in the stub implementation
}

bool publishEspNowMessage_HAL(json payload) {
  printf("ESP-NOW message published (stub implementation): %s", payload.dump().c_str());
  return true; // Always return success in the stub implementation
}

void espnow_shutdown_HAL() {
  printf("ESP-NOW shutdown (stub implementation)");
}