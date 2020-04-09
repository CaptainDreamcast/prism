#include "prism/blitztimelineanimationreader.h"

#include <assert.h>
#include <string.h>
#include <algorithm>

#include "prism/mugendefreader.h"
#include "prism/memoryhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/math.h"

using namespace std;

static BlitzTimelineAnimations makeEmptyTimelineAnimations() {
	BlitzTimelineAnimations ret;
	ret.mAnimations = new_int_map();
	return ret;
}

static int isAnimationHeaderGroup(MugenDefScriptGroup* tGroup) {
	char text[100];
	strcpy(text, tGroup->mName.data());
	turnStringLowercase(text);

	return !strncmp(text, "animationdef", strlen("animationdef"));
}

static BlitzTimelineAnimation* loadBlitzTimelineAnimationHeaderFromGroup(MugenDefScriptGroup* tGroup) {
	BlitzTimelineAnimation* e = (BlitzTimelineAnimation*)allocMemory(sizeof(BlitzTimelineAnimation));
	char mGroupTextString[100];
	int items = sscanf(tGroup->mName.data(), "%s %d", mGroupTextString, &e->mID);
	if (items != 2) {
		logWarningFormat("Unable to parse blitz timeline animation header id: %s", tGroup->mName.data());
		e->mID = -5;
	}
	e->mDuration = getMugenDefIntegerOrDefaultAsGroup(tGroup, "duration", 0);
	e->mIsLooping = getMugenDefIntegerOrDefaultAsGroup(tGroup, "loop", 0);
	e->BlitzTimelineAnimationSteps = new_vector();
	return e;
}

static int isInterpolationGroup(MugenDefScriptGroup* tGroup) {
	char text[100];
	strcpy(text, tGroup->mName.data());
	turnStringLowercase(text);
	return !strcmp("interpolation", text);
}

static int isNamedElement(MugenDefScriptGroupElement* tElement, const char* tName) {
	return !strcmp(tName, tElement->mName.data());
}

static void addStaticAnimationStep(BlitzTimelineAnimation* tAnimation, int tTime, MugenDefScriptGroupElement* tElement, BlitzTimelineAnimationStepTargetType tTargetType, BlitzTimelineAnimationStepType tStepType) {
	BlitzTimelineAnimationStep* e = (BlitzTimelineAnimationStep*)allocMemory(sizeof(BlitzTimelineAnimationStep));

	e->mType = tStepType;
	e->mTargetType = tTargetType;
	e->mTime = tTime;
	e->mDuration = 0;

	char* value = getAllocatedMugenDefStringVariableAsElement(tElement);
	strcpy(e->mStartValue, value);
	strcpy(e->mEndValue, value);
	freeMemory(value);
	
	vector_push_back_owned(&tAnimation->BlitzTimelineAnimationSteps, e);
}

static void fixInterpolationAnimationStep(BlitzTimelineAnimation* tAnimation, BlitzTimelineAnimationStep* tPreviousStep, int tTime, MugenDefScriptGroupElement* tElement) {
	tPreviousStep->mType = BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION;
	tPreviousStep->mDuration = tTime - tPreviousStep->mTime;
	tAnimation->mDuration = max(tAnimation->mDuration, tPreviousStep->mTime + tPreviousStep->mDuration);

	char* value = getAllocatedMugenDefStringVariableAsElement(tElement);
	strcpy(tPreviousStep->mEndValue, value);
	freeMemory(value);
}

static void addInterpolationAnimationStep(BlitzTimelineAnimation* tAnimation, BlitzTimelineAnimationStep* tPreviousStep, int tTime, MugenDefScriptGroupElement* tElement, BlitzTimelineAnimationStepTargetType tTargetType) {
	fixInterpolationAnimationStep(tAnimation, tPreviousStep, tTime, tElement);
	addStaticAnimationStep(tAnimation, tTime, tElement, tTargetType, BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION_END);
}

static void handleSingleInterpolationGroupTargetElement(BlitzTimelineAnimation* tAnimation, int tTime, MugenDefScriptGroupElement* tElement, BlitzTimelineAnimationStepTargetType tTargetType) {
	BlitzTimelineAnimationStep* previousStep = NULL;
	int i;
	for (i = vector_size(&tAnimation->BlitzTimelineAnimationSteps) - 1; i >= 0; i--) {
		BlitzTimelineAnimationStep* step = (BlitzTimelineAnimationStep*)vector_get(&tAnimation->BlitzTimelineAnimationSteps, i);
		if (step->mType != BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION && step->mType != BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION_END) continue;
		if (step->mTargetType != tTargetType) continue;

		previousStep = step;
		break;
	}

	if (previousStep == NULL) {
		addStaticAnimationStep(tAnimation, tTime, tElement, tTargetType, BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION_END);
	}
	else { 
		addInterpolationAnimationStep(tAnimation, previousStep, tTime, tElement, tTargetType);
	}
}

static void handleSingleGroupElement(BlitzTimelineAnimationStepTargetType tTargetType, BlitzTimelineAnimation* tAnimation, int tTime, MugenDefScriptGroupElement* tElement, BlitzTimelineAnimationStepType tStepType) {
	if (tStepType == BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION) {
		handleSingleInterpolationGroupTargetElement(tAnimation, tTime, tElement, tTargetType);
	}
	else if (tStepType == BLITZ_TIMELINE_ANIMATION_STEP_TYPE_SETTING) {
		addStaticAnimationStep(tAnimation, tTime, tElement, tTargetType, BLITZ_TIMELINE_ANIMATION_STEP_TYPE_SETTING);
	}
	else {
		logWarningFormat("Unrecognized step type: %d", tStepType);
	}

}

static void handleSingleInterpolationGroupElement(BlitzTimelineAnimation* tAnimation, int tTime, MugenDefScriptGroupElement* tElement, BlitzTimelineAnimationStepType tStepType) {
	if (isNamedElement(tElement, "time")) {
		return;
	}
	else if (isNamedElement(tElement, "posx")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_POSITION_X, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "posy")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_POSITION_Y, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "scale")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_SCALE, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "scalex")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_SCALE_X, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "scaley")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_SCALE_Y, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "angle")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_ANGLE, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "cb")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_CALLBACK, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "velx")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_VELOCITY_X, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "vely")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_VELOCITY_Y, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "mugentransparency")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_MUGEN_TRANSPARENCY, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "mugenanimation")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_MUGEN_ANIMATION, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "mugenfacedirection")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_MUGEN_FACE_DIRECTION, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "mugenposx")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_MUGEN_POSITION_X, tAnimation, tTime, tElement, tStepType);
	}
	else if (isNamedElement(tElement, "mugenposy")) {
		handleSingleGroupElement(BLITZ_TIMELINE_ANIMATION_STEP_TARGET_TYPE_MUGEN_POSITION_Y, tAnimation, tTime, tElement, tStepType);
	}
	else {
		logWarningFormat("Unrecognized interpolation group element: %s", tElement->mName.data());
	}
}

static void handleInterpolationGroup(BlitzTimelineAnimation* tAnimation, MugenDefScriptGroup* tGroup, BlitzTimelineAnimationStepType tStepType) {
	int time = getMugenDefIntegerOrDefaultAsGroup(tGroup, "time", 0);

	if (!list_size(&tGroup->mOrderedElementList)) return;

	ListIterator iterator = list_iterator_begin(&tGroup->mOrderedElementList);
	while (1) {
		MugenDefScriptGroupElement* element = (MugenDefScriptGroupElement*)list_iterator_get(iterator);
		handleSingleInterpolationGroupElement(tAnimation, time, element, tStepType);

		if (!list_has_next(iterator)) break;
		list_iterator_increase(&iterator);
	}

}

static int isSettingGroup(MugenDefScriptGroup* tGroup) {
	char text[100];
	strcpy(text, tGroup->mName.data());
	turnStringLowercase(text);
	return !strcmp("set", text);
}

static void handleSingleAnimationSubGroup(BlitzTimelineAnimation* tAnimation, MugenDefScriptGroup* tGroup) {
	if (isInterpolationGroup(tGroup)) {
		handleInterpolationGroup(tAnimation, tGroup, BLITZ_TIMELINE_ANIMATION_STEP_TYPE_INTERPOLATION);
	}
	else if (isSettingGroup(tGroup)) {
		handleInterpolationGroup(tAnimation, tGroup, BLITZ_TIMELINE_ANIMATION_STEP_TYPE_SETTING);
	}
}

static int groupSortCB(void* tCaller, void* tData1, void* tData2) {
	(void)tCaller;
	MugenDefScriptGroup* group1 = (MugenDefScriptGroup*)tData1;
	MugenDefScriptGroup* group2 = (MugenDefScriptGroup*)tData2;

	int time1 = getMugenDefIntegerOrDefaultAsGroup(group1, "time", 0);
	int time2 = getMugenDefIntegerOrDefaultAsGroup(group2, "time", 0);
	return time1 > time2;
}

static MugenDefScriptGroup* handleSingleAnimation(BlitzTimelineAnimations* tAnimations, MugenDefScriptGroup* tGroup) {
	assert(isAnimationHeaderGroup(tGroup));
	BlitzTimelineAnimation* animation = loadBlitzTimelineAnimationHeaderFromGroup(tGroup);

	Vector groups = new_vector();

	tGroup = tGroup->mNext;
	while (tGroup && !isAnimationHeaderGroup(tGroup)) {
		vector_push_back(&groups, tGroup);
		tGroup = tGroup->mNext;
	}

	vector_sort(&groups, groupSortCB, NULL);

	int i;
	for (i = 0; i < vector_size(&groups); i++) {
		MugenDefScriptGroup* group = (MugenDefScriptGroup*)vector_get(&groups, i);
		handleSingleAnimationSubGroup(animation, group);
	}

	delete_vector(&groups);

	int_map_push_owned(&tAnimations->mAnimations, animation->mID, animation);

	return tGroup;
}

static void loadAnimationsFromScript(BlitzTimelineAnimations* tAnimations, MugenDefScript* tScript) {

	MugenDefScriptGroup* currentGroup = tScript->mFirstGroup;
	while (currentGroup) {
		currentGroup = handleSingleAnimation(tAnimations, currentGroup);
	}

}

BlitzTimelineAnimations loadBlitzTimelineAnimations(const char * tPath)
{
	BlitzTimelineAnimations ret = makeEmptyTimelineAnimations();
	MugenDefScript script;
	loadMugenDefScript(&script, tPath);

	loadAnimationsFromScript(&ret, &script);

	unloadMugenDefScript(script);
	return ret;
}

BlitzTimelineAnimation * getBlitzTimelineAnimation(BlitzTimelineAnimations * tAnimations, int tAnimationID)
{
	if (!int_map_contains(&tAnimations->mAnimations, tAnimationID)) {
		logErrorFormat("Unable to find blitz timeline animation with id: %d", tAnimationID);
		recoverFromError();
	}

	return (BlitzTimelineAnimation*)int_map_get(&tAnimations->mAnimations, tAnimationID);
}
