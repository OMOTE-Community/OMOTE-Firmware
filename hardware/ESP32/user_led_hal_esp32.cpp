#include <Arduino.h>

#if(OMOTE_HARDWARE_REV >= 5)
const uint8_t USER_LED_GPIO = 45;
#else
const uint8_t USER_LED_GPIO = 2;
#endif

void init_userled_HAL(void) {
  pinMode(USER_LED_GPIO, OUTPUT);
  digitalWrite(USER_LED_GPIO, LOW);  
}

void update_userled_HAL(void) {
  digitalWrite(USER_LED_GPIO, millis() % 2000 > 1000);
}
