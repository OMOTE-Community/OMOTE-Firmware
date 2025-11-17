#include <WiFi.h>
#include <WebSocketsClient.h>
#include <string>
#include <nlohmann/json.hpp>
#include "websocket_hal_esp32.h"
#include "secrets.h"

using json = nlohmann::json;

WebSocketsClient webSocket;
tAnnounceWebSocketMessage_cb thisAnnounceWebSocketMessage_cb = NULL;
bool isConnected = false;

void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      isConnected = false;
      break;
      
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      isConnected = true;
      break;
      
    case WStype_BIN:
      if (thisAnnounceWebSocketMessage_cb == NULL) return;
      
      try {
        auto unpacked_json = json::from_msgpack(payload, payload + length);
        thisAnnounceWebSocketMessage_cb(unpacked_json);
      } catch (const std::exception& e) {
        Serial.printf("Error parsing WebSocket message: %s\n", e.what());
      }
      break;
      
    case WStype_ERROR:
      Serial.println("WebSocket Error");
      break;
      
    default:
      break;
  }
}

void set_announceWebSocketMessage_cb_HAL(tAnnounceWebSocketMessage_cb pAnnounceWebSocketMessage_cb) {
  thisAnnounceWebSocketMessage_cb = pAnnounceWebSocketMessage_cb;
}

void init_websocket_HAL(const char* hub_url) {
  Serial.printf("Initializing WebSocket connection to %s\n", hub_url);
  
  String urlStr(hub_url);
  int portStart = urlStr.indexOf(':', 6);
  int pathStart = urlStr.indexOf('/', 7);
  
  String host;
  uint16_t port = 80;
  String path = "/";
  
  if (portStart > 0) {
    host = urlStr.substring(6, portStart);
    if (pathStart > 0) {
      port = urlStr.substring(portStart + 1, pathStart).toInt();
      path = urlStr.substring(pathStart);
    } else {
      port = urlStr.substring(portStart + 1).toInt();
    }
  } else if (pathStart > 0) {
    host = urlStr.substring(6, pathStart);
    path = urlStr.substring(pathStart);
  } else {
    host = urlStr.substring(6);
  }
  
  Serial.printf("Connecting to host: %s, port: %d, path: %s\n", host.c_str(), port, path.c_str());
  
  webSocket.begin(host, port, path);
  webSocket.onEvent(onWebSocketEvent);
  
  // Try reconnection every 5000ms if connection fails (same as example)
  webSocket.setReconnectInterval(5000);
  
  // Enable heartbeat: ping every 15s, expect pong within 3s, disconnect after 2 missed
  webSocket.enableHeartbeat(15000, 3000, 2);
  
  Serial.println("WebSocket initialized");
}

void websocket_loop_HAL() {
  webSocket.loop();
}

bool publishWebSocketMessage_HAL(json payload) {
  if (!isConnected) {
    Serial.println("WebSocket not connected, cannot send message");
    return false;
  }
  
  std::vector<std::uint8_t> packed_json = json::to_msgpack(payload);
  
  if (packed_json.size() > 1024) {
    Serial.println("Error: Message exceeds reasonable WebSocket size");
    return false;
  }
  
  bool result = webSocket.sendBIN(packed_json.data(), packed_json.size());
  
  if (!result) {
    Serial.println("WebSocket failed to send message");
  }
  
  return result;
}

void websocket_shutdown_HAL() {
  Serial.println("Shutting down WebSocket");
  webSocket.disconnect();
  isConnected = false;
}

bool websocket_is_connected_HAL() {
  return isConnected;
}

const char* get_websocket_hub_url_HAL() {
  return WEBSOCKET_HUB_URL;
}

