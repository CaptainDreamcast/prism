#include "prism/system.h"

#include "prism/wrapper.h"
#include "prism/log.h"

static struct {
	Framerate mFramerate;
} gSystemDataGeneral;

void recoverFromError()
{
	if (isUsingWrapper()) {
		recoverWrapperError();
	}
	else {
		abortSystem();
	}
}

void setFramerate(Framerate tFramerate) {
	logg("Set framerate");
	logInteger(tFramerate);
	gSystemDataGeneral.mFramerate = tFramerate;
}

Framerate getFramerate()
{
	if (!gSystemDataGeneral.mFramerate) return SIXTY_HERTZ;

	return gSystemDataGeneral.mFramerate;
}

double getFramerateFactor() {
	if (!gSystemDataGeneral.mFramerate) return 1;
	else {
		return 60.0 / gSystemDataGeneral.mFramerate;
	}
}

double getInverseFramerateFactor() {
	return 1.0 / getFramerateFactor();
}
