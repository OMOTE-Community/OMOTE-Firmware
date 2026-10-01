// Display driver 1: Arduino_GFX, bit banged GPIO bus (rev5) resp. Arduino_ESP32SPI (rev1-4).
// Compiles to nothing unless DISPLAY_DRIVER selects it.
#include "displaydriver.h"

#if (DISPLAY_DRIVER == 1)
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Adafruit_FT6206.h>


// LCD declarations -------------------------------------------------------------------------------
#if(OMOTE_HARDWARE_REV >= 5)
Arduino_DataBus *agfxBus = new Arduino_ESP32PAR8(
  LCD_DC_GPIO, LCD_CS_GPIO, LCD_WR_GPIO, LCD_RD_GPIO,
  LCD_D0_GPIO, LCD_D1_GPIO, LCD_D2_GPIO, LCD_D3_GPIO,
  LCD_D4_GPIO, LCD_D5_GPIO, LCD_D6_GPIO, LCD_D7_GPIO
);
#else
// rev1-4 drives the ILI9341 over VSPI, using its native pins (sck 18, mosi 23, cs 5).
Arduino_DataBus *agfxBus = new Arduino_ESP32SPI(
  LCD_DC_GPIO, LCD_CS_GPIO, LCD_SCK_GPIO, LCD_MOSI_GPIO,
  GFX_NOT_DEFINED /* miso */, VSPI, false /* is_shared_interface */
);
#endif
Arduino_GFX *agfx = new Arduino_ILI9341(agfxBus, GFX_NOT_DEFINED, 0, false);
Adafruit_FT6206 touch = Adafruit_FT6206();

// Display flushing
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t *px_map ){
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  // Arduino_ESP32PAR8, Arduino_ESP32SPI
  // Arduino_GFX's flush is synchronous - the transfer is done when it returns,
  // so a single buffer is enough and lv_display_flush_ready() can follow directly.
  // lvgl_flush_async is never true here, so there is no second path to choose from.
  // draw16bitRGBBitmap() copies the pixels into a buffer of its own and swaps the bytes on the
  // way, so it expects the CPU byte order - which is what LVGL delivers unless LVGL itself swaps
  // (LVGL_DELIVERS_PANEL_BYTE_ORDER).
  agfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(px_map), w, h);

  lv_display_flush_ready( disp );
}

void my_touchpad_read_display_specific(lv_indev_t * indev_driver, lv_indev_data_t * data) {
  uint16_t x, y;
  TS_Point touchPoint = touch.getPoint();
  x = touchPoint.x;
  y = touchPoint.y;

  if (!touch.touched()) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }
  
  data->state = LV_INDEV_STATE_PRESSED;
  // The touch controller counts from the opposite corner than the panel does.
  data->point.x = SCR_WIDTH - 1 - x;
  data->point.y = SCR_HEIGHT - 1 - y;
}

void draw_touchpoint(uint16_t x, uint16_t y) {
  if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
    agfx->drawPixel(x, y, RGB565_RED);
  }
}

bool touchChipResponds_display_specific() {
  // readRegister8() is private in Adafruit_FT6206, so begin() is the only way to ask.
  // It verifies vendor id and chip id, and apart from rewriting the threshold it is idempotent.
  #ifdef TOUCH_THRESHOLD
  return touch.begin(TOUCH_THRESHOLD);
  #else
  return touch.begin(128); // the Adafruit default
  #endif
}

void init_tft(void) {
  #if(OMOTE_HARDWARE_REV >= 5)
  // Arduino_ESP32PAR8 has no speed setting
  if (!agfx->begin()) {
  #else
  // Arduino_ESP32SPI
  if (!agfx->begin(SPI_FREQUENCY)) {
  #endif
    Serial.println("LCD init failed (Arduino_GFX)");
  }

  // Panel corrections, so that "Arduino_GFX" shows the same picture as LovyanGFX.
  // Arduino_GFX never writes the ILI9341 gamma tables - GMCTRP1/GMCTRN1 are commented out
  // in its ili9341_init_operations[], only the curve selection (GAMMASET) is sent - and it
  // uses weaker power and VCOM settings than LovyanGFX. Black level, contrast and the
  // antialiased edges of LVGL's text all depend on these, which is why the picture looks
  // flatter without them. The values below are the ones LovyanGFX writes in
  // Panel_ILI9341::getInitCommands().
  // Not corrected here, because they are not visible in a still picture: FRMCTR1 (frame
  // rate, 0x00,0x13 vs 0x00,0x1A) and DFUNCTR (0x08,0xC2,0x27 vs 0x08,0x82,0x27).
  static const uint8_t gammaP[15] = {0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00};
  static const uint8_t gammaN[15] = {0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F};
  agfxBus->beginWrite();
  agfxBus->writeC8D8(0xC0, 0x23);                              // PWCTR1  power control, VRH   (Arduino_GFX: 0x10)
  agfxBus->writeC8D8(0xC1, 0x10);                              // PWCTR2  power control, SAP/BT (Arduino_GFX: 0x00)
  agfxBus->writeCommand(0xC5); agfxBus->write(0x3E); agfxBus->write(0x28);  // VMCTR1 VCOM      (Arduino_GFX: 0x30,0x30)
  agfxBus->writeC8D8(0xC7, 0x86);                              // VMCTR2  VCOM offset          (Arduino_GFX: 0xB7)
  agfxBus->writeCommand(0xE0);                                 // GMCTRP1 positive gamma curve (Arduino_GFX: not written)
  for (uint8_t i = 0; i < 15; i++) agfxBus->write(gammaP[i]);
  agfxBus->writeCommand(0xE1);                                 // GMCTRN1 negative gamma curve (Arduino_GFX: not written)
  for (uint8_t i = 0; i < 15; i++) agfxBus->write(gammaN[i]);
  agfxBus->endWrite();

  #ifdef DISPLAY_INVERT_COLORS
  // Arduino_ILI9341::invertDisplay() sends INVON (0x21) through the bus, with its own
  // beginWrite()/endWrite() - so it goes after the block above, not inside it.
  agfx->invertDisplay(true);
  #endif
  agfx->fillScreen(RGB565_BLACK);
}
#endif // DISPLAY_DRIVER == 1
