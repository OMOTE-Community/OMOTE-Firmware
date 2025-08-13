#include "command.h"
#include "device.h"
#include <applicationInternal/omote_log.h>
#include <applicationInternal/scenes/sceneRegistry.h>

namespace {
    static const std::unordered_map<std::string, char> key_to_code = {
        { "OFF", KEY_OFF },
        { "STOP", KEY_STOP },
        { "REWI", KEY_REWI },
        { "PLAY", KEY_PLAY },
        { "FORW", KEY_FORW },
        { "CONF", KEY_CONF },
        { "INFO", KEY_INFO },
        { "UP", KEY_UP },
        { "DOWN", KEY_DOWN },
        { "LEFT", KEY_LEFT },
        { "RIGHT", KEY_RIGHT },
        { "OK", KEY_OK },
        { "BACK", KEY_BACK },
        { "SRC", KEY_SRC },
        { "VOLUP", KEY_VOLUP },
        { "VOLDO", KEY_VOLDO },
        { "MUTE", KEY_MUTE },
        { "REC", KEY_REC },
        { "CHUP", KEY_CHUP },
        { "CHDOW", KEY_CHDOW },
        { "RED", KEY_RED },
        { "GREEN", KEY_GREEN },
        { "YELLO", KEY_YELLO },
        { "BLUE", KEY_BLUE }
    };
}

using namespace config;

char KeyMap::getKeyCode(const std::string& key) {
    auto item = key_to_code.find(key);
    if(item == key_to_code.end()) {
        omote_log_e("Unrecognized key definition: %s", key.c_str());
        return 0;
    }
    return key_to_code.at(key);
}

void CommandSequence::run() {
    for(auto cmd: commands) {
        cmd->execute();
    }
}

void CommandSequence::findDevicesByCategory(std::vector<const Device*>& out, const std::string& category) {
    for(auto cmd : commands) {
        if(cmd->hasCategory(category)) {
            out.push_back(cmd->device());
        }
    }
}

void CommandSequence::dropMatching(const Device* dev, const std::string& category) {
    auto it = commands.begin();
    while(it != commands.end()) {
        if ((*it)->device() == dev && (*it)->hasCategory(category)) {
            it = commands.erase(it);
        }
        else {
            ++it;
        }
    }

}
bool RemoteCommand::hasCategory(const std::string& category_) const {
    
    const char* cat = category();

    if(cat != NULL) {
        if(std::string(cat).find(category_) != std::string::npos) {
            return true;
        }
    }

    return false;

}

void RemoteCommand::execute() const
{
    omote_log_i("Executing command: %s/%s", device_m->ID(), displayName());
    executeCommand(ID);
}
