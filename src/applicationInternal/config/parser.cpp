#include <ArduinoJson.h>
#include <embedded_config.h>
#include <applicationInternal/hardware/hardwarePresenter.h>
#include <applicationInternal/config/registry.h>
#include <applicationInternal/commandHandler.h>
#include <applicationInternal/omote_log.h>
#include <guis/gui_devices.h>
#include <guis/gui_scene.h>

using namespace config;

JsonDocument configuration;

typedef IRprotocols_new IRProtocolType;

namespace {
    t_gui_list scene_guis = {tabName_scene, tabName_devices};
}

std::map<IRProtocolType, uint16_t> BITS = {
    {IR_PROTOCOL_NEC, kNECBits}
};

static IRProtocolType toProtoType(const char* proto)
{
    if (!proto)
        return IR_PROTOCOL_UNKNOWN;

    if (strcmp(proto, "SIRC") == 0)  return IR_PROTOCOL_SONY;
    if (strcmp(proto, "NEC")  == 0)  return IR_PROTOCOL_NEC;
    if (strcmp(proto, "RC5")  == 0)  return IR_PROTOCOL_RC5;
    if (strcmp(proto, "DENON")  == 0)  return IR_PROTOCOL_DENON;
    if (strcmp(proto, "KASEIKYO")  == 0)  return IR_PROTOCOL_PANASONIC;
    return IR_PROTOCOL_UNKNOWN;
}

void parseDevice(JsonPair device)
{

    Device* dev = new Device(device);

    const char* protoStr = device.value()["protocol"];

    IRProtocolType protoType = toProtoType(protoStr);

    if (protoType == IR_PROTOCOL_UNKNOWN) {
        omote_log_e("unsupported protocol %s (skip %s)",
                      protoStr, device.key());
        return;
    }

    uint16_t defaultBits = 0;
    if(BITS.find(protoType) != BITS.end()) {
        defaultBits = BITS[protoType];
    }

    for (JsonObject obj : device.value()["commands"].as<JsonArray>()) {

        const char* name     = obj["name"];
        const char* dataStr = obj["data"];
        uint16_t     nbits    = obj["nbits"] | 0;
        uint8_t     repeats    = obj["repeats"] | 3;        
        
        if (!name || !protoStr || !dataStr) {
            omote_log_e("malformed entry, skipped");
            continue;
        }

        if(!nbits) {
            if(defaultBits == 0) {
                omote_log_e("bits needs to be defined for %s", protoStr);
                continue;
            }
            nbits = defaultBits;
        }
        
        std::string data(dataStr);
        data = data + ":" + std::to_string(nbits) + ":" + std::to_string(repeats);

        commandData cmd = makeCommandData(IR, {std::to_string(protoType), data});

        uint16_t idRef;
        register_command(&idRef, cmd);

        dev->addCommand(obj, idRef);

        omote_log_i("registered %-12s  %s / %s (%u bit) -> %u",
                    name, protoStr, dataStr, nbits, idRef);
    }        
    registerDevice(dev);
}

const DeviceCommand* parseCommandReference(JsonObject cmdRef) {
    Device* dev = getDevice((const char*)cmdRef["device"]);
    if(dev == NULL) {
        omote_log_w("Unknown device reference: %s", (const char*)cmdRef["device"]);
        return NULL;
    }
    const DeviceCommand* cmd = dev->getCommand(cmdRef["command"].as<const char*>());
    if(cmd == NULL) {
        omote_log_w("Unknown command reference: %s/%s", (const char*)cmdRef["device"], (const char*)cmdRef["command"]);
        return NULL;
    }
    omote_log_i("Parsed command reference: %s/%s\n", (const char*)cmdRef["device"], (const char*)cmdRef["command"]);
    return cmd;
}

void parseSequence(JsonArray sequence, commands_t& out) {
    for(JsonObject cmd : sequence) {
        int delay = cmd["delay"];
        if(delay) {
            out.push_back(new DelayCommand(delay));
        }
        else {
            const DeviceCommand* command = parseCommandReference(cmd);
            if(command == NULL) {
                continue;
            }
            out.push_back(command);
        }
    }
}



void parseScene(JsonPair def) {
    ConfigScene* scene = new ConfigScene(def);
    const char* keys_default = def.value()["keys_default"];
    if(keys_default) {
        Device* dev = getDevice(keys_default);
        if(dev == NULL) {
            omote_log_w("Unknown device reference: %s in %s", keys_default, scene->displayName());
        }
        else {
            scene->keys = dev->defaultKeys;
        }
    }

    JsonObject sceneDef = def.value().as<JsonObject>();
    JsonObject keys_short = sceneDef["keys_short"];
    if(keys_short) {
        for(JsonPair kv: keys_short) {
            const DeviceCommand* cmd = parseCommandReference(kv.value());
            if(cmd == NULL) {
                continue;
            }
            scene->keys.keys_short[KeyMap::getKeyCode(kv.key().c_str())] = cmd->ID;
        }
    }

    JsonObject keys_long = sceneDef["keys_long"];
    if(keys_long) {
        for(JsonPair kv: keys_long) {
            const DeviceCommand* cmd = parseCommandReference(kv.value());
            if(cmd == NULL) {
                continue;
            }
            scene->keys.keys_long[KeyMap::getKeyCode(kv.key().c_str())] = cmd->ID;
        }
    }

    JsonArray seq = sceneDef["start"];
    if(seq) {
        parseSequence(seq, scene->startSeq.commands);
    }

    seq = sceneDef["end"];
    if(seq) {
        parseSequence(seq, scene->end.commands);
    }    

    JsonArray shortcuts = sceneDef["shortcuts"];
    if(shortcuts) {
        parseSequence(shortcuts, scene->shortcuts);
    }
    
    registerScene(scene, &scene_guis);
}

void parseConfig() {
    omote_log_i("Loading configuration");
    
    loadConfig(configuration);

    JsonObject root = configuration.as<JsonObject>();
    
    JsonObject devices = root["devices"].as<JsonObject>();
    for(JsonPair kv: devices) {
        parseDevice(kv);
    }

    JsonObject scenes = root["scenes"];
    for(JsonPair kv: scenes) {
        parseScene(kv);
    }
}


