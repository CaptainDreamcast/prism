#include "include/input.h"

#include "include/log.h"

void resetInput() {
	hasPressedAFlank();
	hasPressedBFlank();
	hasPressedXFlank();
	hasPressedYFlank();
	hasPressedLFlank();
	hasPressedRFlank();
	hasPressedLeftFlank();
	hasPressedRightFlank();
	hasPressedUpFlank();
	hasPressedDownFlank();
	hasPressedStartFlank();
	hasPressedAbortFlank();
}

static int hasPressedFlank(int tCurrent, int* tFlank) {
	int returnValue = 0;

	debugInteger(tCurrent);
	debugInteger((*tFlank));
	if (tCurrent && !(*tFlank)) {
		returnValue = 1;
	}

	(*tFlank) = tCurrent;
	return returnValue;
}

static int gAFlank = 0;
int hasPressedAFlank() {
	return hasPressedFlank(hasPressedA(), &gAFlank);
}

static int gBFlank = 0;
int hasPressedBFlank() {
	return hasPressedFlank(hasPressedB(), &gBFlank);
}

static int gXFlank = 0;
int hasPressedXFlank() {
	return hasPressedFlank(hasPressedX(), &gXFlank);
}

static int gYFlank = 0;
int hasPressedYFlank() {
	return hasPressedFlank(hasPressedY(), &gYFlank);
}

static int gLeftFlank = 0;
int hasPressedLeftFlank() {
	return hasPressedFlank(hasPressedLeft(), &gLeftFlank);
}

static int gRightFlank = 0;
int hasPressedRightFlank() {
	return hasPressedFlank(hasPressedRight(), &gRightFlank);
}

static int gUpFlank = 0;
int hasPressedUpFlank() {
	return hasPressedFlank(hasPressedUp(), &gUpFlank);
}

static int gDownFlank = 0;
int hasPressedDownFlank() {
	return hasPressedFlank(hasPressedDown(), &gDownFlank);
}

static int gLFlank = 0;
int hasPressedLFlank() {
	return hasPressedFlank(hasPressedL(), &gLFlank);
}

static int gRFlank = 0;
int hasPressedRFlank() {
	return hasPressedFlank(hasPressedR(), &gRFlank);
}

static int gStartFlank = 0;
int hasPressedStartFlank() {
	return hasPressedFlank(hasPressedStart(), &gStartFlank);
}

static int gAbortFlank = 0;
int hasPressedAbortFlank() {
	debugLog("check abort flank");
	return hasPressedFlank(hasPressedAbort(), &gAbortFlank);
}
