// OMOTE test firmware for ESP32 OMOTE, to test hardware feeatures of the OMOTE board
// 2023-2025 Maximilian Kern, Klaus Musch

#if (DISPLAY_DRIVER == 0)
#include <LovyanGFX.hpp>
#elif (DISPLAY_DRIVER == 1)
#include <Arduino_GFX_Library.h>
#include <Adafruit_FT6206.h>
#endif
#if(OMOTE_HARDWARE_REV >= 5)
  #include <Adafruit_TCA8418.h>
#else
  #include <Keypad.h> // modified for inverted logic
#endif
#include <esp_heap_caps.h>
#include <soc/soc_memory_layout.h> // esp_ptr_internal(), to report where the draw buffer ended up
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <lvgl.h>
#include "WiFi.h"
#include "driver/ledc.h"
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#include "secrets.h"
#if(OMOTE_HARDWARE_REV >= 5)
  #include <SD.h>
  #include <SPI.h>
#endif

#define ENABLE_WIFI // Comment out to diable connected features

// Pin assignment ---------------------------------------------------------------------------------

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

  const uint8_t USER_LED_GPIO = 45;

  const uint8_t IR_RX_GPIO  = 4; // IR receiver input
  const uint8_t IR_VCC_GPIO = 6; // IR receiver power
  const uint8_t IR_LED_GPIO = 5;  // IR LED output

  const uint8_t ACC_INT_GPIO = 2;

  #define LEDC_SPEED_MODE LEDC_LOW_SPEED_MODE

  const uint8_t SD_EN_GPIO = 16;
  const uint8_t SD_CS_GPIO = 18;
  const uint8_t SD_MISO_GPIO = 7;
  const uint8_t SD_MOSI_GPIO = 17;
  const uint8_t SD_SCK_GPIO = 15;
  
  const uint8_t KBD_BL_GPIO = 46;
#else
  const uint8_t SDA_GPIO = 19;
  const uint8_t SCL_GPIO = 22;

  const uint8_t LCD_BL_GPIO = 4;
  const uint8_t LCD_EN_GPIO = 10;
  const uint8_t LCD_CS_GPIO = 5;
  const uint8_t LCD_DC_GPIO = 9;
  const uint8_t LCD_MOSI_GPIO = 23;
  const uint8_t LCD_SCK_GPIO = 18;

  const uint8_t USER_LED_GPIO = 2;

  const uint8_t IR_RX_GPIO  = 15; // IR receiver input
  const uint8_t IR_VCC_GPIO = 25; // IR receiver power
  const uint8_t IR_LED_GPIO = 33; // IR LED output

  const uint8_t ACC_INT_GPIO = 13;

  #define LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#endif

#if (OMOTE_HARDWARE_REV <= 3)
  const uint8_t ADC_BAT_GPIO = 36;  // Battery voltage sense input (1/2 divider), GPIO36, ADC1_CH0, RTC_GPIO0
  // const uint8_t CRG_STAT_GPIO = 21; // battery charger feedback,                  GPIO21, VSPIHD, EMAC_TX_EN
#elif (OMOTE_HARDWARE_REV == 4)
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
  // Initialize MAX17048 battery fuel gauge
  SFE_MAX1704X fuelGauge(MAX1704X_MAX17048);
  const uint8_t CRG_STAT_GPIO = 21; // battery charger feedback,                  GPIO21, VSPIHD, EMAC_TX_EN
#elif (OMOTE_HARDWARE_REV >= 5)
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
  // Initialize MAX17048 battery fuel gauge
  SFE_MAX1704X fuelGauge(MAX1704X_MAX17048);
  const uint8_t CRG_STAT_GPIO = 1;  // battery charger feedback
#endif

const uint8_t keypadROWS = 5; //five rows
const uint8_t keypadCOLS = 5; //five columns
#if(OMOTE_HARDWARE_REV >= 5)
const uint8_t TCA_INT_GPIO = 8;
const uint64_t BUTTON_PIN_BITMASK = 0b0000000000000000000000000000000100000100; //IO02+IO08)

Adafruit_TCA8418 keypad;
byte keyboardBrightness = 255;

char keypadChars[keypadROWS][keypadCOLS] = {
  {'?','p','c','<','='},  //       ?,     play,  config, rewind,   stop
  {'>','o','b','u','l'}, // forward,      off,    back,     up,   left
  {'4','v','1','3','2'}, //    blue, channel-,     red, yellow,  green
  {'i','r','+','k','d'}, //    info,    right, Volume+,     OK,   down
  {'s','^','-','m','e'}, //  source, channel+, Volume-,   mute, record
};

#else
const uint8_t SW_1_GPIO = 32; // 1...5: Output
const uint8_t SW_2_GPIO = 26;
const uint8_t SW_3_GPIO = 27;
const uint8_t SW_4_GPIO = 14;
const uint8_t SW_5_GPIO = 12;
const uint8_t SW_A_GPIO = 37; // A...E: Input
const uint8_t SW_B_GPIO = 38;
const uint8_t SW_C_GPIO = 39;
const uint8_t SW_D_GPIO = 34;
const uint8_t SW_E_GPIO = 35;
const uint64_t BUTTON_PIN_BITMASK = 0b1110110000000000000000000010000000000000; //IO34+IO35+IO37+IO38+IO39(+IO13)
#endif

// Variables and Object declarations --------------------------------------------------------------

// Battery declares -------------------------------------------------------------------------------
int battery_voltage = 0;
int battery_percentage = 100;
bool battery_ischarging = false;

// IMU declarations -------------------------------------------------------------------------------
int IMUInitSuccessful = false;
#define SLEEP_TIMEOUT 20000 // time until device enters sleep mode in milliseconds
#define MOTION_THRESHOLD 80 // motion above threshold keeps device awake
int standbyTimer = SLEEP_TIMEOUT;
bool wakeupByIMUEnabled = true;
LIS3DH IMU(I2C_MODE, 0x19); // Default constructor is I2C, addr 0x19.

// LCD declarations -------------------------------------------------------------------------------
#if (DISPLAY_DRIVER == 0)
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
int backlightBrightness = 255;

// Keypad declarations ----------------------------------------------------------------------------
#if(OMOTE_HARDWARE_REV >= 5)
enum keypad_rawKeyStates {IDLE_RAW, PRESSED_RAW,       RELEASED_RAW};
#else
char hexaKeys[keypadROWS][keypadCOLS] = {
  {'s','^','-','m','e'}, //  source, channel+, Volume-,   mute, record
  {'i','r','+','k','d'}, //    info,    right, Volume+,     OK,   down
  {'4','v','1','3','2'}, //    blue, channel-,     red, yellow,  green
  {'>','o','b','u','l'}, // forward,      off,    back,     up,   left
  {'?','p','c','<','='}  //       ?,     play,  config, rewind,   stop
};

byte rowPins[keypadROWS] = {SW_A_GPIO, SW_B_GPIO, SW_C_GPIO, SW_D_GPIO, SW_E_GPIO}; //connect to the row pinouts of the keypad
byte colPins[keypadCOLS] = {SW_1_GPIO, SW_2_GPIO, SW_3_GPIO, SW_4_GPIO, SW_5_GPIO}; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, keypadROWS, keypadCOLS); 
#endif

// IR declarations --------------------------------------------------------------------------------
IRsend IrSender(IR_LED_GPIO, true);
IRrecv IrReceiver(IR_RX_GPIO);

// Wakeup reason ----------------------------------------------------------------------------------
byte wakeup_reason;
enum Wakeup_reasons{WAKEUP_BY_RESET, WAKEUP_BY_IMU, WAKEUP_BY_KEYPAD};

// Helper Functions -----------------------------------------------------------------------------------------------------------------------

// LVGL declarations ------------------------------------------------------------------------------
// we either show the checksTable with the test results, or an empty screen where all touches are shown as red dots
lv_obj_t* checksTable;
lv_obj_t* touchScreen;
lv_obj_t* touchInfoLabel;

// array for checking which keys have already been recognized
#if(OMOTE_HARDWARE_REV >= 5)
char keysWorking[keypadROWS][keypadCOLS] = {
  {'x','0','0','0','0'},
  {'0','0','0','0','0'},
  {'0','0','0','0','0'},
  {'0','0','0','0','0'},
  {'0','0','0','0','0'},
};
#else
char keysWorking[keypadROWS][keypadCOLS] = {
  {'0','0','0','0','0'},
  {'0','0','0','0','0'},
  {'0','0','0','0','0'},
  {'0','0','0','0','0'},
  {'x','0','0','0','0'},
};
#endif

static void esp32_restart_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    ESP.restart();
  }
}

bool show_touches = false;
uint32_t count_touches = 0;
uint32_t count_touches_old = 0;

static void show_touches_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);

  if(code == LV_EVENT_CLICKED) {
    if (!show_touches) {
      // create new screen, only showing touches
      touchScreen = lv_obj_create(NULL);
      lv_obj_set_style_pad_all(touchScreen, 0, LV_PART_MAIN);
      lv_obj_clear_flag(touchScreen, LV_OBJ_FLAG_SCROLLABLE);
      lv_screen_load(touchScreen);

      // Status line at the bottom.
      touchInfoLabel = lv_label_create(touchScreen);
      lv_obj_set_style_text_font(touchInfoLabel, &lv_font_montserrat_10, LV_PART_MAIN);
      lv_obj_set_style_text_color(touchInfoLabel, lv_color_hex(0x808080), LV_PART_MAIN);
      lv_obj_set_width(touchInfoLabel, SCR_WIDTH);
      lv_obj_align(touchInfoLabel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
      lv_label_set_text(touchInfoLabel, "");

      show_touches = true;
    }
  }
}

// Display flushing
static void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t *px_map ) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  #if (DISPLAY_DRIVER == 0)
  // Synchronous: endWrite() waits until everything has been sent, so LVGL may render into the
  // buffer again as soon as this returns. That is why one buffer is enough here.
  // (The firmware can also send asynchronously, which needs a second buffer and DMA capable
  // memory - see the comments in hardware/ESP32/lvgl_hal_esp32.cpp. For a board test the few
  // frames per second are not worth the memory and the extra complexity.)
  // The last parameter of pushPixels() says whether LovyanGFX has to swap the bytes while
  // sending: with LV_COLOR_16_SWAP = 0 LVGL delivers them in the CPU byte order, so yes.
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushPixels((uint16_t *)px_map, w * h, LV_COLOR_16_SWAP == 0);
  tft.endWrite();
  #elif (DISPLAY_DRIVER == 1)
  // Arduino_GFX's flush is synchronous - the transfer is done when it returns,
  // so a single buffer is enough and lv_disp_flush_ready() can follow directly.
  agfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(px_map), w, h);
  #endif

  lv_display_flush_ready(disp);
}

// Read the touchpad
bool TouchInitSuccessful = false;
bool touchChipResponds();
static void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
    // Ignore the touch until the controller has answered. The retry is done in loop(), which
    // also updates the table - doing it here as well would set the flag without the table.
    if (!TouchInitSuccessful) {
      data->state = LV_INDEV_STATE_REL;
      return;
    }

    uint16_t x, y;
    #if (DISPLAY_DRIVER == 0)
    if (tft.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;

        standbyTimer = SLEEP_TIMEOUT;

        count_touches++;
        // draw touch point
        if (show_touches) {
          if (x >= 0 && x < tft.width() && y >= 0 && y < tft.height()) {
            tft.drawPixel(x, y, TFT_RED);
          }
        }

        lv_table_set_cell_value_fmt(checksTable, 8, 1, "%" LV_PRIu32, count_touches);

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

    standbyTimer = SLEEP_TIMEOUT;

    count_touches++;
    // draw touch point
    if (show_touches) {
      if (data->point.x >= 0 && data->point.x < SCR_WIDTH && data->point.y >= 0 && data->point.y < SCR_HEIGHT) {
        agfx->drawPixel(data->point.x, data->point.y, RGB565_RED);
      }
    }

    lv_table_set_cell_value_fmt(checksTable, 8, 1, "%" LV_PRIu32, count_touches);
    #endif
}

int activityDetection(){
  static int accXold;
  static int accYold;
  static int accZold;
  static int motion = 0;
  int accX = IMU.readFloatAccelX()*1000;
  int accY = IMU.readFloatAccelY()*1000;
  int accZ = IMU.readFloatAccelZ()*1000;

  // determine motion value as da/dt
  motion = (abs(accXold - accX) + abs(accYold - accY) + abs(accZold - accZ));
  // Calculate time to standby
  standbyTimer -= 100;
  if(standbyTimer < 0) standbyTimer = 0;
  // If the motion exceeds the threshold, the standbyTimer is reset
  if(motion > MOTION_THRESHOLD) {
    // Serial.printf("Motion activity %d at %lu ms\r\n", motion, millis());
    standbyTimer = SLEEP_TIMEOUT;
  }

  // Store the current acceleration and time 
  accXold = accX;
  accYold = accY;
  accZold = accZ;

  return motion;
}

void configIMUInterruptsBeforeGoingToSleep()
{
  uint8_t dataToWrite = 0;

  //LIS3DH_INT1_CFG   
  //dataToWrite |= 0x80;//AOI, 0 = OR 1 = AND
  //dataToWrite |= 0x40;//6D, 0 = interrupt source, 1 = 6 direction source
  //Set these to enable individual axes of generation source (or direction)
  // -- high and low are used generically
  dataToWrite |= 0x20;//Z high
  //dataToWrite |= 0x10;//Z low
  dataToWrite |= 0x08;//Y high
  //dataToWrite |= 0x04;//Y low
  dataToWrite |= 0x02;//X high
  //dataToWrite |= 0x01;//X low
  if (wakeupByIMUEnabled) {
    IMU.writeRegister(LIS3DH_INT1_CFG, 0b00101010);
  } else {
    IMU.writeRegister(LIS3DH_INT1_CFG, 0b00000000);
  }
  
  //LIS3DH_INT1_THS   
  dataToWrite = 0;
  //Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x45;
  IMU.writeRegister(LIS3DH_INT1_THS, dataToWrite);
  
  //LIS3DH_INT1_DURATION  
  dataToWrite = 0;
  //minimum duration of the interrupt
  //LSB equals 1/(sample rate)
  dataToWrite |= 0x00; // 1 * 1/50 s = 20ms
  IMU.writeRegister(LIS3DH_INT1_DURATION, dataToWrite);
  
  //LIS3DH_CTRL_REG5
  //Int1 latch interrupt and 4D on  int1 (preserve fifo en)
  IMU.readRegister(&dataToWrite, LIS3DH_CTRL_REG5);
  dataToWrite &= 0xF3; //Clear bits of interest
  dataToWrite |= 0x08; //Latch interrupt (Cleared by reading int1_src)
  //dataToWrite |= 0x04; //Pipe 4D detection from 6D recognition to int1?
  IMU.writeRegister(LIS3DH_CTRL_REG5, dataToWrite);

  //LIS3DH_CTRL_REG5
  //Set interrupt polarity 
  #if(OMOTE_HARDWARE_REV >= 5)
  IMU.writeRegister(LIS3DH_CTRL_REG6, 0x02); // For active-low interrupt
  #else
  IMU.writeRegister(LIS3DH_CTRL_REG6, 0x00); // For active-high interrupt
  #endif

  //LIS3DH_CTRL_REG3
  //Choose source for pin 1
  dataToWrite = 0;
  //dataToWrite |= 0x80; //Click detect on pin 1
  dataToWrite |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
  dataToWrite |= 0x20; //AOI2 event ()
  //dataToWrite |= 0x10; //Data ready
  //dataToWrite |= 0x04; //FIFO watermark
  //dataToWrite |= 0x02; //FIFO overrun
  IMU.writeRegister(LIS3DH_CTRL_REG3, dataToWrite);
  
}

// Enter Sleep Mode
void enterSleep(){

  // Configure IMU
  uint8_t intDataRead;
  // clear interrupt
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC);
  configIMUInterruptsBeforeGoingToSleep();
  // really clear interrupt
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC);

  #ifdef ENABLE_WIFI
  // Power down modem
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  #endif

  // Prepare IO states
  digitalWrite(LCD_DC_GPIO, LOW); // LCD control signals off
  digitalWrite(LCD_CS_GPIO, LOW);
  #if(OMOTE_HARDWARE_REV >= 5)
    digitalWrite(LCD_WR_GPIO, LOW);
    digitalWrite(LCD_RD_GPIO, LOW);
    digitalWrite(LCD_D0_GPIO, LOW);
    digitalWrite(LCD_D1_GPIO, LOW);
    digitalWrite(LCD_D2_GPIO, LOW);
    digitalWrite(LCD_D3_GPIO, LOW);
    digitalWrite(LCD_D4_GPIO, LOW);
    digitalWrite(LCD_D5_GPIO, LOW);
    digitalWrite(LCD_D6_GPIO, LOW);
    digitalWrite(LCD_D7_GPIO, LOW);
  #else
    digitalWrite(LCD_MOSI_GPIO, LOW);
    digitalWrite(LCD_SCK_GPIO, LOW);
  #endif
  digitalWrite(LCD_EN_GPIO, HIGH); // LCD logic off
  digitalWrite(LCD_BL_GPIO, HIGH); // LCD backlight off
  #if(OMOTE_HARDWARE_REV >= 5)
  digitalWrite(SD_EN_GPIO, HIGH); // SD card off
  #endif
  // pinMode(CRG_STAT, INPUT); // Disable Pull-Up
  pinMode(IR_RX_GPIO, INPUT); // force IR receiver pin to INPUT to prevent high current during sleep (additional 60 uA)
  digitalWrite(IR_VCC_GPIO, LOW); // IR Receiver off

  // Configure button matrix for ext1 interrupt  
  #if(OMOTE_HARDWARE_REV < 5)
  pinMode(SW_1_GPIO, OUTPUT);
  pinMode(SW_2_GPIO, OUTPUT);
  pinMode(SW_3_GPIO, OUTPUT);
  pinMode(SW_4_GPIO, OUTPUT);
  pinMode(SW_5_GPIO, OUTPUT);
  digitalWrite(SW_1_GPIO, HIGH);
  digitalWrite(SW_2_GPIO, HIGH);
  digitalWrite(SW_3_GPIO, HIGH);
  digitalWrite(SW_4_GPIO, HIGH);
  digitalWrite(SW_5_GPIO, HIGH);
  gpio_hold_en((gpio_num_t)SW_1_GPIO);
  gpio_hold_en((gpio_num_t)SW_2_GPIO);
  gpio_hold_en((gpio_num_t)SW_3_GPIO);
  gpio_hold_en((gpio_num_t)SW_4_GPIO);
  gpio_hold_en((gpio_num_t)SW_5_GPIO);
  #endif
  // Force display pins to high impedance
  // Without this the display might not wake up from sleep
  pinMode(LCD_BL_GPIO, INPUT);
  pinMode(LCD_EN_GPIO, INPUT);
  gpio_hold_en((gpio_num_t)LCD_BL_GPIO);
  gpio_hold_en((gpio_num_t)LCD_EN_GPIO);  
  gpio_deep_sleep_hold_en();
  
  #if(OMOTE_HARDWARE_REV >= 5)
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_LOW);
  #else
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  #endif

  delay(100);
  // Sleep
  esp_deep_sleep_start();
}

#ifdef ENABLE_WIFI
// WiFi status event
void WiFiEvent(WiFiEvent_t event){
  //Serial.printf("[WiFi-event] event: %d\n", event);
  if(event == ARDUINO_EVENT_WIFI_STA_GOT_IP){

  }
  // Set status bar icon based on WiFi status
  if(event == ARDUINO_EVENT_WIFI_STA_GOT_IP || event == ARDUINO_EVENT_WIFI_STA_GOT_IP6){
    Serial.println(" WiFi Connected!");
    lv_table_set_cell_value_fmt(checksTable, 1, 1, LV_SYMBOL_OK);
  }
}
#endif

static uint32_t my_tick_get_cb(void) {
  return millis();
}

// Setup ----------------------------------------------------------------------------------------------------------------------------------

int fuelGaugeInitSuccessful = false;
int sdCardInitSuccessful = false;

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
// The same switch exists in the firmware, in hardware/ESP32/lvgl_hal_esp32.cpp.
// #define TOUCH_THRESHOLD 128

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

void setup() {  

  Serial.begin(115200);

  setCpuFrequencyMhz(240); // Make sure ESP32 is running at full speed

  // LCD Pin Definition ---------------------------------------------------------------------------
  pinMode(LCD_EN_GPIO, OUTPUT);
  digitalWrite(LCD_EN_GPIO, HIGH);
  pinMode(LCD_BL_GPIO, OUTPUT);
  digitalWrite(LCD_BL_GPIO, HIGH);

  // Find out wakeup cause ------------------------------------------------------------------------
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    if (esp_sleep_get_ext1_wakeup_status() == (0x01<<ACC_INT_GPIO)) {
      wakeup_reason = WAKEUP_BY_IMU;
    } else {
      wakeup_reason = WAKEUP_BY_KEYPAD;
    }
  } else {
    wakeup_reason = WAKEUP_BY_RESET;
  }
  
  pinMode(ACC_INT_GPIO, INPUT);

  // Release GPIO hold in case we are coming out of standby
  #if(OMOTE_HARDWARE_REV < 5)
  gpio_hold_dis((gpio_num_t)SW_1_GPIO);
  gpio_hold_dis((gpio_num_t)SW_2_GPIO);
  gpio_hold_dis((gpio_num_t)SW_3_GPIO);
  gpio_hold_dis((gpio_num_t)SW_4_GPIO);
  gpio_hold_dis((gpio_num_t)SW_5_GPIO);
  #endif
  gpio_hold_dis((gpio_num_t)LCD_EN_GPIO);
  gpio_hold_dis((gpio_num_t)LCD_BL_GPIO);
  gpio_deep_sleep_hold_dis();
   
  // user LED -------------------------------------------------------------------------------------
  pinMode(USER_LED_GPIO, OUTPUT);
  digitalWrite(USER_LED_GPIO, LOW);  

  // setup SD card --------------------------------------------------------------------------------
  #if(OMOTE_HARDWARE_REV >= 5)
  pinMode(SD_EN_GPIO, OUTPUT);
  digitalWrite(SD_EN_GPIO, LOW);
  delay(10); // wait for the SD card to power up
  SPI.begin(SD_SCK_GPIO, SD_MISO_GPIO, SD_MOSI_GPIO, SD_CS_GPIO);
  if (!SD.begin(SD_CS_GPIO, SPI, 10000000)) { // 10 MHz
      Serial.println("No SD card found.");
      sdCardInitSuccessful = false;
  }
  else{
    Serial.println("SD card initialized successfully.");    
    sdCardInitSuccessful = true;
  }
  #endif

  // IR sender ------------------------------------------------------------------------------------
  pinMode(IR_LED_GPIO, OUTPUT);
  digitalWrite(IR_LED_GPIO, HIGH); // HIGH off - LOW on

  IrSender.begin();

  // IR receiver ----------------------------------------------------------------------------------
  // IR Pin Definition
  pinMode(IR_RX_GPIO, INPUT);
  pinMode(IR_VCC_GPIO, OUTPUT);
  // digitalWrite(IR_VCC_GPIO, LOW);  // HIGH on - LOW off
  digitalWrite(IR_VCC_GPIO, HIGH); // Turn on IR receiver
  IrReceiver.enableIRIn();  // Start the receiver

  // init LVGL ------------------------------------------------------------------------------------
  lv_init();

  // setup TFT ------------------------------------------------------------------------------------
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

  delay(100); // Wait for the LCD driver to power on
  #if (DISPLAY_DRIVER == 0)
  tft.init();
  tft.fillScreen(TFT_BLACK);
  // No initDMA() and no setSwapBytes() here: initDMA() does nothing at all on this hardware (it is
  // an empty function for both the parallel and the SPI bus of LovyanGFX), and the byte order is
  // passed to pushPixels() in my_disp_flush() directly.
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
  // on the board defaults SDA=8/SCL=9 - on rev5 that is the keypad interrupt and the backlight.
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
  // Check if the touchscreen is responding
  TouchInitSuccessful = touchChipResponds();

  // setup LVGL -----------------------------------------------------------------------------------
  // new in lvgl 9
  lv_tick_set_cb(my_tick_get_cb);

  /*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
  #define DRAW_BUF_SIZE (SCR_WIDTH * SCR_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#display-api
  // https://docs.lvgl.io/master/get-started/quick-overview.html#add-lvgl-into-your-project
  lv_display_t *disp = lv_display_create(SCR_WIDTH, SCR_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);

  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#migration-guide
  // lv_display_set_buffers(display, buf1, buf2, buf_size_byte, mode) is more or less the equivalent of lv_disp_draw_buf_init(&draw_buf_dsc, buf1, buf2, buf_size_px) from v8, however in v9 the buffer size is set in bytes.
  //
  // One draw buffer. A second one would only help if my_disp_flush() returned before the transfer
  // is finished, and it does not (see there).
  // Internal memory is asked for, because LVGL renders into this buffer pixel by pixel and that is
  // faster in internal RAM than in PSRAM: measured 24.6 instead of 27.0 ms per full screen. A plain
  // malloc() of this size returns PSRAM on rev5, because CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL is
  // 4096. MALLOC_CAP_DMA is not needed, because no DMA reads this buffer directly.
  uint8_t *bufA = (uint8_t *) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_INTERNAL);
  if (!bufA) {
    bufA = (uint8_t *) malloc(DRAW_BUF_SIZE); // internal RAM is tight? then take whatever is left
  }
  Serial.printf("LVGL: one draw buffer of %u bytes in %s RAM\r\n",
                (unsigned)DRAW_BUF_SIZE, esp_ptr_internal(bufA) ? "internal" : "PSRAM");
  lv_display_set_buffers(disp, bufA, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Initialize the touchscreen driver
  // https://github.com/lvgl/lvgl/blob/release/v9.0/docs/CHANGELOG.rst#indev-api
  static lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // create GUI content ---------------------------------------------------------------------------
  // Set the background color
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);

  // we either show the checksTable with the test results, or an empty screen where all touches are shown as red dots

  // Table showing the check results
  checksTable = lv_table_create(lv_screen_active());
  lv_table_set_column_width(checksTable, 0, 157);
  lv_table_set_column_width(checksTable, 1, 80);
  lv_table_set_column_count(checksTable, 2);
  lv_obj_remove_style(checksTable, NULL, LV_PART_ITEMS | LV_STATE_PRESSED);
  lv_obj_set_style_pad_top(checksTable, 5, LV_PART_ITEMS);
  lv_obj_set_style_pad_bottom(checksTable, 5, LV_PART_ITEMS);
  lv_obj_set_style_pad_left(checksTable, 10, LV_PART_ITEMS);
  lv_obj_set_style_pad_right(checksTable, 2, LV_PART_ITEMS);

  lv_table_set_cell_value_fmt(checksTable, 0, 0, "Touchscreen");
  lv_table_set_cell_value_fmt(checksTable, 1, 0, "WiFi");
  lv_table_set_cell_value_fmt(checksTable, 2, 0, "Buttons");
  lv_table_set_cell_value_fmt(checksTable, 3, 0, "IR LED");
  lv_table_set_cell_value_fmt(checksTable, 4, 0, "IR Receiver");
  lv_table_set_cell_value_fmt(checksTable, 5, 0, "IMU");
  lv_table_set_cell_value_fmt(checksTable, 6, 0, "Fuel Gauge (>=rev4)");
  lv_table_set_cell_value_fmt(checksTable, 7, 0, "SD card (>=rev5)");
  lv_table_set_cell_value_fmt(checksTable, 8, 0, "Touch count");
  lv_table_set_cell_value_fmt(checksTable, 9, 0, "Time until sleep");

  for(int i = 0; i < 7; i++){ //lv_table_get_row_cnt(table)
    lv_table_set_cell_value_fmt(checksTable, i, 1, "?");
  }
  lv_table_set_cell_value_fmt(checksTable, 2, 1, "press all");
  lv_table_set_cell_value_fmt(checksTable, 3, 1, "press any");
  lv_table_set_cell_value_fmt(checksTable, 4, 1, "press any");
  lv_table_set_cell_value_fmt(checksTable, 8, 1, "0");
  lv_table_set_cell_value_fmt(checksTable, 9, 1, "%.1f", 0.001f * (float)standbyTimer);

  // restart button
  lv_obj_t * btnRestart = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btnRestart, esp32_restart_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(btnRestart, LV_ALIGN_BOTTOM_MID, -74, -15);
  lv_obj_t * labelRestart = lv_label_create(btnRestart);
  lv_label_set_text(labelRestart, "restart");
  lv_obj_center(labelRestart);

  // show touch hits
  lv_obj_t * btnShowTouches = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btnShowTouches, show_touches_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(btnShowTouches, LV_ALIGN_BOTTOM_MID, 43, -15);
  lv_obj_t * labelShowTouches = lv_label_create(btnShowTouches);
  lv_label_set_text(labelShowTouches, "show touches");
  lv_obj_center(labelShowTouches);

  // Run the LVGL UI once before the loop takes over
  lv_timer_handler();

  // Setup Fuel Gauge IC --------------------------------------------------------------------------
  #if (OMOTE_HARDWARE_REV >= 4)
    // Initialize battery charger indicator input
    pinMode(CRG_STAT_GPIO, INPUT_PULLUP);   
    fuelGaugeInitSuccessful = fuelGauge.begin();
  #else
    // With hardware rev 3 the battery charge status cannot be recognized in a reliable way due to a design flaw in the PCB.
    // See https://github.com/CoretechR/OMOTE/issues/55
    // So charge status is deactivated for now.
    //pinMode(CRG_STAT_GPIO, INPUT_PULLUP);
    pinMode(ADC_BAT_GPIO, INPUT);
  #endif

  // setup keypad ---------------------------------------------------------------------------------
  #if(OMOTE_HARDWARE_REV >= 5)

  if (!keypad.begin(TCA8418_DEFAULT_ADDR, &Wire)) {
    Serial.println("Keypad TCA8418 not found!");
  }
  keypad.matrix(keypadROWS, keypadCOLS);  
  keypad.pinMode(5, INPUT_PULLUP);  // SW_PWR
  keypad.pinMode(6, INPUT_PULLUP);  // SD_DET
  keypad.pinMode(13, INPUT); // USB_3V3

  pinMode(TCA_INT_GPIO, INPUT);
  keypad.flush();
  keypad.writeRegister(TCA8418_REG_CFG, 0b00000001);
  keypad.writeRegister(TCA8418_REG_GPI_EM_1, 0b00111111);
  keypad.writeRegister(TCA8418_REG_GPI_EM_2, 0b00011111); // disable interrupt for COL5 (USB_3V3)
  
  ledcSetup(LEDC_CHANNEL_6, 5000, 8);
  ledcAttachPin(KBD_BL_GPIO, LEDC_CHANNEL_6);
  ledcWrite(LEDC_CHANNEL_6, 0);

  #else
  // Button Pin Definition
  pinMode(SW_1_GPIO, OUTPUT);
  pinMode(SW_2_GPIO, OUTPUT);
  pinMode(SW_3_GPIO, OUTPUT);
  pinMode(SW_4_GPIO, OUTPUT);
  pinMode(SW_5_GPIO, OUTPUT);
  pinMode(SW_A_GPIO, INPUT);
  pinMode(SW_B_GPIO, INPUT);
  pinMode(SW_C_GPIO, INPUT);
  pinMode(SW_D_GPIO, INPUT);
  pinMode(SW_E_GPIO, INPUT);
  #endif

  // Setup IMU ------------------------------------------------------------------------------------
  IMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  IMU.settings.accelRange = 2;        //Max G force readable.  Can be: 2, 4, 8, 16
  IMU.settings.adcEnabled = 0;
  IMU.settings.tempEnabled = 0;
  IMU.settings.xAccelEnabled = 1;
  IMU.settings.yAccelEnabled = 1;
  IMU.settings.zAccelEnabled = 1;
  IMUInitSuccessful = IMU.begin();  
  uint8_t intDataRead;
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC);//clear interrupt
  
  // setup wifi -----------------------------------------------------------------------------------
  #ifdef ENABLE_WIFI
  // Setup WiFi
  WiFi.setHostname("OMOTE"); //define hostname
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setSleep(true);
  #endif

  // finished -------------------------------------------------------------------------------------
  Serial.printf("Setup finished in %lu ms.\r\n", millis());

  // Print MAC address ----------------------------------------------------------------------------
  unsigned char mac_base[6] = {0};
  esp_efuse_mac_get_default(mac_base);
  printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);

  // provide some first test results --------------------------------------------------------------

  if(TouchInitSuccessful) lv_table_set_cell_value_fmt(checksTable, 0, 1, LV_SYMBOL_OK);
  else lv_table_set_cell_value_fmt(checksTable, 0, 1, LV_SYMBOL_WARNING);

  if(IMUInitSuccessful == 0) lv_table_set_cell_value_fmt(checksTable, 5, 1, LV_SYMBOL_OK);
  else lv_table_set_cell_value_fmt(checksTable, 5, 1, LV_SYMBOL_WARNING);

  if(fuelGaugeInitSuccessful) lv_table_set_cell_value_fmt(checksTable, 6, 1, LV_SYMBOL_OK);
  else lv_table_set_cell_value_fmt(checksTable, 6, 1, LV_SYMBOL_WARNING);

  if(sdCardInitSuccessful) lv_table_set_cell_value_fmt(checksTable, 7, 1, LV_SYMBOL_OK);
  else lv_table_set_cell_value_fmt(checksTable, 7, 1, LV_SYMBOL_WARNING);

  lv_table_set_cell_value_fmt(checksTable, 1, 1, LV_SYMBOL_WARNING);

}

// Loop ------------------------------------------------------------------------------------------------------------------------------------

void loop() { 

  // fade tft backlight and keyboard backlight ----------------------------------------------------
  static int fadeInTimer = millis(); // fadeInTimer = time after setup
  // Update Backlight brightness
  if (millis() < fadeInTimer + backlightBrightness) {
    // after boot or wakeup, fade in backlight brightness
    // fade in lasts for <backlightBrightness> ms
    ledcWrite(LEDC_CHANNEL_5, millis() - fadeInTimer);
  } else {
    if(standbyTimer < 2000 && !show_touches) {
      // less than 2000 ms until standby
      // dim backlight
      ledcWrite(LEDC_CHANNEL_5, 85);
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
  #if(OMOTE_HARDWARE_REV >= 5)
  // Update keyboard brightness
  if (millis() < fadeInTimer + keyboardBrightness) {
    // after boot or wakeup, fade in keyboard brightness
    // fade in lasts for <keyboardBrightness> ms
    ledcWrite(LEDC_CHANNEL_6, millis() - fadeInTimer);
  }
  else {
    if(standbyTimer < 2000 && !show_touches) {
      // less than 2000 ms until standby
      // dim keyboard
      ledcWrite(LEDC_CHANNEL_6, 160);
    } else {
      // normal mode, set full keyboardBrightness
      // turn off PWM if keyboard is at full brightness
      if(keyboardBrightness < 255){
        ledcWrite(LEDC_CHANNEL_6, keyboardBrightness);
      }
      else{
        ledc_stop(LEDC_SPEED_MODE, LEDC_CHANNEL_6, 255);
      }
    }
  }
  #endif

  // Update LVGL UI -------------------------------------------------------------------------------
  lv_timer_handler(); 

  // Blink debug LED at 1 Hz ----------------------------------------------------------------------
  digitalWrite(USER_LED_GPIO, millis() % 1000 > 500);

  // Refresh IMU data at 10Hz ---------------------------------------------------------------------
  static unsigned long IMUTaskTimer = millis();
  if(millis() - IMUTaskTimer >= 100) {
    static int currentMotion = 0;
    currentMotion = activityDetection();
    if(IMUInitSuccessful == 0) lv_table_set_cell_value_fmt(checksTable, 5, 1, "%s  %d", LV_SYMBOL_OK, currentMotion);
    else lv_table_set_cell_value_fmt(checksTable, 5, 1, LV_SYMBOL_WARNING);
  
    if((standbyTimer == 0) && !show_touches){
      Serial.println("Entering Sleep Mode. Goodbye.");
      enterSleep();
    }
    IMUTaskTimer = millis();
  }

  // Retry the touch controller check at 2Hz ------------------------------------------------------
  // After a soft restart (ESP.restart()) the touch controller is not power cycled and can need
  // a moment before it answers. Without this retry a single unlucky moment during setup would
  // leave the warning in the table for the rest of the session. Until then my_touchpad_read()
  // ignores the touch, so keep the interval short.
  // This is the only place that retries, because it is the only one that also updates the table.
  static unsigned long touchRetryTimer = millis();
  if(!TouchInitSuccessful && (millis() - touchRetryTimer >= 500)) {
    TouchInitSuccessful = touchChipResponds();
    if(TouchInitSuccessful) lv_table_set_cell_value_fmt(checksTable, 0, 1, LV_SYMBOL_OK);
    touchRetryTimer = millis();
  }

  // Update battery stats at 1Hz ------------------------------------------------------------------
  static unsigned long batteryTaskTimer = millis() + 1000; // add 1s to start immediately
  #if (OMOTE_HARDWARE_REV >= 4)
  if(fuelGaugeInitSuccessful && (millis() - batteryTaskTimer >= 1000)){
    // With hardware rev 4, battery state of charge is monitored by a MAX17048 fuel gauge
    battery_voltage = (int)(fuelGauge.getVoltage()*1000);
    float soc = fuelGauge.getSOC();
    if (soc > 100.0) soc = 100.0;
    battery_percentage = (int)soc;
    battery_ischarging = !digitalRead(CRG_STAT_GPIO);
  #else
  if(millis() - batteryTaskTimer >= 1000){
    int battery_analogRead = 0;

    battery_analogRead = analogRead(ADC_BAT_GPIO);
    battery_voltage = battery_analogRead*2*3350/4095 + 325;
    battery_percentage = constrain(map(battery_voltage, 3700, 4200, 0, 100), 0, 100);
    // Check if battery is charging, fully charged or disconnected
    /*
      "disconnected" cannot be recognized
      https://electronics.stackexchange.com/questions/615215/level-shifting-of-a-3-state-pin
      https://electrical.codidact.com/posts/286209
      https://how2electronics.com/lithium-ion-battery-charger-circuit-using-mcp73831/
    */
    //battery_ischarging = !digitalRead(CRG_STAT_GPIO);
  #endif
    batteryTaskTimer = millis();

    // Serial.printf("Battery: %d V, %d %%, charging: %d\r\n", battery_voltage, battery_percentage, battery_ischarging);
  }

  // standby countdown ----------------------------------------------------------------------------
  static unsigned long standbyTimerInfo = millis();
  if (millis() - standbyTimerInfo >= 100) {
    lv_table_set_cell_value_fmt(checksTable, 9, 1, "%.1f", 0.001f * (float)standbyTimer);
    standbyTimerInfo = millis();
  }

  // Keypad Handling ------------------------------------------------------------------------------
  #if(OMOTE_HARDWARE_REV >= 5)
    byte row = 0;
    byte col = 0;
    int keyCode = 0;
    keypad_rawKeyStates rawKeyState = IDLE_RAW;

    // https://github.com/adafruit/Adafruit_TCA8418/blob/main/examples/tca8418_keypad_gpio_interrupt/tca8418_keypad_gpio_interrupt.ino
    int intStat = keypad.readRegister(TCA8418_REG_INT_STAT);
    if (intStat & 0x01) // Byte 0: K_INT (keyboard interrupt)
    {
      // datasheet page 16 - Table 2
      keyCode = keypad.getEvent();
      if (keyCode & 0x80) rawKeyState = PRESSED_RAW;
      else rawKeyState = RELEASED_RAW;
      // if (keyCode & 0x80) Serial.print("PRESS\t ");
      // else Serial.print("RELEASE\t ");
      //  map keyCode to GPIO nr.
      keyCode &= 0x7F;

      if (keyCode > 96)  //  GPIO
      {
        // process gpio
        keyCode -= 97;
        // this only happens for key 'o' (off). Map this to 1/1
        row = 1;
        col = 1;
        // Serial.print(keyCode);
        // Serial.println();
      }
      else
      {
        // process matrix
        keyCode--;
        row = keyCode / 10;
        col = keyCode % 10;
        //Serial.print(keyCode / 10);
        //Serial.print("\t ");
        //Serial.print(keyCode % 10);
        //Serial.println();
      }

      //  clear the EVENT IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 1);
    }
    if (intStat & 0x02) // Byte 1: GPI_INT (GPIO interrupt)
    {
      //  reading the registers is mandatory to clear IRQ flag
      //  can also be used to find the GPIO changed
      //  as these registers are a bitmap of the gpio pins.
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_1);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_2);
      keypad.readRegister(TCA8418_REG_GPIO_INT_STAT_3);
      //  clear GPIO IRQ flag
      keypad.writeRegister(TCA8418_REG_INT_STAT, 2);
    }

    //  check pending events
    int intstat = keypad.readRegister(TCA8418_REG_INT_STAT);

    if (rawKeyState == IDLE_RAW) {}
    else {

    if (show_touches) {
      // if we are on the screen showing touches, we simply reboot when any key was pressed
      ESP.restart();
    }

    standbyTimer = SLEEP_TIMEOUT; // Reset the sleep timer when a button is pressed

    IrSender.sendRC5(IrSender.encodeRC5X(0x00, 0x69));

    int missingKeys = 24;
    for(int k=0; k<5; k++){
      for(int l=0; l<5; l++){
        if ((row == k) && (col == l)) {
          keysWorking[k][l] = 1;
          Serial.printf("%d %d\r\n", row, col);
        }
        if(keysWorking[k][l] == 1) missingKeys--;
      }
    }
    
    if(missingKeys==0)
      lv_table_set_cell_value_fmt(checksTable, 2, 1, LV_SYMBOL_OK);
    else
      lv_table_set_cell_value_fmt(checksTable, 2, 1, "%d left", missingKeys);
    }
  #else
    customKeypad.getKey(); // Populate key list
    for(int i=0; i<LIST_MAX; i++){ // Handle multiple keys (Not really necessary in this case)
      if(customKeypad.key[i].kstate == PRESSED || customKeypad.key[i].kstate == HOLD){
  
        if (show_touches) {
          // if we are on the screen showing touches, we simply reboot when any key was pressed
          ESP.restart();
        }
  
        standbyTimer = SLEEP_TIMEOUT; // Reset the sleep timer when a button is pressed
        int keyCode = customKeypad.key[i].kcode;
        //Serial.println(customKeypad.key[i].kchar);
        // Always send same IR code
        IrSender.sendRC5(IrSender.encodeRC5X(0x00, 0x69));
  
        int missingKeys = 24;
        for(int k=0; k<5; k++){
          for(int l=0; l<5; l++){
            if(hexaKeys[k][l] == customKeypad.key[i].kchar) keysWorking[k][l] = 1;
            if(keysWorking[k][l] == 1) missingKeys--;
          }
        }
        
        if(missingKeys==0)
          lv_table_set_cell_value_fmt(checksTable, 2, 1, LV_SYMBOL_OK);
        else
          lv_table_set_cell_value_fmt(checksTable, 2, 1, "%d left", missingKeys);
      }
    }
  #endif

  // IR receiver test -----------------------------------------------------------------------------
  decode_results results;
  if (IrReceiver.decode(&results)) {
    //Serial.println(String(results.command));
    lv_table_set_cell_value_fmt(checksTable, 3, 1, LV_SYMBOL_OK);
    lv_table_set_cell_value_fmt(checksTable, 4, 1, LV_SYMBOL_OK);
    IrReceiver.resume(); // Enable receiving of the next value
  }

  // show touch hits, only if show_touches is set -------------------------------------------------
  if (show_touches) {
    static unsigned long touchInfoTimer = millis();
    if ((count_touches_old != count_touches) || (millis() - touchInfoTimer >= 1000)) {
      lv_label_set_text_fmt(touchInfoLabel, "touch count: %d | hit any key to reboot", count_touches);

      count_touches_old = count_touches;
      touchInfoTimer = millis();
    }
  }
}
