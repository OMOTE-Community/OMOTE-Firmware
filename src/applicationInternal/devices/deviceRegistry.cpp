#include "deviceRegistry.h"
#include <ArduinoJson.h>
#include <embedded_config.h>

namespace {
    std::vector<Device> g_devices;
    static const std::unordered_map<std::string_view,char> key_to_code = {
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
        { "RIGHT", KEY_REWI },
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

char KeyMap::getKeyCode(const std::string_view& key) {
    return key_to_code.at(key);
}
Device* deviceRegistry::registerDevice(JsonPair ref) {
    g_devices.push_back({ref});
    return &g_devices.back();
}

std::vector<Device>& deviceRegistry::getDevices() {
    return g_devices;
}
