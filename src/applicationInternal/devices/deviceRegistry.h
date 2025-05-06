#pragma once
#include <vector>
#include <string>

#define REGISTER_DEVICE(func, pretty) \
    do { func(); deviceRegistry::registerDevice(pretty); } while(0)

namespace deviceRegistry {
    void registerDevice(const std::string& niceName);
    const std::vector<std::string>& getDevices();
}
