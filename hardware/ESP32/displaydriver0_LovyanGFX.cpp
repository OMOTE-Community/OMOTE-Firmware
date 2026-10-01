// Display driver 0: LovyanGFX. Compiles to nothing unless DISPLAY_DRIVER selects it.
#include "displaydriver.h"

#if (DISPLAY_DRIVER == 0)
#include <Arduino.h>
#include <LovyanGFX.hpp>


// LCD declarations -------------------------------------------------------------------------------
class LGFX : public lgfx::LGFX_Device{
private:
    lgfx::Panel_ILI9341 _panel_instance;
    #if(OMOTE_HARDWARE_REV >= 5)
    lgfx::Bus_Parallel8 _bus_instance;
    #else
    lgfx::Bus_SPI _bus_instance;
    #endif
    lgfx::Touch_FT5x06 _touch_instance;

public:
    LGFX(void);
};
LGFX::LGFX(void) {
  {
    auto cfg = _bus_instance.config();
    cfg.freq_write = SPI_FREQUENCY;
    #if(OMOTE_HARDWARE_REV >= 5)
    cfg.pin_wr = LCD_WR_GPIO;
    cfg.pin_rd = LCD_RD_GPIO;
    cfg.pin_rs = LCD_DC_GPIO;
    cfg.pin_d0 = LCD_D0_GPIO;
    cfg.pin_d1 = LCD_D1_GPIO;
    cfg.pin_d2 = LCD_D2_GPIO;
    cfg.pin_d3 = LCD_D3_GPIO;
    cfg.pin_d4 = LCD_D4_GPIO;
    cfg.pin_d5 = LCD_D5_GPIO;
    cfg.pin_d6 = LCD_D6_GPIO;
    cfg.pin_d7 = LCD_D7_GPIO;
    #else
    cfg.freq_read  = 16000000;
    cfg.dma_channel = SPI_DMA_CH_AUTO;
    cfg.pin_sclk = LCD_SCK_GPIO;
    cfg.pin_mosi = LCD_MOSI_GPIO;
    cfg.pin_dc   = LCD_DC_GPIO;
    #endif
    _bus_instance.config(cfg);
    _panel_instance.setBus(&_bus_instance);
  }
  {
    auto cfg = _panel_instance.config();
    cfg.pin_cs           = LCD_CS_GPIO;
    cfg.pin_rst          = -1;
    cfg.pin_busy         = -1;
    cfg.memory_width     = SCR_WIDTH;
    cfg.memory_height    = SCR_HEIGHT;
    cfg.panel_width      = SCR_WIDTH;
    cfg.panel_height     = SCR_HEIGHT;
    cfg.offset_rotation  = 2;
    _panel_instance.config(cfg);
  }
  {
    auto cfg = _touch_instance.config();
    cfg.i2c_addr = 0x38;
    cfg.i2c_port = 0;
    cfg.pin_sda = SDA_GPIO;
    cfg.pin_scl = SCL_GPIO;
    cfg.freq = 400000;
    cfg.x_min = 0;
    cfg.x_max = SCR_WIDTH-1;
    cfg.y_min = 0;
    cfg.y_max = SCR_HEIGHT-1;
    _touch_instance.config(cfg);
    _panel_instance.setTouch(&_touch_instance);
  }
  setPanel(&_panel_instance);
}
LGFX tft;

// Display flushing
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t *px_map ){
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

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
    #if (LVGL_DELIVERS_PANEL_BYTE_ORDER == 0)
    swap_rgb565_inplace((uint16_t *)px_map, w * h);
    #endif
    if (tft.getStartCount() == 0) {
      tft.startWrite();
    }
    tft.pushImageDMA(area->x1, area->y1, w, h, (lgfx::swap565_t*)px_map);
  } else {
    // Synchronous: everything has been sent when endWrite() returns, so one buffer is enough.
    // The last parameter says whether LovyanGFX has to swap the bytes while sending: with
    // LVGL_DELIVERS_PANEL_BYTE_ORDER = 0 LVGL delivers them in the CPU byte order, so yes. With 1 they are
    // already in the panel's byte order and must not be swapped a second time.
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixels((uint16_t*)px_map, w * h, LVGL_DELIVERS_PANEL_BYTE_ORDER == 0);
    tft.endWrite();
  }

  lv_display_flush_ready( disp );
}

void my_touchpad_read_display_specific(lv_indev_t * indev_driver, lv_indev_data_t * data) {
  uint16_t x, y;
  if (tft.getTouch(&x, &y)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void draw_touchpoint(uint16_t x, uint16_t y) {
  if (x >= 0 && x < tft.width() && y >= 0 && y < tft.height()) {
    tft.drawPixel(x, y, TFT_RED);
  }
}

bool touchChipResponds_display_specific() {
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
}

void init_tft(void) {
  tft.init();
  tft.initDMA();
  #ifdef DISPLAY_INVERT_COLORS
  // Sends the controller's INVON (0x21). Before fillScreen(), so the screen is already cleared to
  // a real black and not to its inverse.
  tft.invertDisplay(true);
  #endif
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
}
#endif // DISPLAY_DRIVER == 0
