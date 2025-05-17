#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>

struct CommandDetails {
    JsonObject ref;
    uint16_t ID;
    CommandDetails(JsonObject ref, uint16_t ID) : ref(ref), ID(ID)
    {}
    const char* displayName() const {
        return ref["name"];
    }    
};

struct Device {
    JsonPair ref;
    Device(JsonPair ref) : ref(ref)
    {}

    const char* displayName() {
        return ref.value()["display_name"];
    }
    std::vector<CommandDetails> commands;
    void addCommand(JsonObject ref, uint16_t ID) {
        commands.push_back({ref, ID});
    }
};

    
namespace deviceRegistry {
    Device* registerDevice(JsonPair ref);
    std::vector<Device>& getDevices();
}
