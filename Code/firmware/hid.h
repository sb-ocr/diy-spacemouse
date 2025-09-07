#pragma once
#include "globals.h"

struct hidMappedCoordinates {
  int16_t x;
  int16_t y;
  int16_t z;
};

void setupHid();
void setFunction(MotionState requested);
void togglePressureSense();
void centereView();
void homeView();
hidMappedCoordinates clampToHID(magnetCoordinates motion, sensorRange range);
void move(hidMappedCoordinates motion);
void motionTick();