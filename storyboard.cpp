#include "prism/storyboard.h"

#include <stdlib.h>
#include <stdint.h>

#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/file.h"
#include "prism/log.h"
#include "prism/animation.h"
#include "prism/system.h"
#include "prism/physicshandler.h"
#include "prism/soundeffect.h"
#include "prism/timer.h"
#include "prism/texthandler.h"
#include "prism/sound.h"
#include "prism/math.h"
#include "prism/input.h"


#pragma pack(push, 1)

typedef struct {
	AnimationHandlerElement* mElement;
	PhysicsHandlerElement* mPhysicsElement;
	TextureData* mTextures;
	Animation mAnimation;
	Position mPosition;
} StoryboardTexture;

typedef struct {
	int mID;

} StoryboardText;

typedef struct {
	int mID;
} StoryboardSoundEffect;

typedef struct {
	Duration mNow;
	Duration mDuration;
	char* mPosition;
	int mNextAction;

	StoryboardTexture mTextures[StoryBoardMaximumTextureAmount];
	StoryboardText mTexts[StoryBoardMaximumTextAmount];
	StoryboardSoundEffect mSoundEffects[StoryBoardMaximumSoundEffectAmount];

} StoryboardState;

typedef struct {
	Buffer mBuffer;

	StoryboardState mState;
	StoryBoardUberHeaderStruct mHeader;

	StoryboardFinishedCB mFinishedCB;
	void* mFinishedCaller;

} Storyboard;

#pragma pack(pop)

static struct {
	List mStoryboards;
} gPrismStoryboardData;

void setupStoryboards() {
	gPrismStoryboardData.mStoryboards = new_list();
}

static void destroyStoryboardTexture(Storyboard* e, int tSlot) {

	if (e->mState.mTextures[tSlot].mElement == NULL) {
		logError("Attempt to destroy unloaded texture slot.");
		logErrorInteger(tSlot);
		logErrorPointer(e->mState.mTextures[tSlot].mElement);
		recoverFromError();
	}

	if (isHandledAnimation(e->mState.mTextures[tSlot].mElement)) {
		removeHandledAnimation(e->mState.mTextures[tSlot].mElement);
	}

	if (e->mState.mTextures[tSlot].mPhysicsElement != NULL) {
		removeFromPhysicsHandler(e->mState.mTextures[tSlot].mPhysicsElement);
		e->mState.mTextures[tSlot].mPhysicsElement = NULL;
	}

	int i;
	for (i = 0; i < (int)e->mState.mTextures[tSlot].mAnimation.mFrameAmount; i++) {
		unloadTexture(e->mState.mTextures[tSlot].mTextures[i]);
	}

	freeMemory(e->mState.mTextures[tSlot].mTextures);

	e->mState.mTextures[tSlot].mElement = NULL;
}

static void destroyStoryboardText(Storyboard* e, int tSlot) {

	if (e->mState.mTexts[tSlot].mID == -1) {
		logError("Attempt to destroy unloaded text slot.");
		logErrorInteger(tSlot);
		logErrorInteger(e->mState.mTexts[tSlot].mID);
		recoverFromError();
	}

	removeHandledText(e->mState.mTexts[tSlot].mID);
	e->mState.mTexts[tSlot].mID = -1;
}

static void destroyStoryboardSoundEffect(Storyboard* e, int tSlot) {

	if (e->mState.mSoundEffects[tSlot].mID == -1) {
		logError("Attempt to destroy unloaded sound effect slot.");
		logErrorInteger(tSlot);
		logErrorInteger(e->mState.mSoundEffects[tSlot].mID);
		recoverFromError();
	}

	unloadSoundEffect(e->mState.mSoundEffects[tSlot].mID);
	e->mState.mSoundEffects[tSlot].mID = -1;
}

static void unloadStoryboard(Storyboard* e) {

	if (e->mFinishedCB) {
		e->mFinishedCB(e->mFinishedCaller);
	}

	int i;
	for (i = 0; i < StoryBoardMaximumTextureAmount; i++) {
		if (e->mState.mTextures[i].mElement != NULL) {
			destroyStoryboardTexture(e, i);
		}
	}
	for (i = 0; i < StoryBoardMaximumTextAmount; i++) {
		if (e->mState.mTexts[i].mID != -1) {
			destroyStoryboardText(e, i);
		}
	}
	for (i = 0; i < StoryBoardMaximumSoundEffectAmount; i++) {
		if(e->mState.mSoundEffects[i].mID != -1) {
			destroyStoryboardSoundEffect(e, i);
		}
	}

	freeBuffer(e->mBuffer);
	unmountRomdisk("STORY");
}

static int removeStoryboardEntry(void* tCaller, void* tData) {
	(void) tCaller;
	Storyboard* e = (Storyboard*)tData;
	unloadStoryboard(e);
	return 1;
}

void shutdownStoryboards() {
	list_remove_predicate(&gPrismStoryboardData.mStoryboards, removeStoryboardEntry, NULL);
}


static void loadStoryboardTextureTexture(Storyboard* e, StoryBoardTextureStruct* tTexture) {
	int slot = tTexture->TextureID;

	e->mState.mTextures[slot].mAnimation = createEmptyAnimation();
	e->mState.mTextures[slot].mAnimation.mFrameAmount = tTexture->FrameAmount;
	e->mState.mTextures[slot].mAnimation.mDuration = tTexture->Speed;

	e->mState.mTextures[slot].mTextures = (TextureData*)allocMemory(e->mState.mTextures[slot].mAnimation.mFrameAmount*sizeof(TextureData));

	if (!tTexture->WhichFrame) {
		char path[1024];
		sprintf(path, "$/STORY/%d.pkg", tTexture->TextureNameID);
		e->mState.mTextures[slot].mTextures[0] = loadTexture(path);
		e->mState.mTextures[slot].mAnimation.mDuration = INF;
	}
	else {
		int i;
		for (i = 1; i <= tTexture->FrameAmount; i++) {
			char path[1024];
			sprintf(path, "$/STORY/%d_%d.pkg", tTexture->TextureNameID, i);
			e->mState.mTextures[slot].mTextures[i-1] = loadTexture(path);
		}
	}

	Rectangle rect;
	rect.topLeft.x = (int)tTexture->TexturePositionX1*e->mState.mTextures[slot].mTextures[0].mTextureSize.x;
	rect.topLeft.y = (int)tTexture->TexturePositionY1*e->mState.mTextures[slot].mTextures[0].mTextureSize.y;
	rect.bottomRight.x = (int)tTexture->TexturePositionX2*e->mState.mTextures[slot].mTextures[0].mTextureSize.x;
	rect.bottomRight.y = (int)tTexture->TexturePositionY2*e->mState.mTextures[slot].mTextures[0].mTextureSize.y;

	if (tTexture->Loop) { 
		e->mState.mTextures[slot].mElement = playAnimationLoop(Vector3D(0, 0, 0), e->mState.mTextures[slot].mTextures, e->mState.mTextures[slot].mAnimation, rect);
	}
	else {
		e->mState.mTextures[slot].mElement = playAnimation(Vector3D(0, 0, 0), e->mState.mTextures[slot].mTextures, e->mState.mTextures[slot].mAnimation, rect, NULL, NULL);
	}

	Position* pos = &getPhysicsFromHandler(e->mState.mTextures[slot].mPhysicsElement)->mPosition;
	setAnimationBasePositionReference(e->mState.mTextures[slot].mElement, pos);
	
	Vector3D scale = Vector3D(tTexture->SizeX / (double)e->mState.mTextures[slot].mTextures[0].mTextureSize.x, tTexture->SizeY / (double)e->mState.mTextures[slot].mTextures[0].mTextureSize.y, 1);
	setAnimationScale(e->mState.mTextures[slot].mElement, scale, Vector3D(0,0,0));
}

static void loadStoryboardTexture(Storyboard* e, StoryBoardTextureStruct* tTexture) {
	int slot = tTexture->TextureID;

	if (tTexture->PositionX == 0xFFFF) tTexture->PositionX = (uint16_t)e->mState.mTextures[slot].mPosition.x;
	if (tTexture->PositionY == 0xFFFF) tTexture->PositionY = (uint16_t)e->mState.mTextures[slot].mPosition.y;

	if (e->mState.mTextures[slot].mPhysicsElement != NULL) {
		removeFromPhysicsHandler(e->mState.mTextures[slot].mPhysicsElement);
		e->mState.mTextures[slot].mPhysicsElement = NULL;
	}

	e->mState.mTextures[slot].mPhysicsElement = addToPhysicsHandler(Vector3D(tTexture->PositionX, tTexture->PositionY, tTexture->PositionZ));
	addAccelerationToHandledPhysics(e->mState.mTextures[slot].mPhysicsElement, Vector3D(tTexture->MovementX, tTexture->MovementY, 0));

	if (tTexture->TextureAction == StoryBoardLoadTextureIdentifier) {
		loadStoryboardTextureTexture(e, tTexture);
	}
}

static void loadStoryboardTextures(Storyboard* e, StoryBoardHeaderStruct* tHeader) {
	int i;
	for (i = 0; i < tHeader->TextureStructAmount; i++) {
		StoryBoardTextureStruct* texture = (StoryBoardTextureStruct*)e->mState.mPosition;
		e->mState.mPosition += sizeof(StoryBoardTextureStruct);

		if (texture->TextureAction == StoryBoardDestroyTextureIdentifier) {
			int slot = texture->TextureID;
			destroyStoryboardTexture(e, slot);
		}
		else {
			loadStoryboardTexture(e, texture);
		}
	}
}

static uint8_t parseDolmexicaColor(int tDolmexicaColor) {
	switch (tDolmexicaColor) {
	case 0:
		return COLOR_WHITE;
	case 1:
		return COLOR_BLACK;
	case 2:
		return COLOR_RED;
	case 3:
		return COLOR_GREEN;
	case 4:
		return COLOR_BLUE;
	case 5:
		return COLOR_YELLOW;
	default:
		return COLOR_WHITE;
	}
}

static void loadStoryboardText(Storyboard* e, StoryBoardTextStruct* tText) {
	int slot = tText->TextID;

	if (e->mState.mTexts[slot].mID != -1) {
		destroyStoryboardText(e, slot);
	}

	tText->FontColor = parseDolmexicaColor(tText->FontColor);
	if (tText->BuildUp) e->mState.mTexts[slot].mID = addHandledTextWithBuildup(Vector3D(tText->PositionX, tText->PositionY, tText->PositionZ), tText->ActualText, tText->WhichFont, (Color)tText->FontColor, makeFontSize(tText->FontSizeX, tText->FontSizeY), Vector3D(tText->BreakSizeX, tText->BreakSizeY, 0), Vector3D(tText->SizeX, tText->SizeY, 0), INF, tText->BuildUpSpeed);
	else e->mState.mTexts[slot].mID = addHandledText(Vector3D(tText->PositionX, tText->PositionY, tText->PositionZ), tText->ActualText, tText->WhichFont, (Color)tText->FontColor, makeFontSize(tText->FontSizeX, tText->FontSizeY), Vector3D(tText->BreakSizeX, tText->BreakSizeY, 0), Vector3D(tText->SizeX, tText->SizeY, 0), INF);
}

static void loadStoryboardTexts(Storyboard* e, StoryBoardHeaderStruct* tHeader) {
	int i;
	for (i = 0; i < tHeader->TextStructAmount; i++) {
		StoryBoardTextStruct* text = (StoryBoardTextStruct*)e->mState.mPosition;
		e->mState.mPosition += sizeof(StoryBoardTextStruct);
		int slot = text->TextID;

		if (text->TextAction == StoryBoardDestroyTextIdentifier) {
			destroyStoryboardText(e, slot);
		}
		else {
			loadStoryboardText(e, text);
		}
	}
}

static void playStoryboardSoundEffect(void* tCaller) {
	StoryboardSoundEffect* e = (StoryboardSoundEffect*)tCaller;

	if (e->mID == -1) return;

	playSoundEffect(e->mID);
}

static void loadStoryboardSoundEffect(Storyboard* e, StoryBoardSoundEffectStruct* tSoundEffect) {
	int slot = tSoundEffect->SoundEffectID;

	if (e->mState.mSoundEffects[slot].mID != -1) destroyStoryboardSoundEffect(e, slot);

	char path[1024];
	sprintf(path, "$/STORY/%d.wav", tSoundEffect->SoundEffectNameID);
	e->mState.mSoundEffects[slot].mID = loadSoundEffect(path);
	
	addTimerCB(tSoundEffect->SoundEffectPlayTime, playStoryboardSoundEffect, &e->mState.mSoundEffects[slot]);
}

static void loadStoryboardSoundEffects(Storyboard* e, StoryBoardHeaderStruct* tHeader) {
	int i;
	for (i = 0; i < tHeader->SoundEffectStructAmount; i++) {

		StoryBoardSoundEffectStruct* soundEffect = (StoryBoardSoundEffectStruct*)e->mState.mPosition;
		e->mState.mPosition += sizeof(StoryBoardSoundEffectStruct);

		if (soundEffect->SoundEffectAction == StoryBoardDestroySoundEffectIdentifier) {
			int slot = soundEffect->SoundEffectID;
			destroyStoryboardSoundEffect(e, slot);
		}
		else {
			loadStoryboardSoundEffect(e, soundEffect);
		}
	}
}

static void loadNextStoryboardAction(Storyboard* e) {
	StoryBoardHeaderStruct* header = (StoryBoardHeaderStruct*)e->mState.mPosition;
	e->mState.mPosition += sizeof(StoryBoardHeaderStruct);

	if (header->SoundTrack != 0) {
		// playTrack(header->SoundTrack);
	}
	
	loadStoryboardTextures(e, header);
	loadStoryboardTexts(e, header);
	loadStoryboardSoundEffects(e, header);
	
	e->mState.mNow = 0;
	e->mState.mDuration = header->Duration;

	e->mState.mNextAction++;
}

static void checkStoryboardInputs(Storyboard* e) {
	if (hasPressedStartFlank()) {
		e->mState.mDuration = e->mState.mNow;
		e->mState.mNextAction = e->mHeader.ActionAmount;
	}
	else if (hasPressedAFlank()) {
		e->mState.mDuration = e->mState.mNow;
	}
}

static int updateSingleStoryboard(void* tCaller, void* tData) {
	(void) tCaller;
	Storyboard* e = (Storyboard*)tData;

	checkStoryboardInputs(e);

	if (handleDurationAndCheckIfOver(&e->mState.mNow, e->mState.mDuration)) {
		if (e->mState.mNextAction == e->mHeader.ActionAmount) {
			unloadStoryboard(e);
			return 1;
		}

		loadNextStoryboardAction(e);
	}



	return 0;
}

void updateStoryboards() {
	list_remove_predicate(&gPrismStoryboardData.mStoryboards, updateSingleStoryboard, NULL);
}



static void resetStoryboard(Storyboard* e) {
	e->mState.mNow = 0;
	e->mState.mDuration = 0;
	e->mState.mNextAction = 0;

	e->mState.mPosition = ((char*)e->mBuffer.mData)+sizeof(StoryBoardUberHeaderStruct);

	int i;
	for (i = 0; i < StoryBoardMaximumTextureAmount; i++) {
		e->mState.mTextures[i].mElement = NULL;
		e->mState.mTextures[i].mPhysicsElement = NULL;
	}
	for (i = 0; i < StoryBoardMaximumTextAmount; i++) {
		e->mState.mTexts[i].mID = -1;
	}
	for (i = 0; i < StoryBoardMaximumSoundEffectAmount; i++) {
		e->mState.mSoundEffects[i].mID = -1;
	}

}

static Storyboard* loadStoryboard(const char* tPath) {
	Storyboard* e = (Storyboard*)allocMemory(sizeof(Storyboard));

	mountRomdisk(tPath, "STORY");

	e->mBuffer = fileToBuffer("$/STORY/STORYBOARD");
	memcpy(&e->mHeader, e->mBuffer.mData, sizeof(StoryBoardUberHeaderStruct));
	
	e->mFinishedCB = NULL;
	e->mFinishedCaller = NULL;

	resetStoryboard(e);

	return e;
}

int playStoryboard(const char* tPath) {
	if (list_size(&gPrismStoryboardData.mStoryboards)) {
		logError("Unable to play more than one story board at a time.");
		logErrorString(tPath);
		recoverFromError();
	}

	Storyboard* e = loadStoryboard(tPath);
	return list_push_front_owned(&gPrismStoryboardData.mStoryboards, e);
}

void setStoryboardFinishedCB(int tID, StoryboardFinishedCB tCB, void* tCaller) {
	Storyboard* e = (Storyboard*)list_get(&gPrismStoryboardData.mStoryboards, tID);
	e->mFinishedCB = tCB;
	e->mFinishedCaller = tCaller;
}

int isStoryboard(const char* tPath) {
	if (!strcmp("", tPath)) return 0;

	int isBoard = isFile(tPath);
	if (isBoard) return 1;

	char folderPath[1024];
	getPathWithoutFileExtension(folderPath, tPath);
	isBoard = isDirectory(folderPath);
	return isBoard;
}

static void loadStoryboardsCB(void* tCaller) {
	(void)tCaller;
	setProfilingSectionMarkerCurrentFunction();
	setupStoryboards();
}

static void unloadStoryboardsCB(void* tCaller) {
	(void)tCaller;
	setProfilingSectionMarkerCurrentFunction();
	shutdownStoryboards();
}

static void updateStoryboardsCB(void* tCaller) {
	(void)tCaller;
	setProfilingSectionMarkerCurrentFunction();
	updateStoryboards();
}

ActorBlueprint getStoryboardHandlerActorBlueprint()
{
	return makeActorBlueprint(loadStoryboardsCB, unloadStoryboardsCB, updateStoryboardsCB);
}
