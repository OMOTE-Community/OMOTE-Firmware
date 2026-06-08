#include "websocketHubTransport.h"
#include "hubManager.h"
#include "protoCodec.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

// Forward declaration for the internal callback
void websocketMessageReceived_cb_proto(const uint8_t* data, size_t len);

#if (ENABLE_HUB_COMMUNICATION == 3)

static const unsigned long WEBSOCKET_WAKE_QUEUE_GRACE_MS = 1000;

extern void init_websocket(const char* hub_url);
extern void websocket_loop();
extern bool publishWebSocketMessageProto(const uint8_t* data, size_t len);
extern void websocket_shutdown();
extern bool websocket_is_connected();

WebSocketHubTransport::WebSocketHubTransport() = default;

bool WebSocketHubTransport::init() {
  const char* hub_url = get_websocketHubURL();
  if (hub_url == nullptr || strlen(hub_url) == 0) {
    omote_log_e("WebSocket hub URL not configured\n");
    return false;
  }

  omote_log_i("Initializing WebSocket transport to %s\n", hub_url);
  set_websocket_message_callback_proto(&websocketMessageReceived_cb_proto);
  init_websocket(hub_url);
  return true;
}

void WebSocketHubTransport::process() {
  if (!getIsWifiConnected()) {
    return;
  }

  websocket_loop();
}

bool WebSocketHubTransport::sendRemoteEvent(const omote_RemoteEvent& event) {
  omote_log_d("WebSocket: Sending protobuf message for device %s, command %d\n", event.device, event.command);
  
  // Encode protobuf to bytes
  uint8_t buffer[1024];  // WebSocket can handle larger messages than ESP-NOW
  size_t encoded_size = Hub::ProtoCodec::encodeRemoteEvent(event, buffer, sizeof(buffer));
  
  if (encoded_size == 0) {
    omote_log_e("WebSocket: Failed to encode protobuf message\n");
    return false;
  }
  
  omote_log_d("WebSocket: Encoded %d bytes\n", encoded_size);
  return publishWebSocketMessageProto(buffer, encoded_size);
}

bool WebSocketHubTransport::isReady() {
  return websocket_is_connected();
}

unsigned long WebSocketHubTransport::wakeQueueTtlMs() const {
  return get_websocketReconnectIntervalMs() + WEBSOCKET_WAKE_QUEUE_GRACE_MS;
}

void WebSocketHubTransport::shutdown() {
  omote_log_i("WebSocket: Shutting down WebSocket transport\n");
  websocket_shutdown();
}

void websocketMessageReceived_cb_proto(const uint8_t* data, size_t len) {
  omote_log_d("WebSocket: Received protobuf message, %d bytes\n", len);
  
  // Decode protobuf to CommandResult
  omote_CommandResult result = Hub::ProtoCodec::decodeCommandResult(data, len);
  
  // Forward to hub manager
  auto& hubManager = HubManager::getInstance();
  hubManager.handleIncomingCommandResult(result);
}

#endif
