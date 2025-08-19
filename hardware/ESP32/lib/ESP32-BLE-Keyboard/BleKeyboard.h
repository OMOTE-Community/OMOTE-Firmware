// This include is only needed to determine if NimBLE 1.4.x or 2.0.x is used.
// NimBLE 2.0.x is using nimble core 1.5, and only in this version BLE_STORE_OBJ_TYPE_LOCAL_IRK is defined
#include "nimble/nimble/host/include/host/ble_store.h"
#if defined(BLE_STORE_OBJ_TYPE_LOCAL_IRK)
#define NIMBLE_ARDUINO_2_x
#endif

#ifndef ESP32_BLE_KEYBOARD_H
#define ESP32_BLE_KEYBOARD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "NimBLECharacteristic.h"
#include "NimBLEServer.h"
#include "NimBLEHIDDevice.h"

#include "Print.h"

#define BLE_KEYBOARD_VERSION "0.0.4"
#define BLE_KEYBOARD_VERSION_MAJOR 0
#define BLE_KEYBOARD_VERSION_MINOR 0
#define BLE_KEYBOARD_VERSION_REVISION 4

const uint8_t KEY_LEFT_CTRL = 0x80;
const uint8_t KEY_LEFT_SHIFT = 0x81;
const uint8_t KEY_LEFT_ALT = 0x82;
const uint8_t KEY_LEFT_GUI = 0x83;
const uint8_t KEY_RIGHT_CTRL = 0x84;
const uint8_t KEY_RIGHT_SHIFT = 0x85;
const uint8_t KEY_RIGHT_ALT = 0x86;
const uint8_t KEY_RIGHT_GUI = 0x87;

const uint8_t KEY_UP_ARROW = 0xDA;
const uint8_t KEY_DOWN_ARROW = 0xD9;
const uint8_t KEY_LEFT_ARROW = 0xD8;
const uint8_t KEY_RIGHT_ARROW = 0xD7;
const uint8_t KEY_BACKSPACE = 0xB2;
const uint8_t KEY_TAB = 0xB3;
const uint8_t KEY_RETURN = 0xB0;
const uint8_t KEY_ESC = 0xB1;
const uint8_t KEY_INSERT = 0xD1;
const uint8_t KEY_PRTSC = 0xCE;
const uint8_t KEY_DELETE = 0xD4;
const uint8_t KEY_PAGE_UP = 0xD3;
const uint8_t KEY_PAGE_DOWN = 0xD6;
const uint8_t KEY_HOME = 0xD2;
const uint8_t KEY_END = 0xD5;
const uint8_t KEY_CAPS_LOCK = 0xC1;
const uint8_t KEY_F1 = 0xC2;
const uint8_t KEY_F2 = 0xC3;
const uint8_t KEY_F3 = 0xC4;
const uint8_t KEY_F4 = 0xC5;
const uint8_t KEY_F5 = 0xC6;
const uint8_t KEY_F6 = 0xC7;
const uint8_t KEY_F7 = 0xC8;
const uint8_t KEY_F8 = 0xC9;
const uint8_t KEY_F9 = 0xCA;
const uint8_t KEY_F10 = 0xCB;
const uint8_t KEY_F11 = 0xCC;
const uint8_t KEY_F12 = 0xCD;
const uint8_t KEY_F13 = 0xF0;
const uint8_t KEY_F14 = 0xF1;
const uint8_t KEY_F15 = 0xF2;
const uint8_t KEY_F16 = 0xF3;
const uint8_t KEY_F17 = 0xF4;
const uint8_t KEY_F18 = 0xF5;
const uint8_t KEY_F19 = 0xF6;
const uint8_t KEY_F20 = 0xF7;
const uint8_t KEY_F21 = 0xF8;
const uint8_t KEY_F22 = 0xF9;
const uint8_t KEY_F23 = 0xFA;
const uint8_t KEY_F24 = 0xFB;

const uint8_t KEY_NUM_0 = 0xEA;
const uint8_t KEY_NUM_1 = 0xE1;
const uint8_t KEY_NUM_2 = 0xE2;
const uint8_t KEY_NUM_3 = 0xE3;
const uint8_t KEY_NUM_4 = 0xE4;
const uint8_t KEY_NUM_5 = 0xE5;
const uint8_t KEY_NUM_6 = 0xE6;
const uint8_t KEY_NUM_7 = 0xE7;
const uint8_t KEY_NUM_8 = 0xE8;
const uint8_t KEY_NUM_9 = 0xE9;
const uint8_t KEY_NUM_SLASH = 0xDC;
const uint8_t KEY_NUM_ASTERISK = 0xDD;
const uint8_t KEY_NUM_MINUS = 0xDE;
const uint8_t KEY_NUM_PLUS = 0xDF;
const uint8_t KEY_NUM_ENTER = 0xE0;
const uint8_t KEY_NUM_PERIOD = 0xEB;

typedef uint8_t MediaKeyReport[2];

const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};
const MediaKeyReport KEY_MEDIA_WWW_HOME = {128, 0};
const MediaKeyReport KEY_MEDIA_LOCAL_MACHINE_BROWSER = {0, 1}; // Opens "My Computer" on Windows
// original from BLE-Keyboard
// const MediaKeyReport KEY_MEDIA_CALCULATOR = {0, 2};
// changed for usage in OMOTE
const MediaKeyReport KEY_MEDIA_FASTFORWARD = {0, 2};
const MediaKeyReport KEY_MEDIA_WWW_BOOKMARKS = {0, 4};
const MediaKeyReport KEY_MEDIA_WWW_SEARCH = {0, 8};
const MediaKeyReport KEY_MEDIA_WWW_STOP = {0, 16};
const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 32};
const MediaKeyReport KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION = {0, 64}; // Media Selection
// original from BLE-Keyboard
// const MediaKeyReport KEY_MEDIA_EMAIL_READER = {0, 128};
// changed for usage in OMOTE
const MediaKeyReport KEY_MEDIA_REWIND = {0, 128};


//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

typedef void (*tBLEKeyboardMessage_cb)(std::string message);

class BleKeyboard : public Print, public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks
{
private:
  NimBLEHIDDevice* hid;
  NimBLECharacteristic* inputKeyboard;
  NimBLECharacteristic* outputKeyboard;
  NimBLECharacteristic* inputMediaKeys;
  NimBLEAdvertising*    advertising;
  KeyReport             _keyReport;
  MediaKeyReport        _mediaKeyReport;
  std::string           deviceName;
  std::string           deviceManufacturer;
  uint8_t               batteryLevel;
  bool                  _advertising = false;
  bool                  connected = false;
  uint32_t              _delay_ms = 7;
  void delay_ms(uint64_t ms);

  uint16_t vid       = 0x05ac;
  uint16_t pid       = 0x820a;
  uint16_t version   = 0x0210;

  tBLEKeyboardMessage_cb thisBLEKeyboardMessage_cb = NULL;

public:
  BleKeyboard(std::string deviceName = "ESP32 Keyboard", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100);
  void begin(void);
  void end(void);
  void sendReport(KeyReport* keys);
  void sendReport(MediaKeyReport* keys);
  size_t press(uint8_t k);
  size_t press(const MediaKeyReport k);
  size_t release(uint8_t k);
  size_t release(const MediaKeyReport k);
  size_t write(uint8_t c);
  size_t write(const MediaKeyReport c);
  size_t write(const uint8_t *buffer, size_t size);
  void releaseAll(void);
  bool isAdvertising(void);
  bool isConnected(void);
  void setBatteryLevel(uint8_t level);
  void setName(std::string deviceName);  
  void setDelay(uint32_t ms);
  // For connecting multiple peers, please see this discussion: https://github.com/h2zero/NimBLE-Arduino/issues/651
  void startAdvertisingForAll();
  void startAdvertisingWithWhitelist(std::string peersAllowed);
  void startAdvertisingDirected(std::string peerAddress, bool isRandomAddress);
  void stopAdvertising();
  void printConnectedClients();
  void disconnectAllClients();
  void printBonds();
  std::string getBonds();
  void deleteBonds();
  bool startAdvertisingIfExactlyOneBondExists();
  bool advertiseAndWaitForConnection(std::string peerAddress);
  bool forceConnectionToAddress(std::string peerAddress);
  void set_BLEKeyboardMessage_cb(tBLEKeyboardMessage_cb pBLEKeyboardMessage_cb);

  void set_vendor_id(uint16_t vid);
  void set_product_id(uint16_t pid);
  void set_version(uint16_t version);
protected:
  #if !defined(NIMBLE_ARDUINO_2_x)
  // NimBLEServerCallbacks
  virtual void onConnect(NimBLEServer* pServer) override;
  virtual void onDisconnect(NimBLEServer* pServer) override;
  // NimBLECharacteristicCallbacks
  virtual void onWrite(NimBLECharacteristic* pCharacteristic) override;
  #else
  // NimBLEServerCallbacks
  virtual void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
  virtual void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, std::string& name);
  virtual void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
  virtual void onIdentity(NimBLEConnInfo& connInfo) override;
  // NimBLECharacteristicCallbacks
  virtual void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
  #endif
private:
  std::string getAddressTypeStr(NimBLEAddress address);
  void clearWhitelist();
  void prepareAdvertising();

};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_KEYBOARD_H
