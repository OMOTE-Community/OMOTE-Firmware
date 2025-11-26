#include "mqttHubTransport.h"
#include "hubManager.h"
#include "protoCodec.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

// Forward declaration for the internal callback
void mqttMessageReceived_cb(std::string topic, std::string payload);

#if (ENABLE_WIFI_AND_MQTT == 1)
MqttHubTransport::MqttHubTransport() : baseTopic("omote/") {
}

bool MqttHubTransport::init() {
  set_mqtt_message_callback(&mqttMessageReceived_cb);
  init_mqtt();
  return true;
}

void MqttHubTransport::process() {
  mqtt_loop();
}

bool MqttHubTransport::sendRemoteEvent(const omote_RemoteEvent& event) {
  omote_log_d("MQTT: Sending protobuf message for device %s, command %d\n", event.device, event.command);
  
  // Encode protobuf to bytes
  uint8_t buffer[512];  // MQTT can handle larger messages
  size_t encoded_size = Hub::ProtoCodec::encodeRemoteEvent(event, buffer, sizeof(buffer));
  
  if (encoded_size == 0) {
    omote_log_e("MQTT: Failed to encode protobuf message\n");
    return false;
  }
  
  // Note: MQTT HAL needs to be updated to support binary payloads
  // For now, this will not work correctly as MQTT HAL expects string payloads
  omote_log_w("MQTT: Protobuf transport not fully implemented - MQTT HAL needs binary payload support\n");
  return false;
}

bool MqttHubTransport::isReady() {
  return getIsWifiConnected();
}

void MqttHubTransport::shutdown() {
  wifi_shutdown();
}

// Internal callback that routes messages through HubManager
// Note: This still expects JSON - MQTT protobuf support is incomplete
void mqttMessageReceived_cb(std::string topic, std::string payload) {
  omote_log_w("MQTT: Received message but protobuf decoding not implemented\n");
  omote_log_d("MQTT: Topic=%s, Payload=%s\n", topic.c_str(), payload.c_str());
}
#endif 