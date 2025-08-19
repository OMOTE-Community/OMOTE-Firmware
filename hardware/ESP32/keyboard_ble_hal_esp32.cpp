#if (ENABLE_KEYBOARD_BLE == 1)

#include <nvs.h>
#include <nvs_flash.h>

#include "lib/ESP32-BLE-Keyboard/BleKeyboard.h"
#include "battery_hal_esp32.h"
#include "keyboard_ble_hal_esp32.h"

BleKeyboard bleKeyboard("OMOTE Keyboard", "CoretechR");

void keyboardBLE_startAdvertisingForAll_HAL() {
  bleKeyboard.startAdvertisingForAll();
}

void keyboardBLE_startAdvertisingWithWhitelist_HAL(std::string peersAllowed) {
  bleKeyboard.startAdvertisingWithWhitelist(peersAllowed);
}

void keyboardBLE_startAdvertisingDirected_HAL(std::string peerAddress, bool isRandomAddress) {
  bleKeyboard.startAdvertisingDirected(peerAddress, isRandomAddress);
}

void keyboardBLE_stopAdvertising_HAL() {
  bleKeyboard.stopAdvertising();
}

void keyboardBLE_printConnectedClients_HAL() {
  bleKeyboard.printConnectedClients();
}

void keyboardBLE_disconnectAllClients_HAL() {
  bleKeyboard.disconnectAllClients();
}

void keyboardBLE_printBonds_HAL() {
  bleKeyboard.printBonds();
}

std::string keyboardBLE_getBonds_HAL() {
  return bleKeyboard.getBonds();
}

void keyboardBLE_deleteBonds_HAL() {
  bleKeyboard.deleteBonds();
}

bool keyboardBLE_forceConnectionToAddress_HAL(std::string peerAddress) {
  return bleKeyboard.forceConnectionToAddress(peerAddress);
}

tAnnounceBLEmessage_cb thisAnnounceBLEmessage_cb = NULL;
void set_announceBLEmessage_cb_HAL(tAnnounceBLEmessage_cb pAnnounceBLEmessage_cb) {
  // this is the callback in the commandHandler that we call from here
  thisAnnounceBLEmessage_cb = pAnnounceBLEmessage_cb;  
}

void keyboardBLE_BLEkeyboardMessage_cb(std::string message) {
  // this callback is called from BLEKeyboard.cpp
  if (thisAnnounceBLEmessage_cb != NULL) {
    thisAnnounceBLEmessage_cb(message);
  }
}

void delete_bonds_if_NimBLE_version_changed() {
  // This function checks if bonds are already present when changing from NimBLE 1.4.x to 2.0.x or from 2.0.x back to 1.4.x
  // In these cases, we have to delete the already existing bonds.
  // Otherwise the bonds will not work (when going from 1.4.x to 2.0.x) or the ESP32 will even crash (when going from 2.0.x back to 1.4.x).
  // See https://github.com/h2zero/NimBLE-Arduino/issues/740
  // The name of the NVS partition and blobs used in this function can be seen here:
  // <nimble/nimble/host/store/config/src/ble_store_nvs.c>
  // NimBLE 1.4.x -> nimble core 1.4
  // NimBLE 2.0.x -> nimble core 1.5

  // startup: init flash
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    Serial.printf("nvs_flash_init() failed with error=%d, will erase flash\r\n", err);
    err = nvs_flash_erase();
    if (err != ESP_OK) {
      Serial.printf("nvs_flash_erase() failed with error=%d; will return\r\n", err);
      return;
    }
    err = nvs_flash_init();
    if (err != ESP_OK) {
      Serial.printf("nvs_flash_init() failed with error=%d, even after flash was erased; will return\r\n", err);
      return;
    }
  }
 
  // open partition "nimble_bond" where the bonds are stored
  nvs_handle_t nimble_bond_handle;
  err = nvs_open("nimble_bond", NVS_READWRITE, &nimble_bond_handle);
  if (err != ESP_OK) {
    Serial.printf("nvs_open 'nimble_bond' failed with error=%d, will return\r\n", err);
    return;
  }

  size_t required_size = 0;
  // Key generated during the pairing process. Present if a bond exists, used by NimBLE 1.4.x and NimBLE 2.0.x
  err = nvs_get_blob(nimble_bond_handle, "peer_sec_1", NULL, &required_size);
  bool bond_exists = (err == ESP_OK);
  // Resolvable Private Address (RPA): Bluetooth Device Address that changes periodically.
  // Only present in NimBLE 2.0.x
  err = nvs_get_blob(nimble_bond_handle, "rpa_rec_1", NULL, &required_size);
  bool rpa_exists = (err == ESP_OK);
  // Identity Resolving Key (IRK): Key used for Address Resolution (resolves an RPA).
  // Only present in NimBLE 2.0.x
  err = nvs_get_blob(nimble_bond_handle, "local_irk_1", NULL, &required_size);
  bool irk_exists = (err == ESP_OK);
  // and just for information, what an Identity Address is:
  // Identity Address: An address associated with an RPA that does not change over time. An IRK is required to resolve an RPA to its Identity Address.
 
  // Serial.printf("'peer_sec_1' present: %s; 'rpa_rec_1' present: %s; 'local_irk_1' present: %s\r\n", bond_exists ? "yes" : "no", rpa_exists ? "yes" : "no", irk_exists ? "yes" : "no");
  /*
                                              peer_sec_1 rpa_rec_1  local_irk_1     partition 'nimble_bond' should be deleted
  1.4.x, no bonds                             NO         NO         NO
  1.4.x, with bonds from 1.4.x                YES        NO         NO
  1.4.x, with bonds from 2.0.x                YES        YES        YES             x  (otherwise will not work)
  1.4.x, with bonds from 2.0.x deleted        NO         NO         Y/N(*)         (x) (just to be save, would work without)    (*)YES or NO, depending on ESP32 has rebooted at least once in 2.0.x after bond was deleted
  2.0.x, no bonds                             NO         NO         YES
  2.0.x, with bonds from 1.4.x                YES        NO         YES             x  (otherwise will crash)
  2.0.x, with bonds from 1.4.x deleted        NO         NO         YES
  2.0.x, with bonds from 2.0.x                YES        YES        YES
  */

  #if !defined(NIMBLE_ARDUINO_2_x)
  // We are in NimBLE 1.4.x. Check if we downgraded from NimBLE 2.0.x
  bool erase_nimble_partition = (rpa_exists || irk_exists);
  if (erase_nimble_partition) {
    Serial.printf("We are using NimBLE 1.4.x, but bonds from NimBLE 2.0.x are present. We have to delete all bonds, otherwise ESP32 will crash! Please bond your peers again.\r\n");
  }
  #else
  // We are in NimBLE 2.0.x. Check if we upgraded from NimBLE 1.4.x
  bool erase_nimble_partition = bond_exists && !(rpa_exists);
  if (erase_nimble_partition) {
    Serial.printf("We are using NimBLE 2.0.x, but bonds from NimBLE 1.4.x are present. We have to delete all bonds, otherwise they will not work! Please bond your peers again.\r\n");
  }
  #endif

  if (erase_nimble_partition) {
    nvs_erase_all(nimble_bond_handle);
    nvs_commit(nimble_bond_handle);
    nvs_close(nimble_bond_handle);
    // ESP needs to be restarted, because NVS data is still in nimble RAM
    Serial.printf("  NVS partition 'nimble_bond' was erased. Now we have to restart the ESP32 to also clear nimble RAM.\r\n");
    ESP.restart();
  } else {
    nvs_close(nimble_bond_handle);
  }
}

void init_keyboardBLE_HAL() {
  delete_bonds_if_NimBLE_version_changed();

  int battery_voltage;
  int battery_percentage;
  boolean battery_ischarging;
  get_battery_status_HAL(&battery_voltage, &battery_percentage, &battery_ischarging);

  bleKeyboard.set_BLEKeyboardMessage_cb(&keyboardBLE_BLEkeyboardMessage_cb);
  bleKeyboard.setBatteryLevel(battery_percentage);
  bleKeyboard.begin();
  // In case only one peer is bonded, startAdvertisingForAll() is called on initialisation
  bleKeyboard.startAdvertisingIfExactlyOneBondExists();
}

bool keyboardBLE_isAdvertising_HAL() {
  return bleKeyboard.isAdvertising();
}

bool keyboardBLE_isConnected_HAL() {
  return bleKeyboard.isConnected();
}

void keyboardBLE_shutdown_HAL() {
  bleKeyboard.end();
}
    
void keyboardBLE_write_HAL(uint8_t c) {
  bleKeyboard.write(c);
}

void keyboardBLE_longpress_HAL(uint8_t c) {
  bleKeyboard.press(c);
  delay(1000);
  bleKeyboard.release(c);
}

void keyboardBLE_home_HAL() {
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(KEY_ESC);
  bleKeyboard.releaseAll();
}

void keyboardBLE_sendString_HAL(const std::string &s) {
  bleKeyboard.print(s.c_str());
}

void consumerControlBLE_write_HAL(const MediaKeyReport value) {
  bleKeyboard.write(value);
}

void consumerControlBLE_longpress_HAL(const MediaKeyReport value) {
  bleKeyboard.press(value);
  delay(1000);
  bleKeyboard.release(value);
}

#endif
