#include "guiNotification.h"
#include "guiBase.h"
#include "applicationInternal/omote_log.h"

namespace GuiNotification {

static lv_obj_t* notification_container = nullptr;
static lv_obj_t* notification_label = nullptr;
static lv_obj_t* volume_level_label = nullptr;
static lv_obj_t* volume_bar = nullptr;
static lv_timer_t* hide_timer = nullptr;
static lv_anim_t slide_anim;

static const int NOTIFICATION_HEIGHT_SINGLE = 48;
static const int NOTIFICATION_HEIGHT_DOUBLE = 80;
static const int SLIDE_DURATION = 300;
static const int AUTO_HIDE_DELAY = 3000;
static const int RAPID_UPDATE_DELAY = 1500;

static bool notification_visible = false;
static bool animation_in_progress = false;
static NotificationType current_type = NotificationType::MESSAGE;
static void slide_down_anim_cb(void* obj, int32_t value);
static void slide_up_anim_cb(void* obj, int32_t value);
static void slide_down_complete_cb(lv_anim_t* anim);
static void slide_up_complete_cb(lv_anim_t* anim);
static void auto_hide_timer_cb(lv_timer_t* timer);
static void notification_gesture_cb(lv_event_t* e);
static void resizeNotificationContainer(int height);

void init() {
    if (notification_container != nullptr) {
        return;
    }

    lv_obj_t* parent = lv_layer_top();
    if (!parent) {
        parent = lv_scr_act();
    }
    
    notification_container = lv_obj_create(parent);
    lv_obj_set_size(notification_container, SCR_WIDTH, NOTIFICATION_HEIGHT_SINGLE);
    
    lv_obj_move_to_index(notification_container, -1);
    
    int notification_y = statusbarTop + statusbarHeight - NOTIFICATION_HEIGHT_SINGLE;
    lv_obj_set_pos(notification_container, 0, notification_y);
    
    lv_obj_set_style_bg_color(notification_container, lv_color_hex(0x1C1C1E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(notification_container, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_border_width(notification_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(notification_container, 12, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(notification_container, 12, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(notification_container, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(notification_container, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_pad_all(notification_container, 16, LV_PART_MAIN);
    
    notification_label = lv_label_create(notification_container);
    lv_label_set_text(notification_label, "");
    lv_obj_set_style_text_font(notification_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(notification_label, LV_ALIGN_LEFT_MID, 0, 0);
    
    volume_level_label = lv_label_create(notification_container);
    lv_label_set_text(volume_level_label, "");
    lv_obj_set_style_text_font(volume_level_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(volume_level_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(volume_level_label, LV_ALIGN_TOP_RIGHT, -8, 8);
    
    volume_bar = lv_bar_create(notification_container);
    lv_obj_set_size(volume_bar, SCR_WIDTH - 64, 3);
    lv_obj_align(volume_bar, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0x3A3A3C), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0x007AFF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(volume_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(volume_bar, 2, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(volume_bar, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(volume_bar, LV_OPA_100, LV_PART_INDICATOR);
    lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_add_event_cb(notification_container, notification_gesture_cb, LV_EVENT_GESTURE, nullptr);
    lv_obj_clear_flag(notification_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    lv_obj_clear_flag(notification_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(notification_container, LV_OBJ_FLAG_FLOATING);
    
    lv_obj_add_flag(notification_container, LV_OBJ_FLAG_HIDDEN);
    
    omote_log_d("Notification system initialized\r\n");
}

static void slide_down_anim_cb(void* obj, int32_t value) {
    lv_obj_set_y((lv_obj_t*)obj, value);
}

static void slide_up_anim_cb(void* obj, int32_t value) {
    lv_obj_set_y((lv_obj_t*)obj, value);
}

static void slide_down_complete_cb(lv_anim_t* anim) {
    animation_in_progress = false;
    notification_visible = true;
    
    if (hide_timer) {
        lv_timer_del(hide_timer);
    }
    
    int delay = (current_type == NotificationType::VOLUME) ? RAPID_UPDATE_DELAY : AUTO_HIDE_DELAY;
    hide_timer = lv_timer_create(auto_hide_timer_cb, delay, nullptr);
    lv_timer_set_repeat_count(hide_timer, 1);
}

static void slide_up_complete_cb(lv_anim_t* anim) {
    animation_in_progress = false;
    notification_visible = false;
    lv_obj_add_flag(notification_container, LV_OBJ_FLAG_HIDDEN);
}

static void auto_hide_timer_cb(lv_timer_t* timer) {
    if (notification_visible && !animation_in_progress) {
        hideNotification();
    }
}

static void notification_gesture_cb(lv_event_t* e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    
    if (dir == LV_DIR_TOP && notification_visible) {
        hideNotification();
    }
}

static void resizeNotificationContainer(int height) {
    if (!notification_container) return;
    
    lv_obj_set_size(notification_container, SCR_WIDTH, height);
    
    int notification_y = statusbarTop + statusbarHeight - height;
    lv_obj_set_pos(notification_container, 0, notification_y);
    
    if (height == NOTIFICATION_HEIGHT_SINGLE) {
        lv_obj_align(notification_label, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(volume_level_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_align(notification_label, LV_ALIGN_TOP_LEFT, 0, 8);
        lv_obj_align(volume_level_label, LV_ALIGN_TOP_RIGHT, -8, 8);
        lv_obj_clear_flag(volume_level_label, LV_OBJ_FLAG_HIDDEN);
    }
    
    lv_obj_invalidate(notification_container);
}

static void showNotification() {
    if (!notification_container) {
        init();
    }
    
    if (hide_timer) {
        lv_timer_del(hide_timer);
        hide_timer = nullptr;
    }
    
    if (notification_visible && !animation_in_progress) {
        int delay = (current_type == NotificationType::VOLUME) ? RAPID_UPDATE_DELAY : AUTO_HIDE_DELAY;
        hide_timer = lv_timer_create(auto_hide_timer_cb, delay, nullptr);
        lv_timer_set_repeat_count(hide_timer, 1);
        return;
    }
    
    if (animation_in_progress) {
        return;
    }
    
    lv_obj_clear_flag(notification_container, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_move_to_index(notification_container, -1);
    
    int height = (current_type == NotificationType::VOLUME) ? NOTIFICATION_HEIGHT_DOUBLE : NOTIFICATION_HEIGHT_SINGLE;
    
    resizeNotificationContainer(height);
    
    int start_y = statusbarTop + statusbarHeight - height;
    int end_y = statusbarTop + statusbarHeight;
    
    lv_obj_set_pos(notification_container, 0, start_y);
    
    lv_anim_init(&slide_anim);
    lv_anim_set_var(&slide_anim, notification_container);
    lv_anim_set_values(&slide_anim, start_y, end_y);
    lv_anim_set_time(&slide_anim, SLIDE_DURATION);
    lv_anim_set_exec_cb(&slide_anim, slide_down_anim_cb);
    lv_anim_set_ready_cb(&slide_anim, slide_down_complete_cb);
    lv_anim_set_path_cb(&slide_anim, lv_anim_path_ease_out);
    
    animation_in_progress = true;
    lv_anim_start(&slide_anim);
}

void hideNotification() {
    if (!notification_visible || animation_in_progress) {
        return;
    }
    
    if (hide_timer) {
        lv_timer_del(hide_timer);
        hide_timer = nullptr;
    }
    
    int start_y = lv_obj_get_y(notification_container);
    int height = (current_type == NotificationType::VOLUME) ? NOTIFICATION_HEIGHT_DOUBLE : NOTIFICATION_HEIGHT_SINGLE;
    int end_y = statusbarTop + statusbarHeight - height;
    
    lv_anim_init(&slide_anim);
    lv_anim_set_var(&slide_anim, notification_container);
    lv_anim_set_values(&slide_anim, start_y, end_y);
    lv_anim_set_time(&slide_anim, SLIDE_DURATION);
    lv_anim_set_exec_cb(&slide_anim, slide_up_anim_cb);
    lv_anim_set_ready_cb(&slide_anim, slide_up_complete_cb);
    lv_anim_set_path_cb(&slide_anim, lv_anim_path_ease_in);
    
    animation_in_progress = true;
    lv_anim_start(&slide_anim);
}

void showVolumeNotification(double level, bool is_muted) {
    current_type = NotificationType::VOLUME;
    
    char volume_text[64];
    if (is_muted) {
        snprintf(volume_text, sizeof(volume_text), LV_SYMBOL_VOLUME_MID " Volume");
    } else {
        snprintf(volume_text, sizeof(volume_text), LV_SYMBOL_VOLUME_MID " Volume");
    }
    
    lv_label_set_text(notification_label, volume_text);
    
    char level_text[32];
    if (is_muted) {
        snprintf(level_text, sizeof(level_text), "MUTED");
    } else {
        snprintf(level_text, sizeof(level_text), "%.1f dB", level);
    }
    
    lv_label_set_text(volume_level_label, level_text);
    
    lv_obj_clear_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_invalidate(notification_label);
    lv_obj_invalidate(volume_level_label);
    lv_obj_invalidate(volume_bar);
    
    if (is_muted) {
        lv_bar_set_value(volume_bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0x8E8E93), LV_PART_INDICATOR);
    } else {
        const double dB_floor = -80.0;
        const double dB_ceiling = 23.0;
        const double dB_range = dB_ceiling - dB_floor;
        
        int percentage = (int)((level - dB_floor) * 100.0 / dB_range);
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;
        
        lv_bar_set_value(volume_bar, percentage, LV_ANIM_ON);
        lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0x007AFF), LV_PART_INDICATOR);
    }
    
    showNotification();
    
    omote_log_d("Volume notification: %s\r\n", volume_text);
}

void showPowerNotification(bool is_on) {
    current_type = NotificationType::POWER;
    
    if (is_on) {
        lv_label_set_text(notification_label, LV_SYMBOL_POWER "  Device powered on");
    } else {
        lv_label_set_text(notification_label, LV_SYMBOL_POWER "  Device powered off");
    }
    
    lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    showNotification();
    
    omote_log_d("Power notification: %s\r\n", is_on ? "ON" : "OFF");
}

void showMessageNotification(const std::string& message) {
    current_type = NotificationType::MESSAGE;
    
    lv_label_set_text(notification_label, message.c_str());
    lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    showNotification();
    
    omote_log_d("Message notification: %s\r\n", message.c_str());
}

void showErrorNotification(const std::string& message) {
    current_type = NotificationType::ERROR;
    
    lv_label_set_text(notification_label, message.c_str());
    lv_obj_set_style_text_color(notification_label, lv_color_hex(0xFF3B30), LV_PART_MAIN);
    
    showNotification();
    
    omote_log_d("Error notification: %s\r\n", message.c_str());
}

bool isNotificationVisible() {
    return notification_visible;
}

} // namespace GuiNotification
