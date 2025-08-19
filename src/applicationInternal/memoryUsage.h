#pragma once

// activate log on serial output
// log on GUI is activated by button on GUI
//#define SHOW_LOG_ON_SERIAL

// comment it out to see free memory instead of used memory in GUI. Serial log will always show both.
//#define SHOW_USED_MEMORY_INSTEAD_OF_FREE_IN_GUI

bool getShowMemoryUsage();
void setShowMemoryUsage(bool aShowMemoryUsage);
void doLogMemoryUsage(void);
