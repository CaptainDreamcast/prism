#include "prism/mugenanimationreader.h"

#include <stdio.h>
#include <assert.h>
#include <algorithm>

#include "prism/log.h"
#include "prism/collision.h"
#include "prism/system.h"
#include "prism/math.h"

#include "prism/mugendefreader.h"

using namespace std;

static struct {
	int mIsLoaded;

	List mDefaultHitboxes1;
	List mDefaultHitboxes2;

	int mHasOwnHitbox1;
	List mOwnHitbox1;

	int mHasOwnHitbox2;
	List mOwnHitbox2;

	int mIsLoopStart;
	int mIsReadingDefaultHitbox;

	MemoryStack* mMemoryStack;
} gMugenAnimationState;

static void* allocMemoryOnMemoryStackOrMemory(uint32_t tSize) {
	return allocMemory(tSize);
	
	if (gMugenAnimationState.mMemoryStack) return allocMemoryOnMemoryStack(gMugenAnimationState.mMemoryStack, tSize);
	else return allocMemory(tSize);
}

static void setGlobalAnimationState() {
	if (gMugenAnimationState.mIsLoaded) return;

	gMugenAnimationState.mDefaultHitboxes1 = new_list();
	gMugenAnimationState.mDefaultHitboxes2 = new_list();
	gMugenAnimationState.mIsLoaded = 1;
}

static void unsetGlobalAnimationState() {
	if (!gMugenAnimationState.mIsLoaded) return;

	delete_list(&gMugenAnimationState.mDefaultHitboxes1);
	delete_list(&gMugenAnimationState.mDefaultHitboxes2);
	
	gMugenAnimationState.mIsLoaded = 0;
}

static void resetGlobalAnimationState() {
	unsetGlobalAnimationState();
	setGlobalAnimationState();
}

static void setSingleAnimationState() {
	gMugenAnimationState.mIsLoopStart = 0;
	gMugenAnimationState.mHasOwnHitbox1 = 0;
	gMugenAnimationState.mHasOwnHitbox2 = 0;

	gMugenAnimationState.mOwnHitbox1 = new_list();
	gMugenAnimationState.mOwnHitbox2 = new_list();
}

static void unsetSingleAnimationState() {
	delete_list(&gMugenAnimationState.mOwnHitbox1);
	delete_list(&gMugenAnimationState.mOwnHitbox2);

	gMugenAnimationState.mHasOwnHitbox1 = 0;
	gMugenAnimationState.mHasOwnHitbox2 = 0;
}

static void resetSingleAnimationState() {
	unsetSingleAnimationState();
	setSingleAnimationState();
}


static MugenAnimation* makeEmptyMugenAnimation(int tID) {
	MugenAnimation* ret = (MugenAnimation*)allocMemoryOnMemoryStackOrMemory(sizeof(MugenAnimation));
	ret->mLoopStart = 0;
	ret->mSteps = new_vector();
	ret->mID = tID;
	ret->mTotalDuration = 0;
	return ret;
}

static int getAnimationID(MugenDefScriptGroup* tGroup) {
	char text1[20], text2[20];
	int ret;
	sscanf(tGroup->mName, "%s %s %d", text1, text2, &ret);
	return ret;
}

typedef struct {
	MugenAnimations* mAnimations;
	int mGroupID;
} AnimationLoadCaller;

static int isAnimationVector(char* tVariableName) {
	char text[100];
	sscanf(tVariableName, "%s", text);
	return !strcmp(text, "vector_statement");
}

static void insertAnimationStepIntoAnimation(MugenAnimation* e, MugenAnimationStep* tElement) {
	if (gMugenAnimationState.mIsLoopStart) e->mLoopStart = vector_size(&e->mSteps);
	if (isMugenAnimationStepDurationInfinite(tElement->mDuration)) e->mTotalDuration = INF;
	else e->mTotalDuration += tElement->mDuration;
	vector_push_back_owned(&e->mSteps, tElement);
}

static void insertAnimationStepIntoAnimations(MugenAnimations* tAnimations, int tGroupID, MugenAnimationStep* tElement) {
	assert(int_map_contains(&tAnimations->mAnimations, tGroupID));
	
	MugenAnimation* e = (MugenAnimation*)int_map_get(&tAnimations->mAnimations, tGroupID);
	insertAnimationStepIntoAnimation(e, tElement);
}

static void handleNewAnimationStepBlendFlags(MugenAnimationStep* e, char* blendFlags) {
	e->mIsAddition = blendFlags[0] == 'A';
	e->mIsSubtraction = blendFlags[0] == 'S';
	if (e->mIsAddition || e->mIsSubtraction) {
		char* sourcePos = strchr(blendFlags + 1, 'S');
		if (sourcePos == NULL) {
			if (blendFlags[1] == '1') {
				e->mSrcBlendValue = 256;
				e->mDstBlendValue = 128;
			}
			else {
				e->mSrcBlendValue = 256;
				e->mDstBlendValue = 256;
			}
		}
		else {
			char* dstPos = strchr(blendFlags + 1, 'D');
			assert(dstPos != NULL);

			char text1[20];
			char text2[20];

			strcpy(text1, sourcePos);
			*strchr(text1, 'D') = '\0';

			strcpy(text2, dstPos);

			e->mSrcBlendValue = atoi(text1);
			e->mDstBlendValue = atoi(text2);
		}

	}
}

static void copySingleHitboxToNewList(void* tCaller, void* tData) {
	List* dst = (List*)tCaller;
	CollisionRect* e = (CollisionRect*)tData;

	CollisionRect* newRect = (CollisionRect*)allocMemoryOnMemoryStackOrMemory(sizeof(CollisionRect));
	*newRect = *e;

	list_push_back_owned(dst, newRect);
}

static List copyHitboxList(List tSource) {
	List ret = new_list();
	list_map(&tSource, copySingleHitboxToNewList, &ret);
	return ret;
}

static void handleNewAnimationStep(MugenAnimations* tAnimations, int tGroupID, MugenDefScriptGroupElement* tElement) {
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);
	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	
	MugenAnimationStep* e = (MugenAnimationStep*)allocMemoryOnMemoryStackOrMemory(sizeof(MugenAnimationStep));
	
	if (gMugenAnimationState.mHasOwnHitbox1) {
		e->mAttackHitboxes = copyHitboxList(gMugenAnimationState.mOwnHitbox1);
	}
	else {
		e->mAttackHitboxes = copyHitboxList(gMugenAnimationState.mDefaultHitboxes1);
	}

	if (gMugenAnimationState.mHasOwnHitbox2) {
		e->mPassiveHitboxes = copyHitboxList(gMugenAnimationState.mOwnHitbox2);
	}
	else {
		e->mPassiveHitboxes = copyHitboxList(gMugenAnimationState.mDefaultHitboxes2);

	}

	assert(vectorElement->mVector.mSize >= 2);

	e->mGroupNumber = atoi(vectorElement->mVector.mElement[0]);
	e->mSpriteNumber = atoi(vectorElement->mVector.mElement[1]);
	e->mDelta.x = vectorElement->mVector.mSize >= 3 ? atof(vectorElement->mVector.mElement[2]) : 0;
	e->mDelta.y = vectorElement->mVector.mSize >= 4 ? atof(vectorElement->mVector.mElement[3]) : 0;
	e->mDuration = vectorElement->mVector.mSize >= 5 ? atoi(vectorElement->mVector.mElement[4]) : 1;

	if (vectorElement->mVector.mSize >= 6) {
		e->mIsFlippingHorizontally = strchr(vectorElement->mVector.mElement[5], 'H') != NULL;
		e->mIsFlippingVertically = strchr(vectorElement->mVector.mElement[5], 'V') != NULL;
	}
	else {
		e->mIsFlippingHorizontally = 0;
		e->mIsFlippingVertically = 0;
	}

	if (vectorElement->mVector.mSize >= 7) {
		handleNewAnimationStepBlendFlags(e, vectorElement->mVector.mElement[6]);
	}
	else {
		handleNewAnimationStepBlendFlags(e, "");
	}

	e->mInterpolateOffset = 0;
	e->mInterpolateBlend = 0;
	e->mInterpolateScale = 0;
	e->mInterpolateAngle = 0;

	insertAnimationStepIntoAnimations(tAnimations, tGroupID, e);
	resetSingleAnimationState();
}

static void handleHitboxSizeAssignment(char* tName) {
	char name[100];
	sscanf(tName, "%s", name);

	if (!strcmp("clsn1default", name) || !strcmp("clsn2default", name)) {
		gMugenAnimationState.mIsReadingDefaultHitbox = 1;
	}
	else if(!strcmp("clsn1", name) || !strcmp("clsn2", name)) {
		gMugenAnimationState.mIsReadingDefaultHitbox = 0;
	}
	else {
		logError("Unrecognized collision size name.");
		logErrorString(name);
		recoverFromError();
	}
}

static int isHitboxSizeAssignment(char* tName) {
	char name[100];
	sscanf(tName, "%s", name);
	return !strcmp("clsn2default", name) || !strcmp("clsn2", name) || !strcmp("clsn1default", name) || !strcmp("clsn1", name);
}

static void handleCollisionHitboxAssignment(List* tHitboxes, MugenDefScriptGroupElement* tElement) {
	assert(tElement->mType == MUGEN_DEF_SCRIPT_GROUP_VECTOR_ELEMENT);

	MugenDefScriptVectorElement* vectorElement = (MugenDefScriptVectorElement*)tElement->mData;
	
	double x1 = atof(vectorElement->mVector.mElement[0]);
	double y1 = atof(vectorElement->mVector.mElement[1]);
	double x2 = atof(vectorElement->mVector.mElement[2]);
	double y2 = atof(vectorElement->mVector.mElement[3]);

	Position topLeft = makePosition(min(x1, x2), min(y1, y2), 0);
	Position bottomRight = makePosition(max(x1, x2), max(y1, y2), 0);

	CollisionRect* e = (CollisionRect*)allocMemory(sizeof(CollisionRect));
	*e = makeCollisionRect(topLeft, bottomRight);
	
	list_push_back_owned(tHitboxes, e);
}

static void handleHitboxAssignment(MugenDefScriptGroupElement* tElement) {
	char name[100];
	strcpy(name, tElement->mName);
	char* opening = strchr(name, '[');
	assert(opening);
	*opening = '\0';


	if (!strcmp("clsn1", name)) {
		if (gMugenAnimationState.mIsReadingDefaultHitbox) {
			handleCollisionHitboxAssignment(&gMugenAnimationState.mDefaultHitboxes1, tElement);
		}
		else {
			gMugenAnimationState.mHasOwnHitbox1 = 1;
			handleCollisionHitboxAssignment(&gMugenAnimationState.mOwnHitbox1, tElement);
		}
	}
	else if (!strcmp("clsn2", name)) {
		if (gMugenAnimationState.mIsReadingDefaultHitbox) {
			handleCollisionHitboxAssignment(&gMugenAnimationState.mDefaultHitboxes2, tElement);
		}
		else {
			gMugenAnimationState.mHasOwnHitbox2 = 1;
			handleCollisionHitboxAssignment(&gMugenAnimationState.mOwnHitbox2, tElement);
		}
	}
	else {
		logError("Unable to decode assignment type.");
		logErrorString(name);
		recoverFromError();
	}

}

static int isHitboxAssignment(char* tName) {
	char name[100];
	strcpy(name, tName);
	char* opening = strchr(name, '[');
	if (!opening) return 0;

	*opening = '\0';

	return !strcmp("clsn2Default", name) || !strcmp("clsn2", name) || !strcmp("clsn1Default", name) || !strcmp("clsn1", name);
}


static int isLoopStart(char* tVariableName) {
	char text[100];
	sscanf(tVariableName, "%s", text);
	return !strcmp(text, "Loopstart");
}

static void handleLoopStart() {
	gMugenAnimationState.mIsLoopStart = 1;
}

static int isInterpolation(char* tVariableName) {
	char text1[100], text2[100];
	int items = sscanf(tVariableName, "%s %s", text1, text2);
	if (items < 2) return 0;
	
	return !strcmp("Interpolate", text1);
}

static void handleOffsetInterpolation(MugenAnimations* tAnimation, int tGroup) {
	MugenAnimation* anim = getMugenAnimation(tAnimation, tGroup);
	assert(vector_size(&anim->mSteps));
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateOffset = 1;
}

static void handleBlendInterpolation(MugenAnimations* tAnimation, int tGroup) {
	MugenAnimation* anim = getMugenAnimation(tAnimation, tGroup);
	assert(vector_size(&anim->mSteps));
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateBlend = 1;
}

static void handleScaleInterpolation(MugenAnimations* tAnimation, int tGroup) {
	MugenAnimation* anim = getMugenAnimation(tAnimation, tGroup);
	assert(vector_size(&anim->mSteps));
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateScale = 1;
}

static void handleAngleInterpolation(MugenAnimations* tAnimation, int tGroup) {
	MugenAnimation* anim = getMugenAnimation(tAnimation, tGroup);
	assert(vector_size(&anim->mSteps));
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateAngle = 1;
}

static void handleInterpolation(MugenDefScriptGroupElement* e, MugenAnimations* tAnimation, int tGroup) {
	char text1[100], text2[100];
	int items = sscanf(e->mName, "%s %s", text1, text2);
	assert(items == 2);
	assert(!strcmp("Interpolate", text1));

	if (!strcmp("Offset", text2)) {
		handleOffsetInterpolation(tAnimation, tGroup);
	}
	else if (!strcmp("Blend", text2)) {
		handleBlendInterpolation(tAnimation, tGroup);
	}
	else if (!strcmp("Scale", text2)) {
		handleScaleInterpolation(tAnimation, tGroup);
	}
	else if (!strcmp("Angle", text2)) {
		handleAngleInterpolation(tAnimation, tGroup);
	}
	else {
		logError("Unrecognized interpolation type.");
		logErrorString(text2);
		recoverFromError();
	}
}

static void loadSingleAnimationElementStatement(void* tCaller, void* tData) {
	MugenDefScriptGroupElement* element = (MugenDefScriptGroupElement*)tData;
	AnimationLoadCaller* caller = (AnimationLoadCaller*)tCaller;

	if (isAnimationVector(element->mName)) {
		handleNewAnimationStep(caller->mAnimations, caller->mGroupID, element);
	} else if (isHitboxSizeAssignment(element->mName)) {
		handleHitboxSizeAssignment(element->mName);
	}
	else if (isHitboxAssignment(element->mName)) {
		handleHitboxAssignment(element);
	}
	else if (isLoopStart(element->mName)) {
		handleLoopStart();
	}
	else if (isInterpolation(element->mName)) {
		handleInterpolation(element, caller->mAnimations, caller->mGroupID);
	}
	else {
		logWarning("Unrecognized type.");
		logWarningString(element->mName);
	}
}

static int isAnimationGroup(MugenDefScriptGroup* tGroup) {
	char text1[100], text2[100], val[100];

	int items = sscanf(tGroup->mName, "%s %s %s", text1, text2, val);

	if (items != 3) return 0;

	turnStringLowercase(text1);
	turnStringLowercase(text2);
	if (strcmp("begin", text1)) return 0;
	if (strcmp("action", text2)) return 0;

	return 1;
}

static void loadSingleAnimationGroup(MugenAnimations* tAnimations, MugenDefScriptGroup* tGroup) {
	if (!isAnimationGroup(tGroup)) return;

	int id = getAnimationID(tGroup);
	if (int_map_contains(&tAnimations->mAnimations, id)) return; // TODO: check id safe to ignore

	MugenAnimation* anim = makeEmptyMugenAnimation(id);
	int_map_push_owned(&tAnimations->mAnimations, id, anim);
	resetGlobalAnimationState();
	resetSingleAnimationState();

	AnimationLoadCaller caller;
	caller.mAnimations = tAnimations;
	caller.mGroupID = id;
	list_map(&tGroup->mOrderedElementList, loadSingleAnimationElementStatement, &caller);

	unsetSingleAnimationState();
	unsetGlobalAnimationState();
}

static void loadAnimationFileFromDefScript(MugenAnimations* tAnimations, MugenDefScript* tScript) {
	MugenDefScriptGroup* current = tScript->mFirstGroup;

	while (current != NULL) {
		loadSingleAnimationGroup(tAnimations, current);

		current = current->mNext;
	}

}

static MugenAnimations createEmptyMugenAnimationFile() {
	MugenAnimations ret;
	ret.mAnimations = new_int_map();
	int_map_push_owned(&ret.mAnimations, -1, createOneFrameMugenAnimationForSprite(-1, -1));
	return ret;
}

MugenAnimations loadMugenAnimationFile(std::string& tPath) {
	char path[1024];
	strcpy(path, tPath.data());
	return loadMugenAnimationFile(path);
}

MugenAnimations loadMugenAnimationFile(char * tPath)
{
	MugenAnimations ret = createEmptyMugenAnimationFile();

	MugenDefScript defScript = loadMugenDefScript(tPath);

	loadAnimationFileFromDefScript(&ret, &defScript);
	unloadMugenDefScript(defScript);

	return ret;
}

MugenAnimations loadMugenAnimationFileWithMemoryStack(char* tPath, MemoryStack* tMemoryStack) 
{
	gMugenAnimationState.mMemoryStack = tMemoryStack;
	MugenAnimations ret = loadMugenAnimationFile(tPath);
	gMugenAnimationState.mMemoryStack = NULL;
	return ret;
}

static void unloadSingleMugenAnimationStep(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimationStep* e = (MugenAnimationStep*)tData;
	delete_list(&e->mPassiveHitboxes);
	delete_list(&e->mAttackHitboxes);
}

static int unloadSingleMugenAnimation(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimation* e = (MugenAnimation*)tData;
	vector_map(&e->mSteps, unloadSingleMugenAnimationStep, NULL);
	delete_vector(&e->mSteps);
	return 1;
}

void unloadMugenAnimationFile(MugenAnimations * tAnimations)
{
	int_map_remove_predicate(&tAnimations->mAnimations, unloadSingleMugenAnimation, NULL);
	delete_int_map(&tAnimations->mAnimations);
}

MugenAnimation* getMugenAnimation(MugenAnimations * tAnimations, int i)
{
	if (!int_map_contains(&tAnimations->mAnimations, i)) {
		logWarning("Could not load animation. Defaulting to empty.");
		logWarningInteger(i);
		return (MugenAnimation*)int_map_get(&tAnimations->mAnimations, -1);
	}

	return (MugenAnimation*)int_map_get(&tAnimations->mAnimations, i);
}

MugenAnimation * createOneFrameMugenAnimationForSprite(int tSpriteGroup, int tSpriteItem)
{
	MugenAnimation* e = makeEmptyMugenAnimation(-1);
	MugenAnimationStep* step = (MugenAnimationStep*)allocMemoryOnMemoryStackOrMemory(sizeof(MugenAnimationStep));

	step->mPassiveHitboxes = new_list();
	step->mAttackHitboxes = new_list();
	step->mGroupNumber = tSpriteGroup;
	step->mSpriteNumber = tSpriteItem;
	step->mDelta = makePosition(0, 0, 0);
	step->mDuration = INF;
	step->mIsFlippingHorizontally = 0;
	step->mIsFlippingVertically = 0;

	step->mIsAddition = 0;
	step->mIsSubtraction = 0;

	step->mInterpolateOffset = 0;
	step->mInterpolateBlend = 0;
	step->mInterpolateScale = 0;
	step->mInterpolateAngle = 0;

	gMugenAnimationState.mIsLoopStart = 1;
	insertAnimationStepIntoAnimation(e, step);

	return e;
}

void destroyMugenAnimation(MugenAnimation * tAnimation)
{
	vector_empty(&tAnimation->mSteps); // TODO: free everything in steps
	delete_vector(&tAnimation->mSteps); 

	freeMemory(tAnimation);
}

Vector3DI getAnimationFirstElementSpriteSize(MugenAnimation * tAnimation, MugenSpriteFile* tSprites)
{
	assert(vector_size(&tAnimation->mSteps));
	MugenAnimationStep* firstStep = (MugenAnimationStep*)vector_get(&tAnimation->mSteps, 0);
	
	MugenSpriteFileSprite* sprite = getMugenSpriteFileTextureReference(tSprites, firstStep->mGroupNumber, firstStep->mSpriteNumber);
	if(!sprite)	return makeVector3DI(0, 0, 0);
	return makeVector3DI(sprite->mOriginalTextureSize.x, sprite->mOriginalTextureSize.y, 0);
}

Vector3D getAnimationFirstElementSpriteOffset(MugenAnimation * tAnimation, MugenSpriteFile* tSprites)
{
	assert(vector_size(&tAnimation->mSteps));
	MugenAnimationStep* firstStep = (MugenAnimationStep*)vector_get(&tAnimation->mSteps, 0);

	MugenSpriteFileSprite* sprite = getMugenSpriteFileTextureReference(tSprites, firstStep->mGroupNumber, firstStep->mSpriteNumber);
	return sprite->mAxisOffset;
}

int isMugenAnimationStepDurationInfinite(int tDuration)
{
	return tDuration == -1;
}

int hasMugenAnimation(MugenAnimations* tAnimations, int i) {
	return int_map_contains(&tAnimations->mAnimations, i);
}