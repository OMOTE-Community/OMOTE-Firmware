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

void registerRemote(JsonPair remote)
{

    Device* dev = registerDevice(remote);

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

void registerScene(JsonPair def) {
    Scene* scene = addScene(def);
    const char* commands_default = def.value()["commands_default"];
    if(commands_default) {
        Device* dev = getDevice(commands_default);
        if(dev == NULL) {
            omote_log_w("Unkown remote reference: %s in %s", commands_default, scene->displayName());
        }
        else {
            scene->keys = dev->defaultKeys;
        }
    }
        
        

}

void parseConfig() {

    loadConfig(configuration);

    JsonObject root = configuration.as<JsonObject>();
    
    JsonObject remotes = root["remotes"].as<JsonObject>();
    for(JsonPair kv: remotes) {
        registerRemote(kv);
    }        

}
//Serial.println(cfg["mqtt"]["host"].as<const char*>());
