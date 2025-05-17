// gui_deviceDetails.cpp
#include "gui_device_details.h"
#include "applicationInternal/hardware/arduinoLayer.h"
#include "applicationInternal/omote_log.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/devices/deviceRegistry.h"

static lv_obj_t* scr;

static void cmd_btn_cb(lv_event_t* e)
{
    uint16_t command = (uint16_t)((size_t)lv_event_get_user_data(e));
    executeCommand(command);
    lv_scr_load(scr);
}

void show_device_details(Device& entry)
{
    /* First call builds the screen; subsequent calls just refresh list
       so its fast when you go back/forth between devices. */
    if(!scr) {
        scr = lv_obj_create(nullptr);
        lv_obj_set_size(scr, LV_PCT(100), LV_PCT(100));
    }
    lv_obj_clean(scr);                                // clear old list

    /* Title */
    lv_obj_t* lbl = lv_label_create(scr);
    lv_label_set_text_fmt(lbl, "%s", entry.displayName());
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 6);

    /* List of command buttons */
    lv_obj_t* cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(85));
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 6, 0);
    
    bool any = false;
    for(const auto& cmd : entry.commands) {
        any = true;
        lv_obj_t* b = lv_btn_create(cont);
        lv_obj_set_width(b, LV_PCT(95));
        lv_obj_add_event_cb(b, cmd_btn_cb, LV_EVENT_CLICKED, (void*)cmd.ID);

        lv_label_set_text(lv_label_create(b), cmd.displayName());
        omote_log_i("Button created: %s\n", cmd.displayName());

    }
    if(!any) {
        lv_label_set_text(lv_label_create(cont), "No commands registered");
    }
    lv_scr_load(scr);
}
