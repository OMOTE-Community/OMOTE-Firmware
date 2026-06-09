#pragma once

#include <cstddef>
#include <string>
#include "remote_messages.pb.h"

namespace Hub {

// Usable encoded-protobuf bytes a transport must carry to receive a worst-case
// rich single-device StateSync response (one DeviceState with volume + full
// media metadata + envelope). Proven against the encoder by test_hub_manager's
// budget-proof test; bump only alongside that test if the proto/.options grow.
static const size_t STATE_SYNC_SINGLE_DEVICE_BUDGET = 384;

// The request shape syncState() asks the hub for, chosen from the transport's
// usable inbound budget. A transport that cannot carry a shape requests less,
// never silently loses device state.
enum class SyncRequestShape { TimeOnly, SingleDevice, Full };

inline SyncRequestShape pickSyncRequestShape(size_t usableBudget, bool hasActiveDevice) {
  if (usableBudget >= omote_CommandResult_size) {
    return SyncRequestShape::Full;
  }
  if (hasActiveDevice && usableBudget >= STATE_SYNC_SINGLE_DEVICE_BUDGET) {
    return SyncRequestShape::SingleDevice;
  }
  return SyncRequestShape::TimeOnly;
}

// Freeform RemoteEvent.data string (max_size:64) telling the hub what to send.
inline std::string pickSyncRequestData(size_t usableBudget, const std::string& activeDeviceId) {
  switch (pickSyncRequestShape(usableBudget, !activeDeviceId.empty())) {
    case SyncRequestShape::Full:         return "time,devices";
    case SyncRequestShape::SingleDevice: return "time,devices:" + activeDeviceId;
    case SyncRequestShape::TimeOnly:     return "time";
  }
  return "time";
}

}  // namespace Hub
