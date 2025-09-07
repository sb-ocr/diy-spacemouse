#pragma once
#include <stdint.h>

enum MotionState {
  TILT,
  PAN,
  ZOOM,
  SENSE,
  UNKNOWN
};

struct magnetCoordinates { double x, y, z; };

struct sensorRange {
  float xMax, xMin;
  float yMax, yMin;
  float zMax, zMin;
};

const char* motionStateToStr(MotionState s);