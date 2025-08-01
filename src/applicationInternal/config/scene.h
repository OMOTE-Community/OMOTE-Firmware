#pragma once
#include "command.h"

namespace config {

    struct Scene {
        JsonPair ref;
        KeyMap keys;
        CommandID_t command;
        CommandID_t commandForce;
        CommandSequence start;
        CommandSequence end;
        Scene(JsonPair ref) : ref(ref)
        {}

        const char* displayName() {
            return ref.value()["display_name"];
        }
        
    };
}
