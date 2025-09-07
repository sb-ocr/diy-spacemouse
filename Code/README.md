# Firmware:

The Arduino code for your Pi Pico

## Buttons:

### Single Click:

- left: home view
- right: center view

### Single Squeeze (both at the same time):

Control Pan

### Double Click:

- left: Control Zoom
- right: Control Tilt

### Double Squeeze:

Toggle Pressure sensitivity: In this mode, pressing down will activate Tilt Control, and pulling up will activate Zoom Control.
If you prefer this mode as the default, set `pressureSense` to `true` in `hid.cpp` and flash your device.

### Long Press:

- left button: toggle Calibration mode: in this mode, the mouse will register the max X, Y, Z values as you move the stick around. A single click of the left button in this mode will set all the values to zero in case a "hard reset" is needed.
- 5 seconds long squeeze (both buttons): activates Firmware Update Mode. In this mode, the mouse will expose a USB drive to your computer (Pi Pico flashing mode), and dropping a new firmware on that drive will flash the mouse.

# Fusion360 Add-in

Support for the home button
