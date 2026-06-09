#pragma once

#include "hubTransportBase.h"

#if (ENABLE_WIFI_AND_MQTT == 1)
class MqttHubTransport : public HubTransportBase {
public:
  MqttHubTransport();
  
  bool init() override;
  void process() override;
  bool sendRemoteEvent(const omote_RemoteEvent& event) override;
  bool isReady() override;
  // PubSubClient's buffer (512 B, set in mqtt_hal checkMQTTconnection) holds the
  // whole packet, so usable payload is the buffer minus the PUBLISH fixed header
  // and the "remote_responses" topic. Conservative; 5b raises the buffer toward
  // 1470 for full-device headroom. Already covers a single-device snapshot.
  size_t maxInboundCommandResultBytes() const override { return 512 - 32; }
  void shutdown() override;

private:
  // MQTT-specific members
  bool mqttConnected;
  std::string baseTopic;
};
#endif 