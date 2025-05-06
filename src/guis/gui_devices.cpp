#include "gui_devices.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/devices/deviceRegistry.h"

const char* const tabName_devices = "Devices";

static lv_obj_t* list;

static void build(lv_obj_t* parent)
{
    list = lv_list_create(parent);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));

    for (auto& name : deviceRegistry::getDevices()) {
        lv_list_add_btn(list, LV_SYMBOL_RIGHT, name.c_str());
    }
}

static void destroy(void)
{
}

void register_gui_devices()
{
    register_gui(std::string(tabName_devices), & build, & destroy);    
}
