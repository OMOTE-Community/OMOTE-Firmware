// OMOTE test firmware for ESP32 OMOTE, to test hardware feeatures of the OMOTE board
// 2023-2025 Maximilian Kern, Klaus Musch

#include <LovyanGFX.hpp>
#if(OMOTE_HARDWARE_REV >= 5)
  #include <Adafruit_TCA8418.h>
#else
  #include <Keypad.h> // modified for inverted logic
#endif
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
static lv_disp_draw_buf_t draw_buf;
lv_obj_t* checksTable;
lv_obj_t* touchScreen;

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
      lv_scr_load(touchScreen);

      show_touches = true;
    }
  }
}

// Display flushing
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ){
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  // single buffer bufA
  // tft.pushPixels((uint16_t*)&color_p->full, w * h, true);
  // double buffer bufA and bufB
  tft.pushPixelsDMA( ( uint16_t * )&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready( disp );
}

// Read the touchpad
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {
        data->state = LV_INDEV_STATE_PR;
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
        data->state = LV_INDEV_STATE_REL;
    }
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

// Setup ----------------------------------------------------------------------------------------------------------------------------------

int fuelGaugeInitSuccessful = false;
int sdCardInitSuccessful = false;

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
  tft.init();
  tft.initDMA();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  
  // setup LVGL -----------------------------------------------------------------------------------
  // Double buffer
  lv_color_t * bufA = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_color_t * bufB = (lv_color_t *) malloc(sizeof(lv_color_t) * SCR_WIDTH * SCR_HEIGHT / 10);
  lv_disp_draw_buf_init( &draw_buf, bufA, bufB, SCR_WIDTH * SCR_HEIGHT / 10 );

  // Initialize the display driver
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

  // create GUI content ---------------------------------------------------------------------------
  // Set the background color
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);

  // we either show the checksTable with the test results, or an empty screen where all touches are shown as red dots

  // Table showing the check results
  checksTable = lv_table_create(lv_scr_act());
  lv_table_set_col_width(checksTable, 0, 157);
  lv_table_set_col_width(checksTable, 1, 80);
  lv_table_set_col_cnt(checksTable, 2);
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
  lv_obj_t * btnRestart = lv_btn_create(lv_scr_act());
  lv_obj_add_event_cb(btnRestart, esp32_restart_cb, LV_EVENT_ALL, NULL);
  lv_obj_align(btnRestart, LV_ALIGN_BOTTOM_MID, -74, -15);
  lv_obj_t * labelRestart = lv_label_create(btnRestart);
  lv_label_set_text(labelRestart, "restart");
  lv_obj_center(labelRestart);

  // show touch hits
  lv_obj_t * btnShowTouches = lv_btn_create(lv_scr_act());
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
  // Check if the touchscreen is responding
  boolean TouchInitSuccessful = false;
  Wire.beginTransmission(0x38);
  if(Wire.endTransmission() == 0) TouchInitSuccessful = true;

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
      tft.setColor(TFT_BLACK);
      tft.setRawColor(TFT_BLACK);
      tft.setBaseColor(TFT_BLACK);
      tft.fillRect(0, 305, 240, 15);

      tft.setFont(&fonts::Font2);
      tft.setTextSize(0.9);
      tft.setTextColor(TFT_DARKGREY);
      tft.setCursor(0, 305);
      tft.printf("touch count: %d | hit any key to reboot", count_touches);

      count_touches_old = count_touches;
      touchInfoTimer = millis();
    }
  }
}
