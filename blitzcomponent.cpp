#include <prism/blitzcomponent.h>

BlitzComponent makeBlitzComponent(void(*tUnregisterEntity)(int tEntityID)) {
	BlitzComponent ret;
	ret.mUnregisterEntity = tUnregisterEntity;
	return ret;
}