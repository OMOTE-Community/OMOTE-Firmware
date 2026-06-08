#pragma once

#include <stddef.h>

#if defined(ARDUINO)
  #include <Arduino.h>

#elif defined(_WIN32) || defined(WIN32) || defined(__linux__) || defined(__APPLE__)
  #include <stdint.h>
  #include <stdlib.h>
  #include <time.h>
  #if defined(__linux__) || defined(__APPLE__)
    #include <sys/time.h>
  #endif
  #define RTC_DATA_ATTR

  // Windows simulator lacks gmtime_r and settimeofday.
  #if defined(_WIN32) || defined(WIN32)
  static inline struct tm* gmtime_r(const time_t* timep, struct tm* result) {
    struct tm* p = gmtime(timep);
    if (p) { *result = *p; return result; }
    return NULL;
  }
  static inline int settimeofday(const void* tv, void* tz) {
    (void)tv; (void)tz;
    return -1;
  }
  #endif

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
