#pragma once

void updateHardwareStatusAndShowOnGUI(void);
#if (ENABLE_HUB_COMMUNICATION > 0)
void updateTimeOnGUI(void);
void setTime(uint32_t timestamp_utc, int32_t seconds_west_of_utc);
#endif
