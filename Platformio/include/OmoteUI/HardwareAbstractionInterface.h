﻿#pragma once
#include <string>
#include <lvgl.h>
class HardwareAbstractionInterface
{
public:
    typedef void (*display_flush_cb)(struct _lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
    typedef void (*touch_pad_read)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

    HardwareAbstractionInterface() = default;

    virtual void debugPrint(std::string message) = 0;

    virtual void sendIR() = 0;

    virtual void MQTTPublish(const char *topic, const char *payload) = 0;

    virtual void initLVGL(display_flush_cb aDisplayFlushCb,
                          touch_pad_read aTouchPadReadCb) = 0;

    virtual lv_coord_t getScreenWidth() = 0;
    virtual lv_coord_t getScreenHeight() = 0;
};
