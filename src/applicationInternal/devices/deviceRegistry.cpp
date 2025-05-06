#include "deviceRegistry.h"

namespace {
    std::vector<std::string> g_devices;
}

void deviceRegistry::registerDevice(const std::string& n) {
    g_devices.push_back(n);
}
const std::vector<std::string>& deviceRegistry::getDevices() { return g_devices; }
