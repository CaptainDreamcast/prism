#include "prism/framerate.h"

#include "prism/log.h"

// TODO: Refactor into system

Framerate gFramerate;

void setFramerate(Framerate tFramerate) {
  logg("Set framerate");
  logInteger(tFramerate);
  gFramerate = tFramerate;
}

Framerate getFramerate()
{
	if (!gFramerate) return SIXTY_HERTZ;

	return gFramerate;
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
