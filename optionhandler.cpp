#include "prism/optionhandler.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "prism/datastructures.h"
#include "prism/drawing.h"
#include "prism/input.h"
#include "prism/memoryhandler.h"

static struct {
	List mOptionList;
	TextureData mSelector;
	int mSelectedOption;
	int mTextSize;
	int mIsUsingA;
	int mIsUsingStart;
	Color mColor;
	int mIsDisabled;
	int mBreakSize;
	int mIsActive;

	Position mSelectorBasePosition;
} gPrismOptionHandlerData;

typedef struct {
	Position mPosition;
	char mText[100];
	OptionCB mCB;
	void* mCaller;
	int mNumber;

} HandledOption;

void setupOptionHandler(){
	gPrismOptionHandlerData.mOptionList = new_list();
	gPrismOptionHandlerData.mSelector = loadTexturePKG("$/rd/debug/selector.pkg");
	gPrismOptionHandlerData.mSelectedOption = 0;
	gPrismOptionHandlerData.mTextSize = 10;
	gPrismOptionHandlerData.mIsUsingA = 0;
	gPrismOptionHandlerData.mIsUsingStart = 0;
	gPrismOptionHandlerData.mIsDisabled = 0;
	gPrismOptionHandlerData.mColor = COLOR_WHITE;
	gPrismOptionHandlerData.mBreakSize = 0;
	gPrismOptionHandlerData.mIsActive = 1;

	gPrismOptionHandlerData.mSelectorBasePosition = makePosition(0, 0, 10);
}



void shutdownOptionHandler() {
	delete_list(&gPrismOptionHandlerData.mOptionList);
	unloadTexture(gPrismOptionHandlerData.mSelector);

	gPrismOptionHandlerData.mIsActive = 0;
}

void disableOptionHandler() {
	gPrismOptionHandlerData.mIsDisabled = 1;
}

int addOption(Position tPosition, char* tText, OptionCB tCB, void* tCaller){
	HandledOption* o = (HandledOption*)allocMemory(sizeof(HandledOption));
	o->mPosition = tPosition;
	strcpy(o->mText, tText);
	o->mCB = tCB;
	o->mCaller = tCaller;
	o->mNumber = list_size(&gPrismOptionHandlerData.mOptionList);

	int id = list_push_front_owned(&gPrismOptionHandlerData.mOptionList, (void*)o);

	return id;
}

void removeOption(int tID)
{
	list_remove(&gPrismOptionHandlerData.mOptionList, tID);
}

void setOptionTextSize(int tSize){
	gPrismOptionHandlerData.mTextSize = tSize;
}

void setOptionTextBreakSize(int tBreakSize)
{
	gPrismOptionHandlerData.mBreakSize = tBreakSize;
}

void setOptionButtonA(){
	gPrismOptionHandlerData.mIsUsingA = 1;
}
void setOptionButtonStart(){
	gPrismOptionHandlerData.mIsUsingStart = 1;
}

static void performSelectedAction(void* tCaller, void* tRaw) {
	(void) tCaller;
	HandledOption* data = (HandledOption*)tRaw;

	if(data->mNumber != gPrismOptionHandlerData.mSelectedOption) return;

	data->mCB(data->mCaller);
}

void updateOptionHandler(){
	if(!list_size(&gPrismOptionHandlerData.mOptionList)) return;

	if(gPrismOptionHandlerData.mIsDisabled) return;

	if(hasPressedDownFlank()) {
		gPrismOptionHandlerData.mSelectedOption = (gPrismOptionHandlerData.mSelectedOption + 1) % list_size(&gPrismOptionHandlerData.mOptionList);
	} else if(hasPressedUpFlank()) {
		gPrismOptionHandlerData.mSelectedOption--;
		if(gPrismOptionHandlerData.mSelectedOption < 0) gPrismOptionHandlerData.mSelectedOption += list_size(&gPrismOptionHandlerData.mOptionList);
	}

	int hasValidA = gPrismOptionHandlerData.mIsUsingA && hasPressedAFlank();
	int hasValidStart = gPrismOptionHandlerData.mIsUsingStart && hasPressedStartFlank();
	if(hasValidA || hasValidStart) {
		ListIterator iterator = list_iterator_begin(&gPrismOptionHandlerData.mOptionList);
		while (iterator != NULL) {
			performSelectedAction(NULL, list_iterator_get(iterator));
			if (!gPrismOptionHandlerData.mIsActive) return;
			if (list_has_next(iterator)) {
				list_iterator_increase(&iterator);
			}
			else {
				break;
			}
		}
	}
}

static void drawOption(void* tCaller, void* tRaw) {
	(void) tCaller;
	HandledOption* data = (HandledOption*)tRaw;

	drawAdvancedText(data->mText, data->mPosition, makePosition(gPrismOptionHandlerData.mTextSize, gPrismOptionHandlerData.mTextSize, 1), gPrismOptionHandlerData.mColor, gPrismOptionHandlerData.mBreakSize);

	if(data->mNumber == gPrismOptionHandlerData.mSelectedOption) {
		gPrismOptionHandlerData.mSelectorBasePosition = data->mPosition;
	}
}

static void drawSelector() {

	Position p = gPrismOptionHandlerData.mSelectorBasePosition;
	double selectorFactor =  gPrismOptionHandlerData.mTextSize / (double)gPrismOptionHandlerData.mSelector.mTextureSize.x;

	p = vecAdd(p, makePosition(-gPrismOptionHandlerData.mTextSize, 0, 0));
	Rectangle r = makeRectangleFromTexture(gPrismOptionHandlerData.mSelector);
	scaleDrawing(selectorFactor, p);
	drawSprite(gPrismOptionHandlerData.mSelector, p, r);
	setDrawingParametersToIdentity();
}

void drawOptionHandler(){
	if(!list_size(&gPrismOptionHandlerData.mOptionList)) return;
	list_map(&gPrismOptionHandlerData.mOptionList, drawOption, NULL);
	drawSelector();
}

static void setupOptionHandlerBlueprint(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	setupOptionHandler();
}

static void updateOptionHandlerBlueprint(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	updateOptionHandler();
}

static void drawOptionHandlerBlueprint(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	drawOptionHandler();
}

static void shutdownOptionHandlerBlueprint(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	shutdownOptionHandler();
}

ActorBlueprint getOptionHandlerBlueprint()
{
	return makeActorBlueprint(setupOptionHandlerBlueprint, shutdownOptionHandlerBlueprint, updateOptionHandlerBlueprint, drawOptionHandlerBlueprint);
}

