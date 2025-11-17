#include "websocketHubTransport.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"

#if (ENABLE_HUB_COMMUNICATION == 3)

extern void init_websocket(const char* hub_url);
extern void websocket_loop();
extern bool publishWebSocketMessage(json payload);
extern void websocket_shutdown();
extern bool websocket_is_connected();

WebSocketHubTransport::WebSocketHubTransport() = default;

WebSocketHubTransport::~WebSocketHubTransport() {
  shutdown();
}

bool WebSocketHubTransport::init() {
  const char* hub_url = get_websocketHubURL();
  if (hub_url == nullptr || strlen(hub_url) == 0) {
    omote_log_e("WebSocket hub URL not configured\n");
    return false;
  }
  
  omote_log_i("Initializing WebSocket transport to %s\n", hub_url);
  init_websocket(hub_url);
  return true;
}

void WebSocketHubTransport::process() {
  websocket_loop();
}

bool WebSocketHubTransport::sendMessage(const json& payload) {
  std::string device = payload["device"];
  std::string command = payload["command"];
  
  omote_log_d("WebSocket: Sending message for device %s, command %s\n", device.c_str(), command.c_str());
  return publishWebSocketMessage(payload);
}

bool WebSocketHubTransport::isReady() {
  return websocket_is_connected();
}

void WebSocketHubTransport::shutdown() {
  websocket_shutdown();
}

#endif

