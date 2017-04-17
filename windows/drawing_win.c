#include "../include/drawing.h"

#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "../include/log.h"
#include "../include/system.h"
#include "../include/datastructures.h"
#include "../include/memoryhandler.h"




typedef struct {

	double a;
	double r;
	double g;
	double b;

	Vector3D mTranslation;
	Vector3D mScale;
	Vector3D mAngle;
	Position mEffectCenter;
	int mIsEffectCenterAbsolute;

	int mFrameStartTime;

	Vector mEffectStack;
} DrawingData;

typedef struct {
	TextureData mTexture;
	Position mPos;
	Rectangle mTexturePosition;

	DrawingData mData;
} DrawListElement;


static Vector gDrawVector;
static DrawingData gData;

extern SDL_Window* gSDLWindow;
SDL_Renderer* gRenderer;

void initDrawing() {
	logg("Initiate drawing.");
	setDrawingParametersToIdentity();

	gRenderer = SDL_CreateRenderer(gSDLWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL) {
		logError("Unable to create renderer.");
		abortSystem();
	}
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

	IMG_Init(IMG_INIT_PNG);

	gDrawVector = new_vector();
	gData.mFrameStartTime = 0;

	gData.mTranslation = makePosition(0, 0, 0);
	gData.mEffectStack = new_vector();
	gData.mIsEffectCenterAbsolute = 1;
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {

  debugLog("Draw Sprite");
  debugInteger(tTexture.mTextureSize.x);
  debugInteger(tTexture.mTextureSize.y);

  if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0 || tTexture.mTextureSize.x % 2 != 0 || tTexture.mTextureSize.y % 2 != 0) {
    logError("Called with invalid textureSize");
    logErrorInteger(tTexture.mTextureSize.x);
    logErrorInteger(tTexture.mTextureSize.y);

    return;
  }

  DrawListElement* e = allocMemory(sizeof(DrawListElement));
  e->mTexture = tTexture;
  e->mPos = tPos;
  e->mPos = vecAdd(e->mPos, gData.mTranslation);
  e->mTexturePosition = tTexturePosition;
  e->mData = gData;
  vector_push_back_owned(&gDrawVector, e);
}

void startDrawing() {
	SDL_RenderClear(gRenderer);
	vector_empty(&gDrawVector);
}

static int cmpZ(void* tCaller, void* tData1, void* tData2) {
	(void*)tCaller;
	DrawListElement* e1 = tData1;
	DrawListElement* e2 = tData2;
	
	if (e1->mPos.z < e2->mPos.z) return -1;
	if (e1->mPos.z > e2->mPos.z) return 1;
	else return 0;
}

static SDL_Rect makeSDLRectFromRectangle(Rectangle tRect) {
	SDL_Rect ret;
	ret.x = min(tRect.topLeft.x, tRect.bottomRight.x);
	ret.y = min(tRect.topLeft.y, tRect.bottomRight.y);

	ret.w = abs(tRect.bottomRight.x - tRect.topLeft.x);
	ret.h = abs(tRect.bottomRight.y - tRect.topLeft.y);

	return ret;
}

static Rectangle makeRectangleFromSDLRect(SDL_Rect tRect) {
	Rectangle ret;
	ret.topLeft.x = tRect.x;
	ret.topLeft.y = tRect.y;

	ret.bottomRight.x = tRect.x + tRect.w;
	ret.bottomRight.y = tRect.y + tRect.h;

	return ret;
}


static SDL_Rect scaleSDLRect(SDL_Rect tRect, Vector3D tScale, Position tCenter) {
	Rectangle rect = makeRectangleFromSDLRect(tRect);

	rect = translateRectangle(rect, vecScale(tCenter, -1));
	rect = scaleRectangle(rect, tScale);
	rect = translateRectangle(rect, tCenter);

	return makeSDLRectFromRectangle(rect);
}

static void drawSorted(void* tCaller, void* tData) {
	(void)tCaller;

	DrawListElement* e = tData;

	int sizeX = abs(e->mTexturePosition.bottomRight.x - e->mTexturePosition.topLeft.x);
	int sizeY = abs(e->mTexturePosition.bottomRight.y - e->mTexturePosition.topLeft.y);

	double left = e->mTexturePosition.topLeft.x / ((double)(e->mTexture.mTextureSize.x - 1));
	double right = e->mTexturePosition.bottomRight.x / ((double)(e->mTexture.mTextureSize.x - 1));
	double up = e->mTexturePosition.topLeft.y / ((double)(e->mTexture.mTextureSize.y - 1));
	double down = e->mTexturePosition.bottomRight.y / ((double)(e->mTexture.mTextureSize.y - 1));

	SDL_Rect srcRect;
	srcRect.x = min(e->mTexturePosition.topLeft.x, e->mTexturePosition.bottomRight.x);
	srcRect.y = min(e->mTexturePosition.topLeft.y, e->mTexturePosition.bottomRight.y);
	srcRect.w = sizeX;
	srcRect.h = sizeY;

	SDL_Rect dstRect;
	dstRect.x = (int)e->mPos.x;
	dstRect.y = (int)e->mPos.y;
	dstRect.w = sizeX;
	dstRect.h = sizeY;

	dstRect = scaleSDLRect(dstRect, e->mData.mScale, e->mData.mEffectCenter);

	Position realEffectPos;
	if (e->mData.mIsEffectCenterAbsolute) {
		realEffectPos = vecAdd(e->mData.mEffectCenter, vecScale(e->mPos, -1));
	}
	else {
		realEffectPos = e->mData.mEffectCenter;
	}
	realEffectPos = vecScale3D(realEffectPos, e->mData.mScale);

	SDL_Point effectCenter;
	effectCenter.x = (int)realEffectPos.x;
	effectCenter.y = (int)realEffectPos.y;

	int flip = 0;
	if (e->mTexturePosition.bottomRight.x < e->mTexturePosition.topLeft.x) flip |= SDL_FLIP_HORIZONTAL;
	if (e->mTexturePosition.bottomRight.y < e->mTexturePosition.topLeft.y) flip |= SDL_FLIP_VERTICAL;

	double angleDegrees = 360-((e->mData.mAngle.z * 180)/ M_PI);

	SDL_SetTextureColorMod(e->mTexture.mTexture->mTexture, (Uint8)(e->mData.r*0xFF), (Uint8)(e->mData.g*0xFF), (Uint8)(e->mData.b*0xFF));
	SDL_SetTextureAlphaMod(e->mTexture.mTexture->mTexture, (Uint8)(e->mData.a * 0xFF));

	SDL_RenderCopyEx(gRenderer, e->mTexture.mTexture->mTexture, &srcRect, &dstRect, angleDegrees, &effectCenter, flip);
}

void stopDrawing() {
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
	vector_sort(&gDrawVector,cmpZ, NULL);
	vector_map(&gDrawVector, drawSorted, NULL);
	vector_empty(&gDrawVector);
	SDL_RenderPresent(gRenderer);
}

void waitForScreen() {
	double frameMS = (1.0 / 60) * 1000;
	int frameEndTime = (int)(gData.mFrameStartTime + ceil(frameMS));
	int waitTime = frameEndTime-SDL_GetTicks();

	if (waitTime > 0) {
		SDL_Delay(waitTime);
	}

	gData.mFrameStartTime = SDL_GetTicks();
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

void drawText(char tText[], Position tPosition, TextSize tSize, Color tColor) {
	int current = 0;

	setDrawingBaseColor(tColor);

	TextureData fontData = getFontTexture();
	
	while (tText[current] != '\0') {

		FontCharacterData charData = getFontCharacterData(tText[current]);

		Rectangle tTexturePosition;
		tTexturePosition.topLeft.x = (int)(fontData.mTextureSize.x*charData.mFilePositionX1);
		tTexturePosition.topLeft.y = (int)(fontData.mTextureSize.y*charData.mFilePositionY1);
		tTexturePosition.bottomRight.x = (int)(fontData.mTextureSize.x*charData.mFilePositionX2);
		tTexturePosition.bottomRight.y = (int)(fontData.mTextureSize.y*charData.mFilePositionY2);

		scaleDrawing(tSize / fabs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y), tPosition);
		drawSprite(fontData, tPosition, tTexturePosition);


		tPosition.x += tSize;
		current++;
	}

	setDrawingParametersToIdentity();

}

void scaleDrawing(double tFactor, Position tScalePosition){
	gData.mScale = makePosition(tFactor,tFactor,1);
	gData.mEffectCenter = tScalePosition;
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition){
	gData.mScale = tFactor;
	gData.mEffectCenter = tScalePosition;
}

void setDrawingBaseColor(Color tColor){
	getRGBFromColor(tColor, &gData.r, &gData.g, &gData.b);
}

void setDrawingBaseColorAdvanced(double r, double g, double b) {
	gData.r = r;
	gData.g = g;
	gData.b = b;
}

void setDrawingTransparency(double tAlpha){
	gData.a = tAlpha;
}

void setDrawingRotationZ(double tAngle, Position tPosition){
	gData.mAngle.z = tAngle;
	gData.mEffectCenter = tPosition;
}

void setDrawingParametersToIdentity(){
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	scaleDrawing(1, makePosition(0, 0, 0));
	setDrawingRotationZ(0, makePosition(0,0,0));
}

void setEffectCenterRelative() {
	gData.mIsEffectCenterAbsolute = 0;
}

void setEffectCenterAbsolute() {
	gData.mIsEffectCenterAbsolute = 1;
}

typedef struct {
	Vector3D mTranslation;

} TranslationEffect;

typedef struct {
	double mAngle;

} RotationZEffect;

void pushDrawingTranslation(Vector3D tTranslation) {
	tTranslation = vecRotateZ(tTranslation, 2*M_PI-gData.mAngle.z);
	gData.mTranslation = vecAdd(gData.mTranslation, tTranslation);

	TranslationEffect* e = allocMemory(sizeof(TranslationEffect));
	e->mTranslation = tTranslation;
	vector_push_back_owned(&gData.mEffectStack, e);
}
void pushDrawingRotationZ(double tAngle, Vector3D tCenter) {
	gData.mEffectCenter = tCenter;
	gData.mAngle.z += tAngle;

	RotationZEffect* e = allocMemory(sizeof(RotationZEffect));
	e->mAngle = tAngle;
	vector_push_back_owned(&gData.mEffectStack, e);
	
}

void popDrawingRotationZ() {
	int ind = vector_size(&gData.mEffectStack)-1;
	RotationZEffect* e = vector_get(&gData.mEffectStack, ind);
	
	gData.mAngle.z -= e->mAngle;

	vector_remove(&gData.mEffectStack, ind);
}
void popDrawingTranslation() {
	int ind = vector_size(&gData.mEffectStack) - 1;
	TranslationEffect* e = vector_get(&gData.mEffectStack, ind);

	Vector3D tTranslation = e->mTranslation;
	gData.mTranslation = vecAdd(gData.mTranslation, vecScale(tTranslation,-1));

	vector_remove(&gData.mEffectStack, ind);
}


