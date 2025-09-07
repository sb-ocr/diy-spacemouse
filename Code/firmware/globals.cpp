#include "globals.h"

const char* motionStateToStr(MotionState s) {
  switch (s) {
    case TILT:    return "TILT";
    case PAN:     return "PAN";
    case ZOOM:    return "ZOOM";
    case SENSE:    return "SENSE";
    default:      return "UNKNOWN";
  }
}
