#include <string>
#include <list>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/scenes/sceneHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/omote_log.h"
#include "devices/misc/device_specialCommands.h"
// show WiFi status
#include "applicationInternal/gui/guiBase.h"
// show received IR and MQTT messages
#include "guis/gui_irReceiver.h"
// show received BLE connection messages
#include "guis/gui_BLEpairing.h"
#include "hub/hubManager.h"

uint16_t COMMAND_UNKNOWN;

uint16_t KEYBOARD_DUMMY_UP                  ; //"Keyboard_dummy_up"
uint16_t KEYBOARD_DUMMY_DOWN                ; //"Keyboard_dummy_down"
uint16_t KEYBOARD_DUMMY_RIGHT               ; //"Keyboard_dummy_right"
uint16_t KEYBOARD_DUMMY_LEFT                ; //"Keyboard_dummy_left"
uint16_t KEYBOARD_DUMMY_SELECT              ; //"Keyboard_dummy_select"
uint16_t KEYBOARD_DUMMY_SENDSTRING          ; //"Keyboard_dummy_sendstring"
uint16_t KEYBOARD_DUMMY_BACK                ; //"Keyboard_dummy_back"
uint16_t KEYBOARD_DUMMY_HOME                ; //"Keyboard_dummy_home"
uint16_t KEYBOARD_DUMMY_MENU                ; //"Keyboard_dummy_menu"
uint16_t KEYBOARD_DUMMY_SCAN_PREVIOUS_TRACK ; //"Keyboard_dummy_scan_previous_track"
uint16_t KEYBOARD_DUMMY_REWIND_LONG         ; //"Keyboard_dummy_rewind_long"
uint16_t KEYBOARD_DUMMY_REWIND              ; //"Keyboard_dummy_rewind"
uint16_t KEYBOARD_DUMMY_PLAYPAUSE           ; //"Keyboard_dummy_playpause"
uint16_t KEYBOARD_DUMMY_FASTFORWARD         ; //"Keyboard_dummy_fastforward"
uint16_t KEYBOARD_DUMMY_FASTFORWARD_LONG    ; //"Keyboard_dummy_fastforward_long"
uint16_t KEYBOARD_DUMMY_SCAN_NEXT_TRACK     ; //"Keyboard_dummy_scan_next_track"
uint16_t KEYBOARD_DUMMY_MUTE                ; //"Keyboard_dummy_mute"
uint16_t KEYBOARD_DUMMY_VOLUME_INCREMENT    ; //"Keyboard_dummy_volume_increment"
uint16_t KEYBOARD_DUMMY_VOLUME_DECREMENT    ; //"Keyboard_dummy_volume_decrement"

uint16_t KEYBOARD_UP                        ; //PPCAT(KEYBOARD_PREFIX, UP)
uint16_t KEYBOARD_DOWN                      ; //PPCAT(KEYBOARD_PREFIX, DOWN)
uint16_t KEYBOARD_RIGHT                     ; //PPCAT(KEYBOARD_PREFIX, RIGHT)
uint16_t KEYBOARD_LEFT                      ; //PPCAT(KEYBOARD_PREFIX, LEFT)
uint16_t KEYBOARD_SELECT                    ; //PPCAT(KEYBOARD_PREFIX, SELECT)
uint16_t KEYBOARD_SENDSTRING                ; //PPCAT(KEYBOARD_PREFIX, SENDSTRING)
uint16_t KEYBOARD_BACK                      ; //PPCAT(KEYBOARD_PREFIX, BACK)
uint16_t KEYBOARD_HOME                      ; //PPCAT(KEYBOARD_PREFIX, HOME)
uint16_t KEYBOARD_MENU                      ; //PPCAT(KEYBOARD_PREFIX, MENU)
uint16_t KEYBOARD_SCAN_PREVIOUS_TRACK       ; //PPCAT(KEYBOARD_PREFIX, SCAN_PREVIOUS_TRACK)
uint16_t KEYBOARD_REWIND_LONG               ; //PPCAT(KEYBOARD_PREFIX, REWIND_LONG)
uint16_t KEYBOARD_REWIND                    ; //PPCAT(KEYBOARD_PREFIX, REWIND)
uint16_t KEYBOARD_PLAYPAUSE                 ; //PPCAT(KEYBOARD_PREFIX, PLAYPAUSE)
uint16_t KEYBOARD_FASTFORWARD               ; //PPCAT(KEYBOARD_PREFIX, FASTFORWARD)
uint16_t KEYBOARD_FASTFORWARD_LONG          ; //PPCAT(KEYBOARD_PREFIX, FASTFORWARD_LONG)
uint16_t KEYBOARD_SCAN_NEXT_TRACK           ; //PPCAT(KEYBOARD_PREFIX, SCAN_NEXT_TRACK)
uint16_t KEYBOARD_MUTE                      ; //PPCAT(KEYBOARD_PREFIX, MUTE)
uint16_t KEYBOARD_VOLUME_INCREMENT          ; //PPCAT(KEYBOARD_PREFIX, VOLUME_INCREMENT)
uint16_t KEYBOARD_VOLUME_DECREMENT          ; //PPCAT(KEYBOARD_PREFIX, VOLUME_DECREMENT)

std::map<uint16_t, commandData> commands;

uint16_t uniqueCommandID = 0;

// register a command and give it a command id
void register_command(uint16_t *command, commandData aCommandData) {
  *command = uniqueCommandID;
  uniqueCommandID++;

  commands[*command] = aCommandData;
}
// only get a unique ID. used by KEYBOARD_DUMMY and COMMAND_UNKNOWN
void get_uniqueCommandID(uint16_t *command) {
  *command = uniqueCommandID;
  uniqueCommandID++;
}

void register_keyboardCommands() {
  get_uniqueCommandID(&KEYBOARD_DUMMY_UP                  );
  get_uniqueCommandID(&KEYBOARD_DUMMY_DOWN                );
  get_uniqueCommandID(&KEYBOARD_DUMMY_RIGHT               );
  get_uniqueCommandID(&KEYBOARD_DUMMY_LEFT                );
  get_uniqueCommandID(&KEYBOARD_DUMMY_SELECT              );
  get_uniqueCommandID(&KEYBOARD_DUMMY_SENDSTRING          );
  get_uniqueCommandID(&KEYBOARD_DUMMY_BACK                );
  get_uniqueCommandID(&KEYBOARD_DUMMY_HOME                );
  get_uniqueCommandID(&KEYBOARD_DUMMY_MENU                );
  get_uniqueCommandID(&KEYBOARD_DUMMY_SCAN_PREVIOUS_TRACK );
  get_uniqueCommandID(&KEYBOARD_DUMMY_REWIND_LONG         );
  get_uniqueCommandID(&KEYBOARD_DUMMY_REWIND              );
  get_uniqueCommandID(&KEYBOARD_DUMMY_PLAYPAUSE           );
  get_uniqueCommandID(&KEYBOARD_DUMMY_FASTFORWARD         );
  get_uniqueCommandID(&KEYBOARD_DUMMY_FASTFORWARD_LONG    );
  get_uniqueCommandID(&KEYBOARD_DUMMY_SCAN_NEXT_TRACK     );
  get_uniqueCommandID(&KEYBOARD_DUMMY_MUTE                );
  get_uniqueCommandID(&KEYBOARD_DUMMY_VOLUME_INCREMENT    );
  get_uniqueCommandID(&KEYBOARD_DUMMY_VOLUME_DECREMENT    );

#if (ENABLE_KEYBOARD_BLE == 1)
  KEYBOARD_UP                  = KEYBOARD_BLE_UP;
  KEYBOARD_DOWN                = KEYBOARD_BLE_DOWN;
  KEYBOARD_RIGHT               = KEYBOARD_BLE_RIGHT;
  KEYBOARD_LEFT                = KEYBOARD_BLE_LEFT;
  KEYBOARD_SELECT              = KEYBOARD_BLE_SELECT;
  KEYBOARD_SENDSTRING          = KEYBOARD_BLE_SENDSTRING;
  KEYBOARD_BACK                = KEYBOARD_BLE_BACK;
  KEYBOARD_HOME                = KEYBOARD_BLE_HOME;
  KEYBOARD_MENU                = KEYBOARD_BLE_MENU;
  KEYBOARD_SCAN_PREVIOUS_TRACK = KEYBOARD_BLE_SCAN_PREVIOUS_TRACK;
  KEYBOARD_REWIND_LONG         = KEYBOARD_BLE_REWIND_LONG;
  KEYBOARD_REWIND              = KEYBOARD_BLE_REWIND;
  KEYBOARD_PLAYPAUSE           = KEYBOARD_BLE_PLAYPAUSE;
  KEYBOARD_FASTFORWARD         = KEYBOARD_BLE_FASTFORWARD;
  KEYBOARD_FASTFORWARD_LONG    = KEYBOARD_BLE_FASTFORWARD_LONG;
  KEYBOARD_SCAN_NEXT_TRACK     = KEYBOARD_BLE_SCAN_NEXT_TRACK;
  KEYBOARD_MUTE                = KEYBOARD_BLE_MUTE;
  KEYBOARD_VOLUME_INCREMENT    = KEYBOARD_BLE_VOLUME_INCREMENT;
  KEYBOARD_VOLUME_DECREMENT    = KEYBOARD_BLE_VOLUME_DECREMENT;
#elif (ENABLE_KEYBOARD_MQTT == 1)
  KEYBOARD_UP                  = KEYBOARD_MQTT_UP;
  KEYBOARD_DOWN                = KEYBOARD_MQTT_DOWN;
  KEYBOARD_RIGHT               = KEYBOARD_MQTT_RIGHT;
  KEYBOARD_LEFT                = KEYBOARD_MQTT_LEFT;
  KEYBOARD_SELECT              = KEYBOARD_MQTT_SELECT;
  KEYBOARD_SENDSTRING          = KEYBOARD_MQTT_SENDSTRING;
  KEYBOARD_BACK                = KEYBOARD_MQTT_BACK;
  KEYBOARD_HOME                = KEYBOARD_MQTT_HOME;
  KEYBOARD_MENU                = KEYBOARD_MQTT_MENU;
  KEYBOARD_SCAN_PREVIOUS_TRACK = KEYBOARD_MQTT_SCAN_PREVIOUS_TRACK;
  KEYBOARD_REWIND_LONG         = KEYBOARD_MQTT_REWIND_LONG;
  KEYBOARD_REWIND              = KEYBOARD_MQTT_REWIND;
  KEYBOARD_PLAYPAUSE           = KEYBOARD_MQTT_PLAYPAUSE;
  KEYBOARD_FASTFORWARD         = KEYBOARD_MQTT_FASTFORWARD;
  KEYBOARD_FASTFORWARD_LONG    = KEYBOARD_MQTT_FASTFORWARD_LONG;
  KEYBOARD_SCAN_NEXT_TRACK     = KEYBOARD_MQTT_SCAN_NEXT_TRACK;
  KEYBOARD_MUTE                = KEYBOARD_MQTT_MUTE;
  KEYBOARD_VOLUME_INCREMENT    = KEYBOARD_MQTT_VOLUME_INCREMENT;
  KEYBOARD_VOLUME_DECREMENT    = KEYBOARD_MQTT_VOLUME_DECREMENT;
#else
  // Of course keyboard commands will not work if neither BLE nor MQTT keyboard is enabled, but at least code will compile.
  // But you have to change keys.cpp, gui_numpad.cpp and commandHandler.cpp where keyboard commands are used so that a command can be executed successfully.
  // Search for "executeCommand(Key" to find them.
  KEYBOARD_UP                  = KEYBOARD_DUMMY_UP;
  KEYBOARD_DOWN                = KEYBOARD_DUMMY_DOWN;
  KEYBOARD_RIGHT               = KEYBOARD_DUMMY_RIGHT;
  KEYBOARD_LEFT                = KEYBOARD_DUMMY_LEFT;
  KEYBOARD_SELECT              = KEYBOARD_DUMMY_SELECT;
  KEYBOARD_SENDSTRING          = KEYBOARD_DUMMY_SENDSTRING;
  KEYBOARD_BACK                = KEYBOARD_DUMMY_BACK;
  KEYBOARD_HOME                = KEYBOARD_DUMMY_HOME;
  KEYBOARD_MENU                = KEYBOARD_DUMMY_MENU;
  KEYBOARD_SCAN_PREVIOUS_TRACK = KEYBOARD_DUMMY_SCAN_PREVIOUS_TRACK;
  KEYBOARD_REWIND_LONG         = KEYBOARD_DUMMY_REWIND_LONG;
  KEYBOARD_REWIND              = KEYBOARD_DUMMY_REWIND;
  KEYBOARD_PLAYPAUSE           = KEYBOARD_DUMMY_PLAYPAUSE;
  KEYBOARD_FASTFORWARD         = KEYBOARD_DUMMY_FASTFORWARD;
  KEYBOARD_FASTFORWARD_LONG    = KEYBOARD_DUMMY_FASTFORWARD_LONG;
  KEYBOARD_SCAN_NEXT_TRACK     = KEYBOARD_DUMMY_SCAN_NEXT_TRACK;
  KEYBOARD_MUTE                = KEYBOARD_DUMMY_MUTE;
  KEYBOARD_VOLUME_INCREMENT    = KEYBOARD_DUMMY_VOLUME_INCREMENT;
  KEYBOARD_VOLUME_DECREMENT    = KEYBOARD_DUMMY_VOLUME_DECREMENT;
#endif

}

commandData makeCommandData(commandHandlers a, std::list<std::string> b) {
  commandData c = {a, b};
  return c;
}

std::string convertStringListToString(std::list<std::string> listOfStrings) {
  std::string result; 
  for(const auto &word : listOfStrings) { 
	  result += word + ","; 
  } 
  return result;
}

void executeCommandWithData(uint16_t command, commandData commandData, std::string additionalPayload) {
  switch (commandData.commandHandler) {
    case IR: {
      omote_log_v("  generic IR, payloads %s\r\n", convertStringListToString(commandData.commandPayloads).c_str());

      // we received a comma separated list of strings
      // the first string is the IR protocol, the second is the payload to be sent
      std::list<std::string>::iterator it = commandData.commandPayloads.begin();
      // get protocol and erase first element in list
      std::string protocol = *it;
      it = commandData.commandPayloads.erase(it);
      omote_log_v("  protocol %s, payload %s\r\n", protocol.c_str(), convertStringListToString(commandData.commandPayloads).c_str());
      
      sendIRcode(std::stoi(protocol), commandData.commandPayloads, additionalPayload);
      break;
    }

    #if (ENABLE_WIFI_AND_MQTT == 1)
    case MQTT: {
      auto current = commandData.commandPayloads.begin();
      std::string topic = *current;
      std::string payload;
      if (additionalPayload == "") {
        current = std::next(current, 1);
        payload = *current;
      } else {
        payload = additionalPayload;
      }
      omote_log_d("execute: will send MQTT, topic '%s', payload '%s'\r\n", topic.c_str(), payload.c_str());
      publishMQTTMessage(topic.c_str(), payload.c_str());
      break;
    }
    #endif

    #if (ENABLE_KEYBOARD_BLE == 1)
    case BLE_KEYBOARD: {
      omote_log_d("execute: will send BLE keyboard command '%u', payload '%s', additionalPayload '%s'\r\n", command, convertStringListToString(commandData.commandPayloads).c_str(), additionalPayload.c_str());
      keyboard_ble_executeCommand(command, commandData.commandPayloads, additionalPayload);
      break;
    }
    #endif

    case SCENE: {
      // let the sceneHandler do the scene stuff
      omote_log_d("execute: will send scene command to the sceneHandler\r\n");
      handleScene(command, commandData, additionalPayload);
      break;
    }
    
    case GUI: {
      // let the sceneHandler find and show the gui
      omote_log_d("execute: will send gui command to the sceneHandler\r\n");
      handleGUI(command, commandData, additionalPayload);
      break;
    }
    
    case SPECIAL: {
      if (command == MY_SPECIAL_COMMAND) {
        // do your special command here
        omote_log_d("execute: could execute a special command here, if you define one\r\n");

      }
      break;
    }

#if (ENABLE_HUB_COMMUNICATION > 0)
    case HUB: {
      omote_log_w("HUB commands should be handled using the CommandExecutionParams struct\r\n");
      break;
    }
#endif
  }
}

void executeCommandWithData(const CommandExecutionParams& params, commandData commandData) {
#if (ENABLE_HUB_COMMUNICATION > 0)
  if (commandData.commandHandler != HUB) {
    // For non-HUB commands, pass through to the original function
    omote_log_d("command: will execute command '%u'%s%s\r\n", 
               params.commandId, 
               params.additionalPayload.empty() ? "" : " with additionalPayload '",
               params.additionalPayload.empty() ? "" : (params.additionalPayload + "'").c_str());
    
    executeCommandWithData(params.commandId, commandData, params.additionalPayload);
    return;
  }

  auto current = commandData.commandPayloads.begin();
  
  // Extract device and command
  std::string deviceName = *current;
  current = std::next(current, 1);
  
  std::string commandName = *current;
  current = std::next(current, 1);
  
  // Create JSON payload
  json payload;
  payload["device"] = deviceName;
  payload["command"] = commandName;
  payload["type"] = params.commandType;
  
  // Create a data array if we have any additional data
  json dataArray = json::array();
  
  // Add all remaining items from commandPayloads to the data array
  while (current != commandData.commandPayloads.end()) {
    dataArray.push_back(*current);
    current = std::next(current, 1);
  }
  
  // If additionalPayload is provided, add it to the data array
  if (!params.additionalPayload.empty()) {
    dataArray.push_back(params.additionalPayload);
  }
  
  // Only add the data array if it has any content
  if (!dataArray.empty()) {
    payload["data"] = dataArray;
  }
  
  omote_log_d("execute: will send hub message for device '%s', command '%s'\r\n", 
              deviceName.c_str(), commandName.c_str());
  
  // Send using the hub manager
  HubManager::getInstance().sendMessage(payload);
#endif
}

void executeCommand(uint16_t command, std::string additionalPayload) {
  try {
    if (commands.count(command) > 0) {
      omote_log_d("command: will execute command '%u' with additionalPayload '%s'\r\n", 
                 command, additionalPayload.c_str());
      executeCommandWithData(command, commands.at(command), additionalPayload);
    } else {
      omote_log_w("command: command '%u' not found\r\n", command);
    }
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("executeCommand: internal error, command not registered\r\n");
  }
}

void executeCommand(const CommandExecutionParams& params) {
  try {
    // Early return if command not found
    if (commands.count(params.commandId) == 0) {
      omote_log_w("command: command '%u' not found\r\n", params.commandId);
      return;
    }
    
    commandData cmdData = commands.at(params.commandId);
    executeCommandWithData(params, cmdData);
  }
  catch (const std::out_of_range& oor) {
    omote_log_e("executeCommand: internal error, command not registered\r\n");
  }
}

void receiveNewIRmessage_cb(std::string message) {
  showNewIRmessage(message);
}

#if (ENABLE_KEYBOARD_BLE == 1)
// used as callback from hardware
void receiveBLEmessage_cb(std::string message) {
  addBLEmessage(message);
}

#endif
#if (ENABLE_WIFI_AND_MQTT == 1)
void receiveWiFiConnected_cb(bool connected) {
  // show status in header
  showWiFiConnected(connected);

  if (connected) {
    // Here you could add sending a MQTT message. This message could be recognized by your home automation software.
    // When receiving this message, your home automation software could send the states of the smart home devices known to OMOTE.
    // With that, OMOTE could show on startup the correct status of the smart home devices.
    //
    // Remark: in your home automation software, maybe add a short delay (e.g. 100-200 ms) between receiving this message and sending out the status of the smart home devices.
    // WiFi connection could be already available, but MQTT connection could be not completely ready. Just try what works for you.

    // executeCommand(TRIGGER_UPDATE_OF_OMOTE_SMART_HOME_DEVICES);

  }
}
void receiveMQTTmessage_cb(std::string topic, std::string payload) {
  showMQTTmessage(topic, payload);
}
#endif

#if (ENABLE_HUB_COMMUNICATION == 1)
void receiveEspNowMessage_cb(json payload) {
  // Extract device and command from the payload
  std::string device, command, jsonStr;
  
  if (payload.contains("device") && payload.contains("command")) {
    device = payload["device"];
    command = payload["command"];
  
    // Serialize the payload to a string
    std::string jsonStr = payload.dump();
    
    // TODO: Process the command based on device and command
  }
}
#endif
