#include <lvgl.h>
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"

// -----------------------
// https://docs.lvgl.io/master/porting/display.html#two-buffers
// With two buffers, the rendering and refreshing of the display become parallel operations
// Second buffer needs 15.360 bytes more memory in heap.
#define useTwoBuffersForlvgl

// Display flushing
static void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t *px_map ) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  #if (DISPLAY_DRIVER == 0)
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  #ifdef useTwoBuffersForlvgl
  tft.pushPixelsDMA((uint16_t*)px_map, w * h);
  #else
  tft.pushColors((uint16_t *)px_map, w * h, true);
  #endif
  tft.endWrite();
  #elif (DISPLAY_DRIVER == 1)
  // Arduino_GFX's flush is synchronous - the transfer is done when it returns,
  // so a single buffer is enough and lv_disp_flush_ready() can follow directly.
  agfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(px_map), w, h);
  #endif

  lv_display_flush_ready(disp);
}

static bool TouchInitSuccessful = false;
// Ask the touch controller whether it is there.
// This deliberately goes through the same I2C driver that also drives the touch at runtime,
// so a negative result means "the touch controller did not answer" and not "some other
// I2C stack is not in shape".
bool touchChipResponds() {
  #if (DISPLAY_DRIVER == 0)
  // LovyanGFX drives I2C itself (not through Wire), port 0 as configured for the touch above.
  // Register 0xA3 is the chip id of the FT5x06/FT6x06 family.
  return lgfx::i2c::readRegister8(0, 0x38, 0xA3, 400000).has_value();
  #elif (DISPLAY_DRIVER == 1)
  // readRegister8() is private in Adafruit_FT6206, so begin() is the only way to ask.
  // It verifies vendor id and chip id, and apart from rewriting the threshold it is idempotent.
  return touch.begin(128);
  #else
  return false;
  #endif
}

// Read the touchpad
static void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
    // Wait until the touch controller answers. After a soft restart (ESP.restart()) it is not
    // power cycled and can need a moment. With DISPLAY_DRIVER 1 this is also what initializes
    // the touch, because touchChipResponds() calls touch.begin().
    // Retry at most every 500 ms, not on every LVGL poll: with a missing or broken touch
    // controller this would otherwise run every 30 ms forever, and with DISPLAY_DRIVER 1 each
    // attempt deletes and allocates an Adafruit_I2CDevice.
    if (!TouchInitSuccessful) {
      static unsigned long lastTouchRetry = 0;
      if (lastTouchRetry == 0 || millis() - lastTouchRetry >= 500) {
        lastTouchRetry = millis();
        TouchInitSuccessful = touchChipResponds();
      }
      if (!TouchInitSuccessful) {
        data->state = LV_INDEV_STATE_REL;
        return;
      }
    }
  
    uint16_t x, y;
    #if (DISPLAY_DRIVER == 0)
    if (tft.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
        setLastActivityTimestamp_HAL();    
        
        // Uncomment this to show the touchpoint
        //tft.drawFastHLine(0, y, SCR_WIDTH, TFT_RED);
        //tft.drawFastVLine(x, 0, SCR_HEIGHT, TFT_RED);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    #elif (DISPLAY_DRIVER == 1)
    TS_Point touchPoint = touch.getPoint();
    x = touchPoint.x;
    y = touchPoint.y;

    if (!touch.touched()) {
      data->state = LV_INDEV_STATE_REL;
      return;
    }
  
    data->state = LV_INDEV_STATE_PR;
    // The touch controller counts from the opposite corner than the panel does.
    data->point.x = SCR_WIDTH - 1 - x;
    data->point.y = SCR_HEIGHT - 1 - y;
    setLastActivityTimestamp_HAL();    
    #endif
}

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (SCR_WIDTH * SCR_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

static uint32_t my_tick_get_cb(void) {
  return millis();
}

void init_lvgl_HAL() {
  // first init TFT
  init_tft();

  // new in lvgl 9
  lv_tick_set_cb(my_tick_get_cb);
  
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#display-api
  // https://docs.lvgl.io/master/get-started/quick-overview.html#add-lvgl-into-your-project
  lv_display_t *disp = lv_display_create(SCR_WIDTH, SCR_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);

  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#migration-guide
  // lv_display_set_buffers(display, buf1, buf2, buf_size_byte, mode) is more or less the equivalent of lv_disp_draw_buf_init(&draw_buf_dsc, buf1, buf2, buf_size_px) from v8, however in v9 the buffer size is set in bytes.
  #ifdef useTwoBuffersForlvgl
  uint8_t *bufA = (uint8_t *) malloc(DRAW_BUF_SIZE);
  uint8_t *bufB = (uint8_t *) malloc(DRAW_BUF_SIZE);
  lv_display_set_buffers(disp, bufA, bufB, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  #else
  uint8_t *bufA = (uint8_t *) malloc(DRAW_BUF_SIZE);
  lv_display_set_buffers(disp, bufA, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif

  // Initialize the touchscreen driver
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#indev-api
  static lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

}
