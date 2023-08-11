﻿#pragma once
#include "HardwareInterface.h"
#include <iostream>
#include <string>

class HardwareSimulator : public HardwareInterface {
public:
  HardwareSimulator() = default;

  virtual void debugPrint(std::string message) override {
    std::cout << message;
  }

  virtual void sendIR() override {}

  virtual void MQTTPublish(const char *topic, const char *payload) override{

  };

  virtual void init() override { lv_init(); }

  virtual batteryStatus getBatteryPercentage() {
    batteryStatus fakeStatus;
    fakeStatus.isCharging = false;
    fakeStatus.percentage = 100;
    fakeStatus.voltage = 4200;
    return fakeStatus;
  }
};
