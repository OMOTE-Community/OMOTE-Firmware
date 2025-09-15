#pragma once

#include <lvgl.h>
#include <string>

// Global notification system that slides down from under the status bar
namespace GuiNotification {

enum class NotificationType {
    VOLUME,
    POWER,
    MESSAGE,
    ERROR
};

void init();

void showVolumeNotification(double level, bool is_muted);

void showPowerNotification(bool is_on);

void showMessageNotification(const std::string& message);

void showErrorNotification(const std::string& message);

void hideNotification();

bool isNotificationVisible();

} // namespace GuiNotification
