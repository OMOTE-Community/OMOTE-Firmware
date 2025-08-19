#include <sstream>
#include "BleKeyboard.h"

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#include "HIDTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"


#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "NimBLEDevice";
#endif


// Report IDs:
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop Ctrls)
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  // ------------------------------------------------- Keyboard
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1)
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0)
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   Logical Maximum (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 1 byte (Reserved)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8)
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5) ; 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) ; Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) ; Kana
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 3 bits (Padding)
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3)
  HIDOUTPUT(1),       0x01,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6) ; 6 bytes (Keys)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  // https://github.com/T-vK/ESP32-BLE-Keyboard/issues/11
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- Media Keys
  USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
  USAGE(1),           0x01,          // USAGE (Consumer Control)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  REPORT_ID(1),       MEDIA_KEYS_ID, //   REPORT_ID (3)
  USAGE_PAGE(1),      0x0C,          //   USAGE_PAGE (Consumer)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x10,          //   REPORT_COUNT (16)
  USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
  USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
  USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
  USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
  USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
  USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
  USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
  USAGE(2),           0x23, 0x02,    //   Usage (WWW Home)            ; bit 7: 128
  USAGE(2),           0x94, 0x01,    //   Usage (My Computer) ; bit 0: 1
  // original from BLE-Keyboard
  // USAGE(2),           0x92, 0x01,    //   Usage (Calculator)  ; bit 1: 2
  // changed for usage in OMOTE
  USAGE(1),           0xB3,          //   USAGE (Fast Forward); bit 1: 2
  USAGE(2),           0x2A, 0x02,    //   Usage (WWW fav)     ; bit 2: 4
  USAGE(2),           0x21, 0x02,    //   Usage (WWW search)  ; bit 3: 8
  USAGE(2),           0x26, 0x02,    //   Usage (WWW stop)    ; bit 4: 16
  USAGE(2),           0x24, 0x02,    //   Usage (WWW back)    ; bit 5: 32
  USAGE(2),           0x83, 0x01,    //   Usage (Media sel)   ; bit 6: 64
  // original from BLE-Keyboard
  // USAGE(2),           0x8A, 0x01,    //   Usage (Mail)        ; bit 7: 128
  // changed for usage in OMOTE
  USAGE(1),           0xB4,          //   USAGE (Rewind)      ; bit 7: 128
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0)                  // END_COLLECTION
};

BleKeyboard::BleKeyboard(std::string deviceName, std::string deviceManufacturer, uint8_t batteryLevel) 
    : hid(0)
    , deviceName(std::string(deviceName).substr(0, 15))
    , deviceManufacturer(std::string(deviceManufacturer).substr(0,15))
    , batteryLevel(batteryLevel) {}

void BleKeyboard::begin(void)
{
  NimBLEDevice::init(deviceName);
  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(this);

  hid = new NimBLEHIDDevice(pServer);
  #if !defined(NIMBLE_ARDUINO_2_x)
  inputKeyboard = hid->inputReport(KEYBOARD_ID);  // <-- input REPORTID from report map
  outputKeyboard = hid->outputReport(KEYBOARD_ID);
  inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);
  #else
  inputKeyboard = hid->getInputReport(KEYBOARD_ID);  // <-- input REPORTID from report map
  outputKeyboard = hid->getOutputReport(KEYBOARD_ID);
  inputMediaKeys = hid->getInputReport(MEDIA_KEYS_ID);
  #endif

  outputKeyboard->setCallbacks(this);

  #if !defined(NIMBLE_ARDUINO_2_x)
  hid->manufacturer()->setValue(deviceManufacturer);

  hid->pnp(0x02, vid, pid, version);
  hid->hidInfo(0x00, 0x01);
  #else
  hid->setManufacturer(deviceManufacturer);

  hid->setPnp(0x02, vid, pid, version);
  hid->setHidInfo(0x00, 0x01);
  #endif


  NimBLEDevice::setSecurityAuth(true, true, true);

  #if !defined(NIMBLE_ARDUINO_2_x)
  hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  #else
  hid->setReportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  #endif
  hid->startServices();

  advertising = pServer->getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  #if !defined(NIMBLE_ARDUINO_2_x)
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
  #else
  advertising->addServiceUUID(hid->getHidService()->getUUID());
  advertising->enableScanResponse(false);
  #endif
  
  /*
    Don't start advertising here. Call one of
      startAdvertisingForAll();
      startAdvertisingWithWhitelist(std::string peersAllowed);
      startAdvertisingDirected(std::string peerAddress, bool isRandomAddress);
    In case only one peer is bonded, startAdvertisingForAll() is called on initialisation. This is done in startAdvertisingIfExactlyOneBondExists(), which is called in file "keyboard_ble_hal_esp32.cpp" in "void init_keyboardBLE_HAL()"
  */
  //advertising->start();
  //this->_advertising = true;

  // don't do automatic advertising on disconnect
  pServer->advertiseOnDisconnect(false);

  hid->setBatteryLevel(batteryLevel);

  ESP_LOGD(LOG_TAG, "Finished BleKeyboard::begin");
}

void BleKeyboard::end(void)
{
  // deinit() does a completely shutdown of the device. Too much for our needs.
  // * @brief Shutdown the NimBLE stack/controller.
  // * @param [in] clearAll If true, deletes all server/advertising/scan/client objects after deinitializing.
  // * @note If clearAll is true when called, any references to the created objects become invalid.
  // NimBLEDevice::deinit();
}

bool BleKeyboard::isAdvertising(void) {
  return this->_advertising;
}

bool BleKeyboard::isConnected(void) {
  return this->connected;
}

void BleKeyboard::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
    this->hid->setBatteryLevel(this->batteryLevel);
}

//must be called before begin in order to set the name
void BleKeyboard::setName(std::string deviceName) {
  this->deviceName = deviceName;
}

/**
 * @brief Sets the waiting time (in milliseconds) between multiple keystrokes in NimBLE mode.
 * 
 * @param ms Time in milliseconds
 */
void BleKeyboard::setDelay(uint32_t ms) {
  this->_delay_ms = ms;
}

void BleKeyboard::set_vendor_id(uint16_t vid) { 
	this->vid = vid; 
}

void BleKeyboard::set_product_id(uint16_t pid) { 
	this->pid = pid; 
}

void BleKeyboard::set_version(uint16_t version) { 
	this->version = version; 
}

void BleKeyboard::sendReport(KeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
    this->inputKeyboard->notify();
    // vTaskDelay(delayTicks);
    this->delay_ms(_delay_ms);
  }
}

void BleKeyboard::sendReport(MediaKeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
    this->inputMediaKeys->notify();
    //vTaskDelay(delayTicks);
    this->delay_ms(_delay_ms);
  }
}

extern
const uint8_t _asciimap[128] PROGMEM;

#define SHIFT 0x80
const uint8_t _asciimap[128] =
{
	0x00,             // NUL
	0x00,             // SOH
	0x00,             // STX
	0x00,             // ETX
	0x00,             // EOT
	0x00,             // ENQ
	0x00,             // ACK
	0x00,             // BEL
	0x2a,			// BS	Backspace
	0x2b,			// TAB	Tab
	0x28,			// LF	Enter
	0x00,             // VT
	0x00,             // FF
	0x00,             // CR
	0x00,             // SO
	0x00,             // SI
	0x00,             // DEL
	0x00,             // DC1
	0x00,             // DC2
	0x00,             // DC3
	0x00,             // DC4
	0x00,             // NAK
	0x00,             // SYN
	0x00,             // ETB
	0x00,             // CAN
	0x00,             // EM
	0x00,             // SUB
	0x00,             // ESC
	0x00,             // FS
	0x00,             // GS
	0x00,             // RS
	0x00,             // US

	0x2c,		   //  ' '
	0x1e|SHIFT,	   // !
	0x34|SHIFT,	   // "
	0x20|SHIFT,    // #
	0x21|SHIFT,    // $
	0x22|SHIFT,    // %
	0x24|SHIFT,    // &
	0x34,          // '
	0x26|SHIFT,    // (
	0x27|SHIFT,    // )
	0x25|SHIFT,    // *
	0x2e|SHIFT,    // +
	0x36,          // ,
	0x2d,          // -
	0x37,          // .
	0x38,          // /
	0x27,          // 0
	0x1e,          // 1
	0x1f,          // 2
	0x20,          // 3
	0x21,          // 4
	0x22,          // 5
	0x23,          // 6
	0x24,          // 7
	0x25,          // 8
	0x26,          // 9
	0x33|SHIFT,      // :
	0x33,          // ;
	0x36|SHIFT,      // <
	0x2e,          // =
	0x37|SHIFT,      // >
	0x38|SHIFT,      // ?
	0x1f|SHIFT,      // @
	0x04|SHIFT,      // A
	0x05|SHIFT,      // B
	0x06|SHIFT,      // C
	0x07|SHIFT,      // D
	0x08|SHIFT,      // E
	0x09|SHIFT,      // F
	0x0a|SHIFT,      // G
	0x0b|SHIFT,      // H
	0x0c|SHIFT,      // I
	0x0d|SHIFT,      // J
	0x0e|SHIFT,      // K
	0x0f|SHIFT,      // L
	0x10|SHIFT,      // M
	0x11|SHIFT,      // N
	0x12|SHIFT,      // O
	0x13|SHIFT,      // P
	0x14|SHIFT,      // Q
	0x15|SHIFT,      // R
	0x16|SHIFT,      // S
	0x17|SHIFT,      // T
	0x18|SHIFT,      // U
	0x19|SHIFT,      // V
	0x1a|SHIFT,      // W
	0x1b|SHIFT,      // X
	0x1c|SHIFT,      // Y
	0x1d|SHIFT,      // Z
	0x2f,          // [
	0x31,          // bslash
	0x30,          // ]
	0x23|SHIFT,    // ^
	0x2d|SHIFT,    // _
	0x35,          // `
	0x04,          // a
	0x05,          // b
	0x06,          // c
	0x07,          // d
	0x08,          // e
	0x09,          // f
	0x0a,          // g
	0x0b,          // h
	0x0c,          // i
	0x0d,          // j
	0x0e,          // k
	0x0f,          // l
	0x10,          // m
	0x11,          // n
	0x12,          // o
	0x13,          // p
	0x14,          // q
	0x15,          // r
	0x16,          // s
	0x17,          // t
	0x18,          // u
	0x19,          // v
	0x1a,          // w
	0x1b,          // x
	0x1c,          // y
	0x1d,          // z
	0x2f|SHIFT,    // {
	0x31|SHIFT,    // |
	0x30|SHIFT,    // }
	0x35|SHIFT,    // ~
	0				// DEL
};


uint8_t USBPutChar(uint8_t c);

// press() adds the specified key (printing, non-printing, or modifier)
// to the persistent key report and sends the report.  Because of the way
// USB HID works, the host acts like the key remains pressed until we
// call release(), releaseAll(), or otherwise clear the report and resend.
size_t BleKeyboard::press(uint8_t k)
{
	uint8_t i;
	if (k >= 136) {			// it's a non-printing key (not a modifier)
		k = k - 136;
	} else if (k >= 128) {	// it's a modifier key
		_keyReport.modifiers |= (1<<(k-128));
		k = 0;
	} else {				// it's a printing key
		k = pgm_read_byte(_asciimap + k);
		if (!k) {
			setWriteError();
			return 0;
		}
		if (k & 0x80) {						// it's a capital letter or other character reached with shift
			_keyReport.modifiers |= 0x02;	// the left shift modifier
			k &= 0x7F;
		}
	}

	// Add k to the key report only if it's not already present
	// and if there is an empty slot.
	if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
		_keyReport.keys[2] != k && _keyReport.keys[3] != k &&
		_keyReport.keys[4] != k && _keyReport.keys[5] != k) {

		for (i=0; i<6; i++) {
			if (_keyReport.keys[i] == 0x00) {
				_keyReport.keys[i] = k;
				break;
			}
		}
		if (i == 6) {
			setWriteError();
			return 0;
		}
	}
	sendReport(&_keyReport);
	return 1;
}

size_t BleKeyboard::press(const MediaKeyReport k)
{
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);

    mediaKeyReport_16 |= k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

	sendReport(&_mediaKeyReport);
	return 1;
}

// release() takes the specified key out of the persistent key report and
// sends the report.  This tells the OS the key is no longer pressed and that
// it shouldn't be repeated any more.
size_t BleKeyboard::release(uint8_t k)
{
	uint8_t i;
	if (k >= 136) {			// it's a non-printing key (not a modifier)
		k = k - 136;
	} else if (k >= 128) {	// it's a modifier key
		_keyReport.modifiers &= ~(1<<(k-128));
		k = 0;
	} else {				// it's a printing key
		k = pgm_read_byte(_asciimap + k);
		if (!k) {
			return 0;
		}
		if (k & 0x80) {							// it's a capital letter or other character reached with shift
			_keyReport.modifiers &= ~(0x02);	// the left shift modifier
			k &= 0x7F;
		}
	}

	// Test the key report to see if k is present.  Clear it if it exists.
	// Check all positions in case the key is present more than once (which it shouldn't be)
	for (i=0; i<6; i++) {
		if (0 != k && _keyReport.keys[i] == k) {
			_keyReport.keys[i] = 0x00;
		}
	}

	sendReport(&_keyReport);
	return 1;
}

size_t BleKeyboard::release(const MediaKeyReport k)
{
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
    mediaKeyReport_16 &= ~k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

	sendReport(&_mediaKeyReport);
	return 1;
}

void BleKeyboard::releaseAll(void)
{
	_keyReport.keys[0] = 0;
	_keyReport.keys[1] = 0;
	_keyReport.keys[2] = 0;
	_keyReport.keys[3] = 0;
	_keyReport.keys[4] = 0;
	_keyReport.keys[5] = 0;
	_keyReport.modifiers = 0;
    _mediaKeyReport[0] = 0;
    _mediaKeyReport[1] = 0;
	sendReport(&_keyReport);
	sendReport(&_mediaKeyReport);
}

size_t BleKeyboard::write(uint8_t c)
{
	uint8_t p = press(c);  // Keydown
	release(c);            // Keyup
	return p;              // just return the result of press() since release() almost always returns 1
}

size_t BleKeyboard::write(const MediaKeyReport c)
{
	uint16_t p = press(c);  // Keydown
	release(c);            // Keyup
	return p;              // just return the result of press() since release() almost always returns 1
}

size_t BleKeyboard::write(const uint8_t *buffer, size_t size) {
	size_t n = 0;
	while (size--) {
		if (*buffer != '\r') {
			if (write(*buffer)) {
			  n++;
			} else {
			  break;
			}
		}
		buffer++;
	}
	return n;
}

#if !defined(NIMBLE_ARDUINO_2_x)
void BleKeyboard::onConnect(NimBLEServer* pServer) {
#else
void BleKeyboard::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
#endif
  this->connected = true;
  this->_advertising = false;
  std::string message = "";
  char buffer[200];
  if (pServer->getConnectedCount() == 1) {
    #if !defined(NIMBLE_ARDUINO_2_x)
    NimBLEConnInfo connInfo = pServer->getPeerInfo(0);
    #endif
    sprintf(buffer, "BleKeyboard: onConnect: client %s%s, id %s%s, handle %u, isBonded %d",
      NimBLEAddress(connInfo.getAddress()).toString().c_str(),
      this->getAddressTypeStr(connInfo.getAddress()).c_str(),
      NimBLEAddress(connInfo.getIdAddress()).toString().c_str(),
      this->getAddressTypeStr(connInfo.getIdAddress()).c_str(),
      connInfo.getConnHandle(),
      connInfo.isBonded());
  } else {
    sprintf(buffer, "BleKeyboard: onConnect: more than one device connected. How can it be???");
  }
	ESP_LOGI(LOG_TAG, "%s", buffer);
  if (thisBLEKeyboardMessage_cb != NULL) {
    message = buffer;
    thisBLEKeyboardMessage_cb(message);
  }

}

#if defined(NIMBLE_ARDUINO_2_x)
void BleKeyboard::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, std::string& name) {
  this->connected = true;
  this->_advertising = false;
  std::string message = "";
  char buffer[200];
  if (pServer->getConnectedCount() == 1) {
    #if !defined(NIMBLE_ARDUINO_2_x)
    NimBLEConnInfo connInfo = pServer->getPeerInfo(0);
    #endif
    sprintf(buffer, "BleKeyboard: onConnect with name: client %s%s, id %s%s, handle %u, isBonded %d, name %s",
      NimBLEAddress(connInfo.getAddress()).toString().c_str(),
      this->getAddressTypeStr(connInfo.getAddress()).c_str(),
      NimBLEAddress(connInfo.getIdAddress()).toString().c_str(),
      this->getAddressTypeStr(connInfo.getIdAddress()).c_str(),
      connInfo.getConnHandle(),
      connInfo.isBonded(),
      name.c_str());
  } else {
    sprintf(buffer, "BleKeyboard: onConnect with name: more than one device connected. How can it be???");
  }
	ESP_LOGI(LOG_TAG, "%s", buffer);
  if (thisBLEKeyboardMessage_cb != NULL) {
    message = buffer;
    thisBLEKeyboardMessage_cb(message);
  }

}
#endif

#if !defined(NIMBLE_ARDUINO_2_x)
void BleKeyboard::onDisconnect(NimBLEServer* pServer) {
#else
void BleKeyboard::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
#endif
  this->connected = false;
  std::string message = "";
  char buffer[200];
  if (pServer->getConnectedCount() == 0) {
  	sprintf(buffer, "BleKeyboard: onDisconnect: no clients connected");
  } else if (pServer->getConnectedCount() == 1) {
    #if !defined(NIMBLE_ARDUINO_2_x)
    NimBLEConnInfo connInfo = pServer->getPeerInfo(0);
    #endif
    sprintf(buffer, "BleKeyboard: onDisconnect: there is still a client connected %s%s, id %s%s, handle %u, isBonded %d. How can it be???",
      NimBLEAddress(connInfo.getAddress()).toString().c_str(),
      this->getAddressTypeStr(connInfo.getAddress()).c_str(),
      NimBLEAddress(connInfo.getIdAddress()).toString().c_str(),
      this->getAddressTypeStr(connInfo.getIdAddress()).c_str(),
      connInfo.getConnHandle(),
      connInfo.isBonded());
  } else {
  	sprintf(buffer, "BleKeyboard: onDisconnect: there are still %d devices connected. How can it be???", pServer->getConnectedCount());
  }
  ESP_LOGI(LOG_TAG, "%s", buffer);
  if (thisBLEKeyboardMessage_cb != NULL) {
    message = buffer;
    thisBLEKeyboardMessage_cb(message);
  }

}

#if defined(NIMBLE_ARDUINO_2_x)
void BleKeyboard::onIdentity(NimBLEConnInfo& connInfo) {
  std::string message = "";
  char buffer[200];
  sprintf(buffer, "BleKeyboard: onIdentity: client %s%s, id %s%s, handle %u, isBonded %d",
    NimBLEAddress(connInfo.getAddress()).toString().c_str(),
    this->getAddressTypeStr(connInfo.getAddress()).c_str(),
    NimBLEAddress(connInfo.getIdAddress()).toString().c_str(),
    this->getAddressTypeStr(connInfo.getIdAddress()).c_str(),
    connInfo.getConnHandle(),
    connInfo.isBonded());
	ESP_LOGI(LOG_TAG, "%s", buffer);
  if (thisBLEKeyboardMessage_cb != NULL) {
    message = buffer;
    thisBLEKeyboardMessage_cb(message);
  }
}
#endif

#if !defined(NIMBLE_ARDUINO_2_x)
void BleKeyboard::onWrite(NimBLECharacteristic* pCharacteristic) {
#else
void BleKeyboard::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
#endif
  uint8_t* value = (uint8_t*)(pCharacteristic->getValue().c_str());
  (void)value;
  ESP_LOGI(LOG_TAG, "special keys: %d", *value);
}

void BleKeyboard::delay_ms(uint64_t ms) {
  uint64_t m = esp_timer_get_time();
  if(ms){
    uint64_t e = (m + (ms * 1000));
    if(m > e){ //overflow
        while(esp_timer_get_time() > e) { }
    }
    while(esp_timer_get_time() < e) {}
  }
}

std::string BleKeyboard::getAddressTypeStr(NimBLEAddress address) {
  if        (address.getType() == BLE_ADDR_PUBLIC) {
    return ""; // this is the default, don't mention it
  } else if (address.getType() == BLE_ADDR_RANDOM) {
    return " (random)";
  } else if (address.getType() == BLE_ADDR_PUBLIC_ID) {
    return " (public ID)";
  } else if (address.getType() == BLE_ADDR_RANDOM_ID) {
    return " (random ID)";
  } else {
    return "";
  }
}

void BleKeyboard::clearWhitelist() {
  // clear whitelist, if there is any
  if (NimBLEDevice::getWhiteListCount() != 0) {
    for (int i=0; i < NimBLEDevice::getWhiteListCount() -1; i++) {
      ESP_LOGI(LOG_TAG, "Will remove %s from whitelist", NimBLEDevice::getWhiteListAddress(i).toString().c_str());
      NimBLEDevice::whiteListRemove(NimBLEDevice::getWhiteListAddress(i));
    }
  }
}

void BleKeyboard::prepareAdvertising() {
  // stop advertising
  if (advertising->isAdvertising()) {
    advertising->stop();
    this->_advertising = false;
  }
  this->disconnectAllClients();
  this->clearWhitelist();
}

void BleKeyboard::startAdvertisingForAll() {
  // All peers can see the advertising, and all peers are allowed to connect.

  this->prepareAdvertising();

  advertising->setScanFilter(false, false);
  #if !defined(NIMBLE_ARDUINO_2_x)
  advertising->setAdvertisementType(BLE_GAP_CONN_MODE_UND);
  #else
  advertising->setConnectableMode(BLE_GAP_CONN_MODE_UND);
  #endif
  advertising->start();
  this->_advertising = true;

  ESP_LOGI(LOG_TAG, "Advertising started for all");
}

void BleKeyboard::startAdvertisingWithWhitelist(std::string peersAllowed) {
  // All peers can see the advertising, but only peers on the whitelist are allowed to connect.
  
  this->prepareAdvertising();

  // Add peers to whitelist.
  // Only public addresses in the whitelist are supported by this method. If you need random addresses, please change the code accordingly.
  // the peersAllowed are a comma separated list, which we have to break into single addresses
  std::stringstream ss(peersAllowed);
  while(ss.good())  {
    std::string address;
    std::getline(ss, address, ',');
    ESP_LOGI(LOG_TAG, "Will add %s to whitelist", address.c_str());
    NimBLEDevice::whiteListAdd(NimBLEAddress(address, BLE_ADDR_PUBLIC));
  }

  advertising->setScanFilter(true, true);
  #if !defined(NIMBLE_ARDUINO_2_x)
  advertising->setAdvertisementType(BLE_GAP_CONN_MODE_UND);
  #else
  advertising->setConnectableMode(BLE_GAP_CONN_MODE_UND);
  #endif
  advertising->start();
  this->_advertising = true;

  ESP_LOGI(LOG_TAG, "Advertising started with whitelist");
}

void BleKeyboard::startAdvertisingDirected(std::string peerAddress, bool isRandomAddress) {
  // Only one single peer can see (reacts to) the advertisement.
  // This only works for already bonded peers!

  this->prepareAdvertising();

  NimBLEAddress directedAddress;
  if (isRandomAddress) {
    directedAddress = NimBLEAddress(peerAddress, BLE_ADDR_RANDOM);
  } else {
    directedAddress = NimBLEAddress(peerAddress, BLE_ADDR_PUBLIC);
  }
  advertising->setScanFilter(false, false);
  #if !defined(NIMBLE_ARDUINO_2_x)
  advertising->setAdvertisementType(BLE_GAP_CONN_MODE_DIR);
  advertising->start(BLE_HS_FOREVER, nullptr, &directedAddress);
  #else
  advertising->setConnectableMode(BLE_GAP_CONN_MODE_DIR);
  advertising->start(BLE_HS_FOREVER, &directedAddress);
  #endif
  this->_advertising = true;

  std::string message = "";
  char buffer[100];
  sprintf(buffer, "Direct advertising started to address %s%s",
    directedAddress.toString().c_str(),
    this->getAddressTypeStr(directedAddress).c_str());
  ESP_LOGI(LOG_TAG, "%s", buffer);
  if (thisBLEKeyboardMessage_cb != NULL) {
    message = buffer;
    thisBLEKeyboardMessage_cb(message);
  }
}

void BleKeyboard::stopAdvertising() {
  advertising->stop();
  this->_advertising = false;
  ESP_LOGI(LOG_TAG, "Advertising stopped");
}

void BleKeyboard::printConnectedClients() {
  std::string message = "";
  char buffer[50];

  sprintf(buffer, "Connected count: %d", NimBLEDevice::getServer()->getConnectedCount());
  message = buffer;
  ESP_LOGI(LOG_TAG, "%s", buffer);

  std::vector<uint16_t> m_connectedPeersVec = NimBLEDevice::getServer()->getPeerDevices();
  for (std::vector<uint16_t>::iterator it = m_connectedPeersVec.begin() ; it != m_connectedPeersVec.end(); ++it) {
    #if !defined(NIMBLE_ARDUINO_2_x)
    NimBLEConnInfo connInfo = NimBLEDevice::getServer()->getPeerIDInfo(*it);
    #else
    NimBLEConnInfo connInfo = NimBLEDevice::getServer()->getPeerInfoByHandle(*it);
    #endif
    sprintf(buffer, "\n client %d: %s", *it, NimBLEAddress(connInfo.getAddress()).toString().c_str());
    message = message + buffer;
    ESP_LOGI(LOG_TAG, "%s", buffer);
  }
  if (thisBLEKeyboardMessage_cb != NULL) {
    thisBLEKeyboardMessage_cb(message);
  }
}

void BleKeyboard::disconnectAllClients() {
  std::vector<uint16_t> m_connectedPeersVec = NimBLEDevice::getServer()->getPeerDevices();
  for (std::vector<uint16_t>::iterator it = m_connectedPeersVec.begin() ; it != m_connectedPeersVec.end(); ++it) {
    ESP_LOGI(LOG_TAG, "Will disconnect peer: %u", *it);
    // https://github.com/espressif/esp-idf/issues/8555
    NimBLEDevice::getServer()->disconnect(*it, 0x13);
  }
}

void BleKeyboard::printBonds() {
  std::string message = "";
  char buffer[50];

  // https://github.com/h2zero/NimBLE-Arduino/issues/579

  sprintf(buffer, "NumBonds: %d", NimBLEDevice::getNumBonds());
  message = buffer;
  ESP_LOGI(LOG_TAG, "%s", buffer);

  for (int i=0; i<NimBLEDevice::getNumBonds(); i++) {
    sprintf(buffer, "\n bond %d: %s", i, NimBLEDevice::getBondedAddress(i).toString().c_str());
    message = message + buffer;
    ESP_LOGI(LOG_TAG, "%s", buffer);
  }
  if (thisBLEKeyboardMessage_cb != NULL) {
    thisBLEKeyboardMessage_cb(message);
  }
}

std::string BleKeyboard::getBonds() {
  std::string result = "";
  for (int i=0; i<NimBLEDevice::getNumBonds(); i++) {
    if (result == "") {
      result += NimBLEDevice::getBondedAddress(i).toString();
    } else {
      result += ",";
      result += NimBLEDevice::getBondedAddress(i).toString();
    }
  }
  return result;
}

void BleKeyboard::deleteBonds() {
  // https://piratecomm.wordpress.com/2014/01/19/ble-pairing-vs-bonding/
  NimBLEDevice::deleteAllBonds();
}

bool BleKeyboard::startAdvertisingIfExactlyOneBondExists() {
  if (NimBLEDevice::getNumBonds() == 1) {
    ESP_LOGI(LOG_TAG, "BleKeyboard: exactly one bond exists, will start advertising.\n");
    startAdvertisingForAll();
    // why do we use startAdvertisingForAll() and not startAdvertisingDirected()?
    // Because direct advertising does not work for some devices.
    // To support such devices, non direct advertising is more stable. It is safe because we have only one bonded peer.
    // But of course, direct advertisement could also be used.
    // startAdvertisingDirected(
    //   NimBLEDevice::getBondedAddress(0).toString(),
    //   NimBLEDevice::getBondedAddress(0).getType() == BLE_ADDR_PUBLIC ? false : true);
    return true;
  } else {
    ESP_LOGI(LOG_TAG, "BleKeyboard: will not start advertising on init.\n");
    return false;
  }
}

bool BleKeyboard::advertiseAndWaitForConnection(std::string peerAddress) {
  int connectionTimeout   = 10000; // wait until connection is established
  int waitAfterConnection = 2000;  // wait after connection was established, before key is actually being sent

  if (peerAddress == "") {
    startAdvertisingForAll();
    // we will now wait for connection
  } else {
    bool bondFound = false;
    for (int i=0; i<NimBLEDevice::getNumBonds(); i++) {
      if (NimBLEDevice::getBondedAddress(i).toString() == peerAddress) {
        bondFound = true;
        startAdvertisingDirected(
          NimBLEDevice::getBondedAddress(i).toString(),
          NimBLEDevice::getBondedAddress(i).getType() == BLE_ADDR_PUBLIC ? false : true);
         // we will now wait for connection
      }   
    }
    if (!bondFound) {
      ESP_LOGW(LOG_TAG, "BleKeyboard: cannot start direct advertising to address %s, because this address is not bonded\n", peerAddress.c_str());
      return false;
    }
  }
  // advertising was started. Wait for connection
  ESP_LOGD(LOG_TAG, "BleKeyboard: will do a blocking wait for connection\n");

  unsigned long startTimer = millis();
  while ((millis() - startTimer < connectionTimeout) && !this->connected) {
    delay(10);
  }
  if (this->connected) {
    ESP_LOGD(LOG_TAG, "BleKeyboard: connection established\n");
    // have to wait a little bit before sending the first key, otherwise they sometimes do not work ...
    startTimer = millis();
    while (millis() - startTimer < waitAfterConnection) {
      delay(10);
    }
    return true;
  } else {
    ESP_LOGW(LOG_TAG, "BleKeyboard: connection failed\n");
    return false;
  }
}

bool BleKeyboard::forceConnectionToAddress(std::string peerAddress) {

  if (NimBLEDevice::getServer()->getConnectedCount() == 0) {
    if (NimBLEDevice::getNumBonds() == 0) {
      ESP_LOGW(LOG_TAG, "BleKeyboard: currently no client bonded. Please first pair a device. Please see the OMOTE Wiki. Cannot send key.\n");
      return false;
    }
    if (peerAddress == "") {
      ESP_LOGD(LOG_TAG, "BleKeyboard: currently no client connected. No specific address was provided. Will start advertising.\n");
    } else {
      ESP_LOGD(LOG_TAG, "BleKeyboard: currently no client connected. Will start direct advertising to address %s.\n", peerAddress.c_str());
    }
    
    return advertiseAndWaitForConnection(peerAddress);
  }

  if (NimBLEDevice::getServer()->getConnectedCount() == 1) {
    if (peerAddress == "") {
      ESP_LOGD(LOG_TAG, "BleKeyboard: already connected, no specific address was provided, nothing to do.\n");
      return true;
    } else {
      #if !defined(NIMBLE_ARDUINO_2_x)
      NimBLEConnInfo connInfo = NimBLEDevice::getServer()->getPeerIDInfo(0);
      #else
      NimBLEConnInfo connInfo = NimBLEDevice::getServer()->getPeerInfoByHandle(0);
      #endif
      if (NimBLEAddress(connInfo.getAddress()).toString() == peerAddress) {
        ESP_LOGD(LOG_TAG, "BleKeyboard: already connected to address %s, nothing to do\n", peerAddress.c_str());
        return true;
      } else {
        ESP_LOGD(LOG_TAG, "BleKeyboard: will disconnect peer %s and start direct advertising to address %s.\n", NimBLEAddress(connInfo.getAddress()).toString().c_str(), peerAddress.c_str());
  
        return advertiseAndWaitForConnection(peerAddress);
      }
    }
  }

  if (NimBLEDevice::getServer()->getConnectedCount() > 1) {
    ESP_LOGW(LOG_TAG, "BleKeyboard: currently connected to more than one client. How can it be???\n");
    return false;
  }

  // should never be reached
  return false;
}
  
void BleKeyboard::set_BLEKeyboardMessage_cb(tBLEKeyboardMessage_cb pBLEKeyboardMessage_cb) {
  thisBLEKeyboardMessage_cb = pBLEKeyboardMessage_cb;  
}
