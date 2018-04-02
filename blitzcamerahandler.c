#include "prism/blitzcamerahandler.h"

#include "prism/log.h"
#include "prism/math.h"
#include "prism/system.h"
#include "prism/geometry.h"

static struct {
	int mIsActive;
	Position mCameraPosition;
	Vector3D mScale;
	double mAngle;

	GeoRectangle mCameraRange;
} gData;

static void loadBlitzCameraHandler(void* tData) {
	(void)tData;
	gData.mCameraPosition = makePosition(0, 0, 0); 
	gData.mScale = makePosition(1, 1, 1);
	gData.mAngle = 0;
	gData.mCameraRange = makeGeoRectangle(-INF / 2, - INF / 2, INF, INF);
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
	tPos.z = 0;
	gData.mCameraPosition = tPos;
	gData.mCameraPosition = clampPositionToGeoRectangle(gData.mCameraPosition, gData.mCameraRange);
}

void setBlitzCameraHandlerPositionX(double tX)
{
	gData.mCameraPosition.x = tX;
	gData.mCameraPosition = clampPositionToGeoRectangle(gData.mCameraPosition, gData.mCameraRange);
}

void setBlitzCameraHandlerPositionY(double tY)
{
	gData.mCameraPosition.y = tY;
	gData.mCameraPosition = clampPositionToGeoRectangle(gData.mCameraPosition, gData.mCameraRange);

}

Vector3D * getBlitzCameraHandlerScaleReference()
{
	return &gData.mScale;
}

void setBlitzCameraHandlerScale2D(double tScale)
{
	gData.mScale = makePosition(tScale, tScale, 1);
}

void setBlitzCameraHandlerScaleX(double tScaleX)
{
	gData.mScale.x = tScaleX;
}

void setBlitzCameraHandlerScaleY(double tScaleY)
{
	gData.mScale.y = tScaleY;
}

double * getBlitzCameraHandlerRotationZReference()
{
	return &gData.mAngle;
}

void setBlitzCameraHandlerRotationZ(double tAngle)
{
	gData.mAngle = tAngle;
}

int getBlitzCameraHandlerEntityID()
{
	if (!isBlitzCameraHandlerEnabled()) {
		logWarning("Requestion disabled camera handler entity ID.");
	}

	return -10;
}

void setBlitzCameraHandlerRange(GeoRectangle tRectangle)
{
	ScreenSize sz = getScreenSize();
	tRectangle.mBottomRight = vecSub(tRectangle.mBottomRight, makePosition(sz.x, sz.y, 0));
	gData.mCameraRange = tRectangle;
}

void setBlitzCameraPositionBasedOnCenterPoint(Position tCenter)
{
	ScreenSize sz = getScreenSize();
	Position topLeft = vecSub(tCenter, makePosition((sz.x / 2) * gData.mScale.x, (sz.y / 2) * gData.mScale.y, 0));
	setBlitzCameraHandlerPosition(topLeft);
}
