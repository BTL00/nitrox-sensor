#pragma once

#include <Arduino.h>
#include <Adafruit_ST7789.h>

#include "sensor/IUs1010Sensor.h"

struct CalibrationInfo {
  String serialNumber;
  String calibrationDate;
  String airOffset{"+0.0%"};
  String nitrox32Offset{"+0.0%"};
  String nitrox36Offset{"+0.0%"};
  String oxygenOffset{"+0.0%"};
};

class DashboardView {
 public:
  explicit DashboardView(Adafruit_ST7789& tft) : tft_(tft) {}

  void begin();
  void render(const SensorReading& reading, float batteryVoltage, const String& statusText);
  void renderCalibration(const CalibrationInfo& calibrationInfo);

 private:
  void drawGauge(float value);
  void drawText(int x, int y, uint16_t color, uint8_t size, const String& text);
  void drawCenteredText(int centerX, int y, uint16_t color, uint8_t size, const String& text);
  void drawGaugePointer();

  Adafruit_ST7789& tft_;
  uint32_t lastDrawMs_{0};
};
