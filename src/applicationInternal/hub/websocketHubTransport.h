#pragma once

#include "hubTransportBase.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#if (ENABLE_HUB_COMMUNICATION == 3)
class WebSocketHubTransport : public HubTransportBase {
public:
  WebSocketHubTransport();
  ~WebSocketHubTransport() override;
  
  bool init() override;
  void process() override;
  bool sendMessage(const json& payload) override;
  bool isReady() override;
  void shutdown() override;
};
#endif

