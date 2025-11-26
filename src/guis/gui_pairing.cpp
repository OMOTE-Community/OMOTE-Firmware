#include <lvgl.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/omote_log.h"
#include "applicationInternal/hub/pairingManager.h"
#include "guis/gui_pairing.h"

// State for pairing GUI
static std::string pairing_pin_buffer;
static lv_obj_t* pairing_instruction_label = nullptr;
static lv_obj_t* pairing_pin_display = nullptr;
static lv_obj_t* pairing_submit_btn = nullptr;
static lv_obj_t* pairing_cancel_btn = nullptr;
static lv_obj_t* numpad_container = nullptr;

// Pairing submit button handler
static void pairing_submit_event_cb(lv_event_t* e) {
  if (pairing_pin_buffer.empty()) {
    omote_log_w("Cannot submit empty PIN\r\n");
    return;
  }
  
  Hub::PairingManager::getInstance().submitPin(pairing_pin_buffer.c_str());
  pairing_pin_buffer.clear();
  
  if (pairing_pin_display) {
    lv_label_set_text(pairing_pin_display, "");
  }
}

// Pairing cancel button handler
static void pairing_cancel_event_cb(lv_event_t* e) {
  Hub::PairingManager::getInstance().cancel();
  pairing_pin_buffer.clear();
  
  if (pairing_pin_display) {
    lv_label_set_text(pairing_pin_display, "");
  }
}

// Backspace button handler
static void pairing_backspace_event_cb(lv_event_t* e) {
  if (!pairing_pin_buffer.empty()) {
    pairing_pin_buffer.pop_back();
    
    // Update display with asterisks
    std::string display_pin(pairing_pin_buffer.length(), '*');
    if (pairing_pin_display) {
      lv_label_set_text(pairing_pin_display, display_pin.c_str());
    }
  }
}

// Hex button handler (0-9, A-F)
static void pairing_hex_event_cb(lv_event_t* e) {
  lv_obj_t* target = lv_event_get_target(e);
  lv_obj_t* cont = lv_event_get_current_target(e);
  if (target == cont) return;
  
  int user_data = (intptr_t)(target->user_data);
  
  // Check if we've reached the expected PIN length
  uint32_t expected_length = Hub::PairingManager::getInstance().getExpectedPinLength();
  if (expected_length > 0 && pairing_pin_buffer.length() >= expected_length) {
    omote_log_d("PIN length limit reached\r\n");
    return;
  }
  
  // Map user_data to hex character (0-15 -> 0-9, A-F)
  char hex_char;
  if (user_data < 10) {
    hex_char = '0' + user_data;
  } else {
    hex_char = 'A' + (user_data - 10);
  }
  
  // Append hex character to PIN buffer
  pairing_pin_buffer += hex_char;
  
  // Update display with asterisks (for security, but we store the actual hex)
  std::string display_pin(pairing_pin_buffer.length(), '*');
  if (pairing_pin_display) {
    lv_label_set_text(pairing_pin_display, display_pin.c_str());
  }
  
  omote_log_d("Pairing PIN: %zu characters (hex)\r\n", pairing_pin_buffer.length());
}

void create_tab_content_pairing(lv_obj_t* tab) {
  lv_obj_set_style_pad_all(tab, 0, LV_PART_MAIN);
  
  // Instruction label at the top
  pairing_instruction_label = lv_label_create(tab);
  lv_label_set_long_mode(pairing_instruction_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(pairing_instruction_label, SCR_WIDTH - 40);
  lv_obj_set_style_text_align(pairing_instruction_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_font(pairing_instruction_label, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_align(pairing_instruction_label, LV_ALIGN_TOP_MID, 0, 20);
  
  // Get current message if pairing is active
  if (Hub::PairingManager::getInstance().isActive()) {
    lv_label_set_text(pairing_instruction_label, Hub::PairingManager::getInstance().getMessage());
  } else {
    lv_label_set_text(pairing_instruction_label, "Enter PIN to pair device");
  }
  
  // PIN display (shows asterisks)
  pairing_pin_display = lv_label_create(tab);
  lv_label_set_text(pairing_pin_display, "");
  lv_obj_set_style_text_font(pairing_pin_display, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_align(pairing_pin_display, LV_ALIGN_TOP_MID, 0, 80);
  
  // Configure hex keypad grid (4x4 for 0-9, A-F)
  static lv_coord_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
  static lv_coord_t row_dsc[] = { 50, 50, 50, 50, LV_GRID_TEMPLATE_LAST };
  
  // Create hex keypad container
  numpad_container = lv_obj_create(tab);
  lv_obj_set_style_shadow_width(numpad_container, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(numpad_container, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_width(numpad_container, 0, LV_PART_MAIN);
  lv_obj_set_style_grid_column_dsc_array(numpad_container, col_dsc, 0);
  lv_obj_set_style_grid_row_dsc_array(numpad_container, row_dsc, 0);
  lv_obj_set_size(numpad_container, SCR_WIDTH - 20, 220);
  lv_obj_set_layout(numpad_container, LV_LAYOUT_GRID);
  lv_obj_align(numpad_container, LV_ALIGN_TOP_MID, 0, 120);
  lv_obj_set_style_radius(numpad_container, 0, LV_PART_MAIN);
  
  // Create hex buttons (0-9, A-F) - 16 buttons in 4x4 grid
  for (int i = 0; i < 16; i++) {
    uint8_t col = i % 4;
    uint8_t row = i / 4;
    
    lv_obj_t* btn = lv_btn_create(numpad_container);
    lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
    lv_obj_set_style_bg_color(btn, color_primary, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 14, LV_PART_MAIN);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    
    lv_obj_t* label = lv_label_create(btn);
    // Map 0-15 to hex characters: 0-9, A-F
    if (i < 10) {
      lv_label_set_text(label, std::to_string(i).c_str());
    } else {
      char hex_label[2] = { (char)('A' + (i - 10)), '\0' };
      lv_label_set_text(label, hex_label);
    }
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_center(label);
  }
  
  // Add shared event handler for all hex buttons
  lv_obj_add_event_cb(numpad_container, pairing_hex_event_cb, LV_EVENT_CLICKED, NULL);
  
  // Create backspace button below the hex keypad
  lv_obj_t* backspace_btn = lv_btn_create(tab);
  lv_obj_set_size(backspace_btn, 100, 40);
  lv_obj_align(backspace_btn, LV_ALIGN_TOP_MID, 0, 350);
  lv_obj_set_style_bg_color(backspace_btn, lv_color_hex(0x666666), LV_PART_MAIN);
  lv_obj_set_style_radius(backspace_btn, 14, LV_PART_MAIN);
  lv_obj_add_event_cb(backspace_btn, pairing_backspace_event_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* backspace_label = lv_label_create(backspace_btn);
  lv_label_set_text(backspace_label, LV_SYMBOL_BACKSPACE);
  lv_obj_set_style_text_font(backspace_label, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_center(backspace_label);
  
  // Submit button
  pairing_submit_btn = lv_btn_create(tab);
  lv_obj_set_size(pairing_submit_btn, 120, 45);
  lv_obj_align(pairing_submit_btn, LV_ALIGN_BOTTOM_LEFT, 20, -10);
  lv_obj_set_style_bg_color(pairing_submit_btn, lv_color_hex(0x00AA00), LV_PART_MAIN);
  lv_obj_add_event_cb(pairing_submit_btn, pairing_submit_event_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* submit_label = lv_label_create(pairing_submit_btn);
  lv_label_set_text(submit_label, "Submit");
  lv_obj_set_style_text_font(submit_label, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_center(submit_label);
  
  // Cancel button
  pairing_cancel_btn = lv_btn_create(tab);
  lv_obj_set_size(pairing_cancel_btn, 120, 45);
  lv_obj_align(pairing_cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
  lv_obj_set_style_bg_color(pairing_cancel_btn, lv_color_hex(0xAA0000), LV_PART_MAIN);
  lv_obj_add_event_cb(pairing_cancel_btn, pairing_cancel_event_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* cancel_label = lv_label_create(pairing_cancel_btn);
  lv_label_set_text(cancel_label, "Cancel");
  lv_obj_set_style_text_font(cancel_label, &lv_font_montserrat_16, LV_PART_MAIN);
  lv_obj_center(cancel_label);
}

void notify_tab_before_delete_pairing(void) {
  pairing_instruction_label = nullptr;
  pairing_pin_display = nullptr;
  pairing_submit_btn = nullptr;
  pairing_cancel_btn = nullptr;
  numpad_container = nullptr;
  pairing_pin_buffer.clear();
}

void gui_pairing_update_status(const char* message) {
  if (pairing_instruction_label) {
    lv_label_set_text(pairing_instruction_label, message);
  }
}

void gui_pairing_clear_pin() {
  pairing_pin_buffer.clear();
  if (pairing_pin_display) {
    lv_label_set_text(pairing_pin_display, "");
  }
}

void register_gui_pairing(void) {
  register_gui(std::string(tabName_pairing), &create_tab_content_pairing, &notify_tab_before_delete_pairing);
}
