#include <Arduino.h>
#include "driver/ledc.h"
#include "keypad_keys_hal_esp32.h"
#if(OMOTE_HARDWARE_REV >= 5)
  #include <Adafruit_TCA8418.h>
#else
  #include "lib/Keypad/src/Keypad.h" // modified for inverted logic
#endif
#include "sleep_hal_esp32.h"

// has to be exactly the same structure as in hardwarePresenter.h
const uint8_t keypadROWS = 5; //five rows
const uint8_t keypadCOLS = 5; //five columns
enum keypad_rawKeyStates {IDLE_RAW, PRESSED_RAW,       RELEASED_RAW};
struct rawKey {
  unsigned long timestampReceived;
  char keyChar;
  keypad_rawKeyStates rawKeyState;
};

#if(OMOTE_HARDWARE_REV >= 5)
const uint8_t TCA_INT_GPIO = 8;
const uint8_t KBD_BL_GPIO = 46;
const uint64_t BUTTON_PIN_BITMASK = 0b0000000000000000000000000000000100000100; //IO02+IO08)

Adafruit_TCA8418 keypad;
byte keyboardBrightness = 255;

char keypadChars[keypadROWS][keypadCOLS] = {
  {'?','p','c','<','='},  //       ?,     play,  config, rewind,   stop
  {'>','o','b','u','l'}, // forward,      off,    back,     up,   left
  {'4','v','1','3','2'}, //    blue, channel-,     red, yellow,  green
  {'i','r','+','k','d'}, //    info,    right, Volume+,     OK,   down
  {'s','^','-','m','e'}, //  source, channel+, Volume-,   mute, record
};

#else
const uint8_t SW_1_GPIO = 32; // 1...5: Output
const uint8_t SW_2_GPIO = 26;
const uint8_t SW_3_GPIO = 27;
const uint8_t SW_4_GPIO = 14;
const uint8_t SW_5_GPIO = 12;
const uint8_t SW_A_GPIO = 37; // A...E: Input
const uint8_t SW_B_GPIO = 38;
const uint8_t SW_C_GPIO = 39;
const uint8_t SW_D_GPIO = 34;
const uint8_t SW_E_GPIO = 35;
const uint64_t BUTTON_PIN_BITMASK = 0b1110110000000000000000000010000000000000; //IO34+IO35+IO37+IO38+IO39(+IO13)

// Keypad declarations
// define the symbols on the buttons of the keypads
char hexaKeys[keypadROWS][keypadCOLS] = {
  {'s','^','-','m','e'}, //  source, channel+, Volume-,   mute, record
  {'i','r','+','k','d'}, //    info,    right, Volume+,     OK,   down
  {'4','v','1','3','2'}, //    blue, channel-,     red, yellow,  green
  {'>','o','b','u','l'}, // forward,      off,    back,     up,   left
  {'?','p','c','<','='}  //       ?,     play,  config, rewind,   stop
};

byte rowPins[keypadROWS] = {SW_A_GPIO, SW_B_GPIO, SW_C_GPIO, SW_D_GPIO, SW_E_GPIO}; //connect to the row pinouts of the keypad
byte colPins[keypadCOLS] = {SW_1_GPIO, SW_2_GPIO, SW_3_GPIO, SW_4_GPIO, SW_5_GPIO}; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, keypadROWS, keypadCOLS); 
#endif

/*
                                    off o

stop =    rewind <    play p    forward >

    config c                 info i
                   up u
           left l  OK k   right r
                  down d
    back b                   source s

Volume+ +         mute m        channel+ ^
Volume- -         record e      channel- v

red 1     green 2     yellow 3  blue 4
*/

void init_keys_HAL(void) {
  #if(OMOTE_HARDWARE_REV >= 5)

  if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("Keypad TCA8418 not found!");
  }
  keypad.matrix(keypadROWS, keypadCOLS);  
  keypad.pinMode(5, INPUT_PULLUP);  // SW_PWR
  keypad.pinMode(6, INPUT_PULLUP);  // SD_DET
  keypad.pinMode(13, INPUT); // USB_3V3

  pinMode(TCA_INT_GPIO, INPUT);
  keypad.flush();
  keypad.writeRegister(TCA8418_REG_CFG, 0b00000001);
  keypad.writeRegister(TCA8418_REG_GPI_EM_1, 0b00111111);
  keypad.writeRegister(TCA8418_REG_GPI_EM_2, 0b00011111); // disable interrupt for COL5 (USB_3V3)
  
  ledcSetup(LEDC_CHANNEL_6, 5000, 8);
  ledcAttachPin(KBD_BL_GPIO, LEDC_CHANNEL_6);
  ledcWrite(LEDC_CHANNEL_6, 0);

  #else
  // Button Pin Definition
  pinMode(SW_1_GPIO, OUTPUT);
  pinMode(SW_2_GPIO, OUTPUT);
  pinMode(SW_3_GPIO, OUTPUT);
  pinMode(SW_4_GPIO, OUTPUT);
  pinMode(SW_5_GPIO, OUTPUT);
  pinMode(SW_A_GPIO, INPUT);
  pinMode(SW_B_GPIO, INPUT);
  pinMode(SW_C_GPIO, INPUT);
  pinMode(SW_D_GPIO, INPUT);
  pinMode(SW_E_GPIO, INPUT);
  #endif
}

void keys_getKeys_HAL(void* ptr, unsigned long currentMillis) {

  rawKey (*rawKeys)[keypadCOLS] = static_cast<rawKey (*)[keypadCOLS]>(ptr);

  #if(OMOTE_HARDWARE_REV >= 5)
    byte row = 0;
    byte col = 0;
    int keyCode = 0;
    keypad_rawKeyStates rawKeyState = IDLE_RAW;

    // https://github.com/adafruit/Adafruit_TCA8418/blob/main/examples/tca8418_keypad_gpio_interrupt/tca8418_keypad_gpio_interrupt.ino
    int intStat = keypad.readRegister(TCA8418_REG_INT_STAT);
    if (intStat & 0x01) // Byte 0: K_INT (keyboard interrupt)
    {
      // datasheet page 16 - Table 2
      keyCode = keypad.getEvent();
      if (keyCode & 0x80) rawKeyState = PRESSED_RAW;
      else rawKeyState = RELEASED_RAW;
      // if (keyCode & 0x80) Serial.print("PRESS\t ");
      // else Serial.print("RELEASE\t ");
      //  map keyCode to GPIO nr.
      keyCode &= 0x7F;

      if (keyCode > 96)  //  GPIO
      {
        // process gpio
        keyCode -= 97;
        // this only happens for key 'o' (off). Map this to 1/1
        row = 1;
        col = 1;
        // Serial.print(keyCode);
        // Serial.println();
      }
      else
      {
        // process matrix
        keyCode--;
        row = keyCode / 10;
        col = keyCode % 10;
        //Serial.print(keyCode / 10);
        //Serial.print("\t ");
        //Serial.print(keyCode % 10);
        //Serial.println();
      }

      //  clear the EVENT IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
    }
    if (intStat & 0x02) // Byte 1: GPI_INT (GPIO interrupt)
    {
      //  reading the registers is mandatory to clear IRQ flag
      //  can also be used to find the GPIO changed
      //  as these registers are a bitmap of the gpio pins.
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_1);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_2);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_3);
      //  clear GPIO IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 2);
    }

    //  check pending events
    int intstat = keypad.readRegister(TCA8418_REG_INT_STAT);

    if (rawKeyState == IDLE_RAW) {return;}

    rawKeys[row][col].timestampReceived = currentMillis;

    rawKeys[row][col].keyChar = keypadChars[row][col];
    if (rawKeyState == PRESSED_RAW) {
      rawKeys[row][col].rawKeyState = PRESSED_RAW;
      setLastActivityTimestamp_HAL();

    } else if (rawKeyState == RELEASED_RAW) {
      rawKeys[row][col].rawKeyState = RELEASED_RAW;
      setLastActivityTimestamp_HAL();
    }

    // Serial.printf("esp32 TCA8418 event: %c, %d %d, %d\r\n", keypadChars[row][col], row, col, rawKeyState);

  #else

    // Only the current keypad state will be returned by the keypad library. If a key has been pressed and already been released between two calls, the key is lost.
    // But if a keypress has been started (PRESS has been received), all further events for this key will be provided, no following event is missed.
    if (!customKeypad.getKeys()) return;
    uint8_t row;
    uint8_t col;
    for(int i=0; i < LIST_MAX; i++) {
      if (!customKeypad.key[i].stateChanged) continue;

      // get the row and col for this key
      row = customKeypad.key[i].kcode / keypadROWS;
      col = customKeypad.key[i].kcode % keypadCOLS;
    
      rawKeys[row][col].timestampReceived = currentMillis;
      rawKeys[row][col].keyChar = customKeypad.key[i].kchar;
      if (customKeypad.key[i].kstate == PRESSED) {
        rawKeys[row][col].rawKeyState = PRESSED_RAW;
        setLastActivityTimestamp_HAL();
    
      } else if (customKeypad.key[i].kstate == RELEASED) {
        rawKeys[row][col].rawKeyState = RELEASED_RAW;
        setLastActivityTimestamp_HAL();
      }
      // Serial.printf("esp32 keypad event for key %d: %c, %d %d, %d\r\n", i, rawKeys[row][col].keyChar, row, col, customKeypad.key[i].kstate);
    }
  #endif
}

#if(OMOTE_HARDWARE_REV >= 5)
void update_keyboardBrightness_HAL(void) {
  // A variable declared static inside a function is visible only inside that function, exists only once (not created/destroyed for each call) and is permanent. It is in a sense a private global variable.
  static int fadeInTimer = millis(); // fadeInTimer = time after setup
  if (millis() < fadeInTimer + keyboardBrightness) {
    // after boot or wakeup, fade in keyboard brightness
    // fade in lasts for <keyboardBrightness> ms
    ledcWrite(LEDC_CHANNEL_6, millis() - fadeInTimer);
  }
  else {
    // normal mode, set full keyboardBrightness    
    // turn off PWM if keyboard is at full brightness
    if(keyboardBrightness < 255){
      ledcWrite(LEDC_CHANNEL_6, keyboardBrightness);
    }
    else{
      ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_6, 255);
    }
  }
}

uint8_t get_keyboardBrightness_HAL() {
  return keyboardBrightness;
};
void set_keyboardBrightness_HAL(uint8_t aKeyboardBrightness) {
  keyboardBrightness = aKeyboardBrightness;
};
#endif