#pragma once

#include <Arduino.h>

class IUart {
 public:
  virtual ~IUart() = default;

  virtual void begin(unsigned long baud) = 0;
  virtual int available() = 0;
  virtual int read() = 0;
  virtual size_t write(const uint8_t* data, size_t len) = 0;
  virtual void flush() = 0;
};
