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

  // State sync state
  bool stateSyncRequested = false;
  unsigned long stateSyncStartTime = 0;
  static const unsigned long STATE_SYNC_DELAY = 100; // ms

  // Outbound queue: events that arrive before the transport is ready (e.g. the
  // press that wakes the remote, or presses during a WebSocket reconnect) are
  // held here and flushed on the next ready tick instead of being dropped.
  struct QueuedEvent {
    omote_RemoteEvent event;
    unsigned long enqueuedTime;
    unsigned long ttlMs;
  };
  // WAKE_WINDOW (the just-woken state) gives the first connect a generous TTL;
  // once the link has been ready once we move to STEADY and runtime presses
  // expire quickly so a stale intent is not delivered seconds late.
  enum class LinkPhase { WAKE_WINDOW, STEADY };
  static const uint8_t QUEUE_MAX = 12;
  // WAKE_TTL_MS exceeds the WebSocket reconnect interval (5000 ms) so the wake
  // press survives until the first post-wake connect.
  static const unsigned long WAKE_TTL_MS = 6000;
  static const unsigned long RUNTIME_TTL_MS = 1500;
  QueuedEvent eventQueue[QUEUE_MAX];
  uint8_t queueHead = 0;
  uint8_t queueTail = 0;
  uint8_t queueCount = 0;
  LinkPhase linkPhase = LinkPhase::WAKE_WINDOW;

  // Message handling
  std::function<void(const omote_CommandResult&)> messageHandler;

  // Private constructor for singleton
  HubManager();

  // Factory method for creating transports
  static std::unique_ptr<HubTransportBase> createTransport(HubTransport transport);

  // State sync helpers
  bool isStateSyncTimerReady() const;
  void syncState();
  void resetStateSyncTimer();

  // Outbound queue helpers
  bool enqueueEvent(const omote_RemoteEvent& event);
  void flushQueue();
  void popQueueHead();
  void clearQueue();

public:
  static HubManager& getInstance();
  
  // Delete copy constructor and assignment operator
  HubManager(const HubManager&) = delete;
  HubManager& operator=(const HubManager&) = delete;
  
  ~HubManager() = default;
  
  bool init(HubTransport transport);
  
  void process();
  
  // Send a RemoteEvent protobuf message
  bool sendRemoteEvent(const omote_RemoteEvent& event);
  
  bool isReady() const;
  
  void shutdown();
  
  bool isInitialized() const;
  
  HubTransport getCurrentTransport() const;
  
  // State sync interface
  void requestStateSync();
  bool isStateSyncRequested() const;
  
  // Message handling interface
  void setMessageHandler(std::function<void(const omote_CommandResult&)> handler);
  void handleIncomingCommandResult(const omote_CommandResult& result);
}; 