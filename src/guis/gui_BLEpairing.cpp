#include <mutex>
#include <sstream>
#include <lvgl.h>
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/scenes/sceneRegistry.h"
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/keys.h"
#include "applicationInternal/omote_log.h"
#include "guis/gui_BLEpairing.h"

#if (ENABLE_KEYBOARD_BLE == 1)
uint16_t GUI_BLEPAIRING_ACTIVATE;

std::map<char, repeatModes> key_repeatModes_blepairing = {};
std::map<char, uint16_t> key_commands_short_blepairing = {};
std::map<char, uint16_t> key_commands_long_blepairing = {};

lv_obj_t * logText;
lv_obj_t * dropdownPeers;
bool BLEparingTabIsInMemory = false;
// to add text threadsafe
std::mutex mutex_gui_BLEpairing;
bool newMessageAvailable = false;
std::string newMessageText;

void logTextAdd(std::string message) {
  // add text immediately. Only possible when called from the main thread.
  lv_textarea_set_cursor_pos(logText, LV_TEXTAREA_CURSOR_LAST);
  lv_textarea_add_text(logText, message.c_str());
}

void flushBLEMessages() {
  // this is to flush text which was added by another thread via "void addBLEmessage(std::string message)"
  // the function "flushBLEMessages" is called from the main thread
  std::lock_guard<std::mutex> lck(mutex_gui_BLEpairing);
  if (newMessageAvailable) {
    logTextAdd(newMessageText);
    newMessageAvailable = false;
    newMessageText = "";
  }
}

void addBLEmessage(std::string message) {
  // this callback is called from another thread from BLEKeyboard.cpp
  // we cannot add the message directly to the GUI because LVGL is not threadsafe
  if (!BLEparingTabIsInMemory) {return;}
  std::lock_guard<std::mutex> lck(mutex_gui_BLEpairing);
  newMessageAvailable = true;
  newMessageText += message;
  newMessageText += "\n";
}

lv_obj_t * confirmationDialog_disconnectAllClients;
static void confirmationDialog_disconnectAllClients_cb(lv_event_t * e) {
  lv_obj_t * obj = lv_event_get_current_target(e);
  std::string buttonText = lv_msgbox_get_active_btn_text(obj);
  lv_msgbox_close(confirmationDialog_disconnectAllClients);
  if (buttonText == "yes") {
    omote_log_d("BLE pairing: all clients have been disconnected\r\n");
    logTextAdd("all clients have been disconnected\n");
    keyboardBLE_disconnectAllClients();
  } else {
    omote_log_d("BLE pairing: disconnecting all clients has been canceled\r\n");
  }
}

lv_obj_t * confirmationDialog_deleteBonds;
static void confirmationDialog_deleteBonds_cb(lv_event_t * e) {
  lv_obj_t * obj = lv_event_get_current_target(e);
  std::string buttonText = lv_msgbox_get_active_btn_text(obj);
  lv_msgbox_close(confirmationDialog_deleteBonds);
  if (buttonText == "yes") {
    omote_log_d("BLE pairing: all bonded peers have been deleted\r\n");
    logTextAdd("all bonded peers have been deleted\n");
    keyboardBLE_deleteBonds();
  } else {
    omote_log_d("BLE pairing: deleting all bonds has been canceled\r\n");
  }
}

static void BLEpairing_event_cb(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  lv_obj_t* cont = lv_event_get_current_target(e);
  if (target == cont) return; // stop if container was clicked
  
  int user_data = (intptr_t)(target->user_data);

  switch (user_data) {
    case 0: {
      omote_log_d("BLE pairing: advertising started\r\n");
      logTextAdd("advertising started\n");
      keyboardBLE_startAdvertisingForAll();
      break;
    }
    case 1: {
      omote_log_d("BLE pairing: advertising stopped\r\n");
      logTextAdd("advertising stopped\n");
      keyboardBLE_stopAdvertising();
      break;
    }
    case 2: {
      // This event only fires if the dropdown is opened or closed, but not if a value was selected. Use BLEpairing_dropdown_cb()    
      // omote_log_d("BLE pairing: bonded peers dropdown opened or closed\r\n");
      break;
    }
    case 3: {
      omote_log_d("BLE pairing: print connected\r\n");
      keyboardBLE_printConnectedClients();
      break;
    }
    case 4: {
      static const char * btns[] = {"yes", "no", ""};
      confirmationDialog_disconnectAllClients = lv_msgbox_create(NULL, "Confirmation", "Really disconnect client?", btns, false);
      lv_obj_center(confirmationDialog_disconnectAllClients);
      lv_obj_add_event_cb(confirmationDialog_disconnectAllClients, confirmationDialog_disconnectAllClients_cb, LV_EVENT_VALUE_CHANGED, NULL);
      break;
    }
    case 5: {
      omote_log_d("BLE pairing: print bonds\r\n");
      keyboardBLE_printBonds();
      break;
    }
    case 6: {
      static const char * btns[] = {"yes", "no", ""};
      confirmationDialog_deleteBonds = lv_msgbox_create(NULL, "Confirmation", "Really delete all bonds?", btns, false);
      lv_obj_center(confirmationDialog_deleteBonds);
      lv_obj_add_event_cb(confirmationDialog_deleteBonds, confirmationDialog_deleteBonds_cb, LV_EVENT_VALUE_CHANGED, NULL);
      break;
    }
  }

}

static void BLEpairing_dropdown_cb(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  uint16_t selected_index = lv_dropdown_get_selected(target);
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
    // omote_log_d("BLE pairing: bonded peers dropdown selected index %d\r\n", selected_index);
    uint8_t bufferSize = 20;
    char buffer[bufferSize];
    lv_dropdown_get_selected_str(target, buffer, bufferSize);
    // omote_log_d("BLE pairing: bonded peers dropdown selected value %s\r\n", buffer);
    std::string address = buffer;
    if (address != "") {
      omote_log_d("BLE pairing: bonded peers dropdown, will direct advertise to address %s\r\n", address.c_str());
      keyboardBLE_startAdvertisingDirected(address, false);
    } else {
      omote_log_d("BLE pairing: bonded peers dropdown, no address was selected\r\n");
    }

  } else if (lv_event_get_code(e) == LV_EVENT_CANCEL) {
    omote_log_d("BLE pairing: bonded peers dropdown closed\r\n");

  } else if (lv_event_get_code(e) == LV_EVENT_READY) {
    omote_log_d("BLE pairing: bonded peers dropdown opened\r\n");
    std::string bonds = keyboardBLE_getBonds();
    std::string options;
    // the bonds are a comma separated list, which we have to break into single addresses
    std::stringstream ss(bonds);
    while(ss.good())  {
      std::string address;
      std::getline(ss, address, ',');
      if (options == ""){
        options += address;
      } else {
        options += "\n";
        options += address;
      }
    }
    lv_dropdown_set_options(dropdownPeers, options.c_str());
  }
}

void create_button(lv_obj_t* tab, lv_coord_t width, lv_coord_t height, lv_coord_t x_ofs, lv_coord_t y_ofs, std::string text, uint8_t user_data) {
  lv_obj_t* obj;
  lv_obj_t* buttonLabel;

  obj = lv_btn_create(tab);
  lv_obj_align(obj, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
  lv_obj_set_size(obj, width, height);
  lv_obj_set_style_bg_color(obj, color_primary, LV_PART_MAIN);
  lv_obj_set_style_radius(obj, 14, LV_PART_MAIN);
  lv_obj_set_style_shadow_color(obj, lv_color_hex(0x404040), LV_PART_MAIN);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);        // Clicking a button causes an event in its container
  lv_obj_set_user_data(obj,(void *)(intptr_t)user_data); // Add user data so we can identify which button caused the container event
  buttonLabel = lv_label_create(obj);
  lv_label_set_text(buttonLabel, text.c_str());
  lv_obj_set_style_text_font(buttonLabel, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_center(buttonLabel);
}

void create_dropdownPeers(lv_obj_t* tab, lv_coord_t width, lv_coord_t height, lv_coord_t x_ofs, lv_coord_t y_ofs, uint8_t user_data) {
  dropdownPeers = lv_dropdown_create(tab);
  lv_obj_align(dropdownPeers, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
  lv_obj_set_size(dropdownPeers, width, height);
  lv_obj_set_style_pad_top(dropdownPeers, 5, LV_PART_MAIN);
  // font size of dropdown when it is closed
  lv_obj_set_style_text_font(dropdownPeers, &lv_font_montserrat_10, LV_PART_MAIN);
  // font size of the list of the dropdown, when it is opened
  lv_obj_set_style_text_font(lv_dropdown_get_list(dropdownPeers), &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_add_event_cb(dropdownPeers, BLEpairing_dropdown_cb, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)user_data);
  lv_obj_add_event_cb(dropdownPeers, BLEpairing_dropdown_cb, LV_EVENT_CANCEL,        (void *)(intptr_t)user_data);
  lv_obj_add_event_cb(dropdownPeers, BLEpairing_dropdown_cb, LV_EVENT_READY,         (void *)(intptr_t)user_data);
  // This event only fires if the dropdown is opened or closed, but not if a value was selected. Use BLEpairing_dropdown_cb()    
  // lv_obj_add_flag(dropdownPeers, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a dropdown causes a event in its container
  // lv_obj_set_user_data(dropdownPeers,(void *)(intptr_t)user_data); // Add user data so we can identify which button caused the container event
  lv_dropdown_set_options(dropdownPeers, "");
}

void create_textarea(lv_obj_t* tab, lv_coord_t width, lv_coord_t height, lv_coord_t x_ofs, lv_coord_t y_ofs) {
  logText = lv_textarea_create(tab);
  lv_obj_align(logText, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
  lv_obj_set_size(logText, width, height);
  lv_obj_set_style_pad_all(logText, 0, LV_PART_MAIN);
  lv_obj_set_style_text_font(logText, &lv_font_montserrat_10, LV_PART_MAIN);

}

void create_tab_content_blepairing(lv_obj_t* tab) {
  create_button       (tab, 100,  24,   0,   0, "start advert.",  0);
  create_button       (tab, 100,  24, 112,   0, "stop advert.",   1);
  
  create_dropdownPeers(tab, 212,  24,   0,  30,                   2);

  create_button       (tab, 100,  24,   0,  60, "print connected", 3);
  create_button       (tab, 100,  24, 112,  60, "disconnect",     4);

  create_button       (tab, 100,  24,   0,  90, "print bonds",    5);
  create_button       (tab, 100,  24, 112,  90, "delete bonds",   6);

  create_textarea     (tab, 212, 124,   0, 120                     );

  // Create a shared event for all buttons inside container
  lv_obj_add_event_cb(tab, BLEpairing_event_cb, LV_EVENT_CLICKED, NULL);

  BLEparingTabIsInMemory = true;
}

void notify_tab_before_delete_blepairing(void) {
  // remember to set all pointers to lvgl objects to NULL if they might be accessed from outside.
  // They must check if object is NULL and must not use it if so
  BLEparingTabIsInMemory = false;
}

void gui_setKeys_blepairing() {
}

void register_gui_blepairing(void){

  register_gui(
    std::string(tabName_blepairing),
    & create_tab_content_blepairing,
    & notify_tab_before_delete_blepairing,
    & gui_setKeys_blepairing,
    & key_repeatModes_blepairing,
    & key_commands_short_blepairing,
    & key_commands_long_blepairing
    );

  register_command(&GUI_BLEPAIRING_ACTIVATE, makeCommandData(GUI, {std::to_string(MAIN_GUI_LIST), std::string(tabName_blepairing)}));
}
#endif
