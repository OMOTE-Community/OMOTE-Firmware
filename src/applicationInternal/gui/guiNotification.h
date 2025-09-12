#pragma once

#include <lvgl.h>
#include <string>

// Global notification system that slides down from under the status bar
namespace GuiNotification {

// Notification types
enum class NotificationType {
    VOLUME,
    POWER,
    MESSAGE,
    ERROR
};

// Initialize the notification system
void init();

// Show a volume notification with level and mute status
void showVolumeNotification(double level, bool is_muted);

// Show a power notification
void showPowerNotification(bool is_on);

// Show a general message notification
void showMessageNotification(const std::string& message, bool is_error = false);

// Hide the current notification (called by swipe gesture or timer)
void hideNotification();

// Check if a notification is currently visible
bool isNotificationVisible();

} // namespace GuiNotification
