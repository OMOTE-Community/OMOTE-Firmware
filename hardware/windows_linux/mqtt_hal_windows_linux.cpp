#include <string>
#include "mqtt_hal_windows_linux.h"
#include "secrets.h"

#if (ENABLE_WIFI_AND_MQTT == 1)
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

// example is mainly taken from .pio/libdeps/windows_linux_64bit/MQTT-C/tests.c, TEST__api__publish_subscribe__single
#if !defined(WIN32)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#else
#include <ws2tcpip.h>

/* Some shortcuts to call winapi in a posix-like way */
#define close(sock)         closesocket(sock)
#define usleep(usec)        Sleep((usec) / 1000)
#endif

#include "lib/MQTT-C/include/mqtt.h"
#include "lib/MQTT-C/include/posix_sockets.h"

int sockfd = -1;
uint8_t sendmem1[1024], sendmem2[1024];
uint8_t recvmem1[1024], recvmem2[1024];
struct mqtt_client mqttClient;
std::string uniqueClientSuffix = "";
int state = 0;

tAnnounceWiFiconnected_cb thisAnnounceWiFiconnected_cb = NULL;
void set_announceWiFiconnected_cb_HAL(tAnnounceWiFiconnected_cb pAnnounceWiFiconnected_cb) {
  thisAnnounceWiFiconnected_cb = pAnnounceWiFiconnected_cb;  
}

tAnnounceSubscribedTopics_cb thisAnnounceSubscribedTopics_cb = NULL;
void set_announceSubscribedTopics_cb_HAL(tAnnounceSubscribedTopics_cb pAnnounceSubscribedTopics_cb) {
  thisAnnounceSubscribedTopics_cb = pAnnounceSubscribedTopics_cb;  
}

bool getIsWifiConnected_HAL() {
  return (sockfd != -1);
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

void publish_callback(void** state, struct mqtt_response_publish *publish) {
    **(int**)state += 1;
    printf("message nr %d received\r\n", **(int**)state);

    std::string topic((const char*) (publish->topic_name), publish->topic_name_size);
    std::string payload((const char*) (publish->application_message), publish->application_message_size);

    printf("Received a PUBLISH(topic=%s, DUP=%d, QOS=%d, RETAIN=%d, pid=%d) from the broker. Data='%s'\r\n", 
           topic.c_str(), publish->dup_flag, publish->qos_level, publish->retain_flag, publish->packet_id,
           payload.c_str()
    );
    
    if (topic == subscribeTopicOMOTEtest) {
      // Do whatever you want here, if it is Windows/Linux hardware related.
      // ...

      // Or forward the topic to "void receiveMQTTmessage_cb" in the "commandHandler.cpp", if it is not Windows/Linux hardware related
      thisAnnounceSubscribedTopics_cb(topic, payload);

    } else {
      // forward all other topics to the commandHandler
      thisAnnounceSubscribedTopics_cb(topic, payload);
    }
}

void mqtt_subscribeTopics() {
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTEtest.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEstartAdvertisingForAll.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEstartAdvertisingWithWhitelist.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEstartAdvertisingDirected.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEstopAdvertising.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEprintConnectedClients.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEdisconnectAllClients.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEprintBonds.c_str(), 2);
  mqtt_subscribe(&mqttClient, subscribeTopicOMOTE_BLEdeleteBonds.c_str(), 2);

}

void reconnect_mqtt(struct mqtt_client *mqttClient, void**) {
  printf("MQTT: will reconnect ...\r\n");

  mqtt_reinit(mqttClient, sockfd, sendmem1, sizeof(sendmem1), recvmem1, sizeof(recvmem1));

  std::string mqttClientName = std::string(MQTT_CLIENTNAME) + uniqueClientSuffix;
  //                       client_id,              will_topic, will_message, will_message_size, user_name, password,  connect_flags, keep_alive
  mqtt_connect(mqttClient, mqttClientName.c_str(), NULL,       NULL,         0,                 MQTT_USER, MQTT_PASS, 0,             30);
  if (mqttClient->error != MQTT_OK) {
    printf("MQTT: connect error: %s\r\n", mqtt_error_str(mqttClient->error));
    // sockfd = -1;
    // return;
  }
}

#if !defined(WIN32) && !defined(__APPLE__)
std::string getMACaddress() {
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, "eth0");
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    char buffer[6*3];
    int i;
    for (i = 0; i < 6; ++i) {
      sprintf(&buffer[i*3], "%02x:", (unsigned char) s.ifr_addr.sa_data[i]);
      // printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
    }
    //printf("\r\n");

    std::string MACaddress = std::string(buffer, 17);
    printf("  result in MACaddress(): %s\r\n", MACaddress.c_str());
    return MACaddress;
  }
  return "";
}
#endif

void init_mqtt_HAL(void) {
  #if defined(WIN32)
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("Failed to init sockets: %i\r\n", iResult);
        return; // return iResult;
    }
  #endif

  char MACaddress[6*3];
  sockfd = open_nb_socket(MQTT_SERVER, std::to_string(MQTT_SERVER_PORT).c_str(), MACaddress);
  if (sockfd == -1) {
    printf("MQTT: Failed to open socket\r\n");
    return;
  }

  #if !defined(WIN32)
  // MAC address is not the best. You cannot start more than one instance like that, otherwise the MQTT broker will only keep the last connection.
  // printf("MQTT: received MAC address from posix_sockets.h is %s\r\n", MACaddress);
  // uniqueClientSuffix = std::string(MACaddress, 18);
  // simply use a random number
  srand(time(NULL));   // Initialization, should only be called once.
  int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
  uniqueClientSuffix = "_linux_" + std::to_string(r);
  #else
  srand(time(NULL));   // Initialization, should only be called once.
  int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.
  uniqueClientSuffix = "_windows_" + std::to_string(r);
  #endif

  // printf("MQTT: MAC address from getMACaddress() in mqtt_hal_windows_linux.cpp is %s\r\n", getMACaddress().c_str());

  // printf("MQTT: will init ...\r\n");
  // mqtt_init(&mqttClient, sockfd, sendmem1, sizeof(sendmem1), recvmem1, sizeof(recvmem1), publish_callback);
  printf("MQTT: will init with reconnect ...\r\n");
  mqtt_init_reconnect(&mqttClient, reconnect_mqtt, NULL, publish_callback);
  reconnect_mqtt(&mqttClient, NULL);
  mqttClient.publish_response_callback_state = &state;

  mqtt_subscribeTopics();

  thisAnnounceWiFiconnected_cb(true);

}

void mqtt_loop_HAL() {
  if (sockfd != -1) {
    mqtt_sync(&mqttClient);
  }

}

bool publishMQTTMessage_HAL(const char *topic, const char *payload) {

    if (sockfd == -1) {
      init_mqtt_HAL();
    }
  
    mqtt_publish(&mqttClient, topic, payload, strlen(payload), MQTT_PUBLISH_QOS_0);
    if (mqttClient.error != MQTT_OK) {
      printf("MQTT: publish error %s\r\n", mqtt_error_str(mqttClient.error));
      sockfd = -1;
      return false;
    }

  return true;
}

void wifi_shutdown_HAL() {
  /* disconnect */
  if (sockfd != -1) {
    mqtt_disconnect(&mqttClient);
    mqtt_sync(&mqttClient);
  }

  #if defined(WIN32)
  WSACleanup();
  #endif

}

#endif
