#pragma once

#include "hubTransportBase.h"

#if (ENABLE_WIFI_AND_MQTT == 1)
class MqttHubTransport : public HubTransportBase {
public:
  MqttHubTransport();
  ~MqttHubTransport() override;
  
  bool init() override;
  void process() override;
  bool sendRemoteEvent(const omote_RemoteEvent& event) override;
  bool isReady() override;
  void shutdown() override;
  
private:
  // MQTT-specific members
  bool mqttConnected;
  std::string baseTopic;
};
#endif 