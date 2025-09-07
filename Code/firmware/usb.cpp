#include "usb.h"

Adafruit_USBD_MSC usb_msc;

Adafruit_USBD_HID usb_hid;

// define key arrays here so we have HID key macros in scope
uint8_t key_none[6] = {HID_KEY_NONE};
uint8_t key_h[6]    = {HID_KEY_H};

void setupUSB() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  usb_msc.begin();

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
}

void usb_task_if_needed() {
#ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
#endif
}

bool usb_is_mounted() {
  return TinyUSBDevice.mounted();
}
