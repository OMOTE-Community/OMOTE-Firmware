#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <stdint.h>
#include <applicationInternal/hardware/hardwarePresenter.h>
#include <applicationInternal/keys.h>
#include <applicationInternal/commandHandler.h>
#include <applicationInternal/omote_log.h>

#include <ArduinoJson.h>

namespace config {
    typedef uint16_t CommandID_t;
    class Device;
    
    class Command {
    protected:
        Device* device_m;        
    public:
        Command(Device* device) : device_m(device)
        {}
        const Device* device() const {
            return device_m;
        }
        virtual void execute() const = 0;
        virtual const char* displayName() const = 0;
        virtual const char* category() const = 0;
        virtual bool hasCategory(const std::string& category) const {
            return false;
        }
    };
    
    class RemoteCommand : public Command {
    public:
        JsonObject ref;
        const CommandID_t ID;
        RemoteCommand(JsonObject ref, uint16_t ID, Device* device) : ref(ref), ID(ID), Command(device)
        {}
        virtual const char* displayName() const {
            return ref["name"];
        }
        virtual const char* category() const {
            return ref["category"];
        }

        virtual void execute() const;
        virtual bool hasCategory(const std::string& category) const;        
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

    struct CommandSequence {
        std::vector<const Command*> commands;
        void findDevicesByCategory(std::vector<const Device*>& out, const std::string& category);
        void dropMatching(const Device* dev, const std::string& category);
        void run();
    };

    class DelayCommand : public Command {
    protected:
        const uint32_t delay_m;
        std::string displayName_m;
        
    public:
        DelayCommand(uint32_t delay) : delay_m(delay), displayName_m("Delay" ), Command(NULL)
        {
            displayName_m += delay + "ms";
        }
        virtual const char* displayName() const {
            return displayName_m.c_str();
        }
        
        virtual void execute() const
        {
            omote_log_i("Delaying for: %d", delay_m);
            delay(delay_m);
        }

        virtual const char* category() const {
            return NULL;
        }
        
    };
}
