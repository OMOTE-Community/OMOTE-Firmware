#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>
#include "applicationInternal/scenes/sceneRegistry.h"

namespace config {
    typedef uint16_t CommandID_t;

    struct Command {
        virtual void execute() = 0;
        virtual const char* displayName() const = 0;
    };
    
    struct RemoteCommand : public Command {
        JsonObject ref;
        CommandID_t ID;
        RemoteCommand(JsonObject ref, uint16_t ID) : ref(ref), ID(ID)
        {}
        virtual const char* displayName() const {
            return ref["name"];
        }
        virtual void execute()
        {}
    };


    struct KeyMap {

        std::map<char, CommandID_t> keys_short;
        std::map<char, CommandID_t> keys_long;
        static char getKeyCode(const std::string& key);
        void swap(const KeyMap& other) {
            keys_short = other.keys_short;
            keys_long = other.keys_long;
        }
    };



    struct Device {
        JsonPair ref;
        KeyMap defaultKeys;
        Device(JsonPair ref) : ref(ref)
        {}

        const char* displayName() {
            return ref.value()["display_name"];
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
    
    Device* registerDevice(JsonPair ref);
    std::vector<Device>& getDevices();
}
