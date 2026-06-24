#include "sensor/Us1010Sensor.h"

#include <stdlib.h>

void Us1010Sensor::begin() {
  uart_.begin(baud_);
  lineBuffer_.reserve(128);
}

void Us1010Sensor::poll() {
  while (uart_.available() > 0) {
    const int value = uart_.read();
    if (value < 0) {
      return;
    }

    const char c = static_cast<char>(value);
    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      if (!lineBuffer_.isEmpty()) {
        parseLine(lineBuffer_);
      }
      lineBuffer_.clear();
      continue;
    }

    if (lineBuffer_.length() < 127) {
      lineBuffer_ += c;
    } else {
      lineBuffer_.clear();
    }
  }
}

bool Us1010Sensor::parseLine(const String& line) {
  // Draft parser for mock data:
  // O2:20.9,F:2.40,T:24.3
  float o2 = 0.0f;
  float flow = 0.0f;
  float t = 0.0f;

  const int o2Idx = line.indexOf("O2:");
  int flowIdx = line.indexOf("F:");
  if (flowIdx < 0) {
    flowIdx = line.indexOf("P:");
  }
  const int tIdx = line.indexOf("T:");

  if (o2Idx < 0 || flowIdx < 0 || tIdx < 0) {
    return false;
  }

  const int o2End = line.indexOf(',', o2Idx);
  const int flowEnd = line.indexOf(',', flowIdx);
  const int tEnd = line.length();

  const String o2Str = line.substring(o2Idx + 3, o2End > 0 ? o2End : tEnd);
  const String flowStr = line.substring(flowIdx + 2, flowEnd > 0 ? flowEnd : tEnd);
  const String tStr = line.substring(tIdx + 2, tEnd);

  o2 = static_cast<float>(atof(o2Str.c_str()));
  flow = static_cast<float>(atof(flowStr.c_str()));
  t = static_cast<float>(atof(tStr.c_str()));

  latest_.o2Percent = o2;
  latest_.flowLpm = flow;
  latest_.temperatureC = t;
  latest_.valid = true;
  latest_.timestampMs = millis();
  return true;
}
