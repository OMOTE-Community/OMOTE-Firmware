#pragma once

#include <string>
#include "remote_messages.pb.h"

// Define the hub transport types
enum class HubTransport {
  ESPNOW,
  MQTT,
  WEBSOCKET
};

// Abstract interface for hub communication transports
class HubTransportBase {
public:
  virtual ~HubTransportBase() = default;
  
  // Initialize the hub communication transport
  virtual bool init() = 0;
  
  // Process hub communication tasks (called in loop)
  virtual void process() = 0;
  
  // Send a RemoteEvent protobuf message to the hub
  virtual bool sendRemoteEvent(const omote_RemoteEvent& event) = 0;
  
  // Check if connected/ready
  virtual bool isReady() = 0;
  
  // Shutdown the communication
  virtual void shutdown() = 0;
}; 