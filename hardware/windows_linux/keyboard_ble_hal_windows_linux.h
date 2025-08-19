#pragma once

#if (ENABLE_KEYBOARD_BLE == 1)

#include <string>
#include <stdint.h>

typedef uint8_t MediaKeyReport[2];
const uint8_t KEY_UP_ARROW = 0;
const uint8_t KEY_DOWN_ARROW = 0;
const uint8_t KEY_RIGHT_ARROW = 0;
const uint8_t KEY_LEFT_ARROW = 0;
const uint8_t KEY_RETURN = 0;
const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 0};
const MediaKeyReport KEY_MEDIA_WWW_HOME = {0, 0};
const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {0, 0};
const MediaKeyReport KEY_MEDIA_REWIND = {0, 0};
const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {0, 0};
const MediaKeyReport KEY_MEDIA_FASTFORWARD = {0, 0};
const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {0, 0};
const MediaKeyReport KEY_MEDIA_MUTE = {0, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_UP = {0, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {0, 0};

void keyboardBLE_startAdvertisingForAll_HAL();
void keyboardBLE_startAdvertisingWithWhitelist_HAL(std::string peersAllowed);
void keyboardBLE_startAdvertisingDirected_HAL(std::string peerAddress, bool isRandomAddress);
void keyboardBLE_stopAdvertising_HAL();
void keyboardBLE_printConnectedClients_HAL();
void keyboardBLE_disconnectAllClients_HAL();
void keyboardBLE_printBonds_HAL();
std::string keyboardBLE_getBonds_HAL();
void keyboardBLE_deleteBonds_HAL();
bool keyboardBLE_forceConnectionToAddress_HAL(std::string peerAddress);
typedef void (*tAnnounceBLEmessage_cb)(std::string message);
extern tAnnounceBLEmessage_cb thisAnnounceBLEmessage_cb;
void set_announceBLEmessage_cb_HAL(tAnnounceBLEmessage_cb pAnnounceBLEmessage_cb);

void init_keyboardBLE_HAL();
bool keyboardBLE_isAdvertising_HAL();
bool keyboardBLE_isConnected_HAL();
void keyboardBLE_shutdown_HAL();
void keyboardBLE_write_HAL(uint8_t c);
void keyboardBLE_longpress_HAL(uint8_t c);
void keyboardBLE_home_HAL();
void keyboardBLE_sendString_HAL(const std::string &s);
void consumerControlBLE_write_HAL(const MediaKeyReport value);
void consumerControlBLE_longpress_HAL(const MediaKeyReport value);

#endif