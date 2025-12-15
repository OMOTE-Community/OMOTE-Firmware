#include "mqttHubTransport.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

#if (ENABLE_WIFI_AND_MQTT == 1)
MqttHubTransport::MqttHubTransport() : baseTopic("omote/") {
}

MqttHubTransport::~MqttHubTransport() {
  shutdown();
}

bool MqttHubTransport::init() {
  init_mqtt();
  return true;
}

void MqttHubTransport::process() {
  mqtt_loop();
}

bool MqttHubTransport::sendMessage(const json& payload) {
  std::string topic = baseTopic + "remote_commands";
  std::string payloadStr = payload.dump();
  
  // Extract device and command for logging
  std::string device = payload["device"];
  std::string command = payload["command"];
  
  omote_log_d("MQTT: Sending message for device %s, command %s\n", device.c_str(), command.c_str());
  return publishMQTTMessage(topic.c_str(), payloadStr.c_str());
}

bool MqttHubTransport::isReady() {
  return getIsWifiConnected();
}

void MqttHubTransport::shutdown() {
  wifi_shutdown();
}
#endif 