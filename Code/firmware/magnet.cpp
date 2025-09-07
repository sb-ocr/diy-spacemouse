#include "magnet.h"
#include "globals.h"
#include "usb.h"
#include "TLx493D_inc.hpp"
#include <SimpleKalmanFilter.h>
#include <ArduinoLog.h>

using namespace ifx::tlx493d;

TLx493D_A1B6 mag(Wire1, TLx493D_IIC_ADDR_A0_e);

SimpleKalmanFilter xFilter(1, 1, 0.2), yFilter(1, 1, 0.2), zFilter(1, 1, 0.2);

const uint8_t POWER_PIN = 15;

bool calibratingMagnets = false;

sensorRange range = {
  .xMax = 2.0f,
  .xMin = -2.5f,
  .yMax = 2.9f,
  .yMin = -1.0f,
  .zMax = 3.0f,
  .zMin = -14.0f
};

float xOffset = 0, yOffset = 0, zOffset = 0;
float xCurrent = 0, yCurrent = 0, zCurrent = 0;

int calSamples = 50;

void setupMagnets() {
  double x, y, z;

  Wire1.begin();

  // mag sensor init
  mag.setPowerPin(POWER_PIN, OUTPUT, INPUT, HIGH, LOW, 0, 250000);
  mag.begin();

  for (int i = 1; i <= calSamples; i++) {
    mag.getMagneticField(&x, &y, &z);

    xOffset += x;
    yOffset += y;
    zOffset += z;
  }

  xOffset = xOffset / calSamples;
  yOffset = yOffset / calSamples;
  zOffset = zOffset / calSamples;
}

void calibrate() {
  double x, y, z;
  mag.getMagneticField(&x, &y, &z);

  xCurrent = xFilter.updateEstimate(x - xOffset);
  yCurrent = yFilter.updateEstimate(y - yOffset);
  zCurrent = zFilter.updateEstimate(z - zOffset);

  range.xMax = (xCurrent > range.xMax) ? xCurrent : range.xMax;
  range.xMin = (xCurrent < range.xMin) ? xCurrent : range.xMin;
  range.yMax = (yCurrent > range.yMax) ? yCurrent : range.yMax;
  range.yMin = (yCurrent < range.yMin) ? yCurrent : range.yMin;
  range.zMax = (zCurrent > range.zMax) ? zCurrent : range.zMax;
  range.zMin = (zCurrent < range.zMin) ? zCurrent : range.zMin;

  Log.notice("Calibrating: X (%F, %F), Y (%F, %F), Z (%F, %F)\n", range.xMax, range.xMin, range.yMax, range.yMin, range.zMax, range.zMin);
}

void resetRange() {
  if (calibratingMagnets) {
    range.xMax = 0;
    range.xMin = 0;
    range.yMax = 0;
    range.yMin = 0;
    range.zMax = 0;
    range.zMin = 0;
  }
}

void toggleCalibration() {
  calibratingMagnets = !calibratingMagnets;
  if (calibratingMagnets) {
    Log.notice("Calibration Mode ON\n");
  } else {
    Log.notice("Calibration Mode OFF\n");
  }
}

const sensorRange getMagnetRange() {
  return range;
}

magnetCoordinates getMagnet() {
  double x, y, z;
  mag.getMagneticField(&x, &y, &z);

  xCurrent = xFilter.updateEstimate(x - xOffset);
  yCurrent = yFilter.updateEstimate(y - yOffset);
  zCurrent = zFilter.updateEstimate(z - zOffset);

  magnetCoordinates result;
  result.x = xCurrent;
  result.y = yCurrent;
  result.z = zCurrent;
  
  return result;
} 
