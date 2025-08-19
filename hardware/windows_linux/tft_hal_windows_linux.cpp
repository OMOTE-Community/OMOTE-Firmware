#include <stdio.h>
#include <stdint.h>

uint8_t backlightBrightness = 255;

void update_backlightBrightness_HAL(void) {
  // nothing to do, we don't want to dim the simulator

};
uint8_t get_backlightBrightness_HAL() {
  return backlightBrightness;
};
void set_backlightBrightness_HAL(uint8_t aBacklightBrightness) {
  backlightBrightness = aBacklightBrightness;
  printf("backlight brightness set to %u\r\n", aBacklightBrightness);
};
