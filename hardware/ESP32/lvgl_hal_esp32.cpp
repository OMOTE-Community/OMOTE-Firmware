#include <lvgl.h>
#include <esp_heap_caps.h>
#include <soc/soc_memory_layout.h> // esp_ptr_internal(), to report where the draw buffers ended up
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"

// -----------------------
// https://docs.lvgl.io/master/porting/display.html#two-buffers
// With two buffers, the rendering and refreshing of the display become parallel operations
// Second buffer needs 15.360 bytes more memory in heap.

// A second buffer only pays off if my_disp_flush() returns before the transfer is finished. LVGL
// then renders the next area into the second buffer while the first one is still being sent.
// That needs two things: a driver that can send asynchronously, and a flush that does not wait for
// the end of the transfer.
// my_disp_flush() below has both paths: an asynchronous one that keeps the write transaction open
// (startWrite() once, no endWrite()) and sends the buffer with a zero copy DMA, and a synchronous
// one that waits for the transfer in endWrite(). Which one is used is decided at runtime by
// lvgl_flush_async, because the asynchronous path also needs the buffers in DMA capable RAM.
//
// Measured on rev5 (ESP32-S3, 8 bit parallel), full screen redraw with LVGL:
//   LovyanGFX, zero copy and asynchronous  26.2 ms (38 fps)
//   LovyanGFX, synchronous                 30.0 ms (33 fps)
//   Arduino_GFX ESP32PAR8, no DMA          82.9 ms (12 fps) - slow, but chosen here in the hope that the lower switching rate reduces ghost touches
#if (DISPLAY_DRIVER == 0)
  #if(OMOTE_HARDWARE_REV >= 5)
  // LovyanGFX with Bus_Parallel8 (rev5): sending a full screen takes about 4 ms of the 26 ms a
  // frame needs, so the asynchronous flush can hide no more than that - measured about 5 fps more
  // (38 instead of 33 fps), paid with 15.360 bytes of internal RAM. Change this to #undef to get
  // the memory back and run at 33 fps.
  #define useTwoBuffersForlvgl
  #else
  // LovyanGFX with Bus_SPI (rev1-4): sending a full screen takes about 31 ms at 40 MHz, which is
  // longer than LVGL needs to render it. Here an asynchronous flush could hide most of the
  // transfer, so this is the case where a second buffer is worth it. Not measured, no rev1-4 board.
  #define useTwoBuffersForlvgl
  #endif
#elif (DISPLAY_DRIVER == 1)
// Arduino_GFX blocks in every variant: Arduino_ESP32PAR8 and Arduino_ESP32LCD8 (rev5),
// Arduino_ESP32SPI and Arduino_ESP32SPIDMA (rev1-4). draw16bit...Bitmap() only returns when
// everything has been sent, even with DMA - measured on rev5 with ESP32LCD8: the call returns
// after 541 us, the transfer is done after 542 us. A second buffer would only cost memory.
#undef useTwoBuffersForlvgl
#endif

#if (DISPLAY_DRIVER == 0) && (LV_COLOR_16_SWAP == 0)
// LVGL renders RGB565 in the CPU byte order, the ILI9341 expects the high byte first. For the
// asynchronous transfer the DMA sends the buffer exactly as it is, so the bytes have to be swapped
// in the buffer beforehand. This runs while the previous transfer is still going on, so it does not
// cost transfer time. Measured on rev5: 133..145 us for 240x32 pixels.
// noinline + IRAM_ATTR: one fixed copy in internal RAM, otherwise the speed of this loop depends on
// how the compiler inlines it (measured 145 us in one build and 226 us in another).
static void IRAM_ATTR __attribute__((noinline)) swap_rgb565_inplace(lv_color_t *pixels, uint32_t count) {
  uint32_t *p32 = (uint32_t *)pixels;
  uint32_t pairs = count / 2;
  uint32_t i = 0;
  for (; i + 4 <= pairs; i += 4) { // 4 words per round: less loop overhead
    uint32_t a = p32[i], b = p32[i + 1], c = p32[i + 2], d = p32[i + 3];
    p32[i]     = ((a & 0x00FF00FFu) << 8) | ((a >> 8) & 0x00FF00FFu);
    p32[i + 1] = ((b & 0x00FF00FFu) << 8) | ((b >> 8) & 0x00FF00FFu);
    p32[i + 2] = ((c & 0x00FF00FFu) << 8) | ((c >> 8) & 0x00FF00FFu);
    p32[i + 3] = ((d & 0x00FF00FFu) << 8) | ((d >> 8) & 0x00FF00FFu);
  }
  for (; i < pairs; i++) {
    uint32_t v = p32[i];
    p32[i] = ((v & 0x00FF00FFu) << 8) | ((v >> 8) & 0x00FF00FFu);
  }
  if (count & 1) {
    uint16_t *last = (uint16_t *)&pixels[count - 1];
    *last = __builtin_bswap16(*last);
  }
}
#endif

// Is my_disp_flush() allowed to return before the transfer is finished? Only with two LVGL draw
// buffers in internal DMA capable RAM, and only with a driver that can send asynchronously.
// init_lvgl_HAL() sets this, and it stays false in every other case.
static bool lvgl_flush_async = false;

// Display flushing
static void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t *px_map ) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  #if (DISPLAY_DRIVER == 0)
  // Asynchronous: start the transfer and return while it is still running. LVGL then renders the
  // next area into the second buffer. That is the only reason why there are two buffers.
  // Three things are needed for it:
  //  1. the data must already be in the panel's byte order, so that the DMA can send the LVGL
  //     buffer directly (zero copy). swap565_t tells LovyanGFX that it is.
  //  2. the write transaction has to stay open. endWrite() would wait for the end of the transfer.
  //  3. the buffers have to be in internal DMA capable RAM - if that allocation failed,
  //     lvgl_flush_async stays false and the synchronous path below is used instead.
  // The next pushImageDMA() waits for this transfer before it starts the next one, and getTouch()
  // does the same (the touch config keeps the default bus_shared = true).
  if (lvgl_flush_async) {
    #if (LV_COLOR_16_SWAP == 0)
    swap_rgb565_inplace((lv_color_t *)px_map, w * h);
    #endif
    if (tft.getStartCount() == 0) {
      tft.startWrite();
    }
    tft.pushImageDMA(area->x1, area->y1, w, h, (lgfx::swap565_t *)px_map);
  } else {
    // Synchronous: everything has been sent when endWrite() returns, so one buffer is enough.
    // The last parameter says whether LovyanGFX has to swap the bytes while sending: with
    // LV_COLOR_16_SWAP = 0 LVGL delivers them in the CPU byte order, so yes. With 1 they are
    // already in the panel's byte order and must not be swapped a second time.
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t *)px_map, w * h, LV_COLOR_16_SWAP == 0);
    tft.endWrite();
  }
  #elif (DISPLAY_DRIVER == 1)
  // Arduino_GFX's flush is synchronous - the transfer is done when it returns,
  // so a single buffer is enough and lv_disp_flush_ready() can follow directly.
  agfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(px_map), w, h);
  #endif

  lv_display_flush_ready(disp);
}

// -----------------------
// Ghost touches? The FT6206 decides with the threshold in register 0x80 (ID_G_THGROUP) how much
// signal counts as a touch. The lower the value, the more sensitive it is, and the more likely
// noise - from the panel itself, from the backlight PWM or from the display bus - shows up as a
// touch that nobody made.
// The two display drivers treat that register differently:
//   DISPLAY_DRIVER 0 (LovyanGFX)  : never writes it. Measured on a rev5 board, the value after
//                                   power-on is 0, so the controller runs at its most sensitive.
//   DISPLAY_DRIVER 1 (Adafruit)   : touch.begin() writes it, 128 is the library's default.
// If you see touches that nobody made, define a threshold here. It is then used by both drivers,
// so that they behave the same. Higher = less sensitive, the register is 8 bit, so 0..255.
// Start with 128 and go up in steps if it is not enough. If it gets too high, real touches with a
// light finger are lost.
// Undefined = leave the controller as it is with DISPLAY_DRIVER 0, and use the Adafruit default
// of 128 with DISPLAY_DRIVER 1.
// #define TOUCH_THRESHOLD 128

static bool TouchInitSuccessful = false;
// Ask the touch controller whether it is there.
// This deliberately goes through the same I2C driver that also drives the touch at runtime,
// so a negative result means "the touch controller did not answer" and not "some other
// I2C stack is not in shape".
bool touchChipResponds() {
  #if (DISPLAY_DRIVER == 0)
  // LovyanGFX drives I2C itself (not through Wire), port 0 as configured for the touch above.
  // Register 0xA3 is the chip id of the FT5x06/FT6x06 family.
  if (!lgfx::i2c::readRegister8(0, 0x38, 0xA3, 400000).has_value()) {
    return false;
  }
  #ifdef TOUCH_THRESHOLD
  // LovyanGFX does not touch this register, so write it here. This runs once, as soon as the
  // controller answers for the first time - also after a restart, where it is not power cycled.
  if (lgfx::i2c::writeRegister8(0, 0x38, 0x80, TOUCH_THRESHOLD, 0, 400000).has_value()) {
    Serial.printf("Touch threshold (register 0x80) set to %d\r\n", TOUCH_THRESHOLD);
  } else {
    Serial.println("ERROR: could not set the touch threshold (register 0x80)");
  }
  #endif
  return true;
  #elif (DISPLAY_DRIVER == 1)
  // readRegister8() is private in Adafruit_FT6206, so begin() is the only way to ask.
  // It verifies vendor id and chip id, and apart from rewriting the threshold it is idempotent.
  #ifdef TOUCH_THRESHOLD
  return touch.begin(TOUCH_THRESHOLD);
  #else
  return touch.begin(128); // the Adafruit default
  #endif
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
  // first init TFT -----------------------------------------------------------------------------------------
  init_tft();

  // new in lvgl 9
  lv_tick_set_cb(my_tick_get_cb);

  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#display-api
  // https://docs.lvgl.io/master/get-started/quick-overview.html#add-lvgl-into-your-project
  lv_display_t *disp = lv_display_create(SCR_WIDTH, SCR_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);

  // allocate lvgl buffers ----------------------------------------------------------------------------------
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#migration-guide
  // lv_display_set_buffers(display, buf1, buf2, buf_size_byte, mode) is more or less the equivalent
  // of lv_disp_draw_buf_init(&draw_buf_dsc, buf1, buf2, buf_size_px) from v8, however in v9 the
  // buffer size is set in bytes. DRAW_BUF_SIZE is that size.
  #ifdef useTwoBuffersForlvgl
  // The DMA sends directly out of these buffers, and it cannot read PSRAM. So ask for internal
  // memory explicitly: a plain malloc() of this size returns PSRAM on rev5 (measured: address
  // 0x3d80xxxx, esp_ptr_dma_capable() == false, because CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL is
  // 4096), and a DMA transfer from there sends nothing at all.
  uint8_t *bufA = (uint8_t *) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  uint8_t *bufB = (uint8_t *) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  if (bufA && bufB) {
    lvgl_flush_async = true; // this is the one case where the asynchronous flush may be used
    lv_display_set_buffers(disp, bufA, bufB, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  } else {
    // Without internal DMA capable memory the asynchronous transfer cannot work - a DMA cannot read
    // PSRAM, it would send nothing at all. And a second buffer is of no use without it. So fall back
    // to what works in any memory: one buffer and a synchronous transfer (see my_disp_flush()).
    // Should not happen on rev1-4 anyway: there is no PSRAM there, so this is the normal heap.
    Serial.println("ERROR: no internal DMA capable RAM for the LVGL draw buffers, falling back to one buffer and synchronous transfer");
    free(bufA);
    free(bufB);
    bufA = (uint8_t *) malloc(DRAW_BUF_SIZE);
    lvgl_flush_async = false;
    lv_display_set_buffers(disp, bufA, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  }
  #else
  // One buffer, and the transfer is synchronous, so DMA capable memory is not required here.
  // Internal memory is asked for anyway, because LVGL renders into this buffer pixel by pixel and
  // that is faster in internal RAM than in PSRAM: measured 24.6 instead of 27.0 ms per full screen.
  // With WiFi and BLE internal RAM can get tight, so fall back to the normal heap if needed.
  uint8_t *bufA = (uint8_t *) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_INTERNAL);
  if (!bufA) {
    bufA = (uint8_t *) malloc(DRAW_BUF_SIZE);
  }
  lvgl_flush_async = false; // with one buffer LVGL must not render while a transfer is running
  lv_display_set_buffers(disp, bufA, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif

  // Report what LVGL ended up with. "asynchronous" is the only case in which the second buffer
  // does anything, see the comment at the top of this file.
  Serial.printf("LVGL: %s draw buffer%s of %u bytes in %s RAM, flush is %s\r\n",
                lvgl_flush_async ? "two" : "one", lvgl_flush_async ? "s" : "", (unsigned)DRAW_BUF_SIZE,
                esp_ptr_internal(bufA) ? "internal" : "PSRAM",
                lvgl_flush_async ? "asynchronous, the display DMA sends while LVGL renders the next area"
                                 : "synchronous, it returns when everything has been sent");

  // Initialize the touchscreen driver ----------------------------------------------------------------------
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#indev-api
  static lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

}
