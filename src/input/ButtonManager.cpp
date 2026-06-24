#include "input/ButtonManager.h"

void ButtonManager::begin() {
  pinMode(cfg_.leftPin, cfg_.activeLow ? INPUT_PULLUP : INPUT);
  pinMode(cfg_.rightPin, cfg_.activeLow ? INPUT_PULLUP : INPUT);

  lastLeftState_ = isPressed(cfg_.leftPin);
  lastRightState_ = isPressed(cfg_.rightPin);
}

void ButtonManager::poll() {
  const uint32_t now = millis();
  const bool leftState = isPressed(cfg_.leftPin);
  const bool rightState = isPressed(cfg_.rightPin);

  if (leftState != lastLeftState_ && now - lastLeftEdge_ > cfg_.debounceMs) {
    lastLeftEdge_ = now;
    lastLeftState_ = leftState;
    if (leftState && onLeftPressed_) {
      onLeftPressed_();
    }
  }

  if (rightState != lastRightState_ && now - lastRightEdge_ > cfg_.debounceMs) {
    lastRightEdge_ = now;
    lastRightState_ = rightState;
    if (rightState && onRightPressed_) {
      onRightPressed_();
    }
  }
}

bool ButtonManager::isPressed(int pin) const {
  const int raw = digitalRead(pin);
  return cfg_.activeLow ? (raw == LOW) : (raw == HIGH);
}
