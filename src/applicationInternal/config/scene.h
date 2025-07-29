#pragma once
#include "command.h"

namespace config {

    struct Scene {
        JsonPair ref;
        KeyMap keys;
        CommandSequence on;
        CommandSequence off;
        Scene(JsonPair ref) : ref(ref)
        {}

        const char* displayName() {
            return ref.value()["display_name"];
        }
        
    };
}
