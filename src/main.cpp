#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <math.h>

#include "config/Pins.h"
#include "hal/HardwareUartAdapter.h"
#include "hal/MockUart.h"
#include "input/ButtonManager.h"
#include "power/BatteryMonitor.h"
#include "sensor/Us1010Sensor.h"
#include "ui/DashboardView.h"

namespace {
Adafruit_ST7789 tft(pins::TFT_CS, pins::TFT_DC, pins::TFT_RST);

HardwareSerial& sensorSerial = Serial2;
HardwareUartAdapter hwUart(sensorSerial, pins::US1010_RX, pins::US1010_TX);
MockUart mockUart;

#if USE_MOCK_SENSOR
IUart& activeUart = mockUart;
#else
IUart& activeUart = hwUart;
#endif

Us1010Sensor sensor(activeUart, 9600);

BatteryMonitor::Config makeBatteryConfig() {
  BatteryMonitor::Config cfg;
  cfg.adcPin = pins::BAT_ADC;
  cfg.vRef = 3.3f;
  cfg.adcResolution = 4095;
  cfg.dividerRatio = 2.0f;
  cfg.minBatteryV = 3.30f;
  cfg.maxBatteryV = 4.20f;
  return cfg;
}

ButtonManager::Config makeButtonConfig() {
  ButtonManager::Config cfg;
  cfg.leftPin = pins::BTN_LEFT;
  cfg.rightPin = pins::BTN_RIGHT;
  cfg.activeLow = true;
  cfg.debounceMs = 40;
  return cfg;
}

BatteryMonitor battery(makeBatteryConfig());
ButtonManager buttons(makeButtonConfig());

DashboardView dashboard(tft);

CalibrationInfo makeCalibrationInfo() {
  CalibrationInfo info;
  info.serialNumber = "US1010-0001";
  info.calibrationDate = "2026-06-24";
  info.airOffset = "+0.0%";
  info.nitrox32Offset = "+1.2%";
  info.nitrox36Offset = "-0.5%";
  info.oxygenOffset = "+0.8%";
  return info;
}

const CalibrationInfo calibrationInfo = makeCalibrationInfo();

bool freezeDisplay = false;
String statusText = "live";
uint32_t lastMockPushMs = 0;

void pushMockFrame() {
#if USE_MOCK_SENSOR
  const float tSec = millis() / 1000.0f;

  // O2 smoothly sweeps from 19% to 41%.
  const float o2 = 30.0f + 11.0f * sinf((2.0f * PI / 24.0f) * tSec);
  const float flow = 2.4f + 1.1f * sinf((2.0f * PI / 18.0f) * tSec + 0.8f);
  const float t = 24.0f + 1.2f * sinf((2.0f * PI / 30.0f) * tSec + 1.4f);

  String line = "O2:" + String(o2, 1) + ",F:" + String(flow, 2) + ",T:" + String(t, 1) + "\n";
  mockUart.injectRx(line);
#else
  (void)sensor;
#endif
}
}  // namespace

void setup() {
  Serial.begin(115200);

  SPI.begin(pins::TFT_SCLK, -1, pins::TFT_MOSI, pins::TFT_CS);

  pinMode(pins::TFT_BL, OUTPUT);
  digitalWrite(pins::TFT_BL, HIGH);

  battery.begin();
  buttons.begin();
  sensor.begin();
  dashboard.begin();

  buttons.onLeftPressed([] {
    freezeDisplay = !freezeDisplay;
    statusText = freezeDisplay ? "frozen" : "live";
  });
}

void loop() {
  buttons.poll();
  const bool showCalibration = buttons.isRightPressed();

  if (millis() - lastMockPushMs >= 1000) {
    pushMockFrame();
    lastMockPushMs = millis();
  }

  sensor.poll();

  if (showCalibration) {
    dashboard.renderCalibration(calibrationInfo);
  } else if (!freezeDisplay) {
    const SensorReading reading = sensor.latest();
    const float batteryV = battery.readVoltage();
    dashboard.render(reading, batteryV, statusText);
  }

  delay(10);
}
