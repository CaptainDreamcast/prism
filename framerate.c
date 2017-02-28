#include "include/framerate.h"

#include "include/log.h"

// TODO: Refactor into system

Framerate gFramerate;

void setFramerate(Framerate tFramerate) {
  log("Set framerate");
  logInteger(tFramerate);
  gFramerate = tFramerate;
}

double getFramerateFactor() {
  if (!gFramerate) return 1;
  else {
    return 60.0 / gFramerate;
  }
}

double getInverseFramerateFactor() {
  return 1.0 / getFramerateFactor();
}
