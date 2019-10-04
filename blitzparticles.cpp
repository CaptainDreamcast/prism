#include "prism/blitzparticles.h"

#include "prism/math.h"
#include "prism/datastructures.h"
#include "prism/blitzcamerahandler.h"

typedef struct {
	Position mPos;
	Velocity mVel;
	Velocity mGravity;

	double mAngle;
	Vector3D mColor;

	Duration mNow;
	Duration mLifeTime;

} ParticleEntry;


static struct {
	TextureData mWhiteTexture;
	IntMap mParticles;
} gBlitzParticlesData;


static void loadParticleHandler(void* tData) {
	(void)tData;
	gBlitzParticlesData.mWhiteTexture = createWhiteTexture();
	gBlitzParticlesData.mParticles = new_int_map();
}

static int updateSingleParticle(void* tCaller, void* tData) {
	(void)tCaller;
	ParticleEntry* e = (ParticleEntry*)tData;

	e->mVel = vecAdd(e->mVel, e->mGravity);
	e->mPos = vecAdd(e->mPos, e->mVel);

	if (handleDurationAndCheckIfOver(&e->mNow, e->mLifeTime)) {
		return 1;
	}

	return 0;
}

static void updateParticleHandler(void* tData) {
	(void)tData;
	int_map_remove_predicate(&gBlitzParticlesData.mParticles, updateSingleParticle, NULL);
}

static void drawSingleParticle(void* tCaller, void* tData) {
	(void)tCaller;
	ParticleEntry* e = (ParticleEntry*)tData;

	Position pos = vecSub(e->mPos, getBlitzCameraHandlerPosition());

	setDrawingRotationZ(e->mAngle, vecAdd(pos, makePosition(0.5, 0.5, 0)));
	setDrawingBaseColorAdvanced(e->mColor.x, e->mColor.y, e->mColor.z);
	drawSprite(gBlitzParticlesData.mWhiteTexture, pos, makeRectangle(0, 0, 1, 1));
	setDrawingBaseColorAdvanced(1, 1, 1);
	setDrawingRotationZ(-e->mAngle, vecAdd(pos, makePosition(0.5, 0.5, 0)));
}

static void drawParticleHandler(void* tData) {
	(void)tData;
	int_map_map(&gBlitzParticlesData.mParticles, drawSingleParticle, NULL);
}

ActorBlueprint getBlitzParticleHandler() {
	return makeActorBlueprint(loadParticleHandler, NULL, updateParticleHandler, drawParticleHandler);
};

void addBlitzParticles(int tAmount, Position tPosition, Position tPositionRange, double tSpeed, double tSpeedRange, double tAngle, double tAngleRange, Velocity tGravity, Vector3D tColor, Vector3D tColorRange, Duration tLifetime, Duration tLifetimeRange) {
	int i;
	for (i = 0; i < tAmount; i++) {
		addBlitzParticle(tPosition, tPositionRange, tSpeed, tSpeedRange, tAngle, tAngleRange, tGravity, tColor, tColorRange, tLifetime, tLifetimeRange);
	}
}


void addBlitzParticle(Position tPosition, Position tPositionRange, double tSpeed, double tSpeedRange, double tAngle, double tAngleRange, Velocity tGravity, Vector3D tColor, Vector3D tColorRange, Duration tLifetime, Duration tLifetimeRange)
{
	ParticleEntry* e = (ParticleEntry*)allocMemory(sizeof(ParticleEntry));
	
	e->mPos = vecAdd(vecSub(tPosition, vecScale(tPositionRange, 0.5)), vecScale(tPositionRange, randfrom(0, 1)));
	double speed = randfrom(tSpeed - tSpeedRange / 2, tSpeed + tSpeedRange / 2);
	double angle = randfrom(tAngle - tAngleRange / 2, tAngle + tAngleRange / 2);
	e->mVel = vecRotateZ(makePosition(speed, 0, 0), angle);

	e->mGravity = tGravity;
	e->mAngle = randfrom(0, 2 * M_PI);
	e->mColor = vecAdd(vecSub(tColor, vecScale(tColorRange, 0.5)), vecScale(tColorRange, randfrom(0, 1)));

	e->mNow = 0;
	e->mLifeTime = randfrom(tLifetime - tLifetimeRange / 2, tLifetime + tLifetimeRange / 2);

	int_map_push_back_owned(&gBlitzParticlesData.mParticles, e);
}
