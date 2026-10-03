#pragma once

#include "hubTransportBase.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#if (ENABLE_WIFI_AND_MQTT == 1)
class MqttHubTransport : public HubTransportBase {
public:
  MqttHubTransport();
  ~MqttHubTransport() override;
  
  bool init() override;
  void process() override;
  bool sendMessage(const json& payload) override;
  bool isReady() override;
  void shutdown() override;
  
private:
  // MQTT-specific members
  bool mqttConnected;
  std::string baseTopic;
};
#endif 