#if defined(_WIN32) || defined(WIN32) || defined(__linux__) || defined(__APPLE__)

#include "applicationInternal/hardware/arduinoLayer.h"
#include <chrono>
#include <stdarg.h>
#include <stdio.h>

long long current_timestamp() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

void delay(uint32_t ms) {
  unsigned long startTimer = millis();
  while ((millis() - startTimer) < ms) {
  }
}

bool millisInitialized = false;
long long millisStartTimestamp = 0;
unsigned long millis() {
  unsigned long res;
  if (!millisInitialized) {
    millisStartTimestamp = current_timestamp();
    millisInitialized = true;
    res = 0;
  } else {
    res = current_timestamp() - millisStartTimestamp;
  }
  return res;
}

SerialClass Serial;
void SerialClass::begin(unsigned long) {
  millis();
}

size_t SerialClass::printf(const char * format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vprintf(format, args);
  va_end(args);
  return ret;
}

size_t SerialClass::println(const char c[]) {
  return printf("%s\r\n", c);
}

size_t SerialClass::println(int nr) {
  return printf("%d\r\n", nr);
}

#endif
