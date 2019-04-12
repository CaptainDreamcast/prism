#pragma once

#include <string>
#include <functional>

#include "prism/actorhandler.h"
#include "prism/log.h"

int isInDevelopMode();

void initDebug();
ActorBlueprint getPrismDebug();

void setPrismDebugUpdateStartTime();
void setPrismDebugDrawingStartTime();
void setPrismDebugWaitingStartTime();

void addPrismDebugConsoleCommand(std::string tCommand, std::string(*tCB)(void* tCaller, std::string tCommandInput), void* tCaller = NULL);
void submitToPrismDebugConsole(std::string tText);

#define profile(x, tSamples) { \
	uint64_t _startTicks = getSystemTicks(); \
	for (int _i = 0; _i < tSamples; _i++) { \
		x; \
	} \
	uint64_t _endTicks = getSystemTicks(); \
	uint64_t _timeDelta = _endTicks - _startTicks; \
	double _timePerSample = _timeDelta / double(tSamples); \
	logFormat("Profiling %s: %f ticks (%d samples running for %llu from %llu to %llu)", #x, _timePerSample, tSamples, _timeDelta, _startTicks, _endTicks); \
}

