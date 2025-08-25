#include "gui_devices.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/config/registry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"

const char* const tabName_devices = "Devices";


void show_device_details(config::Device& entry);
void show_device_list();

namespace {
    lv_obj_t* root     = nullptr;
    lv_obj_t* header   = nullptr;
    lv_obj_t* content  = nullptr;
    config::KeyMap deviceMap;
}

static void clear()
{
    if(content) {
        lv_obj_clean(content);
    }
    if(header) {
        lv_obj_clean(header);
    }
}

void build_devices_tab(lv_obj_t* parent)
{
    omote_log_d("Buidling devices tab");
    root = parent;
    header = lv_obj_create(root);
    lv_obj_set_size(header, LV_PCT(100), LV_PCT(20));
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_left(header, 4, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

    show_device_list();
}
void show_device_list()
{
    clear();
     
    lv_label_set_text(lv_label_create(header), "Devices");
 
    content = lv_list_create(root);
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_ACTIVE);
    
    
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(80));
    lv_obj_align(content, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    for (auto& dev : config::getDevices()) {
        lv_obj_t* btn = lv_list_add_btn(content, LV_SYMBOL_RIGHT, dev->displayName());
        lv_obj_add_event_cb(btn, [](lv_event_t* e){
            auto* entry = static_cast<config::Device*>(lv_event_get_user_data(e));
            show_device_details(*entry);
        }, LV_EVENT_CLICKED, (void*)dev);
    }
}

void destroy_devices_tab()
{
    //lv_obj_clean(root);
    root = nullptr;
    header = nullptr;
    content = nullptr;
}

void register_gui_devices()
{
    register_gui(std::string(tabName_devices),
                 &build_devices_tab,
                 &destroy_devices_tab,
                 NULL,
                 NULL,
                 &deviceMap.keys_short,
                 &deviceMap.keys_long);
}


void cmd_btn_cb(lv_event_t* e)
{
    config::DeviceCommand* cmd = (config::DeviceCommand*)lv_event_get_user_data(e);
    cmd->execute();
}

void show_device_details(config::Device& entry)
{
    clear();
    
    /* Back button */
    lv_obj_t* back = lv_btn_create(header);
    lv_obj_set_size(back, 35, 25);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_add_event_cb(back, [](lv_event_t* e){
        show_device_list();
    }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* back_label = lv_label_create(back);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);

    /* Title */
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text_fmt(title, "%s", entry.displayName());
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 44, 0);



    // /* Title */    
    // lv_label_set_text_fmt(lv_label_create(header), "%s", entry.displayName());


    deviceMap.swap(entry.defaultKeys);
    /* List of command buttons */
    content = lv_obj_create(root);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(85));
    lv_obj_align(content, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_style_pad_row(cont, 6, 0);
    
    bool any = false;
    for(const auto& cmd : entry.commands) {
        any = true;
        lv_obj_t* b = lv_btn_create(content);
        lv_obj_set_width(b, LV_PCT(95));
        lv_obj_add_event_cb(b, cmd_btn_cb, LV_EVENT_CLICKED, (void*)&cmd);

        lv_label_set_text(lv_label_create(b), cmd.displayName());
        omote_log_i("Button created: %s (%d)", cmd.displayName(), cmd.ID);

    }
    if(!any) {
        lv_label_set_text(lv_label_create(content), "No commands registered");
    }
    //lv_scr_load(scr);
}
