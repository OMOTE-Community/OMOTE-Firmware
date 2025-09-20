#include <string>
#include "applicationInternal/gui/guiMemoryOptimizer.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"

enum keypad_keyStates    {IDLE, PRESSED, HOLD, RELEASED};
// Used in keypad_setKeyStatesAndCheckForHold
// Holds the states including HOLD.
// This state indicates OMOTE what to do: doShortPress or doLongPress
uint32_t KEY_HOLD_TIME = 500; // Keys are considered held after 500 ms
keypad_keyStates keyState[keypadROWS][keypadCOLS] = {
  {IDLE,IDLE,IDLE,IDLE,IDLE},
  {IDLE,IDLE,IDLE,IDLE,IDLE},
  {IDLE,IDLE,IDLE,IDLE,IDLE},
  {IDLE,IDLE,IDLE,IDLE,IDLE},
  {IDLE,IDLE,IDLE,IDLE,IDLE},
};
// Used in keypad_processKeyStates. Holds the states after they have been processed.
struct processedState {
	keypad_keyStates keyState;
  // used to know what has to be done after key was released
  bool keyIsHold;
};
processedState keyStateProcessed[keypadROWS][keypadCOLS] = {
  {{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false}},
  {{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false}},
  {{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false}},
  {{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false}},
  {{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false},{IDLE,false}},
};
// used in doShortPress to know when a shortPress should be sent again and again
int repeatRate = 125; // in milliseconds
unsigned long lastTimeSent[keypadROWS][keypadCOLS] ={
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
};

bool isKeyPressInBounds(int keyCode, int& row, int& col) {
  row = keyCode / keypadCOLS;
  col = keyCode % keypadCOLS;
  
  if (row >= keypadROWS || col >= keypadCOLS) {
    omote_log_e("doShortPress: keyCode %d results in invalid row=%d, col=%d (max: %d, %d)\n", 
                keyCode, row, col, keypadROWS-1, keypadCOLS-1);
    return false;
  }
  return true;
}

bool isKeyPressRateLimited(int row, int col, unsigned long currentMillis) {
  return (currentMillis - lastTimeSent[row][col]) <= repeatRate;
}

void doShortPress(char keyChar, int keyCode){
  unsigned long currentMillis = millis();
  int row, col;
  
  if (!isKeyPressInBounds(keyCode, row, col)) {
    return;
  }
  
  if (isKeyPressRateLimited(row, col, currentMillis)) {
    return;
  }
  
  lastTimeSent[row][col] = currentMillis;

  uint16_t command = get_command_short(gui_memoryOptimizer_getActiveSceneName(), keyChar);
  if (command == COMMAND_UNKNOWN) {
    omote_log_w("key: key '%c', but no command defined\r\n", keyChar);
    return;
  }
  
  omote_log_d("key: key '%c', will use command '%u'\r\n", keyChar, command);
  CommandExecutionParams params;
  params.commandId = command;
  params.commandType = CMD_SHORT;
  executeCommand(params);
}

void doLongPress(char keyChar, int keyCode){
  uint16_t command = get_command_long(gui_memoryOptimizer_getActiveSceneName(), keyChar);
  if (command != COMMAND_UNKNOWN) {
    omote_log_d("key: key '%c' (long press), will use command '%u'\r\n", keyChar, command);
    CommandExecutionParams params;
    params.commandId = command;
    params.commandType = CMD_LONG;
    executeCommand(params);
  } else {
    omote_log_w("key: key '%c' (long press), but no command defined\r\n", keyChar);
  }
}

// After the keyState RELEASED was processed in keypad_processKeyStates(),
// both the keyState and rawKeyState need to be reset to IDLE.
void keypad_resetReleasedKeys() {
  for(uint8_t row=0; row < keypadROWS; row++) {
    for(uint8_t col=0; col < keypadCOLS; col++) {
      if (keyStateProcessed[row][col].keyState == RELEASED) {
        // after state RELEASED was processed, we can set the state to IDLE
        keyState[row][col] = IDLE;
        rawKeys[row][col].rawKeyState = IDLE_RAW;
      }
    }
  }
}

// get raw keys from the keypad (OMOTE_HARDWARE_REV <= 4), the TCA8418 (OMOTE_HARDWARE_REV >= 5) or the simulator.
void keypad_getRawKeys() {
  // we need to provide currentMillis to the hardware, because at least in case of the simulator there is no way to access millis()
  unsigned long currentMillis = millis();
  getKeys(rawKeys, currentMillis);
}

// Now we have the latest rawKeyState[][] for all 25 keys and know when they have been sent
// Next, we add the state HOLD, if needed, and save the result in keyState[][]
void keypad_setKeyStatesAndCheckForHold() {
  unsigned long currentMillis = millis();

  for(uint8_t row=0; row < keypadROWS; row++) {
    for(uint8_t col=0; col < keypadCOLS; col++) {

      if (rawKeys[row][col].rawKeyState == PRESSED_RAW && (currentMillis -  rawKeys[row][col].timestampReceived) > KEY_HOLD_TIME) {
        keyState[row][col] = HOLD;
      } else if (rawKeys[row][col].rawKeyState == PRESSED_RAW) {
        keyState[row][col] = PRESSED;
      } else if (rawKeys[row][col].rawKeyState == RELEASED_RAW) {
        keyState[row][col] = RELEASED;
      }
    }
  }
}

void keypad_processKeyStates() {
  // iterate over all keys and process them
  for(uint8_t row=0; row < keypadROWS; row++) {
    for(uint8_t col=0; col < keypadCOLS; col++) {

      keypad_keyStates singleKeyState = keyState[row][col];
      char keyChar = rawKeys[row][col].keyChar;
      int keyCode = row * keypadCOLS + col;

      if (singleKeyState == PRESSED) {
        omote_log_v("pressed\r\n");

        if ((get_key_repeatMode(gui_memoryOptimizer_getActiveSceneName(), keyChar) == SHORT) && (keyStateProcessed[row][col].keyState != PRESSED)) {
          omote_log_v("key: PRESSED of SHORT key %c (%d)\r\n", keyChar, keyCode);
          doShortPress(keyChar, keyCode);

        } else if ((get_key_repeatMode(gui_memoryOptimizer_getActiveSceneName(), keyChar) == SHORT_REPEATED) && (keyStateProcessed[row][col].keyState != PRESSED)) { // here do not repeat it too early, do the repeat only in HOLD
          omote_log_v("key: PRESSED of SHORT_REPEATED key %c (%d)\r\n", keyChar, keyCode);
          doShortPress(keyChar, keyCode);

        }
        keyStateProcessed[row][col].keyState = PRESSED;

      } else if (singleKeyState == HOLD) {
        omote_log_v("hold\r\n");

        if ((get_key_repeatMode(gui_memoryOptimizer_getActiveSceneName(), keyChar) == SHORTorLONG) && (keyStateProcessed[row][col].keyState != HOLD)) {
          omote_log_v("key: HOLD of SHORTorLONG key %c (%d)\r\n", keyChar, keyCode);
          omote_log_v("will set keyIsHold to TRUE for keycode %d\r\n", keyCode);
          keyStateProcessed[row][col].keyIsHold = true;
          doLongPress(keyChar, keyCode);

        } else if (get_key_repeatMode(gui_memoryOptimizer_getActiveSceneName(), keyChar) == SHORT_REPEATED) { // this is the only case where we do not check the keyStateProcessed, because here it is intended to repeat the action
          omote_log_v("key: HOLD of SHORT_REPEATED key %c (%d)\r\n", keyChar, keyCode);
          doShortPress(keyChar, keyCode);

        }
        keyStateProcessed[row][col].keyState = HOLD;

      } else if (singleKeyState == RELEASED) {
        omote_log_v("released\r\n");
        if ((get_key_repeatMode(gui_memoryOptimizer_getActiveSceneName(), keyChar) == SHORTorLONG) && !keyStateProcessed[row][col].keyIsHold && (keyStateProcessed[row][col].keyState != RELEASED)) {
          omote_log_v("value of keyIsHold for keycode %d is %d\r\n", keyCode, keyStateProcessed[row][col].keyIsHold);
          omote_log_v("key: RELEASED of SHORTorLONG key %c (%d)\r\n", keyChar, keyCode);
          doShortPress(keyChar, keyCode);
        }
        omote_log_v("will set keyIsHold to FALSE for keycode %d\r\n", keyCode);
        keyStateProcessed[row][col].keyIsHold = false;

        omote_log_v("key: press of key %c (%d)\r\n", keyChar, keyCode);
        keyStateProcessed[row][col].keyState = RELEASED;

      } else if (singleKeyState == IDLE) {
        omote_log_v("idle\r\n");

        omote_log_v("key: idle of key %c (%d)\r\n", keyChar, keyCode);
        keyStateProcessed[row][col].keyState = IDLE;

      }
    }
  }
}

void keypad_loop(void) {
  keypad_resetReleasedKeys();
  keypad_getRawKeys();
  keypad_setKeyStatesAndCheckForHold();
  keypad_processKeyStates();
}
