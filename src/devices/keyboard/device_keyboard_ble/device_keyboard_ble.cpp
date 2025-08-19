#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"
#include "device_keyboard_ble.h"
#include "applicationInternal/hardware/hardwarePresenter.h"

#if (ENABLE_KEYBOARD_BLE == 1)

// commands without a specific address
uint16_t KEYBOARD_BLE_UP                  ; //"Keyboard_ble_up";
uint16_t KEYBOARD_BLE_DOWN                ; //"Keyboard_ble_down";
uint16_t KEYBOARD_BLE_RIGHT               ; //"Keyboard_ble_right";
uint16_t KEYBOARD_BLE_LEFT                ; //"Keyboard_ble_left";
uint16_t KEYBOARD_BLE_SELECT              ; //"Keyboard_ble_select";
uint16_t KEYBOARD_BLE_SENDSTRING          ; //"Keyboard_ble_sendstring";
uint16_t KEYBOARD_BLE_BACK                ; //"Keyboard_ble_back";
uint16_t KEYBOARD_BLE_HOME                ; //"Keyboard_ble_home";
uint16_t KEYBOARD_BLE_MENU                ; //"Keyboard_ble_menu";
uint16_t KEYBOARD_BLE_SCAN_PREVIOUS_TRACK ; //"Keyboard_ble_scan_previous_track";
uint16_t KEYBOARD_BLE_REWIND_LONG         ; //"Keyboard_ble_rewind_long";
uint16_t KEYBOARD_BLE_REWIND              ; //"Keyboard_ble_rewind";
uint16_t KEYBOARD_BLE_PLAYPAUSE           ; //"Keyboard_ble_playpause";
uint16_t KEYBOARD_BLE_FASTFORWARD         ; //"Keyboard_ble_fastforward";
uint16_t KEYBOARD_BLE_FASTFORWARD_LONG    ; //"Keyboard_ble_fastforward_long";
uint16_t KEYBOARD_BLE_SCAN_NEXT_TRACK     ; //"Keyboard_ble_scan_next_track";
uint16_t KEYBOARD_BLE_MUTE                ; //"Keyboard_ble_mute";
uint16_t KEYBOARD_BLE_VOLUME_INCREMENT    ; //"Keyboard_ble_volume_increment";
uint16_t KEYBOARD_BLE_VOLUME_DECREMENT    ; //"Keyboard_ble_volume_decrement";

// commands with specific address
uint16_t KEYBOARD_BLE_RIGHT_FIRETV;
uint16_t KEYBOARD_BLE_LEFT_NVIDIASHIELD;

void register_device_keyboard_ble() {
  // The commandData should either
  // a) contain nothing, which means no specific address the command has to be sent to. You can also use this if you only have one single peer.
  // b) or contain an address of the peer the BleKeyboard should sent the command to. You need to use addresses if you have more than one peer.
  register_command(&KEYBOARD_BLE_UP                  , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_DOWN                , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_RIGHT               , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_LEFT                , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_SELECT              , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_SENDSTRING          , makeCommandData(BLE_KEYBOARD, {})); // payload must be set when calling commandHandler
  register_command(&KEYBOARD_BLE_BACK                , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_HOME                , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_MENU                , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_SCAN_PREVIOUS_TRACK , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_REWIND_LONG         , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_REWIND              , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_PLAYPAUSE           , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_FASTFORWARD         , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_FASTFORWARD_LONG    , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_SCAN_NEXT_TRACK     , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_MUTE                , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_VOLUME_INCREMENT    , makeCommandData(BLE_KEYBOARD, {}));
  register_command(&KEYBOARD_BLE_VOLUME_DECREMENT    , makeCommandData(BLE_KEYBOARD, {}));

// commands with specific address
// In the commandData, both the address and the command to be sent have to be provided.
//   In the commandData, only provide a command that already has received its uniqueCommandID. So don't provide the same command as first parameter AND in the commandData.
  //register_command(&KEYBOARD_BLE_RIGHT_FIRETV        , makeCommandData(BLE_KEYBOARD, {"11:22:33:44:55:66", std::to_string(KEYBOARD_BLE_RIGHT)}));
  //register_command(&KEYBOARD_BLE_LEFT_NVIDIASHIELD   , makeCommandData(BLE_KEYBOARD, {"77:88:99:aa:bb:cc", std::to_string(KEYBOARD_BLE_LEFT)}));
}

void keyboard_ble_executeCommand(uint16_t command, std::list<std::string> commandPayloads, std::string additionalPayload) {
  bool doLog = false;

  // in the commandPayloads, we either have
  // a) nothing
  // b) or the address of the device this command has to be sent to, and the command that shall be sent

  // look if an explicit address was provided
  std::string address = "";
  uint16_t commandToBeSent;
  if (commandPayloads.size() == 2) {
    address = commandPayloads.front();
    commandToBeSent = std::stoi(commandPayloads.back());
  } else {
    commandToBeSent = command;
  }
  omote_log_v("  command          : %d\r\n", command);
  omote_log_v("  commandToBeSent  : %d\r\n", commandToBeSent);
  omote_log_v("  address          : %s\r\n", address.c_str());
  omote_log_v("  additionalPayload: %s\r\n", additionalPayload.c_str());

  // connect to a specific address, and ensure that there is a connection
  if (!keyboardBLE_forceConnectionToAddress(address)) {
    omote_log_w("BLE keyboard could not be connected, cannot send key\r\n");
    return;
  }

  if (commandToBeSent == KEYBOARD_BLE_UP) {
    if (doLog) {omote_log_d("UP received\r\n");}
    keyboardBLE_write(BLE_KEY_UP_ARROW);

  } else if (commandToBeSent == KEYBOARD_BLE_DOWN) {
    if (doLog) {omote_log_d("DOWN received\r\n");}
    keyboardBLE_write(BLE_KEY_DOWN_ARROW);

  } else if (commandToBeSent == KEYBOARD_BLE_RIGHT) {
    if (doLog) {omote_log_d("RIGHT received\r\n");}
    keyboardBLE_write(BLE_KEY_RIGHT_ARROW);

  } else if (commandToBeSent == KEYBOARD_BLE_LEFT) {
    if (doLog) {omote_log_d("LEFT received\r\n");}
    keyboardBLE_write(BLE_KEY_LEFT_ARROW);

  } else if (commandToBeSent == KEYBOARD_BLE_SELECT) {
    if (doLog) {omote_log_d("SELECT received\r\n");}
    keyboardBLE_write(BLE_KEY_RETURN);

  } else if (commandToBeSent == KEYBOARD_BLE_SENDSTRING) {
    if (doLog) {omote_log_d("SENDSTRING received\r\n");}
    if (additionalPayload != "") {
      keyboardBLE_sendString(additionalPayload.c_str());
    }



  } else if (commandToBeSent == KEYBOARD_BLE_BACK) {
    if (doLog) {omote_log_d("BACK received\r\n");}
    // test which one works best for your device
    // keyboardBLE_write(KEY_ESC);
    consumerControlBLE_write(BLE_KEY_MEDIA_WWW_BACK);

  } else if (commandToBeSent == KEYBOARD_BLE_HOME) {
    if (doLog) {omote_log_d("HOME received\r\n");}
    // test which one works best for your device
    // keyboardBLE_home();
    consumerControlBLE_write(BLE_KEY_MEDIA_WWW_HOME);

  } else if (commandToBeSent == KEYBOARD_BLE_MENU) {
    if (doLog) {omote_log_d("MENU received\r\n");}
    keyboardBLE_write(0xED); // 0xDA + 13 = 0xED



  // for more consumerControl codes see
  // https://github.com/espressif/arduino-esp32/blob/master/libraries/USB/src/USBHIDConsumerControl.h
  // https://github.com/adafruit/Adafruit_CircuitPython_HID/blob/main/adafruit_hid/consumer_control_code.py
  } else if (commandToBeSent == KEYBOARD_BLE_SCAN_PREVIOUS_TRACK) {
    if (doLog) {omote_log_d("SCAN_PREVIOUS_TRACK received\r\n");}
    consumerControlBLE_write(BLE_KEY_MEDIA_PREVIOUS_TRACK);

  } else if (commandToBeSent == KEYBOARD_BLE_REWIND_LONG) {
    if (doLog) {omote_log_d("REWIND_LONG received\r\n");}
    //keyboardBLE_longpress(KEY_LEFT_ARROW);
    consumerControlBLE_longpress(BLE_KEY_MEDIA_REWIND);

  } else if (commandToBeSent == KEYBOARD_BLE_REWIND) {
    if (doLog) {omote_log_d("REWIND received\r\n");}
    //keyboardBLE_write(KEY_LEFT_ARROW);
    consumerControlBLE_write(BLE_KEY_MEDIA_REWIND);

  } else if (commandToBeSent == KEYBOARD_BLE_PLAYPAUSE) {
    if (doLog) {omote_log_d("PLAYPAUSE received\r\n");}
    consumerControlBLE_write(BLE_KEY_MEDIA_PLAY_PAUSE);

  } else if (commandToBeSent == KEYBOARD_BLE_FASTFORWARD) {
    if (doLog) {omote_log_d("FASTFORWARD received\r\n");}
    //keyboardBLE_write(KEY_RIGHT_ARROW);
    consumerControlBLE_write(BLE_KEY_MEDIA_FASTFORWARD);

  } else if (commandToBeSent == KEYBOARD_BLE_FASTFORWARD_LONG) {
    if (doLog) {omote_log_d("FASTFORWARD_LONG received\r\n");}
    //keyboardBLE_longpress(KEY_RIGHT_ARROW);
    consumerControlBLE_longpress(BLE_KEY_MEDIA_FASTFORWARD);

  } else if (commandToBeSent == KEYBOARD_BLE_SCAN_NEXT_TRACK) {
    if (doLog) {omote_log_d("SCAN_NEXT_TRACK received\r\n");}
    consumerControlBLE_write(BLE_KEY_MEDIA_NEXT_TRACK);



  } else if (commandToBeSent == KEYBOARD_BLE_MUTE) {
    if (doLog) {omote_log_d("MUTE received\r\n");}
    consumerControlBLE_write(BLE_KEY_MEDIA_MUTE);

  } else if (commandToBeSent == KEYBOARD_BLE_VOLUME_INCREMENT) {
    if (doLog) {omote_log_d("VOLUME_INCREMENT received\r\n");}
    consumerControlBLE_write(BLE_KEY_MEDIA_VOLUME_UP);

  } else if (commandToBeSent == KEYBOARD_BLE_VOLUME_DECREMENT) {
    if (doLog) {omote_log_d("VOLUME_DECREMENT received\r\n");}
    consumerControlBLE_write(BLE_KEY_MEDIA_VOLUME_DOWN);

  }
}

/*
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
*/

#endif