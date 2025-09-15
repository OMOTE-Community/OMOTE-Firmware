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
  
  // Metadata polling state
  bool metadataPollRequested = false;
  unsigned long metadataPollStartTime = 0;
  static const unsigned long METADATA_POLL_DELAY = 100; // ms

  // Message handling
  std::function<void(const json&)> messageHandler;

  // Private constructor for singleton
  HubManager();
  
  // Factory method for creating transports
  static std::unique_ptr<HubTransportBase> createTransport(HubTransport transport);
  
  // Metadata polling helpers
  bool isMetadataPollTimerReady() const;
  void pollMetadata();
  void resetMetadataPollTimer();

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
  
  // Metadata polling interface
  void requestMetadataPolling();
  bool isMetadataPollRequested() const;
  
  // Message handling interface
  void setMessageHandler(std::function<void(const json&)> handler);
  void handleIncomingMessage(const json& payload);
}; 