#include <TinyUSB_Mouse_and_Keyboard.h>
#include <OneButton.h>
#include <Tlv493d.h>
#include <SimpleKalmanFilter.h>

Tlv493d mag = Tlv493d();
SimpleKalmanFilter xFilter(1, 1, 0.2), yFilter(1, 1, 0.2), zFilter(1, 1, 0.2);

// Setup buttons
OneButton button1(27, true);
OneButton button2(24, true);

float xOffset = 0, yOffset = 0, zOffset = 0;
float xCurrent = 0, yCurrent = 0, zCurrent = 0;

int calSamples = 300;
int sensivity = 8;
int magRange = 3;
int outRange = 127;      // Max allowed in HID report
float xyThreshold = 0.4; // Center threshold

int inRange = magRange * sensivity;
float zThreshold = xyThreshold * 1.5;

bool isOrbit = false;

void setup()
{

  button1.attachClick(goHome);
  button1.attachLongPressStop(goHome);

  button2.attachClick(fitToScreen);
  button2.attachLongPressStop(fitToScreen);

  // mouse and keyboard init
  Mouse.begin();
  Keyboard.begin();

  Serial.begin(9600);
  Wire1.begin();

  // mag sensor init
  mag.begin(Wire1);
  mag.setAccessMode(mag.MASTERCONTROLLEDMODE);
  mag.disableTemp();

  // crude offset calibration on first boot
  for (int i = 1; i <= calSamples; i++)
  {

    delay(mag.getMeasurementDelay());
    mag.updateData();

    xOffset += mag.getX();
    yOffset += mag.getY();
    zOffset += mag.getZ();

    Serial.print(".");
  }

  xOffset = xOffset / calSamples;
  yOffset = yOffset / calSamples;
  zOffset = zOffset / calSamples;

  Serial.println();
  Serial.println(xOffset);
  Serial.println(yOffset);
  Serial.println(zOffset);
}

void loop()
{

  // keep watching the push buttons
  button1.tick();
  button2.tick();

  // get the mag data
  delay(mag.getMeasurementDelay());
  mag.updateData();

  // update the filters
  xCurrent = xFilter.updateEstimate(mag.getX() - xOffset);
  yCurrent = yFilter.updateEstimate(mag.getY() - yOffset);
  zCurrent = zFilter.updateEstimate(mag.getZ() - zOffset);

  // check the center threshold
  if (abs(xCurrent) > xyThreshold || abs(yCurrent) > xyThreshold)
  {

    int xMove = 0;
    int yMove = 0;

    // map the magnetometer xy to the allowed 127 range in HID repports
    xMove = map(xCurrent, -inRange, inRange, -outRange, outRange);
    yMove = map(yCurrent, -inRange, inRange, -outRange, outRange);

    // press shift to orbit in Fusion 360 if the pan threshold is not corssed (zAxis)
    if (abs(zCurrent) < zThreshold && !isOrbit)
    {
      Keyboard.press(KEY_LEFT_SHIFT);
      isOrbit = true;
    }

    // pan or orbit by holding the middle mouse button and moving propotionaly to the xy axis
    Mouse.press(MOUSE_MIDDLE);
    Mouse.move(yMove, xMove, 0);
  }
  else
  {

    // release the mouse and keyboard if within the center threshold
    Mouse.release(MOUSE_MIDDLE);
    Keyboard.releaseAll();
    isOrbit = false;
  }

  Serial.print(xCurrent);
  Serial.print(",");
  Serial.print(yCurrent);
  Serial.print(",");
  Serial.print(zCurrent);
  Serial.println();
}

// go to home view in Fusion 360 by pressing  (CMD + SHIFT + H) shortcut assigned to the custom Add-in command
void goHome()
{
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.write('h');

  delay(10);
  Keyboard.releaseAll();
  Serial.println("pressed home");
}

// fit to view by pressing the middle mouse button twice
void fitToScreen()
{
  Mouse.press(MOUSE_MIDDLE);
  Mouse.release(MOUSE_MIDDLE);
  Mouse.press(MOUSE_MIDDLE);
  Mouse.release(MOUSE_MIDDLE);

  Serial.println("pressed fit");
}
