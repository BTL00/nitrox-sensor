#pragma once

#include <Arduino.h>

#include "hal/IUart.h"

class HardwareUartAdapter : public IUart {
 public:
  explicit HardwareUartAdapter(HardwareSerial& serial, int8_t rxPin, int8_t txPin)
      : serial_(serial), rxPin_(rxPin), txPin_(txPin) {}

  void begin(unsigned long baud) override {
    serial_.begin(baud, SERIAL_8N1, rxPin_, txPin_);
  }

  int available() override { return serial_.available(); }

  int read() override { return serial_.read(); }

  size_t write(const uint8_t* data, size_t len) override {
    return serial_.write(data, len);
  }

  void flush() override { serial_.flush(); }

 private:
  HardwareSerial& serial_;
  int8_t rxPin_;
  int8_t txPin_;
};
