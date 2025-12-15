#pragma once

#include "hubTransportBase.h"
#include <nlohmann/json.hpp>
#include <memory>

using json = nlohmann::json;

class HubManager {
private:
  std::unique_ptr<HubTransportBase>activeTransport;
  HubTransport currentTransport;

  // Private constructor for singleton
  HubManager();
  
  // Factory method for creating transports
  static std::unique_ptr<HubTransportBase> createTransport(HubTransport transport);

public:
  static HubManager& getInstance();
  
  // Delete copy constructor and assignment operator
  HubManager(const HubManager&) = delete;
  HubManager& operator=(const HubManager&) = delete;
  
  ~HubManager() = default;
  
  // Initialize with the preferred transport
  bool init(HubTransport transport);
  
  void process();
  
  bool sendMessage(const json& payload);
  
  bool isReady() const;
  
  void shutdown();
  
  bool isInitialized() const;
  
  HubTransport getCurrentTransport() const;
}; 