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
  clearQueue();
  linkPhase = LinkPhase::WAKE_WINDOW;
  return true;
}

void HubManager::process() {
  if (!activeTransport) {
    return;
  }
  
  activeTransport->process();

  if (isReady()) {
    flushQueue();
  }

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
    return enqueueEvent(event);
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
  clearQueue();
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

bool HubManager::enqueueEvent(const omote_RemoteEvent& event) {
  unsigned long ttl = (linkPhase == LinkPhase::WAKE_WINDOW) ? WAKE_TTL_MS : RUNTIME_TTL_MS;

  if (queueCount == QUEUE_MAX) {
    if (linkPhase == LinkPhase::WAKE_WINDOW) {
      // Preserve the oldest entry (the wake press) by refusing the newest.
      omote_log_w("Outbound queue full in wake window, dropping newest event\n");
      return false;
    }
    // Favor recent intent: make room by dropping the oldest.
    omote_log_w("Outbound queue full, dropping oldest event\n");
    popQueueHead();
  }

  eventQueue[queueTail].event = event;
  eventQueue[queueTail].enqueuedTime = millis();
  eventQueue[queueTail].ttlMs = ttl;
  queueTail = (queueTail + 1) % QUEUE_MAX;
  queueCount++;

  omote_log_i("Queued event, ttl=%lu\n", ttl);
  return true;
}

void HubManager::flushQueue() {
  // The link is ready, so the wake window is over. Advance once even on an
  // empty queue, so a later runtime reconnect flap uses the short runtime TTL.
  if (linkPhase == LinkPhase::WAKE_WINDOW) {
    linkPhase = LinkPhase::STEADY;
  }

  uint8_t flushed = 0;
  while (queueCount > 0) {
    QueuedEvent& head = eventQueue[queueHead];

    // Unsigned subtraction is rollover-safe across millis() wraparound.
    if ((millis() - head.enqueuedTime) >= head.ttlMs) {
      omote_log_w("Dropped expired queued event\n");
      popQueueHead();
      continue;
    }

    if (!activeTransport->sendRemoteEvent(head.event)) {
      // Flaky send: leave the event at the head and retry on the next tick.
      omote_log_w("Queued event send failed, will retry next tick\n");
      break;
    }

    popQueueHead();
    flushed++;
  }

  if (flushed > 0) {
    omote_log_i("Flushed %u queued events\n", (unsigned)flushed);
  }
}

void HubManager::popQueueHead() {
  queueHead = (queueHead + 1) % QUEUE_MAX;
  queueCount--;
}

void HubManager::clearQueue() {
  queueHead = 0;
  queueTail = 0;
  queueCount = 0;
}

void HubManager::setMessageHandler(std::function<void(const omote_CommandResult&)> handler) {
  messageHandler = handler;
}

void HubManager::handleIncomingCommandResult(const omote_CommandResult& result) {
  if (messageHandler) {
    messageHandler(result);
  }
}
