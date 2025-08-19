#pragma once

#if defined(WIN32) || defined(__linux__) || defined(__APPLE__)
  #include "windows_linux/include_hal_windows_linux.h"
#elif defined(ARDUINO)
  #include "ESP32/include_hal_esp32.h"
#endif
