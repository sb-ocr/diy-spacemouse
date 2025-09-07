#include "usb.h"
#include "hid.h"
#include "buttons.h"
#include "magnet.h"
#include "statusLED.h"
#include "globals.h"
#include <ArduinoLog.h>

void setup() {
  Serial.begin(115200);
  
  setupUSB();
  setupHid();
  setupMagnets();
  setupButtons();
  setupLED();

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
}

void useHIDEmulation(magnetCoordinates motion) {
  sensorRange rangeValues= getMagnetRange();
  hidMappedCoordinates hidCoordinates = clampToHID(motion, rangeValues);

  // Log.notice("reported: (%F, %F, %F) clamped: (%d, %d, %d)\n", motion.x, motion.y, motion.z, hidCoordinates.x, hidCoordinates.y, hidCoordinates.z);

  motionTick();
  if (hidCoordinates.x != 0 || hidCoordinates.y != 0) {
    move(hidCoordinates);
  }
}

void loop() {
  if (!usb_is_mounted()) {
    return;
  }

  usb_task_if_needed();

  buttonsTick();

  if (calibratingMagnets) {
    calibrate();
  } else {
    magnetCoordinates motion = getMagnet();
    useHIDEmulation(motion);
  }
}