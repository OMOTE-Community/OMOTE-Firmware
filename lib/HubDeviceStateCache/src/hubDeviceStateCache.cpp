#include "hubDeviceStateCache.h"
#include <cstring>

bool HubDeviceStateCache::merge(const omote_DeviceState& state) {
  if (state.device_id[0] == '\0') {
    return false;
  }

  const int existing = indexOf(state.device_id);
  if (existing >= 0) {
    entries[existing] = state;
    return true;
  }

  if (count >= CAPACITY) {
    return false;
  }

  entries[count] = state;
  count++;
  return true;
}

const omote_DeviceState* HubDeviceStateCache::find(const std::string& deviceId) const {
  const int idx = indexOf(deviceId.c_str());
  return idx < 0 ? nullptr : &entries[idx];
}

size_t HubDeviceStateCache::size() const {
  return count;
}

void HubDeviceStateCache::clear() {
  count = 0;
}

int HubDeviceStateCache::indexOf(const char* deviceId) const {
  for (size_t i = 0; i < count; i++) {
    if (strncmp(entries[i].device_id, deviceId, sizeof(entries[i].device_id)) == 0) {
      return static_cast<int>(i);
    }
  }
  return -1;
}
