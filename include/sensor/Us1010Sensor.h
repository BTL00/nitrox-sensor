#pragma once

#include <Arduino.h>

#include "hal/IUart.h"
#include "sensor/IUs1010Sensor.h"

class Us1010Sensor : public IUs1010Sensor {
 public:
  explicit Us1010Sensor(IUart& uart, unsigned long baud = 9600)
      : uart_(uart), baud_(baud) {}

  void begin() override;
  void poll() override;
  SensorReading latest() const override { return latest_; }

 private:
  bool parseLine(const String& line);

  IUart& uart_;
  unsigned long baud_;
  String lineBuffer_;
  SensorReading latest_;
};
