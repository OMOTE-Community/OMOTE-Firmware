#include "espNowHubTransport.h"
#include "hubManager.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"
#include <WiFi.h>

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
  // Check if this is a discovery message
  if (payload.contains("type")) {
    std::string messageType = payload["type"];
    
    if (messageType == "discovery_request") {
      // Send discovery response with OMOTE MAC
      json response = {
        {"type", "discovery_response"},
        {"hub_mac", payload["hub_mac"]},
        {"omote_mac", WiFi.macAddress().c_str()}
      };
      
      omote_log_i("Sending discovery response to hub\n");
      publishEspNowMessage(response);
      return;
    }
    else if (messageType == "discovery_response") {
      omote_log_i("Received discovery response from hub\n");
      // Could store hub MAC here if needed for future use
      return;
    }
  }
  
  // Forward non-discovery messages to hub manager
  auto& hubManager = HubManager::getInstance();
  hubManager.handleIncomingMessage(payload);
}
#endif 