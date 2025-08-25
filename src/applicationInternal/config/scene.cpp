#include "scene.h"
#include <applicationInternal/omote_log.h>

using namespace config;

config::Scene* config::Scene::current = nullptr;
const std::string config::SceneCommand::catName = "scene";

void SceneCommand::execute() const {
    omote_log_i("Executing scene command: %s", displayName());
    executeCommand(commandID);
}
    
const char* SceneCommand::displayName() const {
    return scene->displayName();
}

void Scene::start() {
    const char* currentName = "";
    if (current != NULL) {
        currentName = current->displayName();
    }
    omote_log_i("Starting scene: %s (current:%s)", displayName(), currentName);
    CommandSequence seq = startSeq;
    if(current != NULL) {
        //establish which devices have been on in the previous
        //scene so that we don't send the commands again
        //(which would be problematic in case of power toggle
        //commands)
        std::vector<const Device*> devices_on_prev;
        current->startSeq.findDevicesByCategory(devices_on_prev, "on");
        for(auto dev : devices_on_prev) {
            seq.dropMatching(dev, "on");
        }

        //identify devices that have been on in the previous
        //scene, but are not used in the current - we should
        //turn off such devices (if possible)
        std::vector<const Device*> devices_on;

        startSeq.findDevicesByCategory(devices_on, "on");

        std::unordered_set<const Device*> devices_on_set(devices_on.begin(), devices_on.end());
        for(auto dev : devices_on_prev) {
            if(devices_on_set.find(dev) == devices_on_set.end()) {
                omote_log_i("Device %s no longer needed. Turning off.", dev->ID());
                const DeviceCommand* cmd = dev->getCommandByCategory("off");
                if(cmd != NULL) {
                    cmd->execute();
                }
            }
        }
    }
    current = this;
    omote_log_d("Commands to run: %d", seq.commands.size());
    seq.run();
}


