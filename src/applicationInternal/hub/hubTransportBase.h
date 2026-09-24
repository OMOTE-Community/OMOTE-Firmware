#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Define the hub transport types
enum class HubTransport {
  ESPNOW,
  MQTT
};

// Abstract interface for hub communication transports
class HubTransportBase {
public:
  virtual ~HubTransportBase() = default;
  
  // Initialize the hub communication transport
  virtual bool init() = 0;
  
  // Process hub communication tasks (called in loop)
  virtual void process() = 0;
  
  // Send a message to the hub
  virtual bool sendMessage(const json& payload) = 0;
  
  // Check if connected/ready
  virtual bool isReady() = 0;
  
  // Shutdown the communication
  virtual void shutdown() = 0;
}; 