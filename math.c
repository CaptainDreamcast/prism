#include "math.h"

#include <stdlib.h>

double randfrom(double tMin, double tMax) {
	double range = (tMax - tMin); 
	if(range == 0) return tMin;

	double div = RAND_MAX / range;
	return tMin + (rand() / div);
}
