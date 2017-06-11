#include "tari/texthandler.h"

#include <string.h>

#include "tari/datastructures.h"
#include "tari/log.h"
#include "tari/math.h"
#include "tari/memoryhandler.h"

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
} HandledText;

static struct {
	int mIsActive;

	List mTexts;
} gData;

void setupTextHandler() {
	if (gData.mIsActive) {
		logWarning("Trying to setup active text handler.");
		shutdownTextHandler();
	}

	gData.mTexts = new_list();
	gData.mIsActive = 1;
}

void shutdownTextHandler() {
	list_empty(&gData.mTexts);
	gData.mIsActive = 0;
}

static void increaseDrawnText(HandledText* e) {

	int currentPos = strlen(e->mDrawnText);
	e->mDrawnText[currentPos] = e->mText[currentPos];
	e->mDrawnText[currentPos + 1] = '\0';

	if (!strcmp(e->mText, e->mDrawnText)) {
		e->mSingleLetterBuildupDuration = INF;
	}

	e->mSingleLetterBuildupNow = 0;
}

static int updateSingleText(void* tCaller, void* tData) {
	(void) tCaller;
	HandledText* e = tData;

	if (handleDurationAndCheckIfOver(&e->mNow, e->mDuration)) {
		return 1;
	}

	if (handleDurationAndCheckIfOver(&e->mSingleLetterBuildupNow, e->mSingleLetterBuildupDuration)) {
		increaseDrawnText(e);
	}

	return 0;
}

void updateTextHandler() {
	list_remove_predicate(&gData.mTexts, updateSingleText, NULL);
}

static void drawSingleText(void* tCaller, void* tData) {
	(void) tCaller;
	HandledText* e = tData;

	// TODO: set font to correct font 
	e->mColor = COLOR_WHITE;
	drawMultilineText(e->mDrawnText, e->mText, e->mPosition, e->mFontSize, e->mColor, e->mBreakSize, e->mTextBoxSize);
}

void drawHandledTexts() {
	list_map(&gData.mTexts, drawSingleText, NULL);
}

int addHandledText(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize, Duration tDuration) {
	HandledText* e = allocMemory(sizeof(HandledText));

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

	return list_push_front_owned(&gData.mTexts, e);
}

int addHandledTextWithBuildup(Position tPosition, char* tText, int tFont, Color tColor, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize, Duration tDuration, Duration tBuildupSpeed) {
	int id = addHandledText(tPosition, tText, tFont, tColor, tFontSize, tBreakSize, tTextBoxSize, tDuration);
	HandledText* e = list_get(&gData.mTexts, id);
	e->mSingleLetterBuildupDuration = tBuildupSpeed / strlen(tText);
	e->mDrawnText[0] = '\0';
	return id;
}

int addHandledTextWithInfiniteDurationOnOneLine(Position tPosition, char * tText, int tFont, Color tColor, Vector3D tFontSize)
{
	return addHandledText(tPosition, tText, tFont, tColor, tFontSize, makePosition(0, 0, 0), makePosition(INF, INF, INF), INF);
}

void setHandledText(int tID, char * tText)
{
	HandledText* e = list_get(&gData.mTexts, tID);
	strcpy(e->mText, tText);
	strcpy(e->mDrawnText, tText);
}

void removeHandledText(int tID) {
	list_remove(&gData.mTexts, tID);
}
