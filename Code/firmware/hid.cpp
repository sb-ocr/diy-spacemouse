#include "hid.h"
#include "usb.h"
#include "statusLED.h"
#include <ArduinoLog.h>

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(RID_MOUSE))
};

MotionState function = PAN;
bool pressureSense = false;

static uint32_t middleButtonPressedLast = 0;
static uint32_t leftShiftKeyPressedLast = 0;

bool middlePressed = false;
bool leftShiftHeld = false;

int outRange = 127;      // Max allowed in HID report
float timeSlice = 0.05f;

void hidReleaseAll() {
  if (!usb_hid.ready()) return;

  uint8_t keys[6] = {0,0,0,0,0,0};
  usb_hid.keyboardReport(RID_KEYBOARD, 0, keys);

  // Mouse: no buttons, no motion, no wheel/pan
  usb_hid.mouseReport(RID_MOUSE, 0 /*buttons*/, 0 /*x*/, 0 /*y*/, 0 /*wheel*/, 0 /*pan*/);

  // usb_hid.sendReport16(RID_CONSUMER, 0);
}

void setupHid() {
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_NONE);
  usb_hid.setPollInterval(2);
  usb_hid.begin();
  setFunction(PAN);
}

void releaseMiddlebButton() {
  if (millis() - middleButtonPressedLast >= 200 && middlePressed) {
    usb_hid.mouseButtonRelease(RID_MOUSE);
    middlePressed = false;
  }
}

void releaseKeyboard() {
    // if (millis() - leftShiftKeyPressedLast >= 200 && leftShiftHeld) {
    if (leftShiftHeld) {
      usb_hid.keyboardReport(RID_KEYBOARD, 0, key_none);
      leftShiftHeld = false;
    }
}

void setFunction(MotionState requested) {
  if (requested != function) {
    hidReleaseAll();
    function = requested;
    setStatus(requested);
  }
}

void togglePressureSense() {
  pressureSense = !pressureSense;
}

void pressMiddleMouse () {
  if (millis() - middleButtonPressedLast > 200) {
    usb_hid.mouseButtonPress(RID_MOUSE, MOUSE_BUTTON_MIDDLE);
    middlePressed = true;
    delay(2);
  }

  middleButtonPressedLast = millis();
}

void holdLeftShift() {
  if (millis() - leftShiftKeyPressedLast >= 200) {
    usb_hid.keyboardReport(RID_KEYBOARD, KEYBOARD_MODIFIER_LEFTSHIFT, key_none);
    leftShiftHeld = true;
  }

   leftShiftKeyPressedLast = millis();
}

void applyHid(int x, int y) {
  if (usb_hid.ready()) {
    pressMiddleMouse();
    usb_hid.mouseReport(RID_MOUSE, MOUSE_BUTTON_MIDDLE, x, -y, 0, 0 );
  }
  releaseMiddlebButton();
}

void centereView() {
  usb_hid.mouseButtonPress(RID_MOUSE, MOUSE_BUTTON_MIDDLE);
  delay(2);
  usb_hid.mouseButtonRelease(RID_MOUSE);
  delay(2);
  usb_hid.mouseButtonPress(RID_MOUSE, MOUSE_BUTTON_MIDDLE);
  delay(2);
  usb_hid.mouseButtonRelease(RID_MOUSE);
}

void homeView() {
  usb_hid.keyboardReport(RID_KEYBOARD, KEYBOARD_MODIFIER_LEFTSHIFT+KEYBOARD_MODIFIER_LEFTGUI, key_h);
  delay(2);
  usb_hid.keyboardRelease(RID_KEYBOARD);
}

void pan(int x, int y) {
  applyHid(x, y);
}

void tilt(int x, int y) {
  holdLeftShift();
  applyHid(x, y);
  releaseKeyboard();
}

void zoom(int wheel) {
  releaseMiddlebButton();
  releaseKeyboard();
  usb_hid.mouseReport(RID_MOUSE, 0, 0, 0, wheel, 0);
  delay(2);
  usb_hid.mouseReport(RID_MOUSE, 0, 0, 0, 0, 0);
}

// map the magnetometer vlaues to the allowed 127 range in HID repports
int16_t clampToInt(double value, float min, float max) {
  int scaled  = (int)((float)outRange * timeSlice);

  int16_t result = 0;
  if (value > 0 ) {
    result = map(value, 0, max, 0, scaled);
  }

  if (value < 0 ) {
    result = map(value, min, 0, -scaled, 0);
  }
  
  return (result <= scaled && result >= -scaled) ? result : ((value > 0 ) ? scaled : -scaled);

}

hidMappedCoordinates clampToHID(magnetCoordinates motion, sensorRange range) {
  hidMappedCoordinates result;

  result.x = clampToInt(motion.x, range.xMin, range.xMax);
  result.y = clampToInt(motion.y, range.yMin, range.yMax);
  result.z = clampToInt(motion.z, range.zMin, range.zMax);

  return result;
}

void zControl(int z) {
  if (z < -1) {
    return setFunction(TILT);
  }

  if (z > 1) {
    return setFunction(ZOOM);
  }

  setFunction(PAN);
}

void move(hidMappedCoordinates motion) {
  if (pressureSense) {
    zControl(motion.z);
  }

  switch (function) {
    case PAN:   return pan(motion.x, motion.y);
    case TILT:  return tilt(motion.x, motion.y);
    case ZOOM:  return zoom((abs(motion.x) > abs(motion.y)) ? motion.x : motion.y);
    default:   return pan(motion.x, motion.y);
  }
}

void motionTick() {
  releaseMiddlebButton();
  releaseKeyboard();
}