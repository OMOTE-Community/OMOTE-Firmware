#pragma once
#include <string>

// Advertising is started automatically.
// As soon as a device is connected, a small indicator in the top left corner of the screen will appear

#if (ENABLE_KEYBOARD_BLE == 1)

#if (ENABLE_KEYBOARD_BLE == 1) && !(ENABLE_BLUETOOTH == 1)
static_assert(false, "You have to use \"-D ENABLE_BLUETOOTH=1\" in \"platformio.ini\" when having \"-D ENABLE_KEYBOARD_BLE=1\"");
#endif

// commands without a specific address
extern uint16_t KEYBOARD_BLE_UP;
extern uint16_t KEYBOARD_BLE_DOWN;
extern uint16_t KEYBOARD_BLE_RIGHT;
extern uint16_t KEYBOARD_BLE_LEFT;
extern uint16_t KEYBOARD_BLE_SELECT;
extern uint16_t KEYBOARD_BLE_SENDSTRING;
extern uint16_t KEYBOARD_BLE_BACK;
extern uint16_t KEYBOARD_BLE_HOME;
extern uint16_t KEYBOARD_BLE_MENU;
extern uint16_t KEYBOARD_BLE_SCAN_PREVIOUS_TRACK;
extern uint16_t KEYBOARD_BLE_REWIND_LONG;
extern uint16_t KEYBOARD_BLE_REWIND;
extern uint16_t KEYBOARD_BLE_PLAYPAUSE;
extern uint16_t KEYBOARD_BLE_FASTFORWARD;
extern uint16_t KEYBOARD_BLE_FASTFORWARD_LONG;
extern uint16_t KEYBOARD_BLE_SCAN_NEXT_TRACK;
extern uint16_t KEYBOARD_BLE_MUTE;
extern uint16_t KEYBOARD_BLE_VOLUME_INCREMENT;
extern uint16_t KEYBOARD_BLE_VOLUME_DECREMENT;

// commands with specific address
extern uint16_t KEYBOARD_BLE_RIGHT_FIRETV;
extern uint16_t KEYBOARD_BLE_LEFT_NVIDIASHIELD;

void register_device_keyboard_ble();
void keyboard_ble_executeCommand(uint16_t command, std::list<std::string> commandPayloads, std::string additionalPayload = "");

#endif
