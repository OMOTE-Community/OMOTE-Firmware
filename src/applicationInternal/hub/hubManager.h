#pragma once

#include "hubTransportBase.h"
#include "hubOutboundQueue.h"
#include "remote_messages.pb.h"
#include <memory>
#include <functional>

class HubManager {
private:
  std::unique_ptr<HubTransportBase> activeTransport;
  HubTransport currentTransport;

  bool stateSyncRequested = false;
  unsigned long stateSyncStartTime = 0;
  static const unsigned long STATE_SYNC_DELAY = 100; // ms
  static const unsigned long RUNTIME_TTL_MS = 1500;
  HubOutboundQueue outboundQueue;

  std::function<void(const omote_CommandResult&)> messageHandler;

  HubManager();

  static std::unique_ptr<HubTransportBase> createTransport(HubTransport transport);

  bool isStateSyncTimerReady() const;
  void syncState();
  void resetStateSyncTimer();

  bool hasPendingOutboundEvents() const;
  bool shouldQueueRemoteEvent() const;
  bool sendImmediatelyOrQueueForRetry(const omote_RemoteEvent& event);
  unsigned long currentQueueTtlMs() const;
  bool enqueueEvent(const omote_RemoteEvent& event);
  void flushQueue();
  void clearQueue();

public:
  static HubManager& getInstance();
  
  HubManager(const HubManager&) = delete;
  HubManager& operator=(const HubManager&) = delete;
  
  ~HubManager() = default;
  
  bool init(HubTransport transport);
  bool init(std::unique_ptr<HubTransportBase> transport);
  
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
