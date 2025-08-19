#pragma once

#include <list>
#include <string>
#include "applicationInternal/hardware/IRremoteProtocols.h"
#include "applicationInternal/hardware/arduinoLayer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// --- hardware general -------------------------------------------------------
void init_hardware_general(void);

// --- preferences ------------------------------------------------------------
void init_preferences(void);
void save_preferences(void);
std::string get_activeScene();
void set_activeScene(std::string anActiveScene);
std::string get_activeGUIname();
void set_activeGUIname(std::string anActiveGUIname);
int get_activeGUIlist();
void set_activeGUIlist(int anActiveGUIlist);
int get_lastActiveGUIlistIndex();
void set_lastActiveGUIlistIndex(int aGUIlistIndex);

// --- user led ---------------------------------------------------------------
void init_userled(void);
void update_userled();

// --- battery ----------------------------------------------------------------
void init_battery(void);
void get_battery_status(int *battery_voltage, int *battery_percentage, bool *battery_ischarging);

// --- sleep / IMU ------------------------------------------------------------
void init_sleep();
void init_IMU();
void check_activity();
void setLastActivityTimestamp();
uint32_t get_sleepTimeout();
void set_sleepTimeout(uint32_t aSleepTimeout);
bool get_wakeupByIMUEnabled();
void set_wakeupByIMUEnabled(bool aWakeupByIMUEnabled);
uint8_t get_motionThreshold();
void set_motionThreshold(uint8_t aMotionThreshold);

// --- keypad -----------------------------------------------------------------
void init_keys(void);
const char NO_KEY = '\0';
// -- has to be exactly the same structure as in the hardware implementations, because only a pointer is passed
const uint8_t keypadROWS = 5; //five rows
const uint8_t keypadCOLS = 5; //five columns
enum keypad_rawKeyStates {IDLE_RAW, PRESSED_RAW,       RELEASED_RAW};
struct rawKey {
  unsigned long timestampReceived;
  char keyChar;
  keypad_rawKeyStates rawKeyState;
};
// --
extern rawKey rawKeys[][keypadCOLS];
void getKeys(rawKey (*rawKeys)[keypadCOLS], unsigned long currentMillis);
#if(OMOTE_HARDWARE_REV >= 5)
void update_keyboardBrightness(void);
uint8_t get_keyboardBrightness();
void set_keyboardBrightness(uint8_t aKeyboardBrightness);
#endif

// --- SD card ----------------------------------------------------------------
#if(OMOTE_HARDWARE_REV >= 5)
void init_SD_card(void);
#endif

// --- IR sender --------------------------------------------------------------
void init_infraredSender(void);
void sendIRcode(int protocol, std::list<std::string> commandPayloads, std::string additionalPayload);

// --- IR receiver ------------------------------------------------------------
void start_infraredReceiver(void);
void shutdown_infraredReceiver(void);
void infraredReceiver_loop(void);
bool get_irReceiverEnabled();
void set_irReceiverEnabled(bool aIrReceiverEnabled);

// --- BLE keyboard -----------------------------------------------------------
#if (ENABLE_KEYBOARD_BLE == 1)
void init_keyboardBLE();
// used by "device_keyboard_ble.cpp", "sleep.cpp"
typedef uint8_t MediaKeyReport[2];
const uint8_t BLE_KEY_UP_ARROW = 0xDA;
const uint8_t BLE_KEY_DOWN_ARROW = 0xD9;
const uint8_t BLE_KEY_RIGHT_ARROW = 0xD7;
const uint8_t BLE_KEY_LEFT_ARROW = 0xD8;
const uint8_t BLE_KEY_RETURN = 0xB0;
const MediaKeyReport BLE_KEY_MEDIA_WWW_BACK = {0, 32};
const MediaKeyReport BLE_KEY_MEDIA_WWW_HOME = {128, 0};
const MediaKeyReport BLE_KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
const MediaKeyReport BLE_KEY_MEDIA_REWIND = {0, 128};
const MediaKeyReport BLE_KEY_MEDIA_PLAY_PAUSE = {8, 0};
const MediaKeyReport BLE_KEY_MEDIA_FASTFORWARD = {0, 2};
const MediaKeyReport BLE_KEY_MEDIA_NEXT_TRACK = {1, 0};
const MediaKeyReport BLE_KEY_MEDIA_MUTE = {16, 0};
const MediaKeyReport BLE_KEY_MEDIA_VOLUME_UP = {32, 0};
const MediaKeyReport BLE_KEY_MEDIA_VOLUME_DOWN = {64, 0};
void keyboardBLE_startAdvertisingForAll();
void keyboardBLE_startAdvertisingWithWhitelist(std::string peersAllowed);
void keyboardBLE_startAdvertisingDirected(std::string peerAddress, bool isRandomAddress);
void keyboardBLE_stopAdvertising();
void keyboardBLE_printConnectedClients();
void keyboardBLE_disconnectAllClients();
void keyboardBLE_printBonds();
std::string keyboardBLE_getBonds();
void keyboardBLE_deleteBonds();
bool keyboardBLE_forceConnectionToAddress(std::string peerAddress);
bool keyboardBLE_isAdvertising();
bool keyboardBLE_isConnected();
void keyboardBLE_shutdown();
void keyboardBLE_write(uint8_t c);
void keyboardBLE_longpress(uint8_t c);
void keyboardBLE_home();
void keyboardBLE_sendString(const std::string &s);
void consumerControlBLE_write(const MediaKeyReport value);
void consumerControlBLE_longpress(const MediaKeyReport value);
#endif

// --- tft --------------------------------------------------------------------
void update_backlightBrightness(void);
uint8_t get_backlightBrightness();
void set_backlightBrightness(uint8_t aBacklightBrightness);

// --- lvgl -------------------------------------------------------------------
void init_lvgl_hardware();

// --- WiFi / MQTT ------------------------------------------------------------
#if (ENABLE_WIFI_AND_MQTT == 1)
void init_mqtt(void);
// used by "commandHandler.cpp", "sleep.cpp"
bool getIsWifiConnected();
void mqtt_loop();
bool publishMQTTMessage(const char *topic, const char *payload);
void wifi_shutdown();
#endif

// --- memory usage -----------------------------------------------------------
void get_heapUsage(unsigned long *heapSize, unsigned long *freeHeap, unsigned long *maxAllocHeap, unsigned long *minFreeHeap);

// --- ESP-NOW ----------------------------------------------------------------
#if (ENABLE_HUB_COMMUNICATION == 1)
// ESP-NOW hardware presenter functions
void init_espnow();
void espnow_loop();
bool publishEspNowMessage(json payload);
void espnow_shutdown();
#endif
