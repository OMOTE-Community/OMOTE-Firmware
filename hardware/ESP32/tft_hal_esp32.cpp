#include <Arduino.h>
// Not needed by the compiler, only by PlatformIO's library dependency finder (LDF).
// Adafruit_FT6206.h is included only in tft_hal_esp32.h, and in its default mode "chain"
// the LDF does not follow includes in project headers. The FT6206 library is still built
// because it is in lib_deps, but its dependency Adafruit BusIO then gets no include path
// for SPI and Wire, and the build fails with "SPI.h: No such file or directory".
// Including a BusIO header directly in a .cpp makes the LDF resolve BusIO properly.
// On rev5 Adafruit_TCA8418.h (keypad) already does that, on rev1-4 nothing else does.
// Do not remove - the alternative would be "lib_ldf_mode = deep+" in platformio.ini.
#include <Adafruit_I2CDevice.h>
#include <Wire.h>
#include "driver/ledc.h"
#include "tft_hal_esp32.h"
#include "sleep_hal_esp32.h"

// The ESP32 can only do LOW_SPEED_MODE
#if(OMOTE_HARDWARE_REV >= 5)
#define LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE
#else
#define LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#endif

// Set pins for 8-bit mode (ESP32-S3) or SPI (ESP32)
#if(OMOTE_HARDWARE_REV >= 5)
  const uint8_t SDA_GPIO = 20;
  const uint8_t SCL_GPIO = 19;

  const uint8_t LCD_BL_GPIO = 9;
  const uint8_t LCD_EN_GPIO = 38;
  const uint8_t LCD_CS_GPIO = 39;
  const uint8_t LCD_DC_GPIO = 40;
  const uint8_t LCD_WR_GPIO = 41;
  const uint8_t LCD_RD_GPIO = 42;
  const uint8_t LCD_D0_GPIO = 48;
  const uint8_t LCD_D1_GPIO = 47;
  const uint8_t LCD_D2_GPIO = 21;
  const uint8_t LCD_D3_GPIO = 14;
  const uint8_t LCD_D4_GPIO = 13;
  const uint8_t LCD_D5_GPIO = 12;
  const uint8_t LCD_D6_GPIO = 11;
  const uint8_t LCD_D7_GPIO = 10;
#else
  const uint8_t SDA_GPIO = 19;
  const uint8_t SCL_GPIO = 22;

  const uint8_t LCD_BL_GPIO = 4;
  const uint8_t LCD_EN_GPIO = 10;
  const uint8_t LCD_CS_GPIO = 5;
  const uint8_t LCD_DC_GPIO = 9;
  const uint8_t LCD_MOSI_GPIO = 23;
  const uint8_t LCD_SCK_GPIO = 18;
#endif

#if (DISPLAY_DRIVER == 0)
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
#elif (DISPLAY_DRIVER == 1)
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
#endif
byte backlightBrightness = 255;

void init_tft(void) {
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
  delay(5); // Wait for the LCD driver to power on
  #if (DISPLAY_DRIVER == 0)
  tft.init();
  tft.initDMA();
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  #elif (DISPLAY_DRIVER == 1)
  #if(OMOTE_HARDWARE_REV >= 5)
  if (!agfx->begin()) {
  #else
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

  agfx->fillScreen(RGB565_BLACK);
  #endif

  // Make sure the I2C bus runs on OMOTE's pins before the other I2C devices are initialized.
  // Several libraries call Wire.begin() without arguments (LIS3DH, Adafruit BusIO for the
  // TCA8418 keypad and the FT6206 touch). If Wire is not running at that point, they start it
  // on the board defaults - on rev5 SDA=8/SCL=9, which are the keypad interrupt and the backlight.
  // With DISPLAY_DRIVER 0 this normally does nothing: LovyanGFX has already started Wire when it
  // initialized the touch in tft.init(), and it has to stay that way. If Wire is started before
  // tft.init(), LovyanGFX only shares the bus instead of owning it, and then its I2C error
  // recovery (a reset of the I2C peripheral) breaks Wire for everybody else.
  // With DISPLAY_DRIVER 1 this is what starts the bus, before touch.begin() is called.
  // 100 kHz, because that is what LovyanGFX starts Wire with (it passes no frequency), so the
  // bus runs at the same speed with both drivers and in the fallback case above. LovyanGFX's
  // own touch transfers still run at the 400 kHz of its touch config; it sets that per
  // transfer and restores the Wire settings afterwards.
  Wire.begin(SDA_GPIO, SCL_GPIO, 100000);
}

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
