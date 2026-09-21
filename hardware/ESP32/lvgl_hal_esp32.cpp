#include <lvgl.h>
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"

// -----------------------
// https://docs.lvgl.io/8.3/porting/display.html?highlight=lv_disp_draw_buf_init#buffering-modes
// With two buffers, the rendering and refreshing of the display become parallel operations
// Second buffer needs 15.360 bytes more memory in heap.
#define useTwoBuffersForlvgl

// Display flushing
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ){
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  #if (DISPLAY_DRIVER == 0)
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  #ifdef useTwoBuffersForlvgl
  tft.pushPixelsDMA((uint16_t*)&color_p->full, w * h);
  #else
  tft.pushColors((uint16_t*)&color_p->full, w * h, true);
  #endif
  tft.endWrite();
  #elif (DISPLAY_DRIVER == 1)
  // Arduino_GFX's flush is synchronous - the transfer is done when it returns,
  // so a single buffer is enough and lv_disp_flush_ready() can follow directly.
  agfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(color_p), w, h);
  #endif

  lv_disp_flush_ready( disp );
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
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
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
        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
        setLastActivityTimestamp_HAL();    
        
        // Uncomment this to show the touchpoint
        //tft.drawFastHLine(0, y, SCR_WIDTH, TFT_RED);
        //tft.drawFastVLine(x, 0, SCR_HEIGHT, TFT_RED);
    } else {
        data->state = LV_INDEV_STATE_REL;
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

static lv_disp_draw_buf_t draw_buf;

void init_lvgl_HAL() {
  // first init TFT
  init_tft();

  #ifdef useTwoBuffersForlvgl
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_color_t * bufB = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_disp_draw_buf_init(&draw_buf, bufA, bufB, SCR_WIDTH * SCR_HEIGHT / 10);
  #else
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_disp_draw_buf_init(&draw_buf, bufA, NULL, SCR_WIDTH * SCR_HEIGHT / 10);
  #endif

  // Initialize the display driver --------------------------------------------------------------------------
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = SCR_WIDTH;
  disp_drv.ver_res = SCR_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  // Initialize the touchscreen driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

}
