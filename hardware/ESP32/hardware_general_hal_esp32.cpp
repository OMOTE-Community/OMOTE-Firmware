#include <Arduino.h>
#include <Wire.h>
#include "tft_hal_esp32.h"

void init_hardware_general_HAL(void) {
  // Make sure ESP32 is running at full speed
  setCpuFrequencyMhz(240);
  
  // this is power for the TFT IC
  digitalWrite(LCD_EN_GPIO, HIGH);
  pinMode(LCD_EN_GPIO, OUTPUT);
  // this is power for backlight LEDs
  digitalWrite(LCD_BL_GPIO, HIGH);
  pinMode(LCD_BL_GPIO, OUTPUT);

}
