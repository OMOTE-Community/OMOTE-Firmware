#pragma once

#include "hubTransportBase.h"

#if (ENABLE_HUB_COMMUNICATION == 1)
class EspNowHubTransport : public HubTransportBase {
public:
  EspNowHubTransport();
  
  bool init() override;
  void process() override;
  bool sendRemoteEvent(const omote_RemoteEvent& event) override;
  bool isReady() override;
  void shutdown() override;
};
#endif 