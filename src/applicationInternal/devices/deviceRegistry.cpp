#include "deviceRegistry.h"
#include <ArduinoJson.h>
#include <embedded_config.h>

namespace {
    std::vector<Device> g_devices;
}

Device* deviceRegistry::registerDevice(JsonPair ref) {
    g_devices.push_back({ref});
    return &g_devices.back();
}

std::vector<Device>& deviceRegistry::getDevices() {
    return g_devices;
}
