#include <lvgl.h>
#include <time.h>
#include <sys/time.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/memoryUsage.h"
#include "guis/gui_settings.h"
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/omote_log.h"

#if (ENABLE_HUB_COMMUNICATION > 0)
// Hub sends positive seconds WEST of UTC (EDT = +14400)
// RTC_DATA_ATTR preserves this across deep sleep (RTC time keeps running)
RTC_DATA_ATTR static int32_t g_seconds_west_of_utc = 0;
#endif

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

#if (ENABLE_HUB_COMMUNICATION > 0)
static bool isTimeSetOrValid(time_t timestamp) {
  return timestamp != (time_t)-1 && timestamp >= 1000000000; // After year 2001
}

static const char* TIME_PLACEHOLDER = "--:--";

static void formatTime12Hour(const struct tm* timeinfo, char* buffer, size_t buffer_size) {
  strftime(buffer, buffer_size, "%I:%M %p", timeinfo);
  
  // Remove leading zero from hour if present
  if (buffer[0] == '0') {
    memmove(buffer, buffer + 1, strlen(buffer));
  }
}

void updateTimeOnGUI() {
  if (TimeLabel == NULL) return;
  time_t now_utc = time(NULL);
  if (!isTimeSetOrValid(now_utc)) {
    lv_label_set_text(TimeLabel, TIME_PLACEHOLDER);
    return;
  }
  // Render local time explicitly: local = UTC - seconds_west_of_utc
  time_t local_secs = now_utc - (time_t)g_seconds_west_of_utc;
  struct tm tm_local;
  if (gmtime_r(&local_secs, &tm_local) == nullptr) {
    lv_label_set_text(TimeLabel, TIME_PLACEHOLDER);
    return;
  }
  char time_buffer[16];
  formatTime12Hour(&tm_local, time_buffer, sizeof(time_buffer));
  lv_label_set_text(TimeLabel, time_buffer);
}

void setTime(uint32_t timestamp_utc, int32_t seconds_west_of_utc) {
  // Keep device clock in UTC
  g_seconds_west_of_utc = seconds_west_of_utc;
  struct timeval tv;
  tv.tv_sec  = (time_t)timestamp_utc;
  tv.tv_usec = 0;
  if (settimeofday(&tv, nullptr) == 0) {
    omote_log_d("Time synchronized (UTC): ts=%lu, west=%ld",
                (unsigned long)timestamp_utc, (long)g_seconds_west_of_utc);
  } else {
    #if defined(__APPLE__) || defined(__linux__)
    omote_log_d("Time sync failed (expected on macOS/Linux simulator - requires root privileges)");
    #else
    omote_log_e("Failed to set time");
    #endif
  }
}
#endif

// update user_led, battery, BLE, memoryUsage on GUI
void updateHardwareStatusAndShowOnGUI(void) {

  update_userled();

  updateBatteryStatusOnGUI();
  #if (ENABLE_HUB_COMMUNICATION > 0)
  updateTimeOnGUI();
  #endif
  #if (ENABLE_BLUETOOTH == 1)
    // adjust this if you implement other bluetooth devices than the BLE keyboard
    #if (ENABLE_KEYBOARD_BLE == 1)
    updateKeyboardBLEstatusOnGUI();
    #endif
  #endif

  doLogMemoryUsage();

}
