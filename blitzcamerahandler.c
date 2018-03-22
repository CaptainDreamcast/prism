#include "prism/blitzcamerahandler.h"

#include "prism/log.h"

static struct {
	int mIsActive;
	Position mCameraPosition;
} gData;

static void loadBlitzCameraHandler(void* tData) {
	(void)tData;
	gData.mCameraPosition = makePosition(0, 0, 0); 
	gData.mIsActive = 1;
}

ActorBlueprint BlitzCameraHandler = {
	.mLoad = loadBlitzCameraHandler,
};

int isBlitzCameraHandlerEnabled()
{
	return gData.mIsActive;
}

Position * getBlitzCameraHandlerPositionReference()
{
	return &gData.mCameraPosition;
}

Position getBlitzCameraHandlerPosition()
{
	return gData.mCameraPosition;
}

void setBlitzCameraHandlerPosition(Position tPos)
{
	gData.mCameraPosition = tPos;
}

void setBlitzCameraHandlerPositionX(double tX)
{
	gData.mCameraPosition.x = tX;
}

void setBlitzCameraHandlerPositionY(double tY)
{
	gData.mCameraPosition.y = tY;
}

int getBlitzCameraHandlerEntityID()
{
	if (!isBlitzCameraHandlerEnabled()) {
		logWarning("Requestion disabled camera handler entity ID.");
	}

	return -10;
}
