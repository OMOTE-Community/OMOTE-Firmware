#include "device.h"
using namespace config;

const RemoteCommand* Device::getCommand(const std::string& name) const {
    for(auto& cmd : commands) {
        if (cmd.displayName() == name) {
            return &cmd;
        }
    }
    return NULL;
}
const RemoteCommand* Device::getCommandByCategory(const std::string& category) const {
    omote_log_d("%s looking for category: %s", ID(), category.c_str());
    for(auto& cmd : commands) {
        if (cmd.hasCategory(category)) {
            omote_log_d("found %s", cmd.displayName());
            return &cmd;
        }
    }
    return NULL;
}

void Device::addCommand(JsonObject ref, uint16_t ID, Device* dev) {
    commands.push_back({ref, ID, dev});
    const char* map_short = ref["map_short"];
    if(map_short) {
        defaultKeys.keys_short[KeyMap::getKeyCode(map_short)] = ID;
    }
    const char* map_long = ref["map_long"];        
    if(map_long) {
        defaultKeys.keys_long[KeyMap::getKeyCode(map_long)] = ID;
    }

}
