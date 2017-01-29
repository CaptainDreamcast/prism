#include "include/optionhandler.h"

#include <stdlib.h>
#include <string.h>

#include "include/datastructures.h"
#include "include/drawing.h"
#include "include/input.h"

static struct {
	List mOptionList;
	TextureData mSelector;
	int mSelectedOption;
	int mTextSize;
	int mIsUsingA;
	int mIsUsingStart;
	Color mColor;
	int mIsDisabled;
	
	Position mSelectorBasePosition;
} gData;

typedef struct {
	Position mPosition;
	char mText[100];
	OptionCB mCB;
	void* mCaller;
	int mNumber;

} HandledOption;

void setupOptionHandler(){
	gData.mOptionList = new_list();
	gData.mSelector = loadTexturePKG("$/rd/debug/selector.pkg");
	gData.mSelectedOption = 0;
	gData.mTextSize = 10;
	gData.mIsUsingA = 0;
	gData.mIsUsingStart = 0;
	gData.mIsDisabled = 0;
	gData.mColor = COLOR_WHITE;

	gData.mSelectorBasePosition = makePosition(0, 0, 10);
}



void shutdownOptionHandler() {
	delete_list(&gData.mOptionList);
	unloadTexture(gData.mSelector);
}

void disableOptionHandler() {
	gData.mIsDisabled = 1;
}

int addOption(Position tPosition, char* tText, OptionCB tCB, void* tCaller){
	HandledOption* o = malloc(sizeof(HandledOption));
	o->mPosition = tPosition;
	strcpy(o->mText, tText);
	o->mCB = tCB;
	o->mCaller = tCaller;
	o->mNumber = list_size(&gData.mOptionList);

	int id = list_push_front_owned(&gData.mOptionList, (void*)o);

	return id;
}

void setOptionTextSize(int tSize){
	gData.mTextSize = tSize;
}

void setOptionButtonA(){
	gData.mIsUsingA = 1;
}
void setOptionButtonStart(){
	gData.mIsUsingStart = 1;
}

static void performSelectedAction(void* tCaller, void* tRaw) {
	(void) tCaller;
	HandledOption* data = tRaw;

	if(data->mNumber != gData.mSelectedOption) return;

	data->mCB(data->mCaller);
}

void updateOptionHandler(){
	if(!list_size(&gData.mOptionList)) return;

	if(gData.mIsDisabled) return;

	if(hasPressedDownFlank()) {
		gData.mSelectedOption = (gData.mSelectedOption + 1) % list_size(&gData.mOptionList);
	} else if(hasPressedUpFlank()) {
		gData.mSelectedOption--;
		if(gData.mSelectedOption < 0) gData.mSelectedOption += list_size(&gData.mOptionList);
	}

	int hasValidA = gData.mIsUsingA && hasPressedAFlank();
	int hasValidStart = gData.mIsUsingStart && hasPressedStartFlank();
	if(hasValidA || hasValidStart) {
		list_map(&gData.mOptionList, performSelectedAction, NULL);
	}
}

static void drawOption(void* tCaller, void* tRaw) {
	(void) tCaller;
	HandledOption* data = tRaw;

	drawText(data->mText, data->mPosition, gData.mTextSize, gData.mColor);

	if(data->mNumber == gData.mSelectedOption) {
		gData.mSelectorBasePosition = data->mPosition;
	}
}

static void drawSelector() {

	Position p = gData.mSelectorBasePosition;
	double selectorFactor =  gData.mTextSize / (double)gData.mSelector.mTextureSize.x;

	p = vecAdd(p, makePosition(-gData.mTextSize, 0, 0));
	Rectangle r = makeRectangleFromTexture(gData.mSelector);
	scaleDrawing(selectorFactor, p);
	drawSprite(gData.mSelector, p, r);
	setDrawingParametersToIdentity();
}

void drawOptionHandler(){
	if(!list_size(&gData.mOptionList)) return;

	list_map(&gData.mOptionList, drawOption, NULL);
	drawSelector();
}

