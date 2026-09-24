#include "hubManager.h"
#include "hubTransportBase.h"
#include "applicationInternal/omote_log.h"
#include "applicationInternal/hardware/arduinoLayer.h"

#if (ENABLE_HUB_COMMUNICATION == 1)
#include "espNowHubTransport.h"
#endif

#if (ENABLE_WIFI_AND_MQTT == 1 && ENABLE_HUB_COMMUNICATION == 2)
#include "mqttHubTransport.h"
#endif

HubManager& HubManager::getInstance() {
  static HubManager instance;
  return instance;
}

HubManager::HubManager() : currentTransport(HubTransport::ESPNOW) {
  // Initialize with default transport type only
}

std::unique_ptr<HubTransportBase> HubManager::createTransport(HubTransport transport) {
  switch (transport) {
    case HubTransport::ESPNOW:
      #if (ENABLE_HUB_COMMUNICATION != 1)
      omote_log_e("ESP-NOW transport is not available in this build\n");
      return nullptr;
      #else
      return std::unique_ptr<HubTransportBase>(new EspNowHubTransport());
      #endif
    
    case HubTransport::MQTT:
      #if (ENABLE_WIFI_AND_MQTT != 1 || ENABLE_HUB_COMMUNICATION != 2)
      omote_log_e("MQTT transport is not available in this build\n");
      return nullptr;
      #else
      return std::unique_ptr<HubTransportBase>(new MqttHubTransport());
      #endif
    
    default:
      omote_log_e("Invalid hub transport selected\n");
      return nullptr;
  }
}

bool HubManager::init(HubTransport transport) {
  // Store the selected transport type
  currentTransport = transport;

  // Create the new transport
  auto newTransport = createTransport(transport);
  if (!newTransport) {
    return false;
  }

  // Initialize the transport
  if (!newTransport->init()) {
    omote_log_e("Failed to initialize hub transport\n");
    return false;
  }

  // If everything succeeded, replace the active transport
  activeTransport = std::move(newTransport);
  return true;
}

void HubManager::process() {
  if (activeTransport) {
    activeTransport->process();
  }
}

bool HubManager::sendMessage(const json& payload) {
  if (!activeTransport) {
    omote_log_w("Cannot send message: no hub transport initialized\n");
    return false;
  }
  
  if (!activeTransport->isReady()) {
    omote_log_w("Cannot send message: hub transport not ready\n");
    return false;
  }
  
  return activeTransport->sendMessage(payload);
}

bool HubManager::isInitialized() const {
  return activeTransport != nullptr;
}

bool HubManager::isReady() const {
  return activeTransport && activeTransport->isReady();
}

void HubManager::shutdown() {
  if (activeTransport) {
    activeTransport->shutdown();
    activeTransport.reset();
  }
}

HubTransport HubManager::getCurrentTransport() const {
  return currentTransport;
} 