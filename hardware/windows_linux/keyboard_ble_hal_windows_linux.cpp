#if (ENABLE_KEYBOARD_BLE == 1)

#include <string>
#include "keyboard_ble_hal_windows_linux.h"

// all messages are fake
void keyboardBLE_startAdvertisingForAll_HAL() {};
void keyboardBLE_startAdvertisingWithWhitelist_HAL(std::string peersAllowed) {};
void keyboardBLE_startAdvertisingDirected_HAL(std::string peerAddress, bool isRandomAddress) {
  thisAnnounceBLEmessage_cb("Direct advertising started to address " + peerAddress);
};
void keyboardBLE_stopAdvertising_HAL() {};
void keyboardBLE_printConnectedClients_HAL() {
  thisAnnounceBLEmessage_cb("Connected count: 1\n client 0: 11:22:33:44:55:66");
};
void keyboardBLE_disconnectAllClients_HAL() {};
void keyboardBLE_printBonds_HAL() {
  thisAnnounceBLEmessage_cb("NumBonds: 2\n bond 0: 11:22:33:44:55:66\n bond 1: 77:88:99:aa:bb:cc");
};
std::string keyboardBLE_getBonds_HAL() {return "11:22:33:44:55:66,77:88:99:aa:bb:cc";};
void keyboardBLE_deleteBonds_HAL() {};
bool keyboardBLE_forceConnectionToAddress_HAL(std::string peerAddress) {return true;};
tAnnounceBLEmessage_cb thisAnnounceBLEmessage_cb = NULL;
void set_announceBLEmessage_cb_HAL(tAnnounceBLEmessage_cb pAnnounceBLEmessage_cb) {
  // this is the callback in the commandHandler that we call from here
  thisAnnounceBLEmessage_cb = pAnnounceBLEmessage_cb;  
};

void init_keyboardBLE_HAL() {};
bool keyboardBLE_isAdvertising_HAL() {return false;};
bool keyboardBLE_isConnected_HAL() {return false;};
void keyboardBLE_shutdown_HAL() {};
void keyboardBLE_write_HAL(uint8_t c) {};
void keyboardBLE_longpress_HAL(uint8_t c) {};
void keyboardBLE_home_HAL() {};
void keyboardBLE_sendString_HAL(const std::string &s) {};
void consumerControlBLE_write_HAL(const MediaKeyReport value) {};
void consumerControlBLE_longpress_HAL(const MediaKeyReport value) {};

#endif