#include <esp_now.h>
#include <WiFi.h>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "espnow_hal_esp32.h"
#include <esp_wifi.h>
#include "secrets.h"

using json = nlohmann::json;

// Define the MAC address of the Raspberry Pi hub
// This should be defined in secrets.h as ESPNOW_HUB_MAC
// Example: #define ESPNOW_HUB_MAC {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}
uint8_t hub_mac[6] = ESPNOW_HUB_MAC;
esp_now_peer_info_t hub_peer;

// Callback for ESP-NOW received data
tAnnounceEspNowMessage_cb thisAnnounceEspNowMessage_cb = NULL;
void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (thisAnnounceEspNowMessage_cb == NULL) return;
  
  // Convert MAC to string for logging
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  
//   Serial.printf("ESP-NOW message received from %s\n", macStr);
  
  try {
    // Try to parse the received data as MessagePack
    auto unpacked_json = json::from_msgpack(data, data + data_len);
    
    // Forward to callback
    thisAnnounceEspNowMessage_cb(unpacked_json);
  } catch (const std::exception& e) {
    Serial.printf("Error parsing ESP-NOW message: %s\n", e.what());
  }
}

void set_announceEspNowMessage_cb_HAL(tAnnounceEspNowMessage_cb pAnnounceEspNowMessage_cb) {
  thisAnnounceEspNowMessage_cb = pAnnounceEspNowMessage_cb;
}

void init_espnow_HAL(void) {
  Serial.println("Starting ESP-NOW");
  // Set WiFi mode to station (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(8, WIFI_SECOND_CHAN_NONE);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callbacks
  esp_now_register_recv_cb(onDataReceived);
  
  // Register hub as peer
  memcpy(hub_peer.peer_addr, hub_mac, 6);
  // Setting channel0 defaults to existing channel setting
  hub_peer.channel = 0;  
  hub_peer.encrypt = false;
  
  // Add peer
  if (esp_now_add_peer(&hub_peer) != ESP_OK) {
    Serial.println("Failed to add hub peer");
    return;
  }
  
  Serial.println("ESP-NOW initialized successfully");
}

void espnow_loop_HAL() {
  // Nothing to do in the loop for ESP-NOW
  // ESP-NOW callbacks are handled by the ESP32 in the background
}

bool publishEspNowMessage_HAL(json payload) {
  // Pack the JSON into MessagePack format
  std::vector<std::uint8_t> packed_json = json::to_msgpack(payload);
  
  if (packed_json.size() > 250) {
    Serial.println("Error: Message exceeds ESP-NOW maximum size");
    return false;
  }
  
  // Send the message
  esp_err_t result = esp_now_send(hub_peer.peer_addr, packed_json.data(), packed_json.size());
  
  if (result == ESP_OK) {
    // Serial.println("ESP-NOW sent message with success");
    return true;
  }
  
  Serial.println("ESP-NOW failed to send message");
  return false;
}

void espnow_shutdown_HAL() {
  // Unregister peer
  esp_now_del_peer(hub_peer.peer_addr);
  
  // Deinitialize ESP-NOW
  esp_now_deinit();
  
  Serial.println("ESP-NOW shutdown complete");
} 