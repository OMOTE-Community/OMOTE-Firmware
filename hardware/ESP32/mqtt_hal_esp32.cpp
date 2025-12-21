#include <sstream>
#include "WiFi.h"
#include <PubSubClient.h>
#ifdef REMOTE_DEBUG
#include <RemoteDebug.h>
#endif
#include "mqtt_hal_esp32.h"
#if (ENABLE_KEYBOARD_BLE == 1)
#include "keyboard_ble_hal_esp32.h"
#endif
#include "secrets.h"

#if (ENABLE_WIFI_AND_MQTT == 1)
WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool isWifiConnected = false;

#ifdef REMOTE_DEBUG
extern RemoteDebug Debug;
#endif

tAnnounceWiFiconnected_cb thisAnnounceWiFiconnected_cb = NULL;
void set_announceWiFiconnected_cb_HAL(tAnnounceWiFiconnected_cb pAnnounceWiFiconnected_cb) {
  thisAnnounceWiFiconnected_cb = pAnnounceWiFiconnected_cb;  
}

tAnnounceSubscribedTopics_cb thisAnnounceSubscribedTopics_cb = NULL;
void set_announceSubscribedTopics_cb_HAL(tAnnounceSubscribedTopics_cb pAnnounceSubscribedTopics_cb) {
  thisAnnounceSubscribedTopics_cb = pAnnounceSubscribedTopics_cb;  
}

bool getIsWifiConnected_HAL() {
  return isWifiConnected;
}

// WiFi status event
void WiFiEvent(WiFiEvent_t event){
  //Serial.printf("[WiFi-event] event: %d\r\n", event);
  if(event == ARDUINO_EVENT_WIFI_STA_GOT_IP){
    // connection to MQTT server will be done in checkMQTTconnection()
    // mqttClient.setServer(MQTT_SERVER, 1883); // MQTT initialization
    // mqttClient.connect("OMOTE"); // Connect using a client id

  }

  // Set status bar icon based on WiFi status
  if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP || event == ARDUINO_EVENT_WIFI_STA_GOT_IP6) {
    isWifiConnected = true;
    thisAnnounceWiFiconnected_cb(true);
    Serial.printf("WiFi connected, IP address: %s\r\n", WiFi.localIP().toString().c_str());

#ifdef REMOTE_DEBUG
    Debug.begin("omote");
    Debug.setResetCmdEnabled(true);
    Debug.setSerialEnabled(true);
    Debug.println("Remote log ready!");
#endif
  } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    isWifiConnected = false;
    thisAnnounceWiFiconnected_cb(false);
    // automatically try to reconnect
    Serial.printf("WiFi got disconnected. Will try to reconnect.\r\n");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  } else {
    // e.g. ARDUINO_EVENT_WIFI_STA_CONNECTED or many others
    // connected is not enough, will wait for IP
    isWifiConnected = false;
    thisAnnounceWiFiconnected_cb(false);

  }
}

void init_mqtt_HAL(void) {
  // Setup WiFi
  WiFi.setHostname("OMOTE"); //define hostname
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(true);
}

std::string subscribeTopicOMOTEtest = "OMOTE/test";
// For connecting to one or several BLE clients
std::string subscribeTopicOMOTE_BLEstartAdvertisingForAll        = "OMOTE/BLE/startAdvertisingForAll";
std::string subscribeTopicOMOTE_BLEstartAdvertisingWithWhitelist = "OMOTE/BLE/startAdvertisingWithWhitelist";
std::string subscribeTopicOMOTE_BLEstartAdvertisingDirected      = "OMOTE/BLE/startAdvertisingDirected";
std::string subscribeTopicOMOTE_BLEstopAdvertising               = "OMOTE/BLE/stopAdvertising";
std::string subscribeTopicOMOTE_BLEprintConnectedClients         = "OMOTE/BLE/printConnectedClients";
std::string subscribeTopicOMOTE_BLEdisconnectAllClients          = "OMOTE/BLE/disconnectAllClients";
std::string subscribeTopicOMOTE_BLEprintBonds                    = "OMOTE/BLE/printBonds";
std::string subscribeTopicOMOTE_BLEdeleteBonds                   = "OMOTE/BLE/deleteBonds";

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  std::string topicReceived(topic);
  std::string strPayload(reinterpret_cast<const char *>(payload), length);
  Serial.printf("MQTT: received topic %s with payload %s\r\n", topicReceived.c_str(), strPayload.c_str());

  if (topicReceived == subscribeTopicOMOTEtest) {
    // Do whatever you want here, if it is ESP32 hardware related.
    // ...

    // Or forward the topic to "void receiveMQTTmessage_cb" in the "commandHandler.cpp", if it is not ESP32 hardware related
    thisAnnounceSubscribedTopics_cb(topicReceived, strPayload);

  #if (ENABLE_KEYBOARD_BLE == 1)
  } else if (topicReceived == subscribeTopicOMOTE_BLEstartAdvertisingForAll) {
    keyboardBLE_startAdvertisingForAll_HAL();  
  } else if (topicReceived == subscribeTopicOMOTE_BLEstartAdvertisingWithWhitelist) {
    keyboardBLE_startAdvertisingWithWhitelist_HAL(strPayload);  
  } else if (topicReceived == subscribeTopicOMOTE_BLEstartAdvertisingDirected) {
    // the payload are two values, separated by comma: peerAddress and isRandomAddress 
    std::stringstream ss(strPayload);
    if (ss.good())  {
      std::string peerAddress;
      std::getline(ss, peerAddress, ',');
      
      if (ss.good())  {
        std::string isRandomAddressStr;
        std::getline(ss, isRandomAddressStr, ',');
        bool isRandomAddress = false;
        if (isRandomAddressStr == "true") {
          isRandomAddress = true;  
        }
        keyboardBLE_startAdvertisingDirected_HAL(peerAddress, isRandomAddress);  
      }
    }
  } else if (topicReceived == subscribeTopicOMOTE_BLEstopAdvertising) {
    keyboardBLE_stopAdvertising_HAL();  
  } else if (topicReceived == subscribeTopicOMOTE_BLEprintConnectedClients) {
    keyboardBLE_printConnectedClients_HAL();
  } else if (topicReceived == subscribeTopicOMOTE_BLEdisconnectAllClients) {
    keyboardBLE_disconnectAllClients_HAL();  
  } else if (topicReceived == subscribeTopicOMOTE_BLEprintBonds) {
    keyboardBLE_printBonds_HAL();  
  } else if (topicReceived == subscribeTopicOMOTE_BLEdeleteBonds) {
    keyboardBLE_deleteBonds_HAL();  
  #endif

  } else {
    // forward all other topics to the commandHandler
    thisAnnounceSubscribedTopics_cb(topicReceived, strPayload);

  }
}

void mqtt_subscribeTopics() {
  mqttClient.setCallback(&callback);

  mqttClient.subscribe(subscribeTopicOMOTEtest.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEstartAdvertisingForAll.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEstartAdvertisingWithWhitelist.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEstartAdvertisingDirected.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEstopAdvertising.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEprintConnectedClients.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEdisconnectAllClients.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEprintBonds.c_str());
  mqttClient.subscribe(subscribeTopicOMOTE_BLEdeleteBonds.c_str());
  Serial.printf("  Successfully subscribed to MQTT topics\r\n");

}

bool checkMQTTconnection() {

  if (WiFi.isConnected()) {
    if (mqttClient.connected()) {
      return true;
    } else {
      // try to connect to mqtt server
      mqttClient.setBufferSize(512);   // default is 256
      //mqttClient.setKeepAlive(15);     // default is 15   Client will send MQTTPINGREQ to keep connection alive
      //mqttClient.setSocketTimeout(15); // default is 15   This determines how long the client will wait for incoming data when it expects data to arrive - for example, whilst it is in the middle of reading an MQTT packet.
      mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT); // MQTT initialization
      
      std::string mqttClientName = std::string(MQTT_CLIENTNAME) + "_esp32_" + std::string(WiFi.macAddress().c_str());
      if (mqttClient.connect(mqttClientName.c_str(), MQTT_USER, MQTT_PASS)) {
        Serial.printf("  Successfully connected to MQTT broker\r\n");
    
        mqtt_subscribeTopics();

      } else {
        Serial.printf("  MQTT connection failed (but WiFi is available). Will try later ...\r\n");

      }
      return mqttClient.connected();
    }
  } else {
    // Serial.printf("  No connection to MQTT server, because WiFi ist not connected.\r\n");
    return false;
  }  
}

unsigned long reconnectInterval = 100;
// in order to do reconnect immediately ...
unsigned long lastReconnectAttempt = millis() - reconnectInterval - 1;
void mqtt_loop_HAL() {
  if (!mqttClient.connected()) {
    unsigned long currentMillis = millis();
    if ((currentMillis - lastReconnectAttempt) > reconnectInterval) {
      lastReconnectAttempt = currentMillis;
      // Attempt to reconnect
      checkMQTTconnection();
    }
  }  

  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

bool publishMQTTMessage_HAL(const char *topic, const char *payload){

  if (checkMQTTconnection()) {
    // Serial.printf("Sending mqtt payload to topic \"%s\": %s\r\n", topic, payload);
      
    if (mqttClient.publish(topic, payload)) {
      // Serial.printf("Publish ok\r\n");
      return true;
    }
    else {
      Serial.printf("Publish failed\r\n");
    }
  } else {
    Serial.printf("  Cannot publish mqtt message, because checkMQTTconnection failed (WiFi or mqtt is not connected)\r\n");
  }
  return false;
}

void wifi_shutdown_HAL() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

#endif
