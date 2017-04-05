#include "include/animationtree.h"

#include "include/script.h"
#include "include/log.h"
#include "include/system.h"
#include "include/memoryhandler.h"
#include "include/animation.h"
#include "include/file.h"

typedef struct {

	Duration mDuration;
	TreeNode mTree;

} AnimationTreeFrame;

typedef struct {

	Vector mFrames;

} AnimationNode;


static void initTreeNode(TreeNode* e) {
	e->mSubElements = new_string_map();
	e->mAnchor = makePosition(0, 0, 0);
	e->mTranslation = makePosition(0, 0, 0);
	e->mRotationZ = 0;
}

static ScriptPosition loadTreeAnimation(TreeNode* e, ScriptPosition tPos) {
	e->mAnimation = createEmptyAnimation();

	int v;
	tPos = getNextScriptInteger(tPos, &v);
	e->mAnimation.mDuration = v;
	tPos = getNextScriptInteger(tPos, &e->mAnimation.mFrameAmount);

	e->mTextures = allocMemory(e->mAnimation.mFrameAmount*sizeof(TextureData));
	char name[1024];
	int i;
	for (i = 0; i < (int)e->mAnimation.mFrameAmount; i++) {
		tPos = getNextScriptString(tPos, name);
		e->mTextures[i] = loadTexture(name);
	}

	return tPos;
}

static ScriptPosition loadTree(void* tCaller, ScriptPosition tPos) {
	TreeNode* e = tCaller;

	char word[100];
	tPos = getNextScriptString(tPos, word);

	if (!strcmp("SUB", word)) {
		char name[100];
		tPos = getNextScriptString(tPos, name);

		TreeNode* newNode = allocMemory(sizeof(TreeNode));
		initTreeNode(newNode);
		string_map_push_owned(&e->mSubElements, name, newNode);

		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		executeOnScriptRegion(reg, loadTree, newNode);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
	}
	else if (!strcmp("ANIMATION", word)) {
		tPos = loadTreeAnimation(e, tPos);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

static ScriptPosition loadSingleAnimationTree(void* tCaller, ScriptPosition tPos) {
	TreeNode* e = tCaller;

	char word[100];
	tPos = getNextScriptString(tPos, word);

	if (!strcmp("SUB", word)) {
		char name[100];
		tPos = getNextScriptString(tPos, name);

		TreeNode* newNode = allocMemory(sizeof(TreeNode));
		initTreeNode(newNode);
		string_map_push_owned(&e->mSubElements, name, newNode);

		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		executeOnScriptRegion(reg, loadSingleAnimationTree, newNode);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
		
	} else if (!strcmp("ANCHOR", word)) {
		tPos = getNextScriptDouble(tPos, &e->mAnchor.x);
		tPos = getNextScriptDouble(tPos, &e->mAnchor.y);
		tPos = getNextScriptDouble(tPos, &e->mAnchor.z);
	} else if (!strcmp("TRANSLATION", word)) {
		tPos = getNextScriptDouble(tPos, &e->mTranslation.x);
		tPos = getNextScriptDouble(tPos, &e->mTranslation.y);
		tPos = getNextScriptDouble(tPos, &e->mTranslation.z);
	} else if (!strcmp("ROTATION", word)) {
		tPos = getNextScriptDouble(tPos, &e->mRotationZ);
		e->mRotationZ = (e->mRotationZ*M_PI) / 180.0;
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

static ScriptPosition loadFrame(void* tCaller, ScriptPosition tPos) {
	AnimationTreeFrame* e = tCaller;

	char word[100];
	tPos = getNextScriptString(tPos, word);

	if (!strcmp("DURATION", word)) {
		int v;
		tPos = getNextScriptInteger(tPos, &v);
		e->mDuration = v;
	} else if (!strcmp("TREE", word)) {
		initTreeNode(&e->mTree);
		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		executeOnScriptRegion(reg, loadSingleAnimationTree, &e->mTree);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

static void initTreeFrame(AnimationTreeFrame* e) {
	e->mDuration = 100000;
}

static ScriptPosition loadSingleAnimation(void* tCaller, ScriptPosition tPos) {
	AnimationNode* e = tCaller;

	char word[100];
	tPos = getNextScriptString(tPos, word);

	if (!strcmp("FRAME", word)) {
		AnimationTreeFrame* newFrame = allocMemory(sizeof(AnimationTreeFrame));
		initTreeFrame(newFrame);
		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		executeOnScriptRegion(reg, loadFrame, newFrame);
		vector_push_back_owned(&e->mFrames, newFrame);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

static void initAnimationNode(AnimationNode* e) {
	e->mFrames = new_vector();
}

static ScriptPosition loadAnimations(void* tCaller, ScriptPosition tPos) {
	StringMap* animations = tCaller;

	char word[100];
	tPos = getNextScriptString(tPos, word);

	if (!strcmp("ANIMATION", word)) {
		char name[100];
		tPos = getNextScriptString(tPos, name);
		AnimationNode* newNode = allocMemory(sizeof(AnimationNode));
		initAnimationNode(newNode);

		string_map_push_owned(animations, name, newNode);

		ScriptRegion reg = getScriptRegionAtPosition(tPos);
		executeOnScriptRegion(reg, loadSingleAnimation, newNode);
		tPos = getPositionAfterScriptRegion(tPos.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		abortSystem();
	}

	return tPos;
}

AnimationTree loadAnimationTree(char* tPath) {
	AnimationTree ret;

	Script script = loadScript(tPath);

	initTreeNode(&ret.mTree);
	executeOnScriptRegion(getScriptRegion(script, "TREE"), loadTree, &ret.mTree);

	ret.mAnimations = new_string_map();
	executeOnScriptRegion(getScriptRegion(script, "ANIMATIONS"), loadAnimations, &ret.mAnimations);

	return ret;
}



typedef struct {
	AnimationTree mTree;
} AnimationTreeHandlerEntry;

static struct {
	List mAnimationTreeList;

} gData;

void setupAnimationTreeHandling() {
	gData.mAnimationTreeList = new_list();
}


void shutdownAnimationTreeHandling() {

}

void updateAnimationTreeHandling() {

}



static void drawTreeNode(void* tCaller, char* tKey, void* tData) {
	(void)tCaller;
	(void)tKey;
	TreeNode* tNode = tData;

	TextureData currentTexture = tNode->mTextures[tNode->mAnimation.mFrame];

	pushDrawingTranslation(tNode->mTranslation);
	pushDrawingRotationZ(tNode->mRotationZ, tNode->mAnchor);

	drawSprite(currentTexture, makePosition(100,100,1), makeRectangleFromTexture(currentTexture));

	string_map_map(&tNode->mSubElements, drawTreeNode, NULL);

	popDrawingRotationZ();
	popDrawingTranslation();
}

static void drawSingleTree(void* tCaller, void* tData) {
	(void)tCaller;
	AnimationTreeHandlerEntry* e = tData;

	string_map_map(&e->mTree.mTree.mSubElements, drawTreeNode, NULL);
}

void drawAnimationTreeHandling() {
	setEffectCenterRelative();
	list_map(&gData.mAnimationTreeList, drawSingleTree, NULL);
	setEffectCenterAbsolute();
}

static void copySingleTreeNode(TreeNode* tDst, TreeNode* tSrc) {
	tDst->mAnchor = tSrc->mAnchor;
	tDst->mTranslation = tSrc->mTranslation;
	tDst->mRotationZ = tSrc->mRotationZ;
}

static void copyTreeFunction(void* tCaller, char* tKey, void* tData) {
	TreeNode* dstRoot = tCaller;
	TreeNode* srcCur = tData;

	TreeNode* dstCur = string_map_get(&dstRoot->mSubElements, tKey);

	copySingleTreeNode(dstCur, srcCur);

	string_map_map(&srcCur->mSubElements, copyTreeFunction, dstCur);
}

static void setAnimationTreeTreeFromSource(TreeNode* tDst, TreeNode* tSrc) {
	copySingleTreeNode(tDst, tSrc);
	string_map_map(&tSrc->mSubElements, copyTreeFunction, tDst);
}

static void setAnimationTreeFrame(AnimationTree* tTree, int tFrame) {
	AnimationNode* e = string_map_get(&tTree->mAnimations, tTree->mCurrentAnimation);
	AnimationTreeFrame* frame = vector_get(&e->mFrames, tFrame);
	setAnimationTreeTreeFromSource(&tTree->mTree, &frame->mTree);
}

void setAnimationTreeAnimation(AnimationTree* tTree, char* tAnimation) {
	strcpy(tTree->mCurrentAnimation, tAnimation);
	setAnimationTreeFrame(tTree, 0);
}

int playAnimationTreeLoop(Position tPosition, AnimationTree tTree, char* tAnimation) {
	AnimationTreeHandlerEntry* e = allocMemory(sizeof(AnimationTreeHandlerEntry)); 

	e->mTree = tTree;
	setAnimationTreeAnimation(&e->mTree, tAnimation);

	return list_push_back_owned(&gData.mAnimationTreeList, e);
}

void setHandledAnimationTreeAnimation(int tID, char* tAnimation) {
	AnimationTreeHandlerEntry* e = list_get(&gData.mAnimationTreeList, tID);
	setAnimationTreeAnimation(&e->mTree, tAnimation);
}






