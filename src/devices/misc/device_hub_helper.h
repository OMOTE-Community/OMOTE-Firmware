#pragma once

#include <string>
#include <list>
#include "applicationInternal/commandHandler.h"

#if (ENABLE_HUB_COMMUNICATION == 1)
  inline void register_hub_command(uint16_t *command, const char *device, const char *cmd) {
    std::list<std::string> commandPayload = {device, cmd};
    register_command(command, makeCommandData(HUB, commandPayload));
  }

  inline void register_hub_command_with_params(uint16_t *command, const char *device, 
                                      const char *cmd, std::list<std::string> params) {
    std::list<std::string> commandPayload = {device, cmd};
    
    commandPayload.insert(commandPayload.end(), params.begin(), params.end());
    
    register_command(command, makeCommandData(HUB, commandPayload));
  }

  inline void execute_hub_command(uint16_t command, CommandExecutionType type = CMD_SHORT, const std::string& additionalPayload = "") {
    CommandExecutionParams params;
    params.commandId = command;
    params.commandType = type;
    params.additionalPayload = additionalPayload;
    executeCommand(params);
  }
#endif 