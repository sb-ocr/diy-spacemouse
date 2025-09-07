#include "buttons.h"
#include "hid.h"
#include "magnet.h"
#include <OneButton.h>
#include <stdint.h>

extern "C" {
  #include "pico/bootrom.h"
}

static OneButton buttonLeft(24, true);
static OneButton buttonRight(27, true);

static constexpr uint32_t SINGLE_SQUEEZE_WINDOW_MS = 150;  // left+right single within this gap
static constexpr uint32_t DOUBLE_SQUEEZE_WINDOW_MS = 350;  // left+right double within this gap
static constexpr uint32_t BOTH_HOLD_MS             = 5000; // 5 sec squeeze
static constexpr uint32_t SINGLE_LONG_MS = 800;  // per-button long-press threshold


static inline void doLeftClick() {if (calibratingMagnets) { resetRange(); } else { homeView(); }}
static inline void doRightClick()       { centereView(); }
static inline void doLeftDoubleClick()  { setFunction(ZOOM); }
static inline void doRightDoubleClick() { setFunction(TILT); }
static inline void doSingleSqueeze()    { setFunction(PAN); }
static inline void doDoubleSqueeze()    { togglePressureSense(); }
static inline void doFiveSecondSqueeze(){ reset_usb_boot(0,0); }
static inline void doLeftLongPress()    { toggleCalibration(); }
static inline void doRightLongPress()   { /* your action */ }

static inline bool timesWithin(uint32_t a, uint32_t b, uint32_t windowMs) {
  // wrap-safe closeness check without cryptic math
  return (uint32_t)(a - b) <= windowMs || (uint32_t)(b - a) <= windowMs;
}

struct PendingSingles {
  bool leftPending = false;
  bool rightPending = false;
  uint32_t leftTimeMs = 0;
  uint32_t rightTimeMs = 0;

  void markLeft(uint32_t nowMs)  { leftPending  = true; leftTimeMs  = nowMs; }
  void markRight(uint32_t nowMs) { rightPending = true; rightTimeMs = nowMs; }

  void process(uint32_t nowMs) {
    if (leftPending && rightPending && timesWithin(leftTimeMs, rightTimeMs, SINGLE_SQUEEZE_WINDOW_MS)) {
      leftPending = rightPending = false;
      doSingleSqueeze();
      return;
    }
    if (leftPending && !rightPending && (uint32_t)(nowMs - leftTimeMs) > SINGLE_SQUEEZE_WINDOW_MS) {
      leftPending = false;
      doLeftClick();
    }
    if (rightPending && !leftPending && (uint32_t)(nowMs - rightTimeMs) > SINGLE_SQUEEZE_WINDOW_MS) {
      rightPending = false;
      doRightClick();
    }
  }

  void reset() { leftPending = rightPending = false; }
};

struct PendingDoubles {
  bool leftPending = false;
  bool rightPending = false;
  uint32_t leftTimeMs = 0;
  uint32_t rightTimeMs = 0;

  void markLeft(uint32_t nowMs)  { leftPending  = true; leftTimeMs  = nowMs; }
  void markRight(uint32_t nowMs) { rightPending = true; rightTimeMs = nowMs; }

  void process(uint32_t nowMs) {
    if (leftPending && rightPending && timesWithin(leftTimeMs, rightTimeMs, DOUBLE_SQUEEZE_WINDOW_MS)) {
      leftPending = rightPending = false;
      doDoubleSqueeze();
      return;
    }
    if (leftPending && !rightPending && (uint32_t)(nowMs - leftTimeMs) > DOUBLE_SQUEEZE_WINDOW_MS) {
      leftPending = false;
      doLeftDoubleClick();
    }
    if (rightPending && !leftPending && (uint32_t)(nowMs - rightTimeMs) > DOUBLE_SQUEEZE_WINDOW_MS) {
      rightPending = false;
      doRightDoubleClick();
    }
  }

  void reset() { leftPending = rightPending = false; }
};

static PendingSingles pendingSingles;
static PendingDoubles pendingDoubles;

// -------- 5 sec squeeze (both long) --------
static bool bothButtonsHeld(uint32_t holdMs){
  static uint32_t holdStartMs = 0;
  if (buttonLeft.isLongPressed() && buttonRight.isLongPressed()) {
    if (!holdStartMs) holdStartMs = millis();
    if ((uint32_t)(millis() - holdStartMs) >= holdMs) { holdStartMs = 0; return true; }
  } else {
    holdStartMs = 0;
  }
  return false;
}

// -------- OneButton callbacks --------
// Singles are buffered to allow single squeeze detection
static void onLeftSingle()        { pendingSingles.markLeft(millis()); }
static void onRightSingle()       { pendingSingles.markRight(millis()); }

// Doubles are buffered to allow double squeeze detection
static void onLeftDouble()        { pendingDoubles.markLeft(millis()); }
static void onRightDouble()       { pendingDoubles.markRight(millis()); }

static void onLeftLongStart()  {
  if (buttonRight.isLongPressed()) return;
  doLeftLongPress();
}
static void onRightLongStart() {
  if (buttonLeft.isLongPressed()) return;
  doRightLongPress();
}


void setupButtons() {
  buttonLeft.attachClick(onLeftSingle);
  buttonRight.attachClick(onRightSingle);

  buttonLeft.attachDoubleClick(onLeftDouble);
  buttonRight.attachDoubleClick(onRightDouble);

  buttonLeft.setPressTicks(SINGLE_LONG_MS);
  buttonRight.setPressTicks(SINGLE_LONG_MS);
  buttonLeft.attachLongPressStart(onLeftLongStart);
  buttonRight.attachLongPressStart(onRightLongStart);
}

void buttonsTick() {
  buttonLeft.tick();
  buttonRight.tick();

  const uint32_t nowMs = millis();
  pendingSingles.process(nowMs);
  pendingDoubles.process(nowMs);

  if (bothButtonsHeld(BOTH_HOLD_MS)) {
    doFiveSecondSqueeze();
  }
}