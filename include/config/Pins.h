#pragma once

#include <Arduino.h>

namespace pins {
constexpr int TFT_BL = 4;
constexpr int TFT_CS = 5;
constexpr int TFT_DC = 16;
constexpr int TFT_RST = 23;
constexpr int TFT_MOSI = 19;
constexpr int TFT_SCLK = 18;

// TTGO T-Display buttons.
// On this board revision GPIO0 is the physical left button and GPIO35 is the right.
constexpr int BTN_LEFT = 0;
constexpr int BTN_RIGHT = 35;

// Battery ADC input (commonly GPIO34)
constexpr int BAT_ADC = 34;

// UART for external sensor (adjust as needed)
constexpr int US1010_RX = 26;
constexpr int US1010_TX = 27;
}  // namespace pins
