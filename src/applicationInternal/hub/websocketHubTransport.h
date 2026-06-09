#pragma once

#include "hubTransportBase.h"

#if (ENABLE_HUB_COMMUNICATION == 3)
class WebSocketHubTransport : public HubTransportBase {
public:
  WebSocketHubTransport();
  
  bool init() override;
  void process() override;
  bool sendRemoteEvent(const omote_RemoteEvent& event) override;
  bool isReady() override;
  unsigned long wakeQueueTtlMs() const override;
  // WebSocket is effectively unbounded, so it can carry the full StateSync.
  size_t maxInboundCommandResultBytes() const override { return omote_CommandResult_size; }
  void shutdown() override;
};
#endif
