#include <Arduino.h>

void get_heapUsage_HAL(unsigned long *heapSize, unsigned long *freeHeap, unsigned long *maxAllocHeap, unsigned long *minFreeHeap) {
  *heapSize = ESP.getHeapSize();
  *freeHeap = ESP.getFreeHeap();
  *maxAllocHeap = ESP.getMaxAllocHeap();
  *minFreeHeap = ESP.getMinFreeHeap();
}
