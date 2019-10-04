#include "prism/blitzcamerahandler.h"

#include <cstring>

#include "prism/log.h"
#include "prism/math.h"
#include "prism/system.h"
#include "prism/geometry.h"

static struct {
	int mIsActive;
	Position mCameraPosition;
	Vector3D mScale;
	double mAngle;
	Position mEffectOffset;

	GeoRectangle mCameraRange;
} gBlitzCameraHandlerData;

static void loadBlitzCameraHandler(void* tData) {
	(void)tData;
	gBlitzCameraHandlerData.mCameraPosition = makePosition(0, 0, 0); 
	gBlitzCameraHandlerData.mScale = makePosition(1, 1, 1);
	gBlitzCameraHandlerData.mAngle = 0;
	ScreenSize sz = getScreenSize();
	gBlitzCameraHandlerData.mEffectOffset = makePosition(sz.x / 2, sz.y / 2, 0);
	gBlitzCameraHandlerData.mCameraRange = makeGeoRectangle(-INF / 2, - INF / 2, INF, INF);
	gBlitzCameraHandlerData.mIsActive = 1;
}

ActorBlueprint getBlitzCameraHandler()
{
	return makeActorBlueprint(loadBlitzCameraHandler);
}

int isBlitzCameraHandlerEnabled()
{
	return gBlitzCameraHandlerData.mIsActive;
}

Position * getBlitzCameraHandlerPositionReference()
{
	return &gBlitzCameraHandlerData.mCameraPosition;
}

Position getBlitzCameraHandlerPosition()
{
	return gBlitzCameraHandlerData.mCameraPosition;
}

void setBlitzCameraHandlerPosition(Position tPos)
{
	tPos.z = 0;
	gBlitzCameraHandlerData.mCameraPosition = tPos;
	gBlitzCameraHandlerData.mCameraPosition = clampPositionToGeoRectangle(gBlitzCameraHandlerData.mCameraPosition, gBlitzCameraHandlerData.mCameraRange);
}

void setBlitzCameraHandlerPositionX(double tX)
{
	gBlitzCameraHandlerData.mCameraPosition.x = tX;
	gBlitzCameraHandlerData.mCameraPosition = clampPositionToGeoRectangle(gBlitzCameraHandlerData.mCameraPosition, gBlitzCameraHandlerData.mCameraRange);
}

void setBlitzCameraHandlerPositionY(double tY)
{
	gBlitzCameraHandlerData.mCameraPosition.y = tY;
	gBlitzCameraHandlerData.mCameraPosition = clampPositionToGeoRectangle(gBlitzCameraHandlerData.mCameraPosition, gBlitzCameraHandlerData.mCameraRange);

}

Vector3D * getBlitzCameraHandlerScaleReference()
{
	return &gBlitzCameraHandlerData.mScale;
}

Vector3D getBlitzCameraHandlerScale()
{
	return gBlitzCameraHandlerData.mScale;
}

void setBlitzCameraHandlerScale2D(double tScale)
{
	gBlitzCameraHandlerData.mScale = makePosition(tScale, tScale, 1);
}

void setBlitzCameraHandlerScaleX(double tScaleX)
{
	gBlitzCameraHandlerData.mScale.x = tScaleX;
}

void setBlitzCameraHandlerScaleY(double tScaleY)
{
	gBlitzCameraHandlerData.mScale.y = tScaleY;
}

double * getBlitzCameraHandlerRotationZReference()
{
	return &gBlitzCameraHandlerData.mAngle;
}

double getBlitzCameraHandlerRotationZ()
{
	return gBlitzCameraHandlerData.mAngle;
}

void setBlitzCameraHandlerRotationZ(double tAngle)
{
	gBlitzCameraHandlerData.mAngle = tAngle;
}

Position * getBlitzCameraHandlerEffectPositionReference()
{
	return &gBlitzCameraHandlerData.mEffectOffset;
}

void setBlitzCameraHandlerEffectPositionOffset(Position tPosition)
{
	gBlitzCameraHandlerData.mEffectOffset = tPosition;
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
	gBlitzCameraHandlerData.mCameraRange = tRectangle;
}

void setBlitzCameraPositionBasedOnCenterPoint(Position tCenter)
{
	ScreenSize sz = getScreenSize();
	Position topLeft = vecSub(tCenter, makePosition((sz.x / 2) * gBlitzCameraHandlerData.mScale.x, (sz.y / 2) * gBlitzCameraHandlerData.mScale.y, 0));
	setBlitzCameraHandlerPosition(topLeft);
}
