#include <algorithm>
#include <lvgl.h>
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/gui/guiBase.h"
#include "applicationInternal/gui/guiRegistry.h"
#include "applicationInternal/omote_log.h"
#include "guis/gui_irReceiver.h"

lv_obj_t* menuBoxToggle;
lv_obj_t* menuBoxMessages;
int16_t boxHeightDeactivated = 0;
int16_t boxHeightActivated = 200;
const int maxCountMessages = 15;
lv_obj_t* irReceivedMessage[maxCountMessages];
std::string IRmessages[maxCountMessages];
int messagePos = 0;
int messageCount = 0;
bool tabIsInMemory = false;

#if (ENABLE_WIFI_AND_MQTT == 1)

lv_obj_t* objMQTTmessageReceivedTopic;
lv_obj_t* objMQTTmessageReceivedPayload;
std::string lastTopic, lastPayload;

void printMQTTmessage() {
  if (!tabIsInMemory) {return;}

  if (objMQTTmessageReceivedTopic != NULL) {
    lv_label_set_text(objMQTTmessageReceivedTopic, lastTopic.c_str());
  }
  if (objMQTTmessageReceivedPayload != NULL) {
    lv_label_set_text(objMQTTmessageReceivedPayload, lastPayload.c_str());
  }
}

void showMQTTmessage(std::string topic, std::string payload) {
  lastTopic = topic;
  lastPayload = payload;
  printMQTTmessage();
}

#endif // ENABLE_WIFI_AND_MQTT

void printIRMessages(bool clearMessages = false) {
  if (!tabIsInMemory) {return;}

  omote_log_v("\r\n");
  int messagePosLoop;
  if (messageCount < maxCountMessages) {
    messagePosLoop = 0;
  } else {
    messagePosLoop = messagePos;
  }

  omote_log_v("will start printing messages, beginning at position %d\r\n", messagePosLoop);
  for (int i=0; i<maxCountMessages; i++) {
    if (clearMessages) {
      IRmessages[messagePosLoop] = "";
    }
    omote_log_v("will print at line %d the message at position %d: %s\r\n", i, messagePosLoop, IRmessages[messagePosLoop].c_str());
    lv_label_set_text(irReceivedMessage[i], IRmessages[messagePosLoop].c_str());
    messagePosLoop += 1;
    if (messagePosLoop == maxCountMessages) {
      messagePosLoop = 0;  
    }
  }
  if (clearMessages) {
    lv_obj_set_size(menuBoxMessages, lv_pct(100), boxHeightDeactivated);
  }
}

void showNewIRmessage(std::string message) {
  setLastActivityTimestamp(); // Reset the sleep timer when a IR message is received

  omote_log_v("  new IR message received: %s\r\n", message.c_str());
  // const char *a = message.c_str();
  std::string messageStr;
  messageStr.append(message.c_str());
  // std::string aMessage = s(a);
  messageStr.erase(std::remove(messageStr.begin(), messageStr.end(), '\n'), messageStr.cend());
  
  omote_log_v(" will put message %s to list\r\n", messageStr.c_str());
  messageCount += 1;
  IRmessages[messagePos] = (std::to_string(messageCount) + ": " + messageStr).c_str();
  omote_log_v(" this is the message at position %d: %s\r\n", messagePos, IRmessages[messagePos].c_str());
  messagePos += 1;
  if (messagePos == maxCountMessages) {
    messagePos = 0;  
  }
  printIRMessages();
}

// IR receiver on Switch Event handler
static void IRReceiverOnSetting_event_cb(lv_event_t* e){
  set_irReceiverEnabled(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
  if (get_irReceiverEnabled()) {
    omote_log_d("will turn on IR receiver\r\n");
    start_infraredReceiver();
    lv_obj_set_size(menuBoxMessages, lv_pct(100), boxHeightActivated);
    messageCount = 0;
    printIRMessages();
  } else {
    omote_log_d("will turn off IR receiver\r\n");
    shutdown_infraredReceiver();
    printIRMessages(true);
    messagePos = 0;
    messageCount = 0;
    lv_obj_set_size(menuBoxMessages, lv_pct(100), boxHeightDeactivated);
  }
}

void create_tab_content_irReceiver(lv_obj_t* tab) {

  // Add content to the irReceiver tab
  // With a flex layout, setting groups/boxes will position themselves automatically
  lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_ACTIVE);

  menuBoxToggle = lv_obj_create(tab);
  lv_obj_set_size(menuBoxToggle, lv_pct(100), 48);
  lv_obj_set_style_bg_color(menuBoxToggle, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBoxToggle, 0, LV_PART_MAIN);

  lv_obj_t* menuLabel = lv_label_create(menuBoxToggle);
  lv_label_set_text(menuLabel, "Turn on IR receiver");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 3);
  lv_obj_t* irReceiverToggle = lv_switch_create(menuBoxToggle);
  lv_obj_set_size(irReceiverToggle, 40, 22);
  lv_obj_align(irReceiverToggle, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(irReceiverToggle, lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_event_cb(irReceiverToggle, IRReceiverOnSetting_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  menuBoxMessages = lv_obj_create(tab);
  lv_obj_set_size(menuBoxMessages, lv_pct(100), 77); // 125
  lv_obj_set_style_bg_color(menuBoxMessages, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBoxMessages, 0, LV_PART_MAIN);
  
  for (int i=0; i<maxCountMessages; i++) {
    irReceivedMessage[i] = lv_label_create(menuBoxMessages);
    lv_obj_set_style_text_font(irReceivedMessage[i], &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_align(irReceivedMessage[i], LV_ALIGN_TOP_LEFT, 0, 0 + i*11);
  }

  tabIsInMemory = true;

  // if we come back to this page and IR receiver is already enabled, simply print all the received messages
  if (get_irReceiverEnabled()) {
    // set state 
    lv_obj_add_state(irReceiverToggle, LV_STATE_CHECKED);
    // print already received messages
    lv_obj_set_size(menuBoxMessages, lv_pct(100), boxHeightActivated);
    printIRMessages();
  } else {
    printIRMessages(true);
  }

#if (ENABLE_WIFI_AND_MQTT == 1)

  // Show MQTT messages we subscribed to ------------------------------------------------------
  menuLabel = lv_label_create(tab);
  lv_label_set_text(menuLabel, "MQTT messages arrived");
  lv_obj_t* menuBox = lv_obj_create(tab);
  lv_obj_set_size(menuBox, lv_pct(100), 46);
  lv_obj_set_style_bg_color(menuBox, color_primary, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);
  
  objMQTTmessageReceivedTopic = lv_label_create(menuBox);
  lv_label_set_text(objMQTTmessageReceivedTopic, "");
  lv_obj_set_style_text_font(objMQTTmessageReceivedTopic, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_align(objMQTTmessageReceivedTopic, LV_ALIGN_TOP_LEFT, 0, -4);
  objMQTTmessageReceivedPayload = lv_label_create(menuBox);
  lv_label_set_text(objMQTTmessageReceivedPayload, "");
  lv_obj_set_style_text_font(objMQTTmessageReceivedPayload, &lv_font_montserrat_10, LV_PART_MAIN);
  lv_obj_align(objMQTTmessageReceivedPayload, LV_ALIGN_TOP_LEFT, 0, 8);

  printMQTTmessage();
#endif // ENABLE_WIFI_AND_MQTT
}

void notify_tab_before_delete_irReceiver(void) {
  // remember to set all pointers to lvgl objects to NULL if they might be accessed from outside.
  // They must check if object is NULL and must not use it if so
  
  tabIsInMemory = false;
}

void register_gui_irReceiver(void){
  register_gui(std::string(tabName_irReceiver), & create_tab_content_irReceiver, & notify_tab_before_delete_irReceiver);
}

void showEspNowMessage(std::string payload) {
  lastTopic = "ESP-NOW";
  lastPayload = payload;
  printMQTTmessage();
}
