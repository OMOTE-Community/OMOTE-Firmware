Import("env")
import json, pathlib, os


build_flags = env.ParseFlags(env['BUILD_FLAGS'])
cppflags = build_flags.get("CPPDEFINES", [])
deps = env.GetProjectOption("lib_deps") or []

if "YAML_CONFIG" in cppflags:
    try:
        import yaml
    except ImportError:
        print("Installing missing dependecy: PyYAML")
        env.Execute("$PYTHONEXE -m pip install PyYAML")
        import yaml

    deps.append("bblanchon/ArduinoJson@^7.4.1")
    
    #generate embedded_config.h representing the contents of config.yml
    cfg = yaml.safe_load(pathlib.Path("config.yml").read_text())
    header = (
        "#pragma once\n"
        "#include <ArduinoJson.h>\n"
        "static const char RAW_JSON[] = R\"json(" +
        json.dumps(cfg) + ")json\";\n"
        "inline void loadConfig(JsonDocument& doc) "
        "{ deserializeJson(doc, RAW_JSON); }\n"
    )

    out = os.path.join(env['PROJECT_SRC_DIR'], "embedded_config.h")
    pathlib.Path(out).write_text(header)


if "REMOTE_DEBUG" in cppflags:
    deps.append("karol-brejna-i/RemoteDebug")
    

platform = env.PioPlatform()
env_section = "env:" + env["PIOENV"]
platform.config.set(env_section, "lib_deps", deps)




