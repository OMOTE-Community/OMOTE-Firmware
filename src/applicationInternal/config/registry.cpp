#include "registry.h"
#include "parser.h"
#include <map>
#include <ArduinoJson.h>
#include <applicationInternal/scenes/sceneRegistry.h>
#include <applicationInternal/commandHandler.h>
#include <applicationInternal/omote_log.h>

config::DynamicScene allOff("Off");

namespace {
    std::vector<config::Device*> g_devices;
    std::map<std::string, config::Scene*> g_scenes;
}

using namespace config;

Scene* config::getScene(const std::string& name) {
    auto iter = g_scenes.find(name);
    if(iter != g_scenes.end()) {
        return iter->second;
    }
    else {
        return NULL;
    }
}

void config::registerDevice(config::Device* dev) {
    g_devices.push_back(dev);
}

void sceneStartSequenceDispatch() {
    Scene* scene = g_scenes[get_scene_being_handled()];
    scene->start();
}

void sceneEndSequenceDispatch() {
    Scene* scene = g_scenes[get_scene_being_handled()];
    scene->end.run();
}

void sceneSetKeysDispatch() {
    //redundant
}

void config::registerScene(config::Scene* scene, t_gui_list* scene_guis) {
    auto iter = g_scenes.find(scene->displayName());
    if(iter != g_scenes.end()) {
        delete iter->second;
    }
    g_scenes[scene->displayName()] = scene;
    CommandID_t commandID;
    register_command(&commandID, makeCommandData(SCENE, {scene->displayName()}));
    scene->command.setID(commandID);
    
    register_command(&commandID, makeCommandData(SCENE, {scene->displayName(), "FORCE"}));
    scene->commandForce.setID(commandID);
    
    register_scene(scene->displayName(),
                   &sceneSetKeysDispatch,
                   &sceneStartSequenceDispatch,
                   &sceneEndSequenceDispatch,
                   &scene->keys.keys_repeat_modes,
                   &scene->keys.keys_short,
                   &scene->keys.keys_long,
                   scene_guis,
                   scene->command.getID());
                   
    
}

std::vector<config::Device*>& config::getDevices() {
    return g_devices;
}

Device* config::getDevice(const std::string& id) {
    omote_log_i("Get device: %s", id.c_str());
    for(auto& dev : g_devices) {
        omote_log_i("Comparing: %s", dev->ID());
        if(dev->ID() == id) {
            return dev;
        }
    }
    return NULL;
}

void config::init() {
    parseConfig();
    registerScene(&allOff, NULL);
    std::string lastScene = gui_memoryOptimizer_getActiveSceneName();
    if(lastScene != "") {
        omote_log_i("Setting current scene to: %s", lastScene.c_str());
        if(g_scenes.find(lastScene) == g_scenes.end()) {
            omote_log_e("Scene: %s not found", lastScene.c_str());
        }
        else {
            Scene::setCurrent(g_scenes[lastScene]);
        }
    }
}
