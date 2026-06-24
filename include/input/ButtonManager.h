#pragma once

#include <Arduino.h>

#include <functional>

class ButtonManager {
 public:
  struct Config {
    int leftPin{35};
    int rightPin{0};
    bool activeLow{true};
    uint32_t debounceMs{40};
  };

  using ButtonCallback = std::function<void()>;

  explicit ButtonManager(Config cfg) : cfg_(cfg) {}

  void begin();
  void poll();
  bool isLeftPressed() const { return lastLeftState_; }
  bool isRightPressed() const { return lastRightState_; }

  void onLeftPressed(ButtonCallback cb) { onLeftPressed_ = cb; }
  void onRightPressed(ButtonCallback cb) { onRightPressed_ = cb; }

 private:
  bool isPressed(int pin) const;

  Config cfg_;
  uint32_t lastLeftEdge_{0};
  uint32_t lastRightEdge_{0};
  bool lastLeftState_{false};
  bool lastRightState_{false};
  ButtonCallback onLeftPressed_;
  ButtonCallback onRightPressed_;
};
