// Everything that is common to all display drivers. The driver specific part is in
// displaydriver0_LovyanGFX.cpp, displaydriver1_Arduino_GFX_GPIO.cpp and
// displaydriver2_Arduino_GFX_LCD.cpp.
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include <soc/soc_memory_layout.h> // esp_ptr_internal(), to report where the draw buffers ended up
#include "driver/ledc.h"
#include "displaydriver.h"

void setup_tft(void) {
  // Configure the backlight PWM
  // Manual setup because ledcSetup() briefly turns on the backlight
  ledc_channel_config_t ledc_channel_left;
  ledc_channel_left.gpio_num = (gpio_num_t)LCD_BL_GPIO;
  ledc_channel_left.speed_mode = LEDC_SPEED_MODE;
  ledc_channel_left.channel = LEDC_CHANNEL_5;
  ledc_channel_left.intr_type = LEDC_INTR_DISABLE;
  ledc_channel_left.timer_sel = LEDC_TIMER_1;
  // LEDC channel duty, the range of duty setting is [0, (2**duty_resolution)]
  ledc_channel_left.duty = 0;
  // needs to be set to 0, otherwise log message "E (324) ledc: ledc_set_duty_with_hpoint(699): hpoint argument is invalid"
  // https://github.com/mudassar-tamboli/ESP32-OV7670-WebSocket-Camera/issues/13
  // LEDC channel hpoint value, the max value is 0xfffff
  ledc_channel_left.hpoint = 0;
  ledc_channel_left.flags.output_invert = 1; // Can't do this with ledcSetup()
  // hpoint and duty explained:
  // https://miro.medium.com/v2/resize:fit:1400/1*ViqSTFdH9COZ51iKYrIyMA.png
  ledc_channel_config(&ledc_channel_left);

  ledc_timer_config_t ledc_timer;
  ledc_timer.speed_mode = LEDC_SPEED_MODE;
  ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;
  ledc_timer.timer_num = LEDC_TIMER_1;
  ledc_timer.freq_hz = 640;
  // https://github.com/mudassar-tamboli/ESP32-OV7670-WebSocket-Camera/issues/13
  // otherwise crash with "assert failed: ledc_clk_cfg_to_global_clk ledc.c:444 (false)"
  ledc_timer.clk_cfg = LEDC_USE_APB_CLK;
  esp_err_t err = ledc_timer_config(&ledc_timer);
  if (err != ESP_OK) {
    Serial.println("Error when calling ledc_timer_config!");
  }  

  #if (OMOTE_HARDWARE_REV == 1)
  // Slowly charge the VSW voltage to prevent a brownout
  // Workaround for hardware rev 1!
  Serial.println("Will slowly charge VSW voltage to prevent that screen is completely bright, with no content");
  for(int i = 0; i < 100; i++) {
    digitalWrite(LCD_EN_GPIO, HIGH);  // LCD Logic off
    delayMicroseconds(1);
    digitalWrite(LCD_EN_GPIO, LOW);   // LCD Logic on
  }
  #else
  Serial.println("Will immediately charge VSW voltage. If screen is completely bright, with no content, then this is the reason.");
  digitalWrite(LCD_EN_GPIO, LOW);
  #endif

  // https://github.com/CoretechR/OMOTE/issues/70#issuecomment-2016763291
  delay(100); // Wait for the LCD driver to power on
}

// -----------------------
// Ask the touch controller whether it is there.
// This deliberately goes through the same I2C driver that also drives the touch at runtime,
// so a negative result means "the touch controller did not answer" and not "some other
// I2C stack is not in shape".
bool TouchInitSuccessful = false;
bool touchChipResponds(void) {
  return touchChipResponds_display_specific();
}

// -----------------------
// https://docs.lvgl.io/master/porting/display.html#two-buffers
// With two buffers, the rendering and refreshing of the display become parallel operations
// Second buffer needs 15.360 bytes more memory in heap.

// A second buffer only pays off if my_disp_flush() returns before the transfer is finished. LVGL
// then renders the next area into the second buffer while the first one is still being sent.
// That needs two things: a driver that can send asynchronously, and a flush that does not wait for
// the end of the transfer.
// my_disp_flush() in displaydriver0_LovyanGFX.cpp has both paths: an asynchronous one that keeps
// the write transaction open (startWrite() once, no endWrite()) and sends the buffer with a zero
// copy DMA, and a synchronous one that waits for the transfer in endWrite(). Which one is used is
// decided at runtime by lvgl_flush_async, because the asynchronous path also needs the buffers in
// DMA capable RAM.
//
// Measured on rev5 (ESP32-S3, 8 bit parallel), full screen redraw with LVGL:
//   LovyanGFX, zero copy and asynchronous  26.2 ms (38 fps)
//   LovyanGFX, synchronous                 30.0 ms (33 fps)
//   Arduino_GFX ESP32PAR8, no DMA          82.9 ms (12 fps) - slow, but chosen here in the hope that the lower switching rate reduces ghost touches
//
// Which of the two paths is taken is decided by two switches, one at compile time and one at run
// time:
//
//   DISPLAY_DRIVER_CAN_FLUSH_ASYNC        can this driver do it at all?
//     defined right below, for the drivers that can start a transfer and return before it is
//     finished. Every other driver leaves it undefined.
//        |
//        | if defined: ask for two buffers in internal DMA capable RAM (init_lvgl_buffer())
//        v
//   lvgl_flush_async                      does it actually do it?
//     set at run time, and only true if DISPLAY_DRIVER_CAN_FLUSH_ASYNC is defined AND both
//     allocations really succeeded. Driver cannot do it, allocation failed, fallback taken - in
//     all of those cases it stays false, which is also its initial value.
//        |
//        v
//   my_disp_flush()                       has to honour lvgl_flush_async
//     while the flag is false the flush must not return before the transfer is finished. LVGL
//     starts rendering into the buffer again as soon as lv_display_flush_ready() was called, and
//     with one buffer that is the buffer being sent.
//
// A driver that can only send synchronously does not have to look at the flag at all - its
// DISPLAY_DRIVER_CAN_FLUSH_ASYNC is undefined, so the flag is false in any case.
#if (DISPLAY_DRIVER == 0)
  #if(OMOTE_HARDWARE_REV >= 5)
  // LovyanGFX with Bus_Parallel8 (rev5): sending a full screen takes about 4 ms of the 26 ms a
  // frame needs, so the asynchronous flush can hide no more than that - measured about 5 fps more
  // (38 instead of 33 fps), paid with 15.360 bytes of internal RAM. Change this to #undef to get
  // the memory back and run at 33 fps.
  #define DISPLAY_DRIVER_CAN_FLUSH_ASYNC
  #else
  // LovyanGFX with Bus_SPI (rev1-4): sending a full screen takes about 31 ms at 40 MHz, which is
  // longer than LVGL needs to render it. Here an asynchronous flush could hide most of the
  // transfer, so this is the case where a second buffer is worth it. Not measured, no rev1-4 board.
  #define DISPLAY_DRIVER_CAN_FLUSH_ASYNC
  #endif
#elif (DISPLAY_DRIVER == 1)
// Arduino_GFX blocks in every variant: Arduino_ESP32PAR8 and Arduino_ESP32LCD8 (rev5),
// Arduino_ESP32SPI and Arduino_ESP32SPIDMA (rev1-4). draw16bit...Bitmap() only returns when
// everything has been sent, even with DMA - measured on rev5 with ESP32LCD8: the call returns
// after 541 us, the transfer is done after 542 us. A second buffer would only cost memory.
#undef DISPLAY_DRIVER_CAN_FLUSH_ASYNC
#elif (DISPLAY_DRIVER == 2)
// Same for Arduino_ESP32LCD8: draw16bitBeRGBBitmap() does use DMA and does read the LVGL buffer
// directly, but it waits for the transfer before it returns. So DMA yes, asynchronous no, and a
// second buffer would only cost memory.
#undef DISPLAY_DRIVER_CAN_FLUSH_ASYNC
#else
#error "DISPLAY_DRIVER must be 0, 1 or 2"
#endif

bool lvgl_flush_async = false;

// allocate lvgl buffers ----------------------------------------------------------------------------------
// LVGL renders into this buffer, 1/10 of the screen usually works well. In LVGL 9 the buffer size
// is given in bytes, not in pixels as in v8.
#define DRAW_BUF_SIZE (SCR_WIDTH * SCR_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

void init_lvgl_buffer(lv_display_t *disp) {
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#migration-guide
  // lv_display_set_buffers(display, buf1, buf2, buf_size_byte, mode) is the v9 equivalent of
  // lv_disp_draw_buf_init(&draw_buf_dsc, buf1, buf2, buf_size_px) from v8 - the buffers now go to
  // the display object instead of into a descriptor the driver registers afterwards.
  //
  // Internal memory is asked for, because LVGL renders into this buffer pixel by pixel and that is
  // faster in internal RAM than in PSRAM: measured 24.6 instead of 27.0 ms per full screen. A plain
  // malloc() of this size returns PSRAM on rev5, because CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL is
  // 4096.
  // MALLOC_CAP_DMA matters for DISPLAY_DRIVER 2: draw16bitBeRGBBitmap() lets the DMA read out of
  // this buffer directly, but only if esp_ptr_dma_capable() says it may. If not, Arduino_GFX
  // silently copies the data into a buffer of its own instead - it still works, only slower, and
  // nobody would notice. So the requirement is written down here rather than assumed.
  #ifdef DISPLAY_DRIVER_CAN_FLUSH_ASYNC
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
  // One buffer. Internal memory, because LVGL renders into this buffer pixel by pixel and that is
  // faster in internal RAM than in PSRAM: measured 24.6 instead of 27.0 ms per full screen.
  // MALLOC_CAP_DMA on top of that, because the transfer being synchronous does not mean that no
  // DMA is involved: with DISPLAY_DRIVER 2 draw16bitBeRGBBitmap() lets the DMA read out of this
  // buffer directly, and only if esp_ptr_dma_capable() says it may (see the comment above). It
  // costs nothing for the other drivers - internal DRAM is DMA capable on both the ESP32 and the
  // ESP32-S3, the flag only rules out the regions that are not, like RTC fast memory.
  // With WiFi and BLE internal RAM can get tight, so fall back to the normal heap if needed.
  uint8_t *bufA = (uint8_t *) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
  if (!bufA) {
    bufA = (uint8_t *) malloc(DRAW_BUF_SIZE);
  }
  lvgl_flush_async = false; // with one buffer LVGL must not render while a transfer is running
  lv_display_set_buffers(disp, bufA, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif

  // Report what LVGL ended up with. "asynchronous" is the only case in which the second buffer
  // does anything, see the comment above.
  Serial.printf("LVGL: %s draw buffer%s of %u bytes in %s RAM, flush is %s\r\n",
                lvgl_flush_async ? "two" : "one", lvgl_flush_async ? "s" : "", (unsigned)DRAW_BUF_SIZE,
                esp_ptr_internal(bufA) ? "internal" : "PSRAM",
                lvgl_flush_async ? "asynchronous, the display DMA sends while LVGL renders the next area"
                                 : "synchronous, it returns when everything has been sent");
}

#if ((DISPLAY_DRIVER == 0) || (DISPLAY_DRIVER == 2)) && (LVGL_DELIVERS_PANEL_BYTE_ORDER == 0)
// LVGL renders RGB565 in the CPU byte order, the ILI9341 expects the high byte first. For the
// asynchronous transfer the DMA sends the buffer exactly as it is, so the bytes have to be swapped
// in the buffer beforehand. This runs while the previous transfer is still going on, so it does not
// cost transfer time. Measured on rev5: 133..145 us for 240x32 pixels.
// noinline + IRAM_ATTR: one fixed copy in internal RAM, otherwise the speed of this loop depends on
// how the compiler inlines it (measured 145 us in one build and 226 us in another).
void IRAM_ATTR __attribute__((noinline)) swap_rgb565_inplace(uint16_t *pixels, uint32_t count) {
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
    pixels[count - 1] = __builtin_bswap16(pixels[count - 1]);
  }
}
#endif

// Make sure the I2C bus runs on OMOTE's pins before the other I2C devices are initialized.
// Several libraries call Wire.begin() without arguments (LIS3DH, Adafruit BusIO for the
// TCA8418 keypad and the FT6206 touch). If Wire is not running at that point, they start it
// on the board defaults SDA=8/SCL=9 - on rev5 that is the keypad interrupt and the backlight.
// With DISPLAY_DRIVER 0 this normally does nothing: LovyanGFX has already started Wire when it
// initialized the touch in tft.init(), and it has to stay that way. If Wire is started before
// tft.init(), LovyanGFX only shares the bus instead of owning it, and then its I2C error
// recovery (a reset of the I2C peripheral) breaks Wire for everybody else.
// With DISPLAY_DRIVER 1 and 2 this is what starts the bus, before touch.begin() is called.
// 100 kHz, because that is what LovyanGFX starts Wire with (it passes no frequency), so the
// bus runs at the same speed with both drivers and in the fallback case above. LovyanGFX's
// own touch transfers still run at the 400 kHz of its touch config; it sets that per
// transfer and restores the Wire settings afterwards.
void setup_wire(void) {
  Wire.begin(SDA_GPIO, SCL_GPIO, 100000);
}
