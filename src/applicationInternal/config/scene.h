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
        commands_t shortcuts;
        Scene(JsonPair ref) : ref(ref)
        {}

        const char* displayName() {
            return ref.value()["display_name"];
        }
        void start();
        static const Scene* getCurrent() {
            return current;
        }
    protected:
        static Scene* current;
        
    };
}
