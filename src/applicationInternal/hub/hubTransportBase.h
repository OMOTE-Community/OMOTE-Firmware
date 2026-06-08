#pragma once

#include <string>
#include "remote_messages.pb.h"

enum class HubTransport {
  ESPNOW,
  MQTT,
  WEBSOCKET
};

class HubTransportBase {
public:
  static const unsigned long DEFAULT_WAKE_QUEUE_TTL_MS = 1500;

  virtual ~HubTransportBase() = default;
  
  virtual bool init() = 0;
  
  virtual void process() = 0;
  
  virtual bool sendRemoteEvent(const omote_RemoteEvent& event) = 0;
  
  virtual bool isReady() = 0;

  virtual unsigned long wakeQueueTtlMs() const {
    return DEFAULT_WAKE_QUEUE_TTL_MS;
  }
  
  virtual void shutdown() = 0;
};
