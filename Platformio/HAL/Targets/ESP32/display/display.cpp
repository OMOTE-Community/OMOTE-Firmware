
#include "display.hpp"
#include "omoteconfig.h"
#include "Wire.h"

std::shared_ptr<Display> Display::getInstance()
{
    if (DisplayAbstract::mInstance == nullptr)
    {
        DisplayAbstract::mInstance  = std::shared_ptr<Display>(new Display(LCD_EN, LCD_BL));
    }
    return std::static_pointer_cast<Display>(mInstance);
}

Display::Display(int backlight_pin, int enable_pin): DisplayAbstract(),
    mBacklightPin(backlight_pin),
    mEnablePin(enable_pin),
    tft(TFT_eSPI()),
    touch(Adafruit_FT6206())
{
    pinMode(mEnablePin, OUTPUT);
    digitalWrite(mEnablePin, HIGH);
    pinMode(mBacklightPin, OUTPUT);
    digitalWrite(mBacklightPin, HIGH);

    ledcSetup(LCD_BACKLIGHT_LEDC_CHANNEL, LCD_BACKLIGHT_LEDC_FREQUENCY, LCD_BACKLIGHT_LEDC_BIT_RESOLUTION);
    ledcAttachPin(mBacklightPin, LCD_BACKLIGHT_LEDC_CHANNEL);
    ledcWrite(LCD_BACKLIGHT_LEDC_CHANNEL, 0);

    setupTFT();

    // Slowly charge the VSW voltage to prevent a brownout
    // Workaround for hardware rev 1!
    for(int i = 0; i < 100; i++){
        digitalWrite(this->mEnablePin, HIGH);  // LCD Logic off
        delayMicroseconds(1);
        digitalWrite(this->mEnablePin, LOW);  // LCD Logic on
    }  

    setupTouchScreen();
}

void Display::onTouch(Notification<TS_Point>::HandlerTy aTouchHandler){
  mTouchEvent.onNotify(std::move(aTouchHandler));
}

void Display::setupTFT() {
  tft.init();
  tft.initDMA();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
}

void Display::setupTouchScreen(){
    // Configure i2c pins and set frequency to 400kHz
    Wire.begin(SDA, SCL, 400000);
    touch.begin(128); // Initialize touchscreen and set sensitivity threshold
}

void Display::setBrightness(uint8_t brightness)
{
  mAwakeBrightness = brightness;
  startFade();
}

void Display::setCurrentBrightness(uint8_t brightness){
  mBrightness = brightness;
  ledcWrite(LCD_BACKLIGHT_LEDC_CHANNEL, mBrightness);
}

void Display::turnOff()
{
    digitalWrite(this->mBacklightPin, HIGH);
    digitalWrite(this->mEnablePin, HIGH);
    pinMode(this->mBacklightPin, INPUT);
    pinMode(this->mEnablePin, INPUT);
    gpio_hold_en((gpio_num_t) mBacklightPin);
    gpio_hold_en((gpio_num_t) mEnablePin);
}

void Display::screenInput(lv_indev_drv_t *indev_driver, lv_indev_data_t *data){
 // int16_t touchX, touchY;
  touchPoint = touch.getPoint();
  int16_t touchX = touchPoint.x;
  int16_t touchY = touchPoint.y;
  bool touched = false;
  if ((touchX > 0) || (touchY > 0)) {
    touched = true;
    mTouchEvent.notify(touchPoint);
  }

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    // Set the coordinates
    data->point.x = SCREEN_WIDTH - touchX;
    data->point.y = SCREEN_HEIGHT - touchY;

    // Serial.print( "touchpoint: x" );
    // Serial.print( touchX );
    // Serial.print( " y" );
    // Serial.println( touchY );
    // tft.drawFastHLine(0, screenHeight - touchY, screenWidth, TFT_RED);
    // tft.drawFastVLine(screenWidth - touchX, 0, screenHeight, TFT_RED);
  }
}

void Display::fadeImpl(void* ){
  bool fadeDone = false;
  while(!fadeDone){
    fadeDone = getInstance()->fade();
    vTaskDelay(3 / portTICK_PERIOD_MS); // 3 miliseconds between steps 
    // 0 - 255 will take about .75 seconds to fade up. 
  }
  vTaskDelete(getInstance()->mDisplayFadeTask);
  getInstance()->mDisplayFadeTask = nullptr;
}

bool Display::fade(){
  //Early return no fade needed. 
  if (mBrightness == mAwakeBrightness ||
      isAsleep && mBrightness == 0){return true;} 
  
  bool fadeDown = isAsleep || mBrightness > mAwakeBrightness; 
  if (fadeDown){
    setCurrentBrightness(mBrightness - 1);
    auto setPoint = isAsleep ? 0 : mAwakeBrightness;
    return mBrightness == setPoint;
  }else{
    setCurrentBrightness(mBrightness + 1);
    return mBrightness == mAwakeBrightness;
  }
}

void Display::startFade(){
  if(mDisplayFadeTask != nullptr){// Already have fade task no need to start another.
    xTaskCreate(&Display::fadeImpl, "Display Fade Task",
                  1024, nullptr, 5, &mDisplayFadeTask);
  }
}

void Display::flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushPixelsDMA((uint16_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}