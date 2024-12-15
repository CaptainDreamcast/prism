#pragma once

namespace prism {

typedef struct {
	void(*mUnregisterEntity)(int tEntityID);
} BlitzComponent;

BlitzComponent makeBlitzComponent(void(*tUnregisterEntity)(int tEntityID));

}