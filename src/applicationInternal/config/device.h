#pragma once
#include <string>
#include <ArduinoJson.h>
#include "command.h"

namespace config {
    
    struct Device {
        JsonPair ref;
        KeyMap defaultKeys;
        Device(JsonPair ref) : ref(ref)
        {}

        const char* displayName() {
            return ref.value()["display_name"];
        }

        const char* ID() const {
            return ref.key().c_str();
        }
        
        std::vector<DeviceCommand> commands;
        const DeviceCommand* getCommand(const std::string& name) const;
        const DeviceCommand* getCommandByCategory(const std::string& category) const;
        void addCommand(JsonObject ref, uint16_t ID, Device* dev);
    };
    
}
