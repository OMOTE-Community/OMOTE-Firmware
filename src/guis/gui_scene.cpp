#include "gui_scene.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/config/registry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"

const char* const tabName_scene = "Scene";
extern config::DynamicScene allOff;

void show_device_details(config::Device& entry);
void show_device_list();

namespace {
    lv_obj_t* root     = nullptr;
    lv_obj_t* footer   = nullptr;
    lv_obj_t* content  = nullptr;
    void cmd_btn_cb(lv_event_t* e)
    {
        config::Command* cmd = (config::Command*)lv_event_get_user_data(e);
        cmd->execute();
    }
    config::KeyMap sceneKeys;
}

static void clear()
{
    if(content) {
        lv_obj_clean(content);
    }
    if(footer) {
        lv_obj_clean(footer);
    }
}



void build_scene_tab(lv_obj_t* parent)
{
    omote_log_d("Buidling scene tab");
    root = parent;
    
    //shortcuts
    content = lv_obj_create(root);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t* header = lv_obj_create(content);
    lv_obj_set_size(header, LV_PCT(100), LV_PCT(20));
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* shortcuts = lv_label_create(header);
    lv_label_set_text(shortcuts, "Shortcuts");    
    lv_obj_align(shortcuts, LV_ALIGN_LEFT_MID, 0, 0);
    
    lv_obj_t* off = lv_btn_create(header);    
    lv_obj_set_style_radius(off, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(off, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(off, cmd_btn_cb, LV_EVENT_CLICKED, (void*)&allOff.command);
    
    lv_obj_t* lbl = lv_label_create(off);
    lv_label_set_text(lbl, "Off");
    lv_obj_center(lbl);
 
    bool any = false;
    const config::Scene* current = config::Scene::getCurrent();

    if(current != nullptr)  {
        sceneKeys.swap(current->keys);        
        for(const auto& cmd : current->shortcuts) {
            any = true;
            lv_obj_t* b = lv_btn_create(content);
            lv_obj_set_width(b, LV_PCT(45));
            lv_obj_add_event_cb(b, cmd_btn_cb, LV_EVENT_CLICKED, (void*)cmd);

            lv_label_set_text(lv_label_create(b), cmd->displayName());
            omote_log_i("Button created: %s (%d)", cmd->displayName(), ((config::RemoteCommand*)cmd)->ID);   
        }
    }
    if(!any) {
        lv_label_set_text(lv_label_create(content), "No shortcuts");
    }

    //footer
    footer = lv_obj_create(root);
    lv_obj_set_size(footer, LV_PCT(100), LV_PCT(20));
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_left(footer, 4, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t* pointer = lv_label_create(footer);
    lv_label_set_text(pointer, "Devices >>");
    lv_obj_align(pointer, LV_ALIGN_RIGHT_MID, 0, 0);

}

void destroy_scene_tab()
{
    //lv_obj_clean(root);
    root = nullptr;
    footer = nullptr;
    content = nullptr;
}

void register_gui_scene()
{
    register_gui(std::string(tabName_scene),
                 &build_scene_tab,
                 &destroy_scene_tab,
                 NULL,
                 NULL,
                 &sceneKeys.keys_short,
                 &sceneKeys.keys_long);
}



