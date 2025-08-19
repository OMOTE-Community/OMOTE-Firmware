#pragma once

#include <LovyanGFX.hpp>

extern const uint8_t LCD_BL_GPIO;
extern const uint8_t LCD_EN_GPIO;
extern const uint8_t LCD_CS_GPIO;
extern const uint8_t LCD_DC_GPIO;

#if(OMOTE_HARDWARE_REV >= 5)
    extern const uint8_t LCD_WR_GPIO;
    extern const uint8_t LCD_RD_GPIO;
    extern const uint8_t LCD_D0_GPIO;
    extern const uint8_t LCD_D1_GPIO;
    extern const uint8_t LCD_D2_GPIO;
    extern const uint8_t LCD_D3_GPIO;
    extern const uint8_t LCD_D4_GPIO;
    extern const uint8_t LCD_D5_GPIO;
    extern const uint8_t LCD_D6_GPIO;
    extern const uint8_t LCD_D7_GPIO;
#else
    extern const uint8_t LCD_MOSI_GPIO;
    extern const uint8_t LCD_SCK_GPIO;
#endif

// used in lvgl_hal.cpp "void my_disp_flush(..."
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
extern LGFX tft;

// only called from lvgl_hal.cpp, not from the HAL
void init_tft(void);

// called from the HAL
void update_backlightBrightness_HAL(void);
uint8_t get_backlightBrightness_HAL();
void set_backlightBrightness_HAL(uint8_t aBacklightBrightness);
