#pragma once
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <stdint.h>

enum {
  RID_KEYBOARD = 1,
  RID_MOUSE,
};

extern uint8_t const desc_hid_report[];
extern Adafruit_USBD_MSC usb_msc;
extern Adafruit_USBD_HID usb_hid;

// Keycode arrays belong to USB/HID domain
extern uint8_t key_none[6];
extern uint8_t key_h[6];

// moved USB init/task/mounted from .ino
void setupUSB();
void usb_task_if_needed();
bool usb_is_mounted();
