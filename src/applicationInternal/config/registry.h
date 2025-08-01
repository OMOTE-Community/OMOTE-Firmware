#pragma once
#include "device.h"
#include "scene.h"

namespace config {

    void registerRemote(Device* dev);
    Device* getDevice(const std::string& name);
    void registerScene(Scene* scene);
    std::vector<Device*>& getDevices();
    
}
