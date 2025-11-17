#include "espNowHubTransport.h"
#include "hubManager.h"
#include "protoCodec.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

// Forward declaration for the internal callback
void hubMessageReceived_cb_proto(const uint8_t* data, size_t len);

#if (ENABLE_HUB_COMMUNICATION == 1)
EspNowHubTransport::EspNowHubTransport() = default;

EspNowHubTransport::~EspNowHubTransport() {
  shutdown();
}

bool EspNowHubTransport::init() {
  set_espnow_message_callback_proto(&hubMessageReceived_cb_proto);
  init_espnow();
  return true;
}

void EspNowHubTransport::process() {
  espnow_loop();
}

bool EspNowHubTransport::sendRemoteEvent(const omote_RemoteEvent& event) {
  omote_log_d("ESP-NOW: Sending protobuf message for device %s, command %d\n", event.device, event.command);
  
  // Encode protobuf to bytes
  uint8_t buffer[250];  // ESP-NOW max size
  size_t encoded_size = Hub::ProtoCodec::encodeRemoteEvent(event, buffer, sizeof(buffer));
  
  if (encoded_size == 0) {
    omote_log_e("ESP-NOW: Failed to encode protobuf message\n");
    return false;
  }
  
  omote_log_d("ESP-NOW: Encoded %d bytes\n", encoded_size);
  return publishEspNowMessageProto(buffer, encoded_size);
}

bool EspNowHubTransport::isReady() {
  // ESP-NOW is always ready once initialized
  return true;
}

void EspNowHubTransport::shutdown() {
  espnow_shutdown();
}

void hubMessageReceived_cb_proto(const uint8_t* data, size_t len) {
  omote_log_d("ESP-NOW: Received protobuf message, %d bytes\n", len);
  
  // Decode protobuf to CommandResult
  omote_CommandResult result = Hub::ProtoCodec::decodeCommandResult(data, len);
  
  // Forward to hub manager
  auto& hubManager = HubManager::getInstance();
  hubManager.handleIncomingCommandResult(result);
}

// Legacy JSON callback removed - now using protobuf only
#endif 