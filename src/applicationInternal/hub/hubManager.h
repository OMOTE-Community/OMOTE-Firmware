#pragma once

#include "hubTransportBase.h"
#include "remote_messages.pb.h"
#include <memory>
#include <functional>

class HubManager {
private:
  std::unique_ptr<HubTransportBase> activeTransport;
  HubTransport currentTransport;
  
  // State sync state
  bool stateSyncRequested = false;
  unsigned long stateSyncStartTime = 0;
  static const unsigned long STATE_SYNC_DELAY = 100; // ms

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