#pragma once

#include <Arduino.h>

extern int DEFAULT_MOTION_THRESHOLD;
extern int DEFAULT_SLEEP_TIMEOUT;

// wakeup reason
enum Wakeup_reasons{WAKEUP_BY_RESET, WAKEUP_BY_IMU, WAKEUP_BY_KEYPAD};
extern Wakeup_reasons wakeup_reason;

// only called by tft.cpp, needs to know this because tft gets dimmed 2000 ms before going to sleep
uint32_t get_lastActivityTimestamp();

// called from the HAL
void init_sleep_HAL();
void init_IMU_HAL();
void check_activity_HAL();
void setLastActivityTimestamp_HAL();

uint32_t get_sleepTimeout_HAL();
void set_sleepTimeout_HAL(uint32_t aSleepTimeout);
bool get_wakeupByIMUEnabled_HAL();
void set_wakeupByIMUEnabled_HAL(bool aWakeupByIMUEnabled);
uint8_t get_motionThreshold_HAL();
void set_motionThreshold_HAL(uint8_t aMotionThreshold);
