#pragma once

#include <Arduino.h>

#include <deque>
#include <vector>

#include "hal/IUart.h"

class MockUart : public IUart {
 public:
  void begin(unsigned long baud) override { baud_ = baud; }

  int available() override { return static_cast<int>(rx_.size()); }

  int read() override {
    if (rx_.empty()) {
      return -1;
    }
    const uint8_t value = rx_.front();
    rx_.pop_front();
    return value;
  }

  size_t write(const uint8_t* data, size_t len) override {
    tx_.insert(tx_.end(), data, data + len);
    return len;
  }

  void flush() override {}

  void injectRx(const String& payload) {
    for (size_t i = 0; i < payload.length(); ++i) {
      rx_.push_back(static_cast<uint8_t>(payload[i]));
    }
  }

  const std::vector<uint8_t>& txBuffer() const { return tx_; }
  unsigned long baud() const { return baud_; }

 private:
  std::deque<uint8_t> rx_;
  std::vector<uint8_t> tx_;
  unsigned long baud_{0};
};
