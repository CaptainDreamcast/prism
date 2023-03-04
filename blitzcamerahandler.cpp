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
	Position2D mEffectOffset;

	GeoRectangle2D mCameraRange;
} gBlitzCameraHandlerData;

static void loadBlitzCameraHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();

	gBlitzCameraHandlerData.mCameraPosition = Vector3D(0, 0, 0); 
	gBlitzCameraHandlerData.mScale = Vector3D(1, 1, 1);
	gBlitzCameraHandlerData.mAngle = 0;
	ScreenSize sz = getScreenSize();
	gBlitzCameraHandlerData.mEffectOffset = Vector2D(sz.x / 2, sz.y / 2);
	gBlitzCameraHandlerData.mCameraRange = GeoRectangle2D(-INF / 2, - INF / 2, INF, INF);
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

void setBlitzCameraHandlerPosition(const Position& tPos)
{
	gBlitzCameraHandlerData.mCameraPosition = Vector3D(tPos.x, tPos.y, 0);
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
	gBlitzCameraHandlerData.mScale = Vector3D(tScale, tScale, 1);
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

Position2D * getBlitzCameraHandlerEffectPositionReference()
{
	return &gBlitzCameraHandlerData.mEffectOffset;
}

void setBlitzCameraHandlerEffectPositionOffset(const Position2D& tPosition)
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

void setBlitzCameraHandlerRange(const GeoRectangle2D& tRectangle)
{
	const auto sz = getScreenSize();
	auto finalRectangle = tRectangle;
	finalRectangle.mBottomRight = finalRectangle.mBottomRight - Vector2D(sz.x, sz.y);
	gBlitzCameraHandlerData.mCameraRange = finalRectangle;
}

void setBlitzCameraPositionBasedOnCenterPoint(const Position& tCenter)
{
	const auto sz = getScreenSize();
	const auto topLeft = vecSub(tCenter, Vector3D((sz.x / 2) * gBlitzCameraHandlerData.mScale.x, (sz.y / 2) * gBlitzCameraHandlerData.mScale.y, 0));
	setBlitzCameraHandlerPosition(topLeft);
}
