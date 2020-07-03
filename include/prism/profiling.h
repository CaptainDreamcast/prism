#pragma once

#include <stdint.h>

// set active to enable profiling
// #define PRISM_PROFILING_ACTIVE

#ifdef PRISM_PROFILING_ACTIVE
#include <optick.h>
#endif

#include "system.h"

#define timeAction(x) { \
	uint64_t __timingStart = getSystemTicks(); \
	x; \
	printf("Ticks spent executing %s: %llu\n", #x, getSystemTicks() - __timingStart); \
}

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

#ifdef PRISM_PROFILING_ACTIVE
void initProfiling();
void shutdownProfiling();

// these functions don't appear to work with non-static chars
#define setProfilingFrameMarker(tThreadName) OPTICK_FRAME(tThreadName);
#define setProfilingSectionMarker(tSectionName) OPTICK_EVENT(tSectionName);
#define setProfilingSectionMarkerCurrentFunction() OPTICK_EVENT();

#define startProfilingCapture() OPTICK_START_CAPTURE();
#define stopProfilingCapture() OPTICK_STOP_CAPTURE();
#define saveProfilingCapture(name) OPTICK_SAVE_CAPTURE(name);
#else
inline void initProfiling() {}
inline void shutdownProfiling() {}

inline void setProfilingFrameMarker(const char*) {}
inline void setProfilingSectionMarker(const char*) {}
#define setProfilingSectionMarkerCurrentFunction() {}

#define startProfilingCapture() {}
#define stopProfilingCapture() {}
inline void saveProfilingCapture(const char*) {}
#endif