#pragma once

#include <cstdint>
#include <string>

#if (ENABLE_WIFI_AND_MQTT == 1)

void init_mqtt_HAL(void);
bool getIsWifiConnected_HAL();
void mqtt_loop_HAL();
bool publishMQTTMessage_HAL(const char *topic, const char *payload);
bool publishMQTTMessageProto_HAL(const char *topic, const uint8_t* payload, size_t length);
void wifi_shutdown_HAL();

typedef void (*tAnnounceWiFiconnected_cb)(bool connected);
void set_announceWiFiconnected_cb_HAL(tAnnounceWiFiconnected_cb pAnnounceWiFiconnected_cb);
typedef void (*tAnnounceSubscribedTopics_cb)(std::string topic, std::string payload);
void set_announceSubscribedTopics_cb_HAL(tAnnounceSubscribedTopics_cb pAnnounceSubscribedTopics_cb);
typedef void (*tAnnounceMQTTMessageProto_cb)(const uint8_t* data, size_t len);
void set_announceMQTTMessageProto_cb_HAL(tAnnounceMQTTMessageProto_cb pAnnounceMQTTMessageProto_cb);
void set_mqtt_proto_response_topic_HAL(const char* topic);

#endif
