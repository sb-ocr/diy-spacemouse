#pragma once
#include "globals.h"

extern bool calibratingMagnets;

void calibrate();
void resetRange();
void toggleCalibration();
int  getRotation();
bool isRotating();
magnetCoordinates getMagnet();
const sensorRange getMagnetRange();
void setupMagnets();
