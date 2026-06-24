#pragma once

#include <Arduino.h>

class BatteryMonitor {
 public:
  struct Config {
    int adcPin{34};
    float vRef{3.3f};
    int adcResolution{4095};
    float dividerRatio{2.0f};
    float minBatteryV{3.30f};
    float maxBatteryV{4.20f};
  };

  explicit BatteryMonitor(Config cfg) : cfg_(cfg) {}

  void begin();
  float readVoltage() const;
  int readPercent() const;

 private:
  Config cfg_;
};
