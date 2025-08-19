#pragma once

#if(OMOTE_HARDWARE_REV >= 5)

#else
    extern const uint8_t SW_1_GPIO; // 1...5: Output
    extern const uint8_t SW_2_GPIO;
    extern const uint8_t SW_3_GPIO;
    extern const uint8_t SW_4_GPIO;
    extern const uint8_t SW_5_GPIO;
    extern const uint8_t SW_A_GPIO; // A...E: Input
    extern const uint8_t SW_B_GPIO;
    extern const uint8_t SW_C_GPIO;
    extern const uint8_t SW_D_GPIO;
    extern const uint8_t SW_E_GPIO;
#endif
extern const uint64_t BUTTON_PIN_BITMASK;

void init_keys_HAL(void);
void keys_getKeys_HAL(void* ptr, unsigned long currentMillis);

#if(OMOTE_HARDWARE_REV >= 5)
    // called from the HAL
    void update_keyboardBrightness_HAL(void);
    uint8_t get_keyboardBrightness_HAL();
    void set_keyboardBrightness_HAL(uint8_t aKeyboardBrightness);
#endif