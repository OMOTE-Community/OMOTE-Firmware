#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>
#include "deviceRegistry.h"


namespace config {
    struct CommandSequence {
        std::vector<Command*> commands;
        void run();
    };


}
