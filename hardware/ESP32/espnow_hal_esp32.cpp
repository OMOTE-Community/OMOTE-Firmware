#include <esp_now.h>
#include <WiFi.h>
#include <string>
#include <sstream>
#include "espnow_hal_esp32.h"
#include <esp_wifi.h>
#include "secrets.h"

// Function to get MAC address for ESP32
std::string getMACaddress() {
  return std::string(WiFi.macAddress().c_str());
}

// Define the MAC address of the Raspberry Pi hub
// This should be defined in secrets.h as ESPNOW_HUB_MAC
// Example: #define ESPNOW_HUB_MAC {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}
uint8_t hub_mac[6] = ESPNOW_HUB_MAC;
esp_now_peer_info_t hub_peer;

// Callbacks for ESP-NOW received data
tAnnounceEspNowMessageProto_cb thisAnnounceEspNowMessageProto_cb = NULL;

void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  // Convert MAC to string for logging
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  
  if (thisAnnounceEspNowMessageProto_cb != NULL) {
    thisAnnounceEspNowMessageProto_cb(data, data_len);
  }
}

void set_announceEspNowMessageProto_cb_HAL(tAnnounceEspNowMessageProto_cb pAnnounceEspNowMessageProto_cb) {
  thisAnnounceEspNowMessageProto_cb = pAnnounceEspNowMessageProto_cb;
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

bool publishEspNowMessageProto_HAL(const uint8_t* data, size_t len) {
  if (len > 250) {
    Serial.println("Error: Protobuf message exceeds ESP-NOW maximum size");
    return false;
  }
  
  // Send the protobuf message directly
  esp_err_t result = esp_now_send(hub_peer.peer_addr, data, len);
  
  if (result == ESP_OK) {
    return true;
  }
  
  Serial.println("ESP-NOW failed to send protobuf message");
  return false;
}

void espnow_shutdown_HAL() {
  // Unregister peer
  esp_now_del_peer(hub_peer.peer_addr);
  
  // Deinitialize ESP-NOW
  esp_now_deinit();
  
  Serial.println("ESP-NOW shutdown complete");
} 