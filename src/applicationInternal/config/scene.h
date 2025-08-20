#pragma once
#include <unordered_set>
#include <string>
#include "command.h"
#include "device.h"

namespace config {

    class Scene;
    
    class SceneCommand : public Command
    {
    protected:
        const Scene* scene;
        static const std::string catName;
    public:
        CommandID_t commandID;
        SceneCommand(const Scene* scene_) : scene(scene_), Command(NULL)
        {}
        virtual void execute() const;
        virtual const char* displayName() const;
        virtual const char* category() const {
            return catName.c_str();
        }
    };
    
    class Scene {
    public:
        KeyMap keys;
        SceneCommand command;
        SceneCommand commandForce;
        CommandSequence startSeq;
        CommandSequence end;
        commands_t shortcuts;
        void start();
        Scene() : command(this), commandForce(this)
        {}
        static const Scene* getCurrent() {
            return current;
        }
        virtual const char* displayName() const = 0;
    protected:
        static Scene* current;
        
    };

    class ConfigScene : public Scene
    {   
    public:
        JsonPair ref;
        ConfigScene(JsonPair ref) : ref(ref), val(ref.value())
        {}
        virtual const char* displayName() const {
            return val["display_name"];
        }
    protected:        
        JsonObjectConst val;        
    };

    class DynamicScene : public Scene
    {
    protected:
        std::string name;
    public:
        DynamicScene(std::string name_) : name(name_)
        {}
        virtual const char* displayName() const {
            return name.c_str();
        }
    };
}
