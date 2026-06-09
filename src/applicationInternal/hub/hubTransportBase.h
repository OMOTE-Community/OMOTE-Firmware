#pragma once

#include <cstddef>
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

  // Usable encoded-protobuf payload bytes this transport can carry inbound for a
  // CommandResult (StateSync) response — NOT the raw transport buffer/MTU. MQTT
  // subtracts packet/topic overhead; ESP-NOW reports its payload cap directly.
  // Conservative default of 0 makes an un-overridden transport request the
  // time-only floor rather than overflow. Transports override with their budget.
  virtual size_t maxInboundCommandResultBytes() const {
    return 0;
  }

  virtual void shutdown() = 0;
};
