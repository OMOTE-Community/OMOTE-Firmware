#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <stdint.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/keys.h"
#include <applicationInternal/commandHandler.h>

#include <ArduinoJson.h>

namespace config {
    typedef uint16_t CommandID_t;

    class Command {
    public:
        virtual void execute() const = 0;
        virtual const char* displayName() const = 0;
    };
    
    struct RemoteCommand : public Command {
        JsonObject ref;
        const CommandID_t ID;
        RemoteCommand(JsonObject ref, uint16_t ID) : ref(ref), ID(ID)
        {}
        virtual const char* displayName() const {
            return ref["name"];
        }
        virtual void execute() const;
    };

    struct KeyMap {
        std::map<char, repeatModes> keys_repeat_modes;
        std::map<char, CommandID_t> keys_short;
        std::map<char, CommandID_t> keys_long;
        static char getKeyCode(const std::string& key);
        void swap(const KeyMap& other) {
            keys_short = other.keys_short;
            keys_long = other.keys_long;
            keys_repeat_modes = other.keys_repeat_modes;
        }
    };

    class CommandSequence {
    public:
        std::vector<const Command*> commands;
        void run();
    };

    class DelayCommand : public Command {
    protected:
        const uint32_t delay_m;
        std::string displayName_m;
        
    public:
        DelayCommand(uint32_t delay) : delay_m(delay), displayName_m("Delay" )
        {
            displayName_m += delay + "ms";
        }
        virtual const char* displayName() const {
            return displayName_m.c_str();
        }
        
        virtual void execute() const
        {
            delay(delay_m);
        }
        
    };
}
