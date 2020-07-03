#include "prism/animationtree.h"

#include <math.h>

#include "prism/script.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/memoryhandler.h"
#include "prism/animation.h"
#include "prism/file.h"
#include "prism/math.h"

typedef struct {

	Duration mDuration;
	TreeNode mTree;

} AnimationTreeFrame;

typedef struct {

	Vector mFrames;

} AnimationNode;


static void initTreeNode(TreeNode* e) {
	e->mSubElements = new_string_map();
	e->mAnchor = Vector3D(0, 0, 0);
	e->mTranslation = Vector3D(0, 0, 0);
	e->mRotationZ = 0;
}

static ScriptPosition loadTreeAnimation(TreeNode* e, const ScriptPosition& tPos) {
	e->mAnimation = createEmptyAnimation();

	int v;
	auto ret = getNextScriptInteger(tPos, &v);
	e->mAnimation.mDuration = v;
	ret = getNextScriptInteger(ret, &v);
	e->mAnimation.mFrameAmount = v;

	e->mTextures = (TextureData*)allocMemory(e->mAnimation.mFrameAmount*sizeof(TextureData));
	char name[1024];
	int i;
	for (i = 0; i < (int)e->mAnimation.mFrameAmount; i++) {
		ret = getNextScriptString(ret, name);
		e->mTextures[i] = loadTexture(name);
	}

	return ret;
}

static ScriptPosition loadTree(void* tCaller, const ScriptPosition& tPos) {
	TreeNode* e = (TreeNode*)tCaller;

	char word[100];
	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("SUB", word)) {
		char name[100];
		ret = getNextScriptString(ret, name);

		TreeNode* newNode = (TreeNode*)allocMemory(sizeof(TreeNode));
		initTreeNode(newNode);
		string_map_push_owned(&e->mSubElements, name, newNode);

		ScriptRegion reg = getScriptRegionAtPosition(ret);
		executeOnScriptRegion(reg, loadTree, newNode);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
	}
	else if (!strcmp("ANIMATION", word)) {
		ret = loadTreeAnimation(e, ret);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

static ScriptPosition loadSingleAnimationTree(void* tCaller, const ScriptPosition& tPos) {
	TreeNode* e = (TreeNode*)tCaller;

	char word[100];
	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("SUB", word)) {
		char name[100];
		ret = getNextScriptString(ret, name);

		TreeNode* newNode = (TreeNode*)allocMemory(sizeof(TreeNode));
		initTreeNode(newNode);
		string_map_push_owned(&e->mSubElements, name, newNode);

		ScriptRegion reg = getScriptRegionAtPosition(ret);
		executeOnScriptRegion(reg, loadSingleAnimationTree, newNode);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
		
	} else if (!strcmp("ANCHOR", word)) {
		ret = getNextScriptDouble(ret, &e->mAnchor.x);
		ret = getNextScriptDouble(ret, &e->mAnchor.y);
		ret = getNextScriptDouble(ret, &e->mAnchor.z);
	} else if (!strcmp("TRANSLATION", word)) {
		ret = getNextScriptDouble(ret, &e->mTranslation.x);
		ret = getNextScriptDouble(ret, &e->mTranslation.y);
		ret = getNextScriptDouble(ret, &e->mTranslation.z);
	} else if (!strcmp("ROTATION", word)) {
		ret = getNextScriptDouble(ret, &e->mRotationZ);
		e->mRotationZ = (e->mRotationZ*M_PI) / 180.0;
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

static ScriptPosition loadFrame(void* tCaller, const ScriptPosition& tPos) {
	AnimationTreeFrame* e = (AnimationTreeFrame*)tCaller;

	char word[100];
	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("DURATION", word)) {
		int v;
		ret = getNextScriptInteger(ret, &v);
		e->mDuration = v;
	} else if (!strcmp("TREE", word)) {
		initTreeNode(&e->mTree);
		ScriptRegion reg = getScriptRegionAtPosition(ret);
		executeOnScriptRegion(reg, loadSingleAnimationTree, &e->mTree);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

static void initTreeFrame(AnimationTreeFrame* e) {
	e->mDuration = 100000;
}

static ScriptPosition loadSingleAnimation(void* tCaller, const ScriptPosition& tPos) {
	AnimationNode* e = (AnimationNode*)tCaller;

	char word[100];
	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("FRAME", word)) {
		AnimationTreeFrame* newFrame = (AnimationTreeFrame*)allocMemory(sizeof(AnimationTreeFrame));
		initTreeFrame(newFrame);
		ScriptRegion reg = getScriptRegionAtPosition(ret);
		executeOnScriptRegion(reg, loadFrame, newFrame);
		vector_push_back_owned(&e->mFrames, newFrame);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
}

static void initAnimationNode(AnimationNode* e) {
	e->mFrames = new_vector();
}

static ScriptPosition loadAnimations(void* tCaller, const ScriptPosition& tPos) {
	StringMap* animations = (StringMap*)tCaller;

	char word[100];
	auto ret = getNextScriptString(tPos, word);

	if (!strcmp("ANIMATION", word)) {
		char name[100];
		ret = getNextScriptString(ret, name);
		AnimationNode* newNode = (AnimationNode*)allocMemory(sizeof(AnimationNode));
		initAnimationNode(newNode);

		string_map_push_owned(animations, name, newNode);

		ScriptRegion reg = getScriptRegionAtPosition(ret);
		executeOnScriptRegion(reg, loadSingleAnimation, newNode);
		ret = getPositionAfterScriptRegion(ret.mRegion, reg);
	}
	else {
		logError("Unrecognized token.");
		logErrorString(word);
		recoverFromError();
	}

	return ret;
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
	Position mPosition;
} AnimationTreeHandlerEntry;

static struct {
	List mAnimationTreeList;

} gPrismAnimationTreeData;

void setupAnimationTreeHandling() {
	gPrismAnimationTreeData.mAnimationTreeList = new_list();
}


void shutdownAnimationTreeHandling() {

}

void updateAnimationTreeHandling() {

}

static void drawTreeNode(void* tCaller, char* tKey, void* tData) {
	(void)tCaller;
	(void)tKey;
	TreeNode* tNode = (TreeNode*)tData;

	TextureData currentTexture = tNode->mTextures[tNode->mAnimation.mFrame];

	pushDrawingTranslation(tNode->mTranslation);
	pushDrawingRotationZ(tNode->mRotationZ, tNode->mAnchor);

	drawSprite(currentTexture, Vector3D(100,100,1), makeRectangleFromTexture(currentTexture));

	string_map_map(&tNode->mSubElements, drawTreeNode, NULL);

	popDrawingRotationZ();
	popDrawingTranslation();
}

static void drawSingleTree(void* tCaller, void* tData) {
	(void)tCaller;
	AnimationTreeHandlerEntry* e = (AnimationTreeHandlerEntry*)tData;

	string_map_map(&e->mTree.mTree.mSubElements, drawTreeNode, NULL);
}

void drawAnimationTreeHandling() {
	list_map(&gPrismAnimationTreeData.mAnimationTreeList, drawSingleTree, NULL);
}

static void copySingleTreeNode(TreeNode* tDst, TreeNode* tSrc) {
	tDst->mAnchor = tSrc->mAnchor;
	tDst->mTranslation = tSrc->mTranslation;
	tDst->mRotationZ = tSrc->mRotationZ;
}

static void copyTreeFunction(void* tCaller, char* tKey, void* tData) {
	TreeNode* dstRoot = (TreeNode*)tCaller;
	TreeNode* srcCur = (TreeNode*)tData;

	TreeNode* dstCur = (TreeNode*)string_map_get(&dstRoot->mSubElements, tKey);

	copySingleTreeNode(dstCur, srcCur);

	string_map_map(&srcCur->mSubElements, copyTreeFunction, dstCur);
}

static void setAnimationTreeTreeFromSource(TreeNode* tDst, TreeNode* tSrc) {
	copySingleTreeNode(tDst, tSrc);
	string_map_map(&tSrc->mSubElements, copyTreeFunction, tDst);
}

static void setAnimationTreeFrame(AnimationTree* tTree, int tFrame) {
	AnimationNode* e = (AnimationNode*)string_map_get(&tTree->mAnimations, tTree->mCurrentAnimation);
	AnimationTreeFrame* frame = (AnimationTreeFrame*)vector_get(&e->mFrames, tFrame);
	setAnimationTreeTreeFromSource(&tTree->mTree, &frame->mTree);
}

void setAnimationTreeAnimation(AnimationTree* tTree, char* tAnimation) {
	strcpy(tTree->mCurrentAnimation, tAnimation);
	setAnimationTreeFrame(tTree, 0);
}

int playAnimationTreeLoop(const Position& tPosition, const AnimationTree& tTree, char* tAnimation) {
	AnimationTreeHandlerEntry* e = (AnimationTreeHandlerEntry*)allocMemory(sizeof(AnimationTreeHandlerEntry));

	e->mTree = tTree;
	e->mPosition = tPosition;
	setAnimationTreeAnimation(&e->mTree, tAnimation);

	return list_push_back_owned(&gPrismAnimationTreeData.mAnimationTreeList, e);
}

void setHandledAnimationTreeAnimation(int tID, char* tAnimation) {
	AnimationTreeHandlerEntry* e = (AnimationTreeHandlerEntry*)list_get(&gPrismAnimationTreeData.mAnimationTreeList, tID);
	setAnimationTreeAnimation(&e->mTree, tAnimation);
}
