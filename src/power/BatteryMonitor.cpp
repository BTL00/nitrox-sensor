#include "power/BatteryMonitor.h"

void BatteryMonitor::begin() {
  pinMode(cfg_.adcPin, INPUT);
}

float BatteryMonitor::readVoltage() const {
  const int raw = analogRead(cfg_.adcPin);
  const float adcV = (static_cast<float>(raw) / static_cast<float>(cfg_.adcResolution)) * cfg_.vRef;
  return adcV * cfg_.dividerRatio;
}

int BatteryMonitor::readPercent() const {
  const float v = readVoltage();
  const float normalized = (v - cfg_.minBatteryV) / (cfg_.maxBatteryV - cfg_.minBatteryV);
  int pct = static_cast<int>(normalized * 100.0f);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return pct;
}
