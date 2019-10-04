#include "prism/texthandler.h"

#include <string.h>

#include "prism/datastructures.h"
#include "prism/log.h"
#include "prism/math.h"
#include "prism/memoryhandler.h"
#include "prism/stlutil.h"

typedef struct {
	char mText[1024];
	char mDrawnText[1024];

	Duration mNow;
	Duration mDuration;

	Position mPosition;

	int mFont;
	Color mColor;
	Vector3D mFontSize;

	Vector3D mBreakSize;
	Vector3D mTextBoxSize;

	Duration mSingleLetterBuildupNow;
	Duration mSingleLetterBuildupDuration;

	int mHasSoundEffects;
	SoundEffectCollection mSoundEffects;

	int mHasBasePositionReference;
	Position* mBasePositionReference;
} HandledText;

static struct {
	int mIsActive;

	List mTexts;
	std::map<int, std::pair<std::string, std::string>> mFonts; // stores (HeaderPath, TexturePath)
} gPrismTextHandlerData;

void setupTextHandler() {
	if (gPrismTextHandlerData.mIsActive) {
		logWarning("Trying to setup active text handler.");
		shutdownTextHandler();
	}

	gPrismTextHandlerData.mTexts = new_list();
	gPrismTextHandlerData.mFonts.clear();
	gPrismTextHandlerData.mIsActive = 1;
}

void shutdownTextHandler() {
	list_empty(&gPrismTextHandlerData.mTexts);
	gPrismTextHandlerData.mFonts.clear();
	gPrismTextHandlerData.mIsActive = 0;
}

static void increaseDrawnText(HandledText* e) {

	int currentPos = strlen(e->mDrawnText);
	e->mDrawnText[currentPos] = e->mText[currentPos];
	e->mDrawnText[currentPos + 1] = '\0';

	if (!strcmp(e->mText, e->mDrawnText)) {
		e->mSingleLetterBuildupDuration = INF;
	}

	e->mSingleLetterBuildupNow = 0;

	if (e->mHasSoundEffects) {
		playRandomSoundEffectFromCollection(e->mSoundEffects);
	}
	
}

static int updateSingleText(void* tCaller, void* tData) {
	(void) tCaller;
	HandledText* e = (HandledText*)tData;

	if (handleDurationAndCheckIfOver(&e->mNow, e->mDuration)) {
		return 1;
	}

	if (handleDurationAndCheckIfOver(&e->mSingleLetterBuildupNow, e->mSingleLetterBuildupDuration)) {
		increaseDrawnText(e);
	}

	return 0;
}

void updateTextHandler() {
	list_remove_predicate(&gPrismTextHandlerData.mTexts, updateSingleText, NULL);
}

static void drawSingleText(void* tCaller, void* tData) {
	(void) tCaller;
	HandledText* e = (HandledText*)tData;

	Position p = e->mPosition;
	if (e->mHasBasePositionReference) {
		p = vecAdd(p, *e->mBasePositionReference);
	}
	if (gPrismTextHandlerData.mFonts.find(e->mFont) != gPrismTextHandlerData.mFonts.end()) {
		setFont(gPrismTextHandlerData.mFonts[e->mFont].first.c_str(), gPrismTextHandlerData.mFonts[e->mFont].second.c_str());
	}
	drawMultilineText(e->mDrawnText, e->mText, p, e->mFontSize, e->mColor, e->mBreakSize, e->mTextBoxSize);
}

void drawHandledTexts() {
	list_map(&gPrismTextHandlerData.mTexts, drawSingleText, NULL);
}

int addHandledText(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize, Duration tDuration) {
	HandledText* e = (HandledText*)allocMemory(sizeof(HandledText));

	strcpy(e->mText, tText);
	strcpy(e->mDrawnText, tText);

	e->mNow = 0;
	e->mDuration = tDuration;

	e->mPosition = tPosition;

	e->mFont = tFont;
	e->mColor = tColor;
	e->mFontSize = tFontSize;

	e->mBreakSize = tBreakSize;
	e->mTextBoxSize = tTextBoxSize;

	e->mSingleLetterBuildupNow = 0;
	e->mSingleLetterBuildupDuration = INF;

	e->mHasSoundEffects = 0;

	e->mHasBasePositionReference = 0;
	e->mBasePositionReference = NULL;

	return list_push_front_owned(&gPrismTextHandlerData.mTexts, e);
}

int addHandledTextWithBuildup(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize, Duration tDuration, Duration tBuildupDuration) {
	int id = addHandledText(tPosition, tText, tFont, tColor, tFontSize, tBreakSize, tTextBoxSize, tDuration);
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, id);
	e->mSingleLetterBuildupDuration = tBuildupDuration / strlen(tText);
	e->mDrawnText[0] = '\0';
	return id;
}

int addHandledTextWithInfiniteDurationOnOneLine(Position tPosition, char * tText, int tFont, Color tColor, Vector3D tFontSize)
{
	return addHandledText(tPosition, tText, tFont, tColor, tFontSize, makePosition(0, 0, 0), makePosition(INF, INF, INF), INF);
}

void setHandledText(int tID, char * tText)
{
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, tID);
	strcpy(e->mText, tText);
	strcpy(e->mDrawnText, tText);
}

void setHandledTextSoundEffects(int tID, SoundEffectCollection tSoundEffects)
{
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, tID);
	e->mHasSoundEffects = 1;
	e->mSoundEffects = tSoundEffects;
}

void setHandledTextPosition(int tID, Position tPosition)
{
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, tID);
	e->mPosition = tPosition;
}

void setHandledTextBasePositionReference(int tID, Position * tPosition)
{
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, tID);
	e->mHasBasePositionReference = 1;
	e->mBasePositionReference = tPosition;
}

void setHandledTextBuiltUp(int tID)
{
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, tID);
	strcpy(e->mDrawnText, e->mText);
}

int isHandledTextBuiltUp(int tID)
{
	HandledText* e = (HandledText*)list_get(&gPrismTextHandlerData.mTexts, tID);
	return !strcmp(e->mDrawnText, e->mText);
}

void addTextHandlerFont(int tID, const char* tHeaderPath, const char* tTexturePath)
{
	gPrismTextHandlerData.mFonts[tID] = std::make_pair(tHeaderPath, tTexturePath);
}

void removeHandledText(int tID) {
	list_remove(&gPrismTextHandlerData.mTexts, tID);
}

static void setupTextHandlerCB(void* tData) {
	(void)tData;
	setupTextHandler();
}

static void updateTextHandlerCB(void* tData) {
	(void)tData;
	updateTextHandler();
}

static void drawTextHandlerCB(void* tData) {
	(void)tData;
	drawHandledTexts();
}

static void shutdownTextHandlerCB(void* tData) {
	(void)tData;
	shutdownTextHandler();
}

ActorBlueprint getTextHandler() {
	return makeActorBlueprint(setupTextHandlerCB, shutdownTextHandlerCB, updateTextHandlerCB, drawTextHandlerCB);
};
