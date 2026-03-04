#pragma once

#include <stddef.h>

#if defined(ARDUINO)
  // for env:esp32 we need "Arduino.h" e.g. for Serial, delay(), millis()
  #include <Arduino.h>

#elif defined(WIN32) || defined(__linux__) || defined(__APPLE__)
  #include <stdint.h>
  #include <stdlib.h>
  #include <time.h>
  #include <sys/time.h>
  #define RTC_DATA_ATTR

  // MinGW lacks gmtime_r and settimeofday
  #if defined(WIN32)
  static inline struct tm* gmtime_r(const time_t* timep, struct tm* result) {
    struct tm* p = gmtime(timep);
    if (p) { *result = *p; return result; }
    return NULL;
  }
  static inline int settimeofday(const struct timeval* tv, void* tz) {
    (void)tv; (void)tz;
    return -1; // not supported on Windows simulator
  }
  #endif

  // For Windows and Linux there is no Arduino framework available. So we have to simulate at least those very few calls to Arduino functions which are left in the code.
  // Note: Of course there is a lot more Arduino code in folder "hardware/ESP32/*", but this code is only active in case of esp32, so we don't have to simulate this in the Arduino layer if Windows/Linux is active.
  void delay(uint32_t ms);
  unsigned long millis();
  class SerialClass {
  public:
    void begin(unsigned long);
    size_t printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));
    size_t println(const char c[]);
    size_t println(int nr);
  };
  extern SerialClass Serial;

#endif
