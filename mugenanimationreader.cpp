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
	int mIsReadingHitbox1;
} gMugenAnimationState;

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
	MugenAnimation* ret = (MugenAnimation*)allocMemory(sizeof(MugenAnimation));
	ret->mLoopStart = 0;
	ret->mSteps = new_vector();
	ret->mID = tID;
	ret->mTotalDuration = 0;
	return ret;
}

static int getAnimationID(MugenDefScriptGroup* tGroup) {
	char text1[20], text2[20];
	int ret;
	int items = sscanf(tGroup->mName.data(), "%s %s %d", text1, text2, &ret);
	if (items != 3) {
		logWarningFormat("Unable to parse animation ID: %s", tGroup->mName.data());
		ret = -5;
	}
	return ret;
}

typedef struct {
	MugenAnimations* mAnimations;
	int mGroupID;
	int mInterpolateLastOffset;
	int mInterpolateLastBlend;
	int mInterpolateLastScale;
	int mInterpolateLastAngle;
} AnimationLoadCaller;

static int isAnimationVector(const char* tVariableName) {
	char text[100];
	int items = sscanf(tVariableName, "%s", text);
	return items == 1 && !strcmp(text, "vector_statement");
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

static void handleNewAnimationStepBlendFlags(MugenAnimationStep* e, const char* blendFlagsRandomCase) {
	std::string blendFlags; 
	copyStringLowercase(blendFlags, blendFlagsRandomCase);
	
	setPrismFlagConditionalDynamic(e->mFlags, MugenAnimationStepFlags::IS_ADDITION, blendFlags[0] == 'a');
	setPrismFlagConditionalDynamic(e->mFlags, MugenAnimationStepFlags::IS_SUBTRACTION, blendFlags[0] == 's');
	if (hasPrismFlagDynamic(e->mFlags, MugenAnimationStepFlags::IS_ADDITION) || hasPrismFlagDynamic(e->mFlags, MugenAnimationStepFlags::IS_SUBTRACTION)) {
		const char* sourcePos = strchr(blendFlags.c_str() + 1, 's');
		if (sourcePos == NULL) {
			if (blendFlags[1] == '1') {
				e->mSrcBlendFactor = 256 / double(256);
				e->mDstBlendFactor = 128 / double(256);
			}
			else {
				e->mSrcBlendFactor = 256 / double(256);
				e->mDstBlendFactor = 256 / double(256);
			}
		}
		else {
			const char* dstPos = strchr(blendFlags.c_str() + 1, 'd');
			assert(dstPos != NULL);

			char text1[20];
			char text2[20];

			strcpy(text1, sourcePos + 1);
			*strchr(text1, 'd') = '\0';

			strcpy(text2, dstPos + 1);

			e->mSrcBlendFactor = atoi(text1) / double(256);
			e->mDstBlendFactor = atoi(text2) / double(256);
		}
	}
	else {
		e->mSrcBlendFactor = e->mDstBlendFactor = 1.0;
	}
}

static void copySingleHitboxToNewList(void* tCaller, void* tData) {
	List* dst = (List*)tCaller;
	CollisionRect* e = (CollisionRect*)tData;

	CollisionRect* newRect = (CollisionRect*)allocMemory(sizeof(CollisionRect));
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
	
	MugenAnimationStep* e = (MugenAnimationStep*)allocMemory(sizeof(MugenAnimationStep));
	e->mFlags = uint32_t(MugenAnimationStepFlags::NONE);
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
		turnStringLowercase(vectorElement->mVector.mElement[5]);
		setPrismFlagConditionalDynamic(e->mFlags, MugenAnimationStepFlags::IS_FLIPPING_HORIZONTALLY, strchr(vectorElement->mVector.mElement[5], 'h') != NULL);
		setPrismFlagConditionalDynamic(e->mFlags, MugenAnimationStepFlags::IS_FLIPPING_VERTICALLY, strchr(vectorElement->mVector.mElement[5], 'v') != NULL);
	}

	if (vectorElement->mVector.mSize >= 7) {
		handleNewAnimationStepBlendFlags(e, vectorElement->mVector.mElement[6]);
	}
	else {
		handleNewAnimationStepBlendFlags(e, "");
	}

	e->mScaleX = vectorElement->mVector.mSize >= 8 ? atof(vectorElement->mVector.mElement[7]) : 1.0;
	e->mScaleY = vectorElement->mVector.mSize >= 9 ? atof(vectorElement->mVector.mElement[8]) : 1.0;
	e->mAngleRad = vectorElement->mVector.mSize >= 10 ? degreesToRadians(double(atoi(vectorElement->mVector.mElement[9]))) : 0.0;

	e->mInterpolateOffset = 0;
	e->mInterpolateBlend = 0;
	e->mInterpolateScale = 0;
	e->mInterpolateAngle = 0;

	insertAnimationStepIntoAnimations(tAnimations, tGroupID, e);
	resetSingleAnimationState();
}

static void handleHitboxSizeAssignment(const char* tName) {
	char name[100];
	int items = sscanf(tName, "%s", name);
	if (items != 1) {
		logError("Unparseable collision size name.");
		logErrorString(name);
		recoverFromError();
	}

	if (!strcmp("clsn1default", name)) {
		gMugenAnimationState.mIsReadingDefaultHitbox = 1;
		gMugenAnimationState.mIsReadingHitbox1 = 1;
	}
	else if(!strcmp("clsn2default", name)) {
		gMugenAnimationState.mIsReadingDefaultHitbox = 1;
		gMugenAnimationState.mIsReadingHitbox1 = 0;
	}
	else if (!strcmp("clsn1", name)) {
		gMugenAnimationState.mIsReadingDefaultHitbox = 0;
		gMugenAnimationState.mIsReadingHitbox1 = 1;
	}
	else if (!strcmp("clsn2", name)) {
		gMugenAnimationState.mIsReadingDefaultHitbox = 0;
		gMugenAnimationState.mIsReadingHitbox1 = 0;
	}
	else {
		logError("Unrecognized collision size name.");
		logErrorString(name);
		recoverFromError();
	}
}

static int isHitboxSizeAssignment(const char* tName) {
	char name[100];
	int items = sscanf(tName, "%s", name);
	return items == 1 && (!strcmp("clsn2default", name) || !strcmp("clsn2", name) || !strcmp("clsn1default", name) || !strcmp("clsn1", name));
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
	if (gMugenAnimationState.mIsReadingHitbox1) {
		if (gMugenAnimationState.mIsReadingDefaultHitbox) {
			handleCollisionHitboxAssignment(&gMugenAnimationState.mDefaultHitboxes1, tElement);
		}
		else {
			gMugenAnimationState.mHasOwnHitbox1 = 1;
			handleCollisionHitboxAssignment(&gMugenAnimationState.mOwnHitbox1, tElement);
		}
	}
	else {
		if (gMugenAnimationState.mIsReadingDefaultHitbox) {
			handleCollisionHitboxAssignment(&gMugenAnimationState.mDefaultHitboxes2, tElement);
		}
		else {
			gMugenAnimationState.mHasOwnHitbox2 = 1;
			handleCollisionHitboxAssignment(&gMugenAnimationState.mOwnHitbox2, tElement);
		}
	}
}

static int isHitboxAssignment(const char* tName) {
	char name[100];
	strcpy(name, tName);
	char* opening = strchr(name, '[');
	if (!opening) return 0;

	*opening = '\0';

	return !strcmp("clsn2Default", name) || !strcmp("clsn2", name) || !strcmp("clsn1Default", name) || !strcmp("clsn1", name);
}


static int isLoopStart(const char* tVariableName) {
	char text[100];
	int items = sscanf(tVariableName, "%s", text);
	return items == 1 && !strcmp(text, "Loopstart");
}

static void handleLoopStart() {
	gMugenAnimationState.mIsLoopStart = 1;
}

static int isInterpolation(const char* tVariableName) {
	char text1[100], text2[100];
	int items = sscanf(tVariableName, "%s %s", text1, text2);
	if (items < 2) return 0;
	turnStringLowercase(text1);
	return !strcmp("interpolate", text1);
}

static void handleOffsetInterpolation(AnimationLoadCaller* caller) {
	MugenAnimation* anim = getMugenAnimation(caller->mAnimations, caller->mGroupID);
	if (!vector_size(&anim->mSteps)) {
		caller->mInterpolateLastOffset = 1;
		return;
	}
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateOffset = 1;
}

static void handleBlendInterpolation(AnimationLoadCaller* caller) {
	MugenAnimation* anim = getMugenAnimation(caller->mAnimations, caller->mGroupID);
	if (!vector_size(&anim->mSteps)) {
		caller->mInterpolateLastBlend = 1;
		return;
	}
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateBlend = 1;
}

static void handleScaleInterpolation(AnimationLoadCaller* caller) {
	MugenAnimation* anim = getMugenAnimation(caller->mAnimations, caller->mGroupID);
	if (!vector_size(&anim->mSteps)) {
		caller->mInterpolateLastScale = 1;
		return;
	}
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateScale = 1;
}

static void handleAngleInterpolation(AnimationLoadCaller* caller) {
	MugenAnimation* anim = getMugenAnimation(caller->mAnimations, caller->mGroupID);
	if (!vector_size(&anim->mSteps)) {
		caller->mInterpolateLastAngle = 1;
		return;
	}
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&anim->mSteps, vector_size(&anim->mSteps) - 1);

	step->mInterpolateAngle = 1;
}

static void handleInterpolation(AnimationLoadCaller* caller, MugenDefScriptGroupElement* e) {
	char text1[100], text2[100];
	int items = sscanf(e->mName.data(), "%s %s", text1, text2);
	(void)items;
	assert(items == 2);
	turnStringLowercase(text1);
	assert(!strcmp("interpolate", text1));

	turnStringLowercase(text2);
	if (!strcmp("offset", text2)) {
		handleOffsetInterpolation(caller);
	}
	else if (!strcmp("blend", text2)) {
		handleBlendInterpolation(caller);
	}
	else if (!strcmp("scale", text2)) {
		handleScaleInterpolation(caller);
	}
	else if (!strcmp("angle", text2)) {
		handleAngleInterpolation(caller);
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

	if (isAnimationVector(element->mName.data())) {
		handleNewAnimationStep(caller->mAnimations, caller->mGroupID, element);
	} else if (isHitboxSizeAssignment(element->mName.data())) {
		handleHitboxSizeAssignment(element->mName.data());
	}
	else if (isHitboxAssignment(element->mName.data())) {
		handleHitboxAssignment(element);
	}
	else if (isLoopStart(element->mName.data())) {
		handleLoopStart();
	}
	else if (isInterpolation(element->mName.data())) {
		handleInterpolation(caller, element);
	}
	else {
		logWarning("Unrecognized type.");
		logWarningString(element->mName.data());
	}
}

static int isAnimationGroup(MugenDefScriptGroup* tGroup) {
	char text1[100], text2[100], val[100];

	int items = sscanf(tGroup->mName.data(), "%s %s %s", text1, text2, val);

	if (items != 3) return 0;

	turnStringLowercase(text1);
	turnStringLowercase(text2);
	if (strcmp("begin", text1)) return 0;
	if (strcmp("action", text2)) return 0;

	return 1;
}

static void setFinalAnimationElementInterpolation(AnimationLoadCaller* tCaller, MugenAnimation* tAnimation) {
	if (!vector_size(&tAnimation->mSteps)) return;
	MugenAnimationStep* step = (MugenAnimationStep*)vector_get(&tAnimation->mSteps, vector_size(&tAnimation->mSteps) - 1);
	step->mInterpolateOffset = tCaller->mInterpolateLastOffset;
	step->mInterpolateBlend = tCaller->mInterpolateLastBlend;
	step->mInterpolateScale = tCaller->mInterpolateLastScale;
	step->mInterpolateAngle = tCaller->mInterpolateLastAngle;
}

static void loadSingleAnimationGroup(MugenAnimations* tAnimations, MugenDefScriptGroup* tGroup) {
	if (!isAnimationGroup(tGroup)) return;

	int id = getAnimationID(tGroup);
	if (int_map_contains(&tAnimations->mAnimations, id)) return;

	MugenAnimation* anim = makeEmptyMugenAnimation(id);
	int_map_push_owned(&tAnimations->mAnimations, id, anim);
	resetGlobalAnimationState();
	resetSingleAnimationState();

	AnimationLoadCaller caller;
	caller.mAnimations = tAnimations;
	caller.mGroupID = id;
	caller.mInterpolateLastOffset = 0;
	caller.mInterpolateLastBlend = 0;
	caller.mInterpolateLastScale = 0;
	caller.mInterpolateLastAngle = 0;
	list_map(&tGroup->mOrderedElementList, loadSingleAnimationElementStatement, &caller);
	setFinalAnimationElementInterpolation(&caller, anim);

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

MugenAnimations loadMugenAnimationFile(std::string tPath) {
	return loadMugenAnimationFile(tPath.data());
}

MugenAnimations loadMugenAnimationFile(const char * tPath)
{
	MugenAnimations ret = createEmptyMugenAnimationFile();

	MugenDefScript defScript; 
	loadMugenDefScript(&defScript, tPath);

	loadAnimationFileFromDefScript(&ret, &defScript);
	unloadMugenDefScript(defScript);

	return ret;
}

static void unloadSingleMugenAnimationStep(void* tCaller, void* tData) {
	(void)tCaller;
	MugenAnimationStep* e = (MugenAnimationStep*)tData;
	delete_list(&e->mPassiveHitboxes);
	delete_list(&e->mAttackHitboxes);
}

static void unloadSingleMugenAnimation(MugenAnimation* e) {
	vector_map(&e->mSteps, unloadSingleMugenAnimationStep, NULL);
	delete_vector(&e->mSteps);
}

static int unloadSingleMugenAnimation(void* /*tCaller*/, void* tData) {
	unloadSingleMugenAnimation((MugenAnimation*)tData);
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
	MugenAnimationStep* step = (MugenAnimationStep*)allocMemory(sizeof(MugenAnimationStep));

	step->mPassiveHitboxes = new_list();
	step->mAttackHitboxes = new_list();
	step->mGroupNumber = tSpriteGroup;
	step->mSpriteNumber = tSpriteItem;
	step->mDelta = makePosition(0, 0, 0);
	step->mDuration = INF;
	step->mFlags = uint32_t(MugenAnimationStepFlags::NONE);
	step->mSrcBlendFactor = step->mDstBlendFactor = 1.0;

	step->mScaleX = step->mScaleY = 1.0;
	step->mAngleRad = 0.0;

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
	unloadSingleMugenAnimation(tAnimation);
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
	if (!sprite) return makePosition(0, 0, 0);
	return sprite->mAxisOffset;
}

int isMugenAnimationStepDurationInfinite(int tDuration)
{
	return tDuration == -1;
}

int hasMugenAnimation(MugenAnimations* tAnimations, int i) {
	return int_map_contains(&tAnimations->mAnimations, i);
}
