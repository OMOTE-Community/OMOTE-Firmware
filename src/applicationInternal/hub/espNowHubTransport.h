#pragma once

#include "hubTransportBase.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#if (ENABLE_HUB_COMMUNICATION == 1)
class EspNowHubTransport : public HubTransportBase {
public:
  EspNowHubTransport();
  ~EspNowHubTransport() override;
  
  bool init() override;
  void process() override;
  bool sendMessage(const json& payload) override;
  bool isReady() override;
  void shutdown() override;
};
#endif 