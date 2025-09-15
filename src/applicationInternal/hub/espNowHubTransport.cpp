#include "espNowHubTransport.h"
#include "hubManager.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

// Forward declaration for the internal callback
void hubMessageReceived_cb(json payload);

#if (ENABLE_HUB_COMMUNICATION == 1)
EspNowHubTransport::EspNowHubTransport() = default;

EspNowHubTransport::~EspNowHubTransport() {
  shutdown();
}

bool EspNowHubTransport::init() {
  set_espnow_message_callback(&hubMessageReceived_cb);
  init_espnow();
  return true;
}

void EspNowHubTransport::process() {
  espnow_loop();
}

bool EspNowHubTransport::sendMessage(const json& payload) {
  // Extract device and command for logging
  std::string device = payload["device"];
  std::string command = payload["command"];
  
  omote_log_d("ESP-NOW: Sending message for device %s, command %s\n", device.c_str(), command.c_str());
  return publishEspNowMessage(payload);
}

bool EspNowHubTransport::isReady() {
  // ESP-NOW is always ready once initialized
  return true;
}

void EspNowHubTransport::shutdown() {
  espnow_shutdown();
}

void hubMessageReceived_cb(json payload) {
  auto& hubManager = HubManager::getInstance();
  hubManager.handleIncomingMessage(payload);
}
#endif 