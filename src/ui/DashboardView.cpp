#include "ui/DashboardView.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <math.h>

namespace {
constexpr int kScreenW = 135;
constexpr int kScreenH = 240;
constexpr int kGaugeCx = 67;
constexpr int kGaugeCy = 230;
constexpr int kGaugeR = 86;
constexpr int kGaugeTop = 142;
constexpr int kGaugeHeight = 82;
constexpr int kO2ValueY = 188;
constexpr float kGaugeMin = 20.5f;
constexpr float kGaugeMax = 95.6f;
constexpr float kGaugeFocusMax = 40.0f;
constexpr float kGaugeSpanDeg = 220.0f;
constexpr float kGaugeFocusSpanDeg = 142.0f;
constexpr float kPointerAngleDeg = -90.0f;
constexpr uint16_t kDarkGrey = 0x39E7;
constexpr uint16_t kLightGrey = 0xBDF7;
constexpr uint16_t kSkyBlue = 0x867D;
constexpr uint16_t kSkyBlueBright = 0x5D9B;
constexpr uint16_t kPaleBlue = 0xD71C;
constexpr uint16_t kOrange = 0xFD20;
constexpr uint16_t kOrangeSoft = 0xFBE0;
constexpr int kMajorMarkerCount = 8;
constexpr int kMajorMarkers[kMajorMarkerCount] = {21, 28, 32, 36, 40, 50, 75, 95};

enum class ViewMode {
  None,
  Live,
  Calibration,
};

ViewMode lastViewMode = ViewMode::None;

float clampf(const float v, const float lo, const float hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

float valueToAngle(const float value) {
  const float clamped = clampf(value, kGaugeMin, kGaugeMax);
  float normalized = 0.0f;

  if (clamped <= kGaugeFocusMax) {
    const float local = (clamped - kGaugeMin) / (kGaugeFocusMax - kGaugeMin);
    normalized = (local * kGaugeFocusSpanDeg) / kGaugeSpanDeg;
  } else {
    const float local = (clamped - kGaugeFocusMax) / (kGaugeMax - kGaugeFocusMax);
    normalized = (kGaugeFocusSpanDeg + (local * (kGaugeSpanDeg - kGaugeFocusSpanDeg))) /
                 kGaugeSpanDeg;
  }

  return (-kGaugeSpanDeg * 0.5f) + (kGaugeSpanDeg * normalized);
}

bool isVisibleY(const int y) {
  return y >= kGaugeTop && y < kScreenH;
}

String formatMod(float o2Percent, float ppo2Max) {
  if (o2Percent <= 0.0f) {
    return "--m";
  }

  const float fraction = o2Percent / 100.0f;
  const float mod = (10.0f * ppo2Max / fraction) - 10.0f;
  const long rounded = lroundf(mod);
  return String(rounded) + "m";
}
}  // namespace

void DashboardView::begin() {
  tft_.init(135, 240);
  tft_.setRotation(2);
  tft_.fillScreen(ST77XX_BLACK);
  tft_.setTextWrap(false);
  drawGaugePointer();
}

void DashboardView::render(const SensorReading& reading, float batteryVoltage, const String& statusText) {
  const uint32_t now = millis();
  if (now - lastDrawMs_ < 150) {
    return;
  }
  lastDrawMs_ = now;

  static bool firstFrame = true;
  static String lastStatus;
  static String lastBattery;
  static String lastO2;
  static String lastMod14;
  static String lastMod16;
  static String lastPressure;
  static String lastTemp;
  static float lastGaugeValue = -1000.0f;

  if (lastViewMode != ViewMode::Live) {
    firstFrame = true;
    tft_.fillScreen(ST77XX_BLACK);
    lastViewMode = ViewMode::Live;
  }

  const bool lowBattery = batteryVoltage < 3.0f;
  const String statusValue = statusText;
  const String batteryText = String(batteryVoltage, 2) + "V";
  const String o2 = reading.valid ? String(reading.o2Percent, 1) + "%" : "--.-%";
  const String mod14 = reading.valid ? formatMod(reading.o2Percent, 1.4f) : "--m";
  const String mod16 = reading.valid ? formatMod(reading.o2Percent, 1.6f) : "--m";
  const String p = reading.valid ? String(reading.flowLpm, 2) + " L/min" : "--.-- L/min";
  const String t = reading.valid ? String(reading.temperatureC, 1) + " C" : "--.- C";
  const float gaugeValue = reading.valid ? reading.o2Percent : kGaugeMin;

  if (firstFrame || batteryText != lastBattery) {
    tft_.fillRect(0, 4, kScreenW, 14, ST77XX_BLACK);
    drawCenteredText(kScreenW / 2, 4, lowBattery ? kOrange : kSkyBlueBright, 2,
                     "BAT " + batteryText);
    lastBattery = batteryText;
  }

  if (firstFrame || p != lastPressure || t != lastTemp) {
    tft_.fillRect(0, 22, kScreenW, 48, ST77XX_BLACK);
    drawCenteredText(kScreenW / 2, 22, kOrange, 2, p);
    drawCenteredText(kScreenW / 2, 40, kOrange, 2, t);
    lastPressure = p;
    lastTemp = t;
  }

  if (firstFrame || statusValue != lastStatus) {
    tft_.fillRect(0, 72, kScreenW, 10, ST77XX_BLACK);
    drawCenteredText(kScreenW / 2, 72, kPaleBlue, 1, "STATUS " + statusValue);
    lastStatus = statusValue;
  }

  if (firstFrame || mod14 != lastMod14 || mod16 != lastMod16) {
    tft_.fillRect(0, 84, kScreenW, 40, ST77XX_BLACK);
    drawCenteredText(34, 84, kSkyBlueBright, 1, "MOD 1.4");
    drawCenteredText(101, 84, kSkyBlueBright, 1, "MOD 1.6");
    drawCenteredText(34, 98, kSkyBlueBright, 2, mod14);
    drawCenteredText(101, 98, kOrange, 2, mod16);
    lastMod14 = mod14;
    lastMod16 = mod16;
  }

  if (firstFrame || fabsf(gaugeValue - lastGaugeValue) >= 0.1f) {
    tft_.fillRect(0, kGaugeTop, kScreenW, kGaugeHeight, ST77XX_BLACK);
    drawGauge(gaugeValue);
    drawCenteredText(kScreenW / 2, kO2ValueY, kSkyBlueBright, 3, o2);
    drawGaugePointer();
    lastO2 = o2;
    lastGaugeValue = gaugeValue;
  }

  firstFrame = false;
}

void DashboardView::renderCalibration(const CalibrationInfo& calibrationInfo) {
  static bool firstFrame = true;
  static String lastSerial;
  static String lastDate;
  static String lastAir;
  static String lastNx32;
  static String lastNx36;
  static String lastOxygen;

  if (lastViewMode != ViewMode::Calibration) {
    firstFrame = true;
    tft_.fillScreen(ST77XX_BLACK);
    lastViewMode = ViewMode::Calibration;
  }

  const uint32_t now = millis();
  if (!firstFrame && now - lastDrawMs_ < 150) {
    return;
  }
  lastDrawMs_ = now;

  const String serialText = calibrationInfo.serialNumber;
  const String dateText = calibrationInfo.calibrationDate;
  const String airText = calibrationInfo.airOffset;
  const String nx32Text = calibrationInfo.nitrox32Offset;
  const String nx36Text = calibrationInfo.nitrox36Offset;
  const String oxygenText = calibrationInfo.oxygenOffset;
  if (firstFrame || serialText != lastSerial) {
    tft_.fillRect(0, 10, kScreenW, 18, ST77XX_BLACK);
    drawCenteredText(kScreenW / 2, 10, kSkyBlue, 1, "US1010 SERIAL");
    drawCenteredText(kScreenW / 2, 22, ST77XX_WHITE, 2, serialText);
    lastSerial = serialText;
  }

  if (firstFrame || dateText != lastDate) {
    tft_.fillRect(0, 52, kScreenW, 18, ST77XX_BLACK);
    drawCenteredText(kScreenW / 2, 52, kSkyBlue, 1, "CAL DATE");
    drawCenteredText(kScreenW / 2, 64, ST77XX_WHITE, 2, dateText);
    lastDate = dateText;
  }

  if (firstFrame || airText != lastAir || nx32Text != lastNx32 || nx36Text != lastNx36 ||
      oxygenText != lastOxygen) {
    tft_.fillRect(0, 102, kScreenW, 120, ST77XX_BLACK);
    drawCenteredText(34, 102, kSkyBlueBright, 1, "AIR");
    drawCenteredText(101, 102, kSkyBlueBright, 1, "EAN32");
    drawCenteredText(34, 118, ST77XX_WHITE, 2, airText);
    drawCenteredText(101, 118, ST77XX_WHITE, 2, nx32Text);
    drawCenteredText(34, 154, kOrange, 1, "EAN36");
    drawCenteredText(101, 154, kOrange, 1, "O2");
    drawCenteredText(34, 170, ST77XX_WHITE, 2, nx36Text);
    drawCenteredText(101, 170, ST77XX_WHITE, 2, oxygenText);
    drawCenteredText(kScreenW / 2, 212, kOrangeSoft, 1, "HOLD RIGHT TO VIEW");
    lastAir = airText;
    lastNx32 = nx32Text;
    lastNx36 = nx36Text;
    lastOxygen = oxygenText;
  }

  firstFrame = false;
}

void DashboardView::drawGauge(float value) {
  const float rotationOffset = kPointerAngleDeg - valueToAngle(value);
  int selectedMarkers[3] = {-1, -1, -1};
  float selectedDistances[3] = {1000.0f, 1000.0f, 1000.0f};

  for (int i = 0; i < kMajorMarkerCount; ++i) {
    const int marker = kMajorMarkers[i];
    if (marker == 21 || marker == 95) {
      continue;
    }
    const float distance = fabsf(static_cast<float>(marker) - value);

    if (distance < selectedDistances[0]) {
      selectedDistances[2] = selectedDistances[1];
      selectedMarkers[2] = selectedMarkers[1];
      selectedDistances[1] = selectedDistances[0];
      selectedMarkers[1] = selectedMarkers[0];
      selectedDistances[0] = distance;
      selectedMarkers[0] = marker;
    } else if (distance < selectedDistances[1]) {
      selectedDistances[2] = selectedDistances[1];
      selectedMarkers[2] = selectedMarkers[1];
      selectedDistances[1] = distance;
      selectedMarkers[1] = marker;
    } else if (distance < selectedDistances[2]) {
      selectedDistances[2] = distance;
      selectedMarkers[2] = marker;
    }
  }

  tft_.drawCircle(kGaugeCx, kGaugeCy, kGaugeR, kDarkGrey);
  tft_.drawCircle(kGaugeCx, kGaugeCy, kGaugeR - 1, kDarkGrey);

  for (int marker = 21; marker <= 95; ++marker) {
    const bool major = (marker == 21 || marker == 28 || marker == 32 || marker == 36 ||
                        marker == 40 || marker == 50 || marker == 75 || marker == 95);
    const float angle = (valueToAngle(static_cast<float>(marker)) + rotationOffset) * DEG_TO_RAD;
    const int inner = kGaugeR - (major ? 12 : 7);
    const int outer = kGaugeR - 2;
    const int x0 = kGaugeCx + static_cast<int>(cosf(angle) * inner);
    const int y0 = kGaugeCy + static_cast<int>(sinf(angle) * inner);
    const int x1 = kGaugeCx + static_cast<int>(cosf(angle) * outer);
    const int y1 = kGaugeCy + static_cast<int>(sinf(angle) * outer);

    if (isVisibleY(y0) || isVisibleY(y1)) {
      uint16_t tickColor = kLightGrey;
      if (marker >= 80) {
        tickColor = kOrange;
      } else if (marker >= 50) {
        tickColor = kOrangeSoft;
      } else if (marker >= 32) {
        tickColor = kOrangeSoft;
      } else if (marker < 24) {
        tickColor = kSkyBlue;
      } else {
        tickColor = kSkyBlueBright;
      }
      tft_.drawLine(x0, y0, x1, y1, tickColor);
    }

    if (major) {
      const int xl = kGaugeCx + static_cast<int>(cosf(angle) * (kGaugeR - 24));
      const int yl = kGaugeCy + static_cast<int>(sinf(angle) * (kGaugeR - 24));
      const String markerText = String(marker);
      const bool selected = (marker == selectedMarkers[0] || marker == selectedMarkers[1] ||
                             marker == selectedMarkers[2]);
      if (selected) {
        drawCenteredText(xl, yl - 3, ST77XX_WHITE, 1, markerText);
      }
    }
  }
}

void DashboardView::drawText(int x, int y, uint16_t color, uint8_t size, const String& text) {
  tft_.setTextSize(size);
  tft_.setTextColor(color, ST77XX_BLACK);
  tft_.setCursor(x, y);
  tft_.print(text);
}

void DashboardView::drawCenteredText(int centerX, int y, uint16_t color, uint8_t size, const String& text) {
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;

  tft_.setTextSize(size);
  tft_.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  drawText(centerX - static_cast<int>(w / 2), y, color, size, text);
}

void DashboardView::drawGaugePointer() {
  const int tipY = kGaugeTop + 2;
  tft_.fillTriangle(kGaugeCx, tipY, kGaugeCx - 5, tipY + 9, kGaugeCx + 5, tipY + 9, kOrange);
  tft_.drawFastHLine(kGaugeCx - 1, tipY + 11, 3, kOrange);
}
