#include <Arduino.h>
#include "SparkFunLIS3DH.h"
#include "sleep_hal_esp32.h"
// before going to sleep, some tasks have to be done
// save settings
#include "preferencesStorage_hal_esp32.h"
// turn off power of IR receiver
#include "infrared_receiver_hal_esp32.h"
// turn off tft
#include "tft_hal_esp32.h"
// turn off SD card
#include "sd_card_hal_esp32.h"
// disconnect WiFi
#include "mqtt_hal_esp32.h"
// disconnect BLE keyboard
#include "keyboard_ble_hal_esp32.h"
// disconnect ESP-NOW
#include "espnow_hal_esp32.h"
// prepare keypad keys to wakeup
#include "keypad_keys_hal_esp32.h"

#if (OMOTE_HARDWARE_REV >= 5)
  const uint8_t ACC_INT_GPIO = 2;
#else
  const uint8_t ACC_INT_GPIO = 13;
#endif

int DEFAULT_MOTION_THRESHOLD = 80; // motion above threshold keeps device awake
int DEFAULT_SLEEP_TIMEOUT = 20000; // default time until device enters sleep mode in milliseconds. Can be overridden.

// is "lift to wake" enabled
bool wakeupByIMUEnabled = true;
// timeout before going to sleep
uint32_t sleepTimeout;
// threshold for motion detection
uint8_t motionThreshold;
// Timestamp of the last activity. Go to sleep if (millis() - lastActivityTimestamp > sleepTimeout)
uint32_t lastActivityTimestamp;

LIS3DH IMU(I2C_MODE, 0x19);
Wakeup_reasons wakeup_reason;

void setLastActivityTimestamp_HAL() {
  // There was motion, touchpad or key hit.
  // Set the time where this happens.
  lastActivityTimestamp = millis();
}

void activityDetection() {
  // if there is any motion, setLastActivityTimestamp_HAL() is called 
  int motion = 0;
  
  // A variable declared static inside a function is visible only inside that function, exists only once (not created/destroyed for each call) and is permanent. It is in a sense a private global variable.
  static int accXold;
  static int accYold;
  static int accZold;
  int accX = IMU.readFloatAccelX()*1000;
  int accY = IMU.readFloatAccelY()*1000;
  int accZ = IMU.readFloatAccelZ()*1000;

  // determine motion value as da/dt
  motion = (abs(accXold - accX) + abs(accYold - accY) + abs(accZold - accZ));
  // If the motion exceeds the threshold, the lastActivityTimestamp is updated
  if(motion > motionThreshold) {
    // Serial.printf("Motion activity %d at %lu ms\r\n", motion, millis());
    setLastActivityTimestamp_HAL();
  }

  // Store the current acceleration and time 
  accXold = accX;
  accYold = accY;
  accZold = accZ;
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
  // Save settings to internal flash memory
  save_preferences_HAL();

  // Configure IMU
  uint8_t intDataRead;
  // clear interrupt
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC);
  configIMUInterruptsBeforeGoingToSleep();
  // really clear interrupt
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC);

  #if (ENABLE_WIFI_AND_MQTT == 1)
  // Power down modem
  wifi_shutdown_HAL();
  #endif

  #if (ENABLE_KEYBOARD_BLE == 1)
  keyboardBLE_shutdown_HAL();
  #endif

  #if (ENABLE_HUB_COMMUNICATION == 1)
  espnow_shutdown_HAL();
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

void init_sleep_HAL() {
  // will be called after boot or wakeup. Releases GPIO hold and sets wakeup_reason
  if (sleepTimeout == 0){
    sleepTimeout = DEFAULT_SLEEP_TIMEOUT;
  }
  if (motionThreshold == 0){
    motionThreshold = DEFAULT_MOTION_THRESHOLD;
  }

  // Find out wakeup cause
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
}

void init_IMU_HAL(void) {
  // setup IMU to recognize motion
  IMU.settings.accelSampleRate = 50;  //Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  IMU.settings.accelRange = 2;        //Max G force readable.  Can be: 2, 4, 8, 16
  IMU.settings.adcEnabled = 0;
  IMU.settings.tempEnabled = 0;
  IMU.settings.xAccelEnabled = 1;
  IMU.settings.yAccelEnabled = 1;
  IMU.settings.zAccelEnabled = 1;
  IMU.begin();  
  uint8_t intDataRead;
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC);//clear interrupt

}

void check_activity_HAL() {
  activityDetection();
  if(millis() - lastActivityTimestamp > sleepTimeout){
    Serial.println("Entering Sleep Mode. Goodbye.");
    enterSleep();
  }
}

uint32_t get_sleepTimeout_HAL() {
  return sleepTimeout;
}
void set_sleepTimeout_HAL(uint32_t aSleepTimeout) {
  sleepTimeout = aSleepTimeout;
  // For reason unknown, some users reported sleepTimeout was set to 0. In this case device would immediately go to sleep. Prevent this.
  if (sleepTimeout == 0) {
    sleepTimeout = DEFAULT_SLEEP_TIMEOUT;
  }
}
bool get_wakeupByIMUEnabled_HAL() {
  return wakeupByIMUEnabled;
}
void set_wakeupByIMUEnabled_HAL(bool aWakeupByIMUEnabled) {
  wakeupByIMUEnabled = aWakeupByIMUEnabled;
}
uint32_t get_lastActivityTimestamp() {
  return lastActivityTimestamp;
}
uint8_t get_motionThreshold_HAL() {
  return motionThreshold;
}
void set_motionThreshold_HAL(uint8_t aMotionThreshold) {
  motionThreshold = aMotionThreshold;
  if (motionThreshold == 0) {
    motionThreshold = DEFAULT_MOTION_THRESHOLD;
  }
}
