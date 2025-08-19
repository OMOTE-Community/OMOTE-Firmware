#pragma once

#include <string>
#include "applicationInternal/commandHandler.h"

void setLabelActiveScene();
void handleScene(uint16_t command, commandData commandData, std::string additionalPayload = "");
void handleGUI  (uint16_t command, commandData commandData, std::string additionalPayload = "");
