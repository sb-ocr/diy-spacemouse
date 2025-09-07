#include <Adafruit_NeoPixel.h>
#include "statusLED.h"
#include <ArduinoLog.h>

#define NUMPIXELS        1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

uint32_t defaultColor = pixels.Color(255, 0, 255); ;
uint32_t color = defaultColor ;

void setupLED() {
  #if defined(NEOPIXEL_POWER)
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);
  #endif

  pixels.begin();
  pixels.setBrightness(100);
  pixels.setPixelColor(0, color);
  pixels.show();
}

void setStatus(MotionState status) {

  switch (status) {
    case TILT: color = pixels.Color(255, 0, 0);  break; // Red
    case PAN:  color = pixels.Color(255, 255, 255);   break;// White
    case ZOOM: color = pixels.Color(0, 0, 255);   break;// Blue
    default:   color = defaultColor; break;// Magenta (fallback)
  }

  pixels.setPixelColor(0, color);
  pixels.show();
}

void LED_Off() {
  pixels.setPixelColor(0, pixels.Color(255, 0, 255));
  pixels.setBrightness(0);
}