#include <ArduinoJson.h>
#include <embedded_config.h>
#include <applicationInternal/hardware/hardwarePresenter.h>
#include <applicationInternal/devices/deviceRegistry.h>
#include <applicationInternal/commandHandler.h>


JsonDocument config;

typedef IRprotocols_new IRProtocolType;

static IRProtocolType toProtoType(const char* proto)
{
    if (!proto)
        return IR_PROTOCOL_UNKNOWN;

    if (strcmp(proto, "SIRC") == 0)  return IR_PROTOCOL_SONY;
    if (strcmp(proto, "NEC")  == 0)  return IR_PROTOCOL_NEC;
    if (strcmp(proto, "RC5")  == 0)  return IR_PROTOCOL_RC5;
    return IR_PROTOCOL_UNKNOWN;
}

void registerRemote(JsonPair remote)
{

    Device* dev = deviceRegistry::registerDevice(remote);

    for (JsonObject obj : remote.value()["commands"].as<JsonArray>()) {

        const char* name     = obj["name"];
        const char* protoStr = obj["protocol"];
        const char* dataStr = obj["data"];
        uint8_t     nbits    = obj["nbits"] | 0;
        
        if (!name || !protoStr || !dataStr) {
            Serial.println("malformed entry, skipped");
            continue;
        }

        IRProtocolType protoType = toProtoType(protoStr);
         
        if (protoType == IR_PROTOCOL_UNKNOWN) {
            Serial.printf("[IR-CFG] unsupported protocol %s (skip %s)\n",
                          protoStr, name);
            continue;
        }

        std::string data(dataStr);
        data = data + ":" + std::to_string(nbits) + ":3";

        commandData cmd = makeCommandData(IR, {std::to_string(protoType), data});
    
        uint16_t idRef;
        register_command(&idRef, cmd);

        dev->addCommand(obj, idRef);
        
        Serial.printf("[IR-CFG] registered %-12s  %s / %s (%u bit)\n",
                      name, protoStr, dataStr, nbits);
    }        

}

void parseConfig() {

    loadConfig(config);

    JsonObject root = config.as<JsonObject>();
    
    JsonObject remotes = root["remotes"].as<JsonObject>();
    for(JsonPair kv: remotes) {
        registerRemote(kv);
    }        

}
//Serial.println(cfg["mqtt"]["host"].as<const char*>());
