#include <prism/debug.h>

#include <prism/mugentexthandler.h>
#include <prism/system.h>

static struct {
	
	uint64_t mPrevTime;
	int mFPSCounterTextID;

	uint64_t mStartDrawingTime;
	uint64_t mEndDrawingTime;
	int mDrawingTimeCounterTextID;

	uint64_t mStartUpdateTime;
	uint64_t mPreviousStartUpdateTime;
	uint64_t mEndUpdateTime;

	int mUpdateTimeCounterTextID;

	uint64_t mStartWaitingTime;
	uint64_t mEndWaitingTime;
	int mWaitingTimeCounterTextID;

	int mIsActive;

} gPrismDebug;

static void loadPrismDebug(void* tData) {
	(void)tData;

	ScreenSize sz = getScreenSize();
	double offset = (sz.y / 480.0) * 20;
	int dy = 10;

	gPrismDebug.mPrevTime = getSystemTicks();
	gPrismDebug.mFPSCounterTextID = addMugenTextMugenStyle("00.0", makePosition(sz.x - offset, offset, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mStartDrawingTime = gPrismDebug.mEndDrawingTime = getSystemTicks();
	gPrismDebug.mDrawingTimeCounterTextID = addMugenTextMugenStyle("000", makePosition(sz.x - offset, offset + dy, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mStartUpdateTime = gPrismDebug.mEndUpdateTime = getSystemTicks();
	gPrismDebug.mPreviousStartUpdateTime = getSystemTicks();
	gPrismDebug.mUpdateTimeCounterTextID = addMugenTextMugenStyle("000", makePosition(sz.x - offset, offset + dy * 2, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mStartWaitingTime = gPrismDebug.mEndWaitingTime = getSystemTicks();
	gPrismDebug.mWaitingTimeCounterTextID = addMugenTextMugenStyle("000", makePosition(sz.x - offset, offset + dy * 3, 95), makeVector3DI(-1, 1, -1));

	gPrismDebug.mIsActive = 1;
}

static void unloadPrismDebug(void* tData) {
	(void)tData;
}

static void updatePrismDebug(void* tData) {
	(void)tData;

	uint64_t currentTime = getSystemTicks();

	uint64_t dt = currentTime - gPrismDebug.mPrevTime;
	double ds = dt / 1000.0;
	double fps = 1 / ds;

	char text[200];
	sprintf(text, "%.1f", fps);
	changeMugenText(gPrismDebug.mFPSCounterTextID, text);
	gPrismDebug.mPrevTime = currentTime;


	sprintf(text, "%lu", (unsigned long)(gPrismDebug.mEndDrawingTime - gPrismDebug.mStartDrawingTime));
	changeMugenText(gPrismDebug.mDrawingTimeCounterTextID, text);

	sprintf(text, "%lu", (unsigned long)(gPrismDebug.mEndUpdateTime - gPrismDebug.mPreviousStartUpdateTime));
	changeMugenText(gPrismDebug.mUpdateTimeCounterTextID, text);

	sprintf(text, "%lu", (unsigned long)(gPrismDebug.mEndWaitingTime - gPrismDebug.mStartWaitingTime));
	changeMugenText(gPrismDebug.mWaitingTimeCounterTextID, text);

}

ActorBlueprint getPrismDebug()
{
	return makeActorBlueprint(loadPrismDebug, unloadPrismDebug, updatePrismDebug);
}

void setPrismDebugUpdateStartTime()
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mEndDrawingTime = getSystemTicks();
	gPrismDebug.mPreviousStartUpdateTime = gPrismDebug.mStartUpdateTime;
	gPrismDebug.mStartUpdateTime = getSystemTicks();
}

void setPrismDebugDrawingStartTime()
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mEndWaitingTime = getSystemTicks();
	gPrismDebug.mStartDrawingTime = getSystemTicks();
}

void setPrismDebugWaitingStartTime()
{
	if (!gPrismDebug.mIsActive) return;
	gPrismDebug.mEndUpdateTime = getSystemTicks();
	gPrismDebug.mStartWaitingTime = getSystemTicks();
}
