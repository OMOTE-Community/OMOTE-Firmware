#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>
#pragma once
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

        const char* ID() {
            return ref.key().c_str();
        }
        
        std::vector<RemoteCommand> commands;
        void addCommand(JsonObject ref, uint16_t ID) {
            commands.push_back({ref, ID});
            const char* map_short = ref["map_short"];
            if(map_short) {
                defaultKeys.keys_short[KeyMap::getKeyCode(map_short)] = ID;
            }
            const char* map_long = ref["map_long"];        
            if(map_long) {
                defaultKeys.keys_long[KeyMap::getKeyCode(map_long)] = ID;
            }

        }
    };
    
}
