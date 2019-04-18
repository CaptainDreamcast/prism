#pragma once

#include <stdint.h>

#include "system.h"

#define timeAction(x) { \
	uint64_t __timingStart = getSystemTicks(); \
	x; \
	printf("Ticks spent executing %s: %llu\n", #x, getSystemTicks() - __timingStart); \
}
