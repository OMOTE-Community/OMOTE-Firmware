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
  // ESP-NOW v1 payload cap; esp_now_send caps on payload length, so this is
  // already the usable budget. Too small for one rich device -> time-only sync.
  // 5c raises this to 1470 once ESP-NOW v2 is negotiated.
  size_t maxInboundCommandResultBytes() const override { return 250; }
  void shutdown() override;
};
#endif 