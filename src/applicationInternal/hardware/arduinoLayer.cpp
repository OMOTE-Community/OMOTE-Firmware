#if defined(WIN32) || defined(__linux__) || defined(__APPLE__)

#include "applicationInternal/hardware/arduinoLayer.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\r\n", milliseconds);
    return milliseconds;
}

void delay(uint32_t ms) {
  unsigned long startTimer = millis();
  while ((millis() - startTimer) < ms) {
  }
}

// we have to simulate millis()
// the first call sets firstTimestampAtProgramstart and is the reference from that on
bool millisAlreadyInitialized = false;
long long firstTimestampAtProgramstart = 0;
unsigned long millis() {
  unsigned long res;
  if (!millisAlreadyInitialized) {
    firstTimestampAtProgramstart = current_timestamp();
    millisAlreadyInitialized = true;
    res = 0;
  } else {
    res = current_timestamp() - firstTimestampAtProgramstart;
  }
  // printf("millis(): %lu\r\n", res);
  return res;
}

SerialClass Serial;
void SerialClass::enableMQTT(PubSubClient* client, const std::string& topic)
{}

void SerialClass::begin(unsigned long) {
  // Serial.begin is one of the first methods called in main.cpp
  // So we use this to initialize the timer
  unsigned long dummy = millis();
}

size_t SerialClass::printf(const char * format, ...) {
  // see how they are doing it in lv_log.c
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
