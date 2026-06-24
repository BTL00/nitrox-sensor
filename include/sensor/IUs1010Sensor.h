#pragma once

#include <Arduino.h>

struct SensorReading {
  float o2Percent{0.0f};
  float flowLpm{0.0f};
  float temperatureC{0.0f};
  bool valid{false};
  uint32_t timestampMs{0};
};

class IUs1010Sensor {
 public:
  virtual ~IUs1010Sensor() = default;

  virtual void begin() = 0;
  virtual void poll() = 0;
  virtual SensorReading latest() const = 0;
};
