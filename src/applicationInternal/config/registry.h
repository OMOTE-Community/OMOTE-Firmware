#pragma once
#include "device.h"
#include "scene.h"
#include <applicationInternal/scenes/sceneRegistry.h>

namespace config {

    void registerRemote(Device* dev);
    Device* getDevice(const std::string& name);
    void registerScene(Scene* scene, t_gui_list* scene_guis);
    std::vector<Device*>& getDevices();
    
}
