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
  void shutdown() override;
};
#endif
