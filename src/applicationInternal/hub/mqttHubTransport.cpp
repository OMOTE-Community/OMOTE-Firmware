#include "mqttHubTransport.h"
#include "hubManager.h"
#include "protoCodec.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

// Forward declaration for the internal proto callback
void mqttMessageReceived_cb_proto(const uint8_t* data, size_t len);

#if (ENABLE_WIFI_AND_MQTT == 1)

static const char* COMMAND_TOPIC = "remote_commands";
static const char* RESPONSE_TOPIC = "remote_responses";

MqttHubTransport::MqttHubTransport() : baseTopic("omote/") {
}

bool MqttHubTransport::init() {
  set_mqtt_message_callback_proto(&mqttMessageReceived_cb_proto);
  init_mqtt();
  return true;
}

void MqttHubTransport::process() {
  mqtt_loop();
}

bool MqttHubTransport::sendRemoteEvent(const omote_RemoteEvent& event) {
  omote_log_d("MQTT: Sending protobuf message for device %s, command %d\n", event.device, event.command);

  uint8_t buffer[512];
  size_t encoded_size = Hub::ProtoCodec::encodeRemoteEvent(event, buffer, sizeof(buffer));

  if (encoded_size == 0) {
    omote_log_e("MQTT: Failed to encode protobuf message\n");
    return false;
  }

  return publishMQTTMessageProto(COMMAND_TOPIC, buffer, encoded_size);
}

bool MqttHubTransport::isReady() {
  return getIsWifiConnected();
}

void MqttHubTransport::shutdown() {
  wifi_shutdown();
}

// Proto callback that decodes binary protobuf and routes through HubManager
void mqttMessageReceived_cb_proto(const uint8_t* data, size_t len) {
  omote_CommandResult result = Hub::ProtoCodec::decodeCommandResult(data, len);

  if (result.kind == omote_ResponseKind_ERROR && result.which_data == omote_CommandResult_error_tag) {
    // Check if this is a decode failure (ProtoCodec returns error on decode failure)
    omote_log_w("MQTT: Received CommandResult with error: %s\n", result.data.error.message);
  }

  HubManager::getInstance().handleIncomingCommandResult(result);
}
#endif
