#pragma once

typedef struct {
	void(*mUnregisterEntity)(int tEntityID);
} BlitzComponent;

BlitzComponent makeBlitzComponent(void(*tUnregisterEntity)(int tEntityID));