#pragma once
#include <unordered_set>
#include "command.h"
#include "device.h"

namespace config {

    class Scene {
    public:
        JsonPair ref;
        KeyMap keys;
        CommandID_t command;
        CommandID_t commandForce;
        CommandSequence startSeq;
        CommandSequence end;
        Scene(JsonPair ref) : ref(ref)
        {}

        const char* displayName() {
            return ref.value()["display_name"];
        }
        void start();
        
    protected:
        static Scene* current;
        
    };
}
