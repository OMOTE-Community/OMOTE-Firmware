#include <stdint.h>

#include "keypad_gui/keypad_gui.h"

void init_keys_HAL(void) {
}

// has to be exactly the same structure as in hardwarePresenter.h
const uint8_t keypadROWS = 5; //five rows
const uint8_t keypadCOLS = 5; //five columns
enum keypad_rawKeyStates {IDLE_RAW, PRESSED_RAW,       RELEASED_RAW};
struct rawKey {
  unsigned long timestampReceived;
  char keyChar;
  keypad_rawKeyStates rawKeyState;
};

void keys_getKeys_HAL(void* ptr, unsigned long currentMillis) {

  if (keyEventsQueue.empty()) {
    return;
  }

  // https://stackoverflow.com/questions/68788080/is-it-possible-to-pass-a-2d-array-using-a-void-pointer-in-c
  // https://www.geeksforgeeks.org/pass-2d-array-parameter-c/
  // https://stackoverflow.com/questions/16724368/how-to-pass-a-2d-array-by-pointer-in-c
  // cast the pointer to the same structure as in hardwarePresenter.h
  rawKey (*rawKeys)[keypadCOLS] = static_cast<rawKey (*)[keypadCOLS]>(ptr);

  // get first event in queue
  KeyEvent event = keyEventsQueue.front();

  // get the row and col from the lastActiveKey
  uint8_t row = event.keyCode / keypadROWS;
  uint8_t col = event.keyCode % keypadCOLS;

  rawKeys[row][col].timestampReceived = currentMillis;

  rawKeys[row][col].keyChar = event.keyChar;
  if (event.keyState == PRESSED_SIMULATOR) {
    rawKeys[row][col].rawKeyState = PRESSED_RAW;

  } else if (event.keyState == RELEASED_SIMULATOR) {
    rawKeys[row][col].rawKeyState = RELEASED_RAW;
  }

  // printf("simulator key event: %c, %d %d, %d, removed from queue\r\n", rawKeys[row][col].keyChar, row, col, rawKeys[row][col].rawKeyState);
  // remove first event
  keyEventsQueue.pop();
}