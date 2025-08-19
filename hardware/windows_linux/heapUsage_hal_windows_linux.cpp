#if defined(__APPLE__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#if defined(WIN32)
// https://www.daniweb.com/programming/software-development/threads/135188/calculate-the-amount-of-heap-memory
// returns used heap size in bytes or negative if heap is corrupted.
// It seems the while loop sometimes never ends. Reason unknown.
// If this happens, stop after 200 iterations and return the fake numer 800000, as we are doing in Linux.
// Update: and even with this fallback sometimes the function does not return. Weird. So simply return 800000.
long HeapUsed()
{
    return 800000;

    _HEAPINFO info = { 0, 0, 0 };
    long used = 0;
    int rc;
    
    int loopCounter = 0;
    while (((rc=_heapwalk(&info)) == _HEAPOK) && (loopCounter < 200))
    {
        if (info._useflag == _USEDENTRY)
            used += info._size;
        loopCounter++;
        }
    if (rc != _HEAPEND && rc != _HEAPEMPTY)
        used = (used?-used:-1);

    if (loopCounter <= 200) {
      return used;
    } else {
      return 800000;
    }
}

#elif defined(__linux__) || defined(__APPLE__)
long HeapUsed() {
  // don't know how to get used heap size in linux
  return 800000;
}
#endif

void get_heapUsage_HAL(unsigned long *heapSize, unsigned long *freeHeap, unsigned long *maxAllocHeap, unsigned long *minFreeHeap) {
  *freeHeap =  HeapUsed();
  // Don't know how big heap can get including swap. So we report twice the size of used heap. With that, free heap is the same as used heap.
  *heapSize = 2 * *freeHeap;
  *maxAllocHeap = 0;
  *minFreeHeap = 0;
}
