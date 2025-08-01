#include <ArduinoJson.h>
#include <embedded_config.h>
#include <applicationInternal/hardware/hardwarePresenter.h>
#include <applicationInternal/config/registry.h>
#include <applicationInternal/commandHandler.h>
#include <applicationInternal/omote_log.h>

using namespace config;

JsonDocument configuration;

typedef IRprotocols_new IRProtocolType;

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

void parseRemote(JsonPair remote)
{

    Device* dev = new Device(remote);

    const char* protoStr = remote.value()["protocol"];

    IRProtocolType protoType = toProtoType(protoStr);

    if (protoType == IR_PROTOCOL_UNKNOWN) {
        Serial.printf("[IR-CFG] unsupported protocol %s (skip %s)\n",
                      protoStr, remote.key());
        return;
    }

    for (JsonObject obj : remote.value()["commands"].as<JsonArray>()) {

        const char* name     = obj["name"];
        const char* dataStr = obj["data"];
        uint8_t     nbits    = obj["nbits"] | 0;
        uint8_t     repeats    = obj["repeats"] | 3;        
        
        if (!name || !protoStr || !dataStr) {
            Serial.println("malformed entry, skipped");
            continue;
        }
        
        std::string data(dataStr);
        data = data + ":" + std::to_string(nbits) + ":" + std::to_string(repeats);

        commandData cmd = makeCommandData(IR, {std::to_string(protoType), data});

        uint16_t idRef;
        register_command(&idRef, cmd);

        dev->addCommand(obj, idRef);

        Serial.printf("[IR-CFG] registered %-12s  %s / %s (%u bit)\n",
                      name, protoStr, dataStr, nbits);
    }        

}

const RemoteCommand* parseCommandReference(JsonObject cmdRef) {
    Device* dev = getDevice(cmdRef["remote"]);
    if(dev == NULL) {
        omote_log_w("Unknown remote reference: %s", (const char*)cmdRef["remote"]);
        return NULL;
    }
    const RemoteCommand* cmd = dev->getCommand(cmdRef["command"]);
    if(cmd == NULL) {
        omote_log_w("Unknown command reference: %s/%s", (const char*)cmdRef["remote"], (const char*)cmdRef["command"]);
        return NULL;
    }
    omote_log_i("Parsed command reference: %s/%s\n", (const char*)cmdRef["remote"], (const char*)cmdRef["command"]);
    return cmd;
}

void parseSequence(JsonArray sequence, CommandSequence& out) {
    for(JsonObject cmd : sequence) {
        int delay = cmd["delay"];
        if(delay) {
            out.commands.push_back(new DelayCommand(delay));
        }
        else {
            const RemoteCommand* command = parseCommandReference(cmd);
            if(command == NULL) {
                continue;
            }
            out.commands.push_back(command);
        }
    }
}



void parseScene(JsonPair def) {
    Scene* scene = new Scene(def);
    const char* commands_default = def.value()["commands_default"];
    if(commands_default) {
        Device* dev = getDevice(commands_default);
        if(dev == NULL) {
            omote_log_w("Unknown remote reference: %s in %s", commands_default, scene->displayName());
        }
        else {
            scene->keys = dev->defaultKeys;
        }
    }

    JsonObject sceneDef = def.value().as<JsonObject>();
    JsonObject commands_short = sceneDef["commands_short"];
    if(commands_short) {
        for(JsonPair kv: commands_short) {
            const RemoteCommand* cmd = parseCommandReference(kv.value());
            if(cmd == NULL) {
                continue;
            }
            scene->keys.keys_short[KeyMap::getKeyCode(kv.key().c_str())] = cmd->ID;
        }
    }
    JsonArray seq = sceneDef["start"];
    if(seq) {
        parseSequence(seq, scene->start);
    }

    seq = sceneDef["end"];
    if(seq) {
        parseSequence(seq, scene->end);
    }    

    registerScene(scene);
}

void parseConfig() {

    loadConfig(configuration);

    JsonObject root = configuration.as<JsonObject>();
    
    JsonObject remotes = root["remotes"].as<JsonObject>();
    for(JsonPair kv: remotes) {
        parseRemote(kv);
    }

    JsonObject scenes = root["scenes"];
    for(JsonPair kv: scenes) {
        parseScene(kv);
    }

}

