#pragma once

#include "hubTransportBase.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <functional>

using json = nlohmann::json;

class HubManager {
private:
  std::unique_ptr<HubTransportBase> activeTransport;
  HubTransport currentTransport;
  
  // State sync state
  bool stateSyncRequested = false;
  unsigned long stateSyncStartTime = 0;
  static const unsigned long STATE_SYNC_DELAY = 100; // ms

  // Message handling
  std::function<void(const json&)> messageHandler;

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
  
  bool sendMessage(const json& payload);
  
  bool isReady() const;
  
  void shutdown();
  
  bool isInitialized() const;
  
  HubTransport getCurrentTransport() const;
  
  // State sync interface
  void requestStateSync();
  bool isStateSyncRequested() const;
  
  // Message handling interface
  void setMessageHandler(std::function<void(const json&)> handler);
  void handleIncomingMessage(const json& payload);
}; 