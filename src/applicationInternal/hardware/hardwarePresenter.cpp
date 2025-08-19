#include <string>
#include <list>
#include "applicationInternal/hardware/hardwarePresenter.h"
// for registering the callback to show received IR messages
#include "guis/gui_irReceiver.h"
// for registering the callback to receive MQTT messages
#include "../commandHandler.h"
// for registering the callback to show WiFi status
#include "applicationInternal/gui/guiBase.h"

// This include of "hardwareLayer.h" is the one and only link to folder "hardware". The file "hardwareLayer.h" does the differentiation between ESP32 and Windows/Linux.
// "hardwareLayer.h" includes all the other hardware header files as well. So everything from all hardware header files is available here - and only here.
// This include has to be here in "hardwarePresenter.cpp" and not in "hardwarePresenter.h". Otherwise the whole rest of the code would have access to the hardware too.
// The rest of the code is only allowed to use "hardwarePresenter.h".
#include "hardwareLayer.h"

// --- hardware general -------------------------------------------------------
void init_hardware_general(void) {
  init_hardware_general_HAL();
}

// --- preferences ------------------------------------------------------------
void init_preferences(void) {
  init_preferences_HAL();
};
void save_preferences(void) {
  save_preferences_HAL();
};
std::string get_activeScene() {
  return get_activeScene_HAL();
}
void set_activeScene(std::string anActiveScene) {
  set_activeScene_HAL(anActiveScene);
}
std::string get_activeGUIname() {
  return get_activeGUIname_HAL();
}
void set_activeGUIname(std::string anActiveGUIname) {
  set_activeGUIname_HAL(anActiveGUIname);
}
int get_activeGUIlist() {
  return get_activeGUIlist_HAL();
}
void set_activeGUIlist(int anActiveGUIlist) {
  set_activeGUIlist_HAL(anActiveGUIlist);
}
int get_lastActiveGUIlistIndex() {
  return get_lastActiveGUIlistIndex_HAL();
}
void set_lastActiveGUIlistIndex(int aGUIlistIndex) {
  set_lastActiveGUIlistIndex_HAL(aGUIlistIndex);
}

// --- user led ---------------------------------------------------------------
void init_userled(void) {
  init_userled_HAL();
}
void update_userled() {
  update_userled_HAL();
}

// --- SD card ----------------------------------------------------------------
#if(OMOTE_HARDWARE_REV >= 5)
void init_SD_card(void) {
  init_SD_HAL();
}
#endif

// --- battery ----------------------------------------------------------------
void init_battery(void) {
  init_battery_HAL();
}
void get_battery_status(int *battery_voltage, int *battery_percentage, bool *battery_ischarging) {
  get_battery_status_HAL(battery_voltage, battery_percentage, battery_ischarging);
}

// --- sleep / IMU ------------------------------------------------------------
void init_sleep() {
  init_sleep_HAL();
};
void init_IMU() {
  init_IMU_HAL();
};
void check_activity() {
  check_activity_HAL();
};
void setLastActivityTimestamp() {
  setLastActivityTimestamp_HAL();
};
uint32_t get_sleepTimeout() {
  return get_sleepTimeout_HAL();
}
void set_sleepTimeout(uint32_t aSleepTimeout) {
  set_sleepTimeout_HAL(aSleepTimeout);
}
bool get_wakeupByIMUEnabled() {
  return get_wakeupByIMUEnabled_HAL();
}
void set_wakeupByIMUEnabled(bool aWakeupByIMUEnabled) {
  set_wakeupByIMUEnabled_HAL(aWakeupByIMUEnabled);
}
uint8_t get_motionThreshold() {
  return get_motionThreshold_HAL();
}
void set_motionThreshold(uint8_t aMotionThreshold) {
  set_motionThreshold_HAL(aMotionThreshold);
}

// --- keypad -----------------------------------------------------------------
void init_keys(void) {
  init_keys_HAL();  
}
// Used in keypad_getRawKeys to save the raw key states.
// Holds the raw keystates as received from the keypad (OMOTE_HARDWARE_REV <= 4), the TCA8418 (OMOTE_HARDWARE_REV >= 5) or the simulator.
// We expect only IDLE_PRESSED and IDLE_RELEASED, because this is what all three sources can deliver (only the keypad could also deliver IDLE and HOLD)
// The whole array is passed as a pointer to the hardware implementation, which fills it with the raw key states.
rawKey rawKeys[][keypadCOLS] = {
  {{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW}},
  {{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW}},
  {{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW}},
  {{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW}},
  {{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW},{0, NO_KEY, IDLE_RAW}},
};
void getKeys(rawKey rawKeys[keypadROWS][keypadCOLS], unsigned long currentMillis) {
  // we need to provide currentMillis to the hardware, because at least in case of the simulator there is no way to access millis()
  keys_getKeys_HAL(rawKeys, currentMillis);
}
#if(OMOTE_HARDWARE_REV >= 5)
void update_keyboardBrightness(void) {
  update_keyboardBrightness_HAL();
}
uint8_t get_keyboardBrightness() {
  return get_keyboardBrightness_HAL();
}
void set_keyboardBrightness(uint8_t aKeyboardBrightness){
  set_keyboardBrightness_HAL(aKeyboardBrightness);
}
#endif
// --- IR sender --------------------------------------------------------------
void init_infraredSender(void) {
  init_infraredSender_HAL();  
}
void sendIRcode(int protocol, std::list<std::string> commandPayloads, std::string additionalPayload) {
  sendIRcode_HAL(protocol, commandPayloads, additionalPayload);
}

// --- IR receiver ------------------------------------------------------------
void start_infraredReceiver(void) {
  start_infraredReceiver_HAL();
};
void shutdown_infraredReceiver(void) {
  shutdown_infraredReceiver_HAL();
};
void infraredReceiver_loop(void) {
  infraredReceiver_loop_HAL();
};
bool get_irReceiverEnabled() {
  return get_irReceiverEnabled_HAL();
}
void set_irReceiverEnabled(bool aIrReceiverEnabled) {
  if (aIrReceiverEnabled) {
    set_announceNewIRmessage_cb_HAL(&receiveNewIRmessage_cb);
  } else {
    set_announceNewIRmessage_cb_HAL(NULL);
  }
  set_irReceiverEnabled_HAL(aIrReceiverEnabled);
}

// --- BLE keyboard -----------------------------------------------------------
#if (ENABLE_KEYBOARD_BLE == 1)
void init_keyboardBLE() {
  set_announceBLEmessage_cb_HAL(&receiveBLEmessage_cb);
  init_keyboardBLE_HAL();
}
// used by "device_keyboard_ble.cpp", "sleep.cpp"

void keyboardBLE_startAdvertisingForAll() {
  keyboardBLE_startAdvertisingForAll_HAL();
}
void keyboardBLE_startAdvertisingWithWhitelist(std::string peersAllowed) {
  keyboardBLE_startAdvertisingWithWhitelist_HAL(peersAllowed);
}
void keyboardBLE_startAdvertisingDirected(std::string peerAddress, bool isRandomAddress) {
  keyboardBLE_startAdvertisingDirected_HAL(peerAddress, isRandomAddress);
}
void keyboardBLE_stopAdvertising() {
  keyboardBLE_stopAdvertising_HAL();
}
void keyboardBLE_printConnectedClients() {
  keyboardBLE_printConnectedClients_HAL();
}
void keyboardBLE_disconnectAllClients() {
  keyboardBLE_disconnectAllClients_HAL();
}
void keyboardBLE_printBonds() {
  keyboardBLE_printBonds_HAL();
}
std::string keyboardBLE_getBonds() {
  return keyboardBLE_getBonds_HAL();
}
void keyboardBLE_deleteBonds() {
  keyboardBLE_deleteBonds_HAL();
}
bool keyboardBLE_forceConnectionToAddress(std::string peerAddress) {
  return keyboardBLE_forceConnectionToAddress_HAL(peerAddress);
}
bool keyboardBLE_isAdvertising() {
  return keyboardBLE_isAdvertising_HAL();
}
bool keyboardBLE_isConnected() {
  return keyboardBLE_isConnected_HAL();
}
void keyboardBLE_shutdown() {
  keyboardBLE_shutdown_HAL();
}
void keyboardBLE_write(uint8_t c) {
  keyboardBLE_write_HAL(c);
}
void keyboardBLE_longpress(uint8_t c) {
  keyboardBLE_longpress_HAL(c);
}
void keyboardBLE_home() {
  keyboardBLE_home_HAL();
}
void keyboardBLE_sendString(const std::string &s) {
  keyboardBLE_sendString_HAL(s);
}
void consumerControlBLE_write(const MediaKeyReport value) {
  consumerControlBLE_write_HAL(value);
}
void consumerControlBLE_longpress(const MediaKeyReport value) {
  consumerControlBLE_longpress_HAL(value);
}
#endif

// --- tft --------------------------------------------------------------------
void update_backlightBrightness(void) {
  update_backlightBrightness_HAL();
}
uint8_t get_backlightBrightness() {
  return get_backlightBrightness_HAL();
}
void set_backlightBrightness(uint8_t aBacklightBrightness){
  set_backlightBrightness_HAL(aBacklightBrightness);
}

// --- lvgl -------------------------------------------------------------------
void init_lvgl_hardware() {
  init_lvgl_HAL();
};

// --- WiFi / MQTT ------------------------------------------------------------
#if (ENABLE_WIFI_AND_MQTT == 1)
void init_mqtt(void) {
  set_announceWiFiconnected_cb_HAL(&receiveWiFiConnected_cb);
  set_announceSubscribedTopics_cb_HAL(receiveMQTTmessage_cb);
  init_mqtt_HAL();
}
// used by "commandHandler.cpp", "sleep.cpp"
bool getIsWifiConnected() {
  return getIsWifiConnected_HAL();
}
void mqtt_loop() {
  mqtt_loop_HAL();
}
bool publishMQTTMessage(const char *topic, const char *payload) {
  return publishMQTTMessage_HAL(topic, payload);
}
void wifi_shutdown() {
  wifi_shutdown_HAL();
}
#endif

// --- memory usage -----------------------------------------------------------
void get_heapUsage(unsigned long *heapSize, unsigned long *freeHeap, unsigned long *maxAllocHeap, unsigned long *minFreeHeap) {
  get_heapUsage_HAL(heapSize, freeHeap, maxAllocHeap, minFreeHeap);
}

// --- ESP-NOW ----------------------------------------------------------------
#if (ENABLE_HUB_COMMUNICATION == 1)
// ESP-NOW hardware presenter functions
void init_espnow() {
  set_announceEspNowMessage_cb_HAL(&receiveEspNowMessage_cb);
  init_espnow_HAL();
}

void espnow_loop() {
  espnow_loop_HAL();
}

bool publishEspNowMessage(json payload) {
  return publishEspNowMessage_HAL(payload);
}

void espnow_shutdown() {
  espnow_shutdown_HAL();
}
#endif
