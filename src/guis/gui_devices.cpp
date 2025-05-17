#include "gui_devices.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "guis/gui_device_details.h"
#include "applicationInternal/devices/deviceRegistry.h"

const char* const tabName_devices = "Devices";

static lv_obj_t* list;

static void build(lv_obj_t* parent)
{
    list = lv_list_create(parent);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));

    for (auto& dev : deviceRegistry::getDevices()) {
        lv_obj_t* btn = lv_list_add_btn(list, LV_SYMBOL_RIGHT, dev.displayName());
        lv_obj_add_event_cb(btn, [](lv_event_t* e){
            auto* entry = static_cast<Device*>(lv_event_get_user_data(e));
            show_device_details(*entry);
        }, LV_EVENT_CLICKED, (void*)&dev);
    }
}

static void destroy(void)
{
}

void register_gui_devices()
{
    register_gui(std::string(tabName_devices), & build, & destroy);    
}
