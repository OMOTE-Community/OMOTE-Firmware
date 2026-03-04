#include <WiFi.h>
#include <WebSocketsClient.h>
#include <string>
#include "websocket_hal_esp32.h"
#include "secrets.h"

WebSocketsClient webSocket;
tAnnounceWebSocketMessageProto_cb thisAnnounceWebSocketMessageProto_cb = NULL;
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
      if (thisAnnounceWebSocketMessageProto_cb != NULL) {
        thisAnnounceWebSocketMessageProto_cb(payload, length);
      }
      break;
      
    case WStype_ERROR:
      Serial.println("WebSocket Error");
      break;
      
    default:
      break;
  }
}

void set_announceWebSocketMessageProto_cb_HAL(tAnnounceWebSocketMessageProto_cb pAnnounceWebSocketMessageProto_cb) {
  thisAnnounceWebSocketMessageProto_cb = pAnnounceWebSocketMessageProto_cb;
}

void init_websocket_HAL(const char* hub_url) {
  Serial.printf("Initializing WebSocket Client connection to %s\n", hub_url);
  
  String urlStr(hub_url);
  // Find the start of the host (after "ws://" or "wss://")
  int hostStart = urlStr.indexOf("://");
  if (hostStart < 0) {
    Serial.println("Error: Invalid WebSocket Server URL format");
    return;
  }
  hostStart += 3; // Skip past "://"
  
  int portStart = urlStr.indexOf(':', hostStart);
  int pathStart = urlStr.indexOf('/', hostStart);
  
  String host;
  uint16_t port = 80;
  String path = "/";
  
  if (portStart > 0) {
    host = urlStr.substring(hostStart, portStart);
    if (pathStart > 0) {
      port = urlStr.substring(portStart + 1, pathStart).toInt();
      path = urlStr.substring(pathStart);
    } else {
      port = urlStr.substring(portStart + 1).toInt();
    }
  } else if (pathStart > 0) {
    host = urlStr.substring(hostStart, pathStart);
    path = urlStr.substring(pathStart);
  } else {
    host = urlStr.substring(hostStart);
  }
  
  Serial.printf("Connecting to host: %s, port: %d, path: %s\n", host.c_str(), port, path.c_str());
  
  webSocket.begin(host, port, path);
  webSocket.onEvent(onWebSocketEvent);
  
  webSocket.setReconnectInterval(5000);
  
  // Enable heartbeat: ping every 5s, expect pong within 1s, disconnect after 2 missed
  webSocket.enableHeartbeat(5000, 1000, 2);
  
  Serial.println("WebSocket Client initialized");
}

void websocket_loop_HAL() {
  webSocket.loop();
}

bool publishWebSocketMessageProto_HAL(const uint8_t* data, size_t len) {
  if (!isConnected) {
    Serial.println("WebSocket Client not connected, cannot send protobuf message");
    return false;
  }
  
  if (len > 1024) {
    Serial.println("Error: Protobuf message exceeds reasonable WebSocket size");
    return false;
  }
  
  bool result = webSocket.sendBIN(data, len);
  
  if (!result) {
    Serial.println("WebSocket Client failed to send protobuf message");
  }
  
  return result;
}

void websocket_shutdown_HAL() {
  Serial.println("Shutting down WebSocket Client");
  webSocket.disconnect();
  delay(50);  // Allow TCP stack to flush close frame before WiFi teardown
  isConnected = false;
}

bool websocket_is_connected_HAL() {
  return isConnected;
}

const char* get_websocket_hub_url_HAL() {
  return WEBSOCKET_HUB_URL;
}
