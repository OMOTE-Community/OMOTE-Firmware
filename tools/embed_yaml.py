Import("env")
import yaml, json, pathlib, os

cfg = yaml.safe_load(pathlib.Path("config.yml").read_text())
header = (
    "#pragma once\n"
    "#include <ArduinoJson.h>\n"
    "static const char RAW_JSON[] = R\"json(" +
    json.dumps(cfg) + ")json\";\n"
    "inline void loadConfig(JsonDocument& doc) "
    "{ deserializeJson(doc, RAW_JSON); }\n"
)

# header = (
#     "#pragma once\n"
#     "#include <ArduinoJson.h>\n"
#     "static const char RAW_JSON[] = F(R\"" +
#     json.dumps(cfg) + "\");\n"
#     "inline void loadConfig(JsonDocument& doc) "
#     "{ deserializeJson(doc, RAW_JSON); }\n"
# )

out = os.path.join(env['PROJECT_SRC_DIR'], "embedded_config.h")
pathlib.Path(out).write_text(header)
