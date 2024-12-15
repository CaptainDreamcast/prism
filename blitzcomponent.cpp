#include <prism/blitzcomponent.h>

namespace prism {

	BlitzComponent makeBlitzComponent(void(*tUnregisterEntity)(int tEntityID)) {
		BlitzComponent ret;
		ret.mUnregisterEntity = tUnregisterEntity;
		return ret;
	}

}