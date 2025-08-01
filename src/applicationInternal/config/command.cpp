#include "command.h"
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

using namespace config;

char KeyMap::getKeyCode(const std::string& key) {
    return key_to_code.at(key);
}

void CommandSequence::run() {
    for(auto cmd: commands) {
        cmd->execute();
    }
}


void RemoteCommand::execute() const
{
    executeCommand(ID);
}
