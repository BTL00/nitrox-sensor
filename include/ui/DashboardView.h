#pragma once

#include <Arduino.h>
#include <Adafruit_ST7789.h>

#include "sensor/IUs1010Sensor.h"

class DashboardView {
 public:
  explicit DashboardView(Adafruit_ST7789& tft) : tft_(tft) {}

  void begin();
  void render(const SensorReading& reading, float batteryVoltage, const String& statusText);

 private:
  void drawGauge(float value);
  void drawText(int x, int y, uint16_t color, uint8_t size, const String& text);
  void drawGaugeFace();
  void drawGaugeNeedle(float value, uint16_t color);

  Adafruit_ST7789& tft_;
  uint32_t lastDrawMs_{0};
};
