#include "guiNotification.h"
#include "guiBase.h"
#include "applicationInternal/omote_log.h"

namespace GuiNotification {

// Global notification objects
static lv_obj_t* notification_container = nullptr;
static lv_obj_t* notification_label = nullptr;
static lv_obj_t* volume_bar = nullptr;
static lv_timer_t* hide_timer = nullptr;
static lv_anim_t slide_anim;

// Animation and positioning constants
static const int NOTIFICATION_HEIGHT = 60;
static const int SLIDE_DURATION = 300;  // ms
static const int AUTO_HIDE_DELAY = 3000;  // ms
static const int RAPID_UPDATE_DELAY = 1500;  // ms for volume updates

// State tracking
static bool notification_visible = false;
static bool animation_in_progress = false;
static NotificationType current_type = NotificationType::MESSAGE;

// Forward declarations
static void slide_down_anim_cb(void* obj, int32_t value);
static void slide_up_anim_cb(void* obj, int32_t value);
static void slide_down_complete_cb(lv_anim_t* anim);
static void slide_up_complete_cb(lv_anim_t* anim);
static void auto_hide_timer_cb(lv_timer_t* timer);
static void notification_gesture_cb(lv_event_t* e);

void init() {
    if (notification_container != nullptr) {
        return; // Already initialized
    }

    // Create notification container positioned just below status bar
    // Use lv_layer_top() to ensure it's always on top, or fallback to screen
    lv_obj_t* parent = lv_layer_top();
    if (!parent) {
        parent = lv_scr_act();
    }
    
    notification_container = lv_obj_create(parent);
    lv_obj_set_size(notification_container, SCR_WIDTH, NOTIFICATION_HEIGHT);
    
    // Move to top layer so it appears above all other content (like CSS z-index)
    lv_obj_move_to_index(notification_container, -1);
    
    // Position it hidden above the visible area initially
    // statusbarTop + statusbarHeight is where status bar ends
    int notification_y = statusbarTop + statusbarHeight - NOTIFICATION_HEIGHT;
    lv_obj_set_pos(notification_container, 0, notification_y);
    
    // Style the container
    lv_obj_set_style_bg_color(notification_container, lv_color_hex(0x2C2C2C), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(notification_container, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_border_width(notification_container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(notification_container, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(notification_container, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(notification_container, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(notification_container, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_pad_all(notification_container, 12, LV_PART_MAIN);
    
    // Create label for text content
    notification_label = lv_label_create(notification_container);
    lv_label_set_text(notification_label, "");
    lv_obj_set_style_text_font(notification_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(notification_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(notification_label, LV_ALIGN_LEFT_MID, 0, 0);
    
    // Create volume bar (initially hidden)
    volume_bar = lv_bar_create(notification_container);
    lv_obj_set_size(volume_bar, 120, 12);
    lv_obj_align(volume_bar, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_radius(volume_bar, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(volume_bar, 4, LV_PART_INDICATOR);
    lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    // Add swipe gesture support
    lv_obj_add_event_cb(notification_container, notification_gesture_cb, LV_EVENT_GESTURE, nullptr);
    lv_obj_clear_flag(notification_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // Set additional flags to ensure it stays on top
    lv_obj_clear_flag(notification_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(notification_container, LV_OBJ_FLAG_FLOATING);
    
    // Initially hidden
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
    
    // Start auto-hide timer
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
        // Swipe up to dismiss
        hideNotification();
    }
}

static void showNotification() {
    if (!notification_container) {
        init(); // Initialize if not already done
    }
    
    // Stop any existing timer
    if (hide_timer) {
        lv_timer_del(hide_timer);
        hide_timer = nullptr;
    }
    
    // If already visible, just update the timer for auto-hide
    if (notification_visible && !animation_in_progress) {
        int delay = (current_type == NotificationType::VOLUME) ? RAPID_UPDATE_DELAY : AUTO_HIDE_DELAY;
        hide_timer = lv_timer_create(auto_hide_timer_cb, delay, nullptr);
        lv_timer_set_repeat_count(hide_timer, 1);
        return;
    }
    
    // If animation in progress, let it complete
    if (animation_in_progress) {
        return;
    }
    
    // Show and animate down
    lv_obj_clear_flag(notification_container, LV_OBJ_FLAG_HIDDEN);
    
    // Ensure notification is on top of all other content
    lv_obj_move_to_index(notification_container, -1);
    
    int start_y = statusbarTop + statusbarHeight - NOTIFICATION_HEIGHT;
    int end_y = statusbarTop + statusbarHeight;
    
    lv_obj_set_y(notification_container, start_y);
    
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
    
    // Stop auto-hide timer
    if (hide_timer) {
        lv_timer_del(hide_timer);
        hide_timer = nullptr;
    }
    
    int start_y = lv_obj_get_y(notification_container);
    int end_y = statusbarTop + statusbarHeight - NOTIFICATION_HEIGHT;
    
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
    
    // Format volume text
    char volume_text[64];
    if (is_muted) {
        snprintf(volume_text, sizeof(volume_text), "Volume: MUTED");
    } else {
        snprintf(volume_text, sizeof(volume_text), "Volume: %.1f dB", level);
    }
    
    lv_label_set_text(notification_label, volume_text);
    
    // Show/update volume bar
    lv_obj_clear_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    if (is_muted) {
        // Muted state (Sony reports -∞ dB which effectively means -92dB or lower)
        lv_bar_set_value(volume_bar, 0, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0x808080), LV_PART_INDICATOR);
    } else {
        // Convert dB to percentage using Sony STR-DA3100ES range: -80dB to +23dB
        const double dB_floor = -80.0;
        const double dB_ceiling = 23.0;
        const double dB_range = dB_ceiling - dB_floor; // 103dB total range
        
        // Calculate percentage: 0% at -80dB, 100% at +23dB
        int percentage = (int)((level - dB_floor) * 100.0 / dB_range);
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;
        
        lv_bar_set_value(volume_bar, percentage, LV_ANIM_ON);
        lv_obj_set_style_bg_color(volume_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    }
    
    showNotification();
    
    omote_log_d("Volume notification: %s\r\n", volume_text);
}

void showPowerNotification(bool is_on) {
    current_type = NotificationType::POWER;
    
    // Hide volume bar for power notifications
    lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    lv_label_set_text(notification_label, is_on ? "Power: ON" : "Power: OFF");
    lv_obj_set_style_text_color(notification_label, lv_color_white(), LV_PART_MAIN);
    
    showNotification();
    
    omote_log_d("Power notification: %s\r\n", is_on ? "ON" : "OFF");
}

void showMessageNotification(const std::string& message, bool is_error) {
    current_type = is_error ? NotificationType::ERROR : NotificationType::MESSAGE;
    
    // Hide volume bar for message notifications
    lv_obj_add_flag(volume_bar, LV_OBJ_FLAG_HIDDEN);
    
    lv_label_set_text(notification_label, message.c_str());
    lv_obj_set_style_text_color(notification_label, 
                               is_error ? lv_color_hex(0xFF4444) : lv_color_white(), 
                               LV_PART_MAIN);
    
    showNotification();
    
    omote_log_d("Message notification: %s\r\n", message.c_str());
}

bool isNotificationVisible() {
    return notification_visible;
}

} // namespace GuiNotification
