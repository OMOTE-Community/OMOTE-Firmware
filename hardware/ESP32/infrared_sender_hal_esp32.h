#pragma once

#include <string>
#include <list>

// infrared
void init_infraredSender_HAL(void);
void sendIRcode_HAL(int protocol, std::list<std::string> commandPayloads, std::string additionalPayload);