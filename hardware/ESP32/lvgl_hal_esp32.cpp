#include <lvgl.h>
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"
#include "displaydriver.h"

// Read the touchpad
void my_touchpad_read(lv_indev_t * indev_driver, lv_indev_data_t * data) {
  // Wait until the touch controller answers. After a soft restart (ESP.restart()) it is not
  // power cycled and can need a moment. With DISPLAY_DRIVER 1 and 2 this is also what initializes
  // the touch, because touchChipResponds() calls touch.begin().
  // Retry at most every 500 ms, not on every LVGL poll: with a missing or broken touch
  // controller this would otherwise run every 30 ms forever, and with DISPLAY_DRIVER 1 and 2 each
  // attempt deletes and allocates an Adafruit_I2CDevice.
  if (!TouchInitSuccessful) {
    static unsigned long lastTouchRetry = 0;
    if (lastTouchRetry == 0 || millis() - lastTouchRetry >= 500) {
      lastTouchRetry = millis();
      TouchInitSuccessful = touchChipResponds();
    }
    if (!TouchInitSuccessful) {
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
  }
  
  my_touchpad_read_display_specific(indev_driver, data);

  if (data->state == LV_INDEV_STATE_PRESSED) {
    setLastActivityTimestamp_HAL();
  }
}

// new in lvgl 9
static uint32_t my_tick_get_cb(void) {
  return millis();
}

void init_lvgl_HAL() {
  // first init TFT -----------------------------------------------------------------------------------------
  setup_tft();
  init_tft();

  setup_wire();

  // new in lvgl 9
  lv_tick_set_cb(my_tick_get_cb);

  // Initialize the display driver --------------------------------------------------------------------------
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#display-api
  // https://docs.lvgl.io/master/get-started/quick-overview.html#add-lvgl-into-your-project
  lv_display_t *disp = lv_display_create(SCR_WIDTH, SCR_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);

  // allocate lvgl buffers ----------------------------------------------------------------------------------
  // Has to come after lv_display_create(): in LVGL 9 the buffers are handed to the display, not
  // registered through a descriptor as in v8.
  init_lvgl_buffer(disp);

  // Initialize the touchscreen driver ----------------------------------------------------------------------
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#indev-api
  static lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

}
