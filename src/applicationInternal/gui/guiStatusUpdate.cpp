#include <lvgl.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/memoryUsage.h"
#include "guis/gui_settings.h"
#include "applicationInternal/gui/guiBase.h"

// --- regularly update hardware values and update GUI, used by "main.cpp" ----
void updateBatteryStatusOnGUI() {
  int battery_voltage;
  int battery_percentage;
  bool battery_ischarging;
  get_battery_status(&battery_voltage, &battery_percentage, &battery_ischarging);

  char buffer1[20];
  snprintf(buffer1, sizeof(buffer1), "Voltage: %.2f V", (float)battery_voltage / 1000);

  // GUI settings
  if (objBattSettingsVoltage    != NULL) {lv_label_set_text_fmt(objBattSettingsVoltage, "%s", buffer1);}
  if (objBattSettingsPercentage != NULL) {lv_label_set_text_fmt(objBattSettingsPercentage, "Percentage: %d%%", battery_percentage);}
  //lv_label_set_text_fmt(objBattSettingsIscharging, "Is charging: %s",  battery_ischarging ? "yes" : "no");

  // GUI status bar at the top
  char buffer2[12];
  // Voltage and percentage
  // snprintf(buffer2, sizeof(buffer2), "%.1fV, %d%%", (float)getBatteryVoltage() / 1000, battery_percentage);
  // only percentage
  snprintf(buffer2, sizeof(buffer2), "%d%%", battery_percentage);
  for (int i=0; i<strlen(buffer2); i++) {
    if (buffer2[i] == '.') {
      buffer2[i] = ',';
    }
  }

  #if (OMOTE_HARDWARE_REV >= 4)
    if (battery_ischarging /*|| (!battery_ischarging && getBatteryVoltage() > 4350)*/){
      // if (BattPercentageLabel != NULL) {lv_label_set_text(BattPercentageLabel, "");}
      // lv_label_set_text_fmt(BattPercentageLabel, "%d%%", battery_percentage);
      // lv_label_set_text_fmt(BattPercentageLabel, "%.1f, %d%%", (float)getBatteryVoltage() / 1000, battery_percentage);
      if (BattPercentageLabel != NULL) {lv_label_set_text(BattPercentageLabel, buffer2);}
      if (BattIconLabel != NULL) {lv_label_set_text(BattIconLabel, LV_SYMBOL_USB);}
    } else
  #endif
  {
    // Update status bar battery indicator
    // lv_label_set_text_fmt(BattPercentageLabel, "%.1f, %d%%", (float)getBatteryVoltage() / 1000, battery_percentage);
    if (BattPercentageLabel != NULL) {lv_label_set_text(BattPercentageLabel, buffer2);}
    if (BattIconLabel != NULL) {
           if(battery_percentage > 95) lv_label_set_text(BattIconLabel, LV_SYMBOL_BATTERY_FULL);
      else if(battery_percentage > 75) lv_label_set_text(BattIconLabel, LV_SYMBOL_BATTERY_3);
      else if(battery_percentage > 50) lv_label_set_text(BattIconLabel, LV_SYMBOL_BATTERY_2);
      else if(battery_percentage > 25) lv_label_set_text(BattIconLabel, LV_SYMBOL_BATTERY_1);
      else                                 lv_label_set_text(BattIconLabel, LV_SYMBOL_BATTERY_EMPTY);
    }
  }

}

#if (ENABLE_KEYBOARD_BLE == 1)
// if bluetooth is in pairing mode (pairing mode is always on, if not connected), but not connected, then blink
bool blinkBluetoothLabelIsOn = false;
void updateKeyboardBLEstatusOnGUI() {
  if (keyboardBLE_isAdvertising()) {
    blinkBluetoothLabelIsOn = !blinkBluetoothLabelIsOn;
    if (blinkBluetoothLabelIsOn) {
      if (BluetoothLabel != NULL) {lv_label_set_text(BluetoothLabel, LV_SYMBOL_BLUETOOTH);}
    } else {
      if (BluetoothLabel != NULL) {lv_label_set_text(BluetoothLabel, "");}
    }
  } else if (keyboardBLE_isConnected()) {
    if (!blinkBluetoothLabelIsOn) {
      blinkBluetoothLabelIsOn = true;
      if (BluetoothLabel != NULL) {lv_label_set_text(BluetoothLabel, LV_SYMBOL_BLUETOOTH);}
    }
  } else {
    if (blinkBluetoothLabelIsOn) {
      blinkBluetoothLabelIsOn = false;
      if (BluetoothLabel != NULL) {lv_label_set_text(BluetoothLabel, "");}
    }
  }
}
#endif

// update user_led, battery, BLE, memoryUsage on GUI
void updateHardwareStatusAndShowOnGUI(void) {

  update_userled();

  updateBatteryStatusOnGUI();
  #if (ENABLE_BLUETOOTH == 1)
    // adjust this if you implement other bluetooth devices than the BLE keyboard
    #if (ENABLE_KEYBOARD_BLE == 1)
    updateKeyboardBLEstatusOnGUI();
    #endif
  #endif

  doLogMemoryUsage();

}
