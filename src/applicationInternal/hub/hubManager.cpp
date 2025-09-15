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
  currentTransport = transport;

  auto newTransport = createTransport(transport);
  if (!newTransport) {
    return false;
  }

  if (!newTransport->init()) {
    omote_log_e("Failed to initialize hub transport\n");
    return false;
  }

  activeTransport = std::move(newTransport);
  return true;
}

void HubManager::process() {
  if (!activeTransport) {
    return;
  }
  
  activeTransport->process();
  
  if (!isMetadataPollRequested()) {
    return;
  }
  
  pollMetadata();
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

void HubManager::requestMetadataPolling() {
  if (!metadataPollRequested) {
    metadataPollRequested = true;
    metadataPollStartTime = millis();
    omote_log_i("Metadata polling requested\n");
  }
}

bool HubManager::isMetadataPollRequested() const {
  return metadataPollRequested;
}

void HubManager::resetMetadataPollTimer() {
  metadataPollStartTime = millis();
}

bool HubManager::isMetadataPollTimerReady() const {
  unsigned long currentTime = millis();
  return (currentTime - metadataPollStartTime) >= METADATA_POLL_DELAY;
}

void HubManager::pollMetadata() {
  if (!isMetadataPollTimerReady()) {
    return;
  }
  
  if (!isReady()) {
    omote_log_d("Transport not ready for metadata polling, will retry\n");
    resetMetadataPollTimer();
    return;
  }
  
  json payload = {
    {"device", "APPLE_TV"},
    {"command", "GET_METADATA"},
    {"type", "SHORT"}
  };
  
  bool sendSuccess = sendMessage(payload);
  
  if (!sendSuccess) {
    omote_log_w("Failed to send metadata polling request, will retry\n");
    resetMetadataPollTimer();
    return;
  }
  
  metadataPollRequested = false;
  omote_log_i("Metadata polling request sent successfully\n");
}

void HubManager::setMessageHandler(std::function<void(const json&)> handler) {
  messageHandler = handler;
}

void HubManager::handleIncomingMessage(const json& payload) {
  if (messageHandler) {
    messageHandler(payload);
  }
}
