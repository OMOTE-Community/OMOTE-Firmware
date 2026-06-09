#pragma once

#include "remote_messages.pb.h"
#include <cstddef>
#include <string>

// Bounded, allocation-free cache of the latest per-device snapshots from the hub.
// Sized to StateSync.devices[8]; a DeviceState is a full snapshot, so merging an
// existing device replaces the whole record (clearing stale optionals).
class HubDeviceStateCache {
public:
  static const size_t CAPACITY = 8;

  // Returns true if the snapshot was inserted or updated; false if ignored
  // (empty device_id) or dropped (cache full and device unknown).
  bool merge(const omote_DeviceState& state);

  const omote_DeviceState* find(const std::string& deviceId) const;
  size_t size() const;
  void clear();

private:
  int indexOf(const char* deviceId) const;

  omote_DeviceState entries[CAPACITY];
  size_t count = 0;
};
