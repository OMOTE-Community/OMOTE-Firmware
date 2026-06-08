#pragma once

#include "hubTransportBase.h"
#include "remote_messages.pb.h"
#include <memory>
#include <functional>
#include <cstdint>

class HubManager {
private:
  std::unique_ptr<HubTransportBase> activeTransport;
  HubTransport currentTransport;

  bool stateSyncRequested = false;
  unsigned long stateSyncStartTime = 0;
  static const unsigned long STATE_SYNC_DELAY = 100; // ms

  struct QueuedEvent {
    omote_RemoteEvent event;
    unsigned long enqueuedTime;
    unsigned long ttlMs;
  };

  enum class LinkPhase { WAKE_WINDOW, STEADY };
  static const uint8_t QUEUE_MAX = 12;
  // Must exceed the WebSocket reconnect interval.
  static const unsigned long WAKE_TTL_MS = 6000;
  static const unsigned long RUNTIME_TTL_MS = 1500;
  QueuedEvent eventQueue[QUEUE_MAX];
  uint8_t queueHead = 0;
  uint8_t queueTail = 0;
  uint8_t queueCount = 0;
  LinkPhase linkPhase = LinkPhase::WAKE_WINDOW;

  std::function<void(const omote_CommandResult&)> messageHandler;

  HubManager();

  static std::unique_ptr<HubTransportBase> createTransport(HubTransport transport);

  bool isStateSyncTimerReady() const;
  void syncState();
  void resetStateSyncTimer();

  bool hasPendingOutboundEvents() const;
  bool shouldQueueRemoteEvent() const;
  bool sendImmediatelyOrQueueForRetry(const omote_RemoteEvent& event);
  bool enqueueEvent(const omote_RemoteEvent& event);
  void flushQueue();
  void finishWakeWindow();
  bool isQueuedEventExpired(const QueuedEvent& event) const;
  void popQueueHead();
  void clearQueue();

public:
  static HubManager& getInstance();
  
  HubManager(const HubManager&) = delete;
  HubManager& operator=(const HubManager&) = delete;
  
  ~HubManager() = default;
  
  bool init(HubTransport transport);
  
  void process();
  
  bool sendRemoteEvent(const omote_RemoteEvent& event);
  
  bool isReady() const;
  
  void shutdown();
  
  bool isInitialized() const;
  
  HubTransport getCurrentTransport() const;
  
  void requestStateSync();
  bool isStateSyncRequested() const;
  
  void setMessageHandler(std::function<void(const omote_CommandResult&)> handler);
  void handleIncomingCommandResult(const omote_CommandResult& result);
};
