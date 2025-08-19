#pragma once

void init_battery_HAL(void);
void get_battery_status_HAL(int *battery_voltage, int *battery_percentage, bool *battery_ischarging);
