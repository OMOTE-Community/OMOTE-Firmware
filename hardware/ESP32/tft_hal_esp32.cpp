#include <Arduino.h>
// Not needed by the compiler, only by PlatformIO's library dependency finder (LDF).
// The FT6206 library is always built, because it is in lib_deps - also with DISPLAY_DRIVER 0,
// which does not use it at all. Its dependency Adafruit BusIO then gets no include path for
// SPI and Wire unless some source file pulls BusIO in, and the build fails with
// "SPI.h: No such file or directory".
// Including a BusIO header directly in a .cpp makes the LDF resolve BusIO properly.
// On rev5 Adafruit_TCA8418.h (keypad) already does that, on rev1-4 nothing else does.
// Do not remove - the alternative would be "lib_ldf_mode = deep+" in platformio.ini.
#include <Adafruit_I2CDevice.h>
#include "driver/ledc.h"
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"
#include "displaydriver.h"

byte backlightBrightness = 255;

void update_backlightBrightness_HAL(void) {
  // A variable declared static inside a function is visible only inside that function, exists only once (not created/destroyed for each call) and is permanent. It is in a sense a private global variable.
  static int fadeInTimer = millis(); // fadeInTimer = time after setup
  if (millis() < fadeInTimer + backlightBrightness) {
    // after boot or wakeup, fade in backlight brightness
    // fade in lasts for <backlightBrightness> ms
    ledcWrite(LEDC_CHANNEL_5, millis() - fadeInTimer);
  } else {
    if (millis() - get_lastActivityTimestamp() > get_sleepTimeout_HAL() - 2000) {
      // less than 2000 ms until standby
      // dim backlight
      ledcWrite(LEDC_CHANNEL_5, get_backlightBrightness_HAL() * 0.3);
    } else {
      // normal mode, set full backlightBrightness
      // turn off PWM if backlight is at full brightness
      if(backlightBrightness < 255){
        ledcWrite(LEDC_CHANNEL_5, backlightBrightness);
      }
      else{
        ledc_stop(LEDC_SPEED_MODE, LEDC_CHANNEL_5, 255);
      }
    }
  }
}

uint8_t get_backlightBrightness_HAL() {
  return backlightBrightness;
};
void set_backlightBrightness_HAL(uint8_t aBacklightBrightness) {
  backlightBrightness = aBacklightBrightness;
};
