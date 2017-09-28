#include "tari/math.h"

#include <kos.h>

double fstsqrt(double x) {
	return fsqrt(x);
}

double cos(double x) {
	return fcos(x);
}

double sin(double x) {
	return fsin(x);
}

double exp(double x) {
	return x; // TODO: recompile toolchains + fix math stuff
}

double log(double x) {
	return x; // TODO: recompile toolchains + fix math stuff
}


double floor(double x) {
	return (int)x; // TODO: recompile toolchains + fix math stuff
}

double ceil(double x) {
	return floor(x) != x ? floor(x)+1 : x; // TODO: recompile toolchains + fix math stuff
}

double pow(double x, double y) {
	return x; // TODO: recompile toolchains + fix math stuff
}

double fmod(double x, double y) {
	return x; // TODO: recompile toolchains + fix math stuff
}


