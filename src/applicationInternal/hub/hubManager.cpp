#include "hubManager.h"
#include "hubTransportBase.h"
#include "protoCodec.h"
#include "applicationInternal/omote_log.h"
#include "applicationInternal/hardware/arduinoLayer.h"

#if (ENABLE_HUB_COMMUNICATION == 1)
#include "espNowHubTransport.h"
#endif

#if (ENABLE_WIFI_AND_MQTT == 1 && ENABLE_HUB_COMMUNICATION == 2)
#include "mqttHubTransport.h"
#endif

#if (ENABLE_WIFI_AND_MQTT == 1 && ENABLE_HUB_COMMUNICATION == 3)
#include "websocketHubTransport.h"
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
    
    case HubTransport::WEBSOCKET:
      #if (ENABLE_WIFI_AND_MQTT != 1 || ENABLE_HUB_COMMUNICATION != 3)
      omote_log_e("WebSocket transport is not available in this build\n");
      return nullptr;
      #else
      return std::unique_ptr<HubTransportBase>(new WebSocketHubTransport());
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
  
  if (!isStateSyncRequested()) {
    return;
  }
  
  syncState();
}

bool HubManager::sendRemoteEvent(const omote_RemoteEvent& event) {
  if (!activeTransport) {
    omote_log_w("Cannot send message: no hub transport initialized\n");
    return false;
  }
  
  if (!activeTransport->isReady()) {
    omote_log_w("Cannot send message: hub transport not ready\n");
    return false;
  }
  
  return activeTransport->sendRemoteEvent(event);
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


void HubManager::requestStateSync() {
  if (!stateSyncRequested) {
    stateSyncRequested = true;
    stateSyncStartTime = millis();
    omote_log_i("State sync requested\n");
  }
}

bool HubManager::isStateSyncRequested() const {
  return stateSyncRequested;
}

void HubManager::resetStateSyncTimer() {
  stateSyncStartTime = millis();
}

bool HubManager::isStateSyncTimerReady() const {
  unsigned long currentTime = millis();
  return (currentTime - stateSyncStartTime) >= STATE_SYNC_DELAY;
}

void HubManager::syncState() {
  if (!isStateSyncTimerReady()) {
    return;
  }
  
  if (!isReady()) {
    omote_log_d("Transport not ready for state sync, will retry\n");
    resetStateSyncTimer();
    return;
  }
  
  omote_RemoteEvent event = Hub::ProtoCodec::createRemoteEvent(
    "HUB",
    omote_OmoteCommand_SYNC_STATE,
    omote_OmoteCommandType_SHORT
  );
  
  bool sendSuccess = sendRemoteEvent(event);
  
  if (!sendSuccess) {
    omote_log_w("Failed to send state sync request, will retry\n");
    resetStateSyncTimer();
    return;
  }
  
  stateSyncRequested = false;
  omote_log_i("State sync request sent successfully\n");
}

void HubManager::setMessageHandler(std::function<void(const omote_CommandResult&)> handler) {
  messageHandler = handler;
}

void HubManager::handleIncomingCommandResult(const omote_CommandResult& result) {
  if (messageHandler) {
    messageHandler(result);
  }
}
