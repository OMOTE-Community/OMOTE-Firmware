#include "scene_registry.h"

using namespace config;


void CommandSequence::run() {
    for(auto cmd: commands) {
        cmd->execute();
    }
}
