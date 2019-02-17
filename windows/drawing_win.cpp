#include "../include/prism/drawing.h"

#include <stdlib.h>
#include <assert.h>
#include <algorithm>

#include <SDL.h>
#include <GL/glew.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#elif defined _WIN32
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include "prism/log.h"
#include "prism/system.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/math.h"



static const GLchar *vertex_shader =
"uniform mat4 ProjMtx;\n"
"attribute vec3 Position;\n"
"attribute vec2 UV;\n"
"attribute vec4 Color;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	Frag_UV = UV;\n"
"	Frag_Color = Color;\n"
"	gl_Position = ProjMtx * vec4(Position.xyz,1);\n"
"}\n";

static const GLchar* fragment_shader =
#ifdef __EMSCRIPTEN__
// WebGL requires precision specifiers but OpenGL 2.1 disallows
// them, so I define the shader without it and then add it here.
"precision mediump float;\n"
#endif
"uniform sampler2D Texture;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV);\n"
"}\n";

using namespace std;

typedef struct {

	double a;
	double r;
	double g;
	double b;

	Matrix4D mTransformationMatrix;
	int mIsIdentity;

	Vector mEffectStack;

	int mIsDisabled;

	BlendType mBlendType;

	SDL_Color mPalettes[4][256];
} DrawingData;

typedef struct {
	TextureData mTexture;
	Position mPos;
	Rectangle mTexturePosition;

	DrawingData mData;

} DrawListSpriteElement;

typedef struct {
	char mText[1024];
	TTF_Font* mFont;
	Position mPos;
	Vector3DI mTextSize;
	Vector3D mColor;
	double mTextBoxWidth;

	DrawingData mData;
} DrawListTruetypeElement;

typedef enum {
	DRAW_LIST_ELEMENT_TYPE_SPRITE,
	DRAW_LIST_ELEMENT_TYPE_TRUETYPE,

} DrawListElementType;

typedef struct {
	DrawListElementType mType;
	void* mData;
	double mZ;
} DrawListElement;


static struct {
	int          mShaderHandle, mVertHandle, mFragHandle;
	int          mAttribLocationTex, mAttribLocationProjMtx;
	int          mAttribLocationPosition, mAttribLocationUV, mAttribLocationColor;
	unsigned int mVboHandle, mElementsHandle;

	Vector3D mScreenScale;
} gOpenGLData;

static struct {
	int mFrameStartTime;
	double mRealFramerate;
} gBookkeepingData;

static Vector gDrawVector;
static DrawingData gData;

extern SDL_Window* gSDLWindow;

static void initOpenGL() {

	gOpenGLData.mShaderHandle = glCreateProgram();
	gOpenGLData.mVertHandle = glCreateShader(GL_VERTEX_SHADER);
	gOpenGLData.mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(gOpenGLData.mVertHandle, 1, &vertex_shader, NULL);
	glShaderSource(gOpenGLData.mFragHandle, 1, &fragment_shader, 0);
	glCompileShader(gOpenGLData.mVertHandle);
	glCompileShader(gOpenGLData.mFragHandle);
	glAttachShader(gOpenGLData.mShaderHandle, gOpenGLData.mVertHandle);
	glAttachShader(gOpenGLData.mShaderHandle, gOpenGLData.mFragHandle);
	glLinkProgram(gOpenGLData.mShaderHandle);

	gOpenGLData.mAttribLocationTex = glGetUniformLocation(gOpenGLData.mShaderHandle, "Texture");
	gOpenGLData.mAttribLocationProjMtx = glGetUniformLocation(gOpenGLData.mShaderHandle, "ProjMtx");
	gOpenGLData.mAttribLocationPosition = glGetAttribLocation(gOpenGLData.mShaderHandle, "Position");
	gOpenGLData.mAttribLocationUV = glGetAttribLocation(gOpenGLData.mShaderHandle, "UV");
	gOpenGLData.mAttribLocationColor = glGetAttribLocation(gOpenGLData.mShaderHandle, "Color");

	glGenBuffers(1, &gOpenGLData.mVboHandle);
	glGenBuffers(1, &gOpenGLData.mElementsHandle);

	glUseProgram(gOpenGLData.mShaderHandle);
	glUniform1i(gOpenGLData.mAttribLocationTex, 0);

	// Render command lists
	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glEnableVertexAttribArray(gOpenGLData.mAttribLocationPosition);
	glEnableVertexAttribArray(gOpenGLData.mAttribLocationUV);
	glEnableVertexAttribArray(gOpenGLData.mAttribLocationColor);
	int stride = sizeof(GLfloat) * 9;
	glVertexAttribPointer(gOpenGLData.mAttribLocationPosition, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glVertexAttribPointer(gOpenGLData.mAttribLocationUV, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 3));
	glVertexAttribPointer(gOpenGLData.mAttribLocationColor, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 5));
	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gOpenGLData.mElementsHandle);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	glViewport(0, 0, 640, 480);
	glClearColor(0, 0, 0, 1);
}

void setDrawingScreenScale(double tScaleX, double tScaleY);

static void createEmptySDLPalette(SDL_Color* tColors) {

	int i;
	for (i = 0; i < 256; i++) {
		tColors[i].a = tColors[i].r = tColors[i].g = tColors[i].b = 0;
	}
}

void initDrawing() {
	if (gSDLWindow == NULL) {
		logError("Unable to init drawing without SDL window.");
		recoverFromError();
	}

	ScreenSize sz = getScreenSize();
	setDrawingScreenScale((640.0 / sz.x), (480.0 / sz.y));
	setDrawingParametersToIdentity();


	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	gDrawVector = new_vector();
	gBookkeepingData.mFrameStartTime = 0;

	gData.mEffectStack = new_vector();

	int i;
	for (i = 0; i < 4; i++) {
		createEmptySDLPalette(gData.mPalettes[i]);
	}

	gData.mIsDisabled = 0;

	initOpenGL();
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {
	if (gData.mIsDisabled) return;
	
	if (gData.mIsIdentity) {
		double sx = tTexture.mTextureSize.x;
		double sy = tTexture.mTextureSize.y;
		double maxDelta = max(sx, sy);
		ScreenSize sz = getScreenSize(); // TODO: fix
		if (tPos.x + maxDelta < 0) return;
		if (tPos.x - maxDelta >= sz.x) return;
		if (tPos.y + maxDelta < 0) return;
		if (tPos.y - maxDelta >= sz.y) return;
	}

	debugLog("Draw Sprite");
	debugInteger(tTexture.mTextureSize.x);
	debugInteger(tTexture.mTextureSize.y);

	if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0) {
		logError("Called with invalid textureSize");
		logErrorInteger(tTexture.mTextureSize.x);
		logErrorInteger(tTexture.mTextureSize.y);

		return;
	}


	DrawListSpriteElement* e = (DrawListSpriteElement*)allocMemory(sizeof(DrawListSpriteElement));
	e->mTexture = tTexture;
	e->mPos = tPos;
	e->mTexturePosition = tTexturePosition;
	e->mData = gData;

	DrawListElement* listElement = (DrawListElement*)allocMemory(sizeof(DrawListElement));
	listElement->mType = DRAW_LIST_ELEMENT_TYPE_SPRITE;
	listElement->mData = e;
	listElement->mZ = tPos.z;
	vector_push_back_owned(&gDrawVector, listElement);
}

static void clearSingleDrawElement(void* tCaller, void* tData) {
	(void)tCaller;
	DrawListElement* e = (DrawListElement*)tData;
	freeMemory(e->mData);
}

static void clearDrawVector() {
	vector_map(&gDrawVector, clearSingleDrawElement, NULL);
	vector_empty(&gDrawVector);
}

void startDrawing() {
	glClear(GL_COLOR_BUFFER_BIT);

	clearDrawVector();
}

static int cmpZ(void* tCaller, void* tData1, void* tData2) {
	(void)tCaller;
	DrawListElement* e1 = (DrawListElement*)tData1;
	DrawListElement* e2 = (DrawListElement*)tData2;

	if (e1->mZ < e2->mZ) return -1;
	if (e1->mZ > e2->mZ) return 1;
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

static void setSingleVertex(GLfloat* tDst, Position tPosition, double tU, double tV, Position tColor, double tAlpha) {
	tDst[0] = (GLfloat)tPosition.x;
	tDst[1] = (GLfloat)tPosition.y;
	tDst[2] = (GLfloat)0;
	tDst[3] = (GLfloat)tU;
	tDst[4] = (GLfloat)tV;
	tDst[5] = (GLfloat)tColor.x;
	tDst[6] = (GLfloat)tColor.y;
	tDst[7] = (GLfloat)tColor.z;
	tDst[8] = (GLfloat)tAlpha;
}

static void drawOpenGLTexture(GLuint tTextureID, GeoRectangle tSrcRect, GeoRectangle tDstRect, DrawingData* tData) {
	Matrix4D* finalMatrix = &tData->mTransformationMatrix;

	float matrix[4][4];
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			matrix[y][x] = (float)finalMatrix->m[y][x];
		}
	}

	glUniformMatrix4fv(gOpenGLData.mAttribLocationProjMtx, 1, GL_FALSE, &matrix[0][0]);

	GLfloat vertices[4 * 9];
	setSingleVertex(&vertices[0 * 9], tDstRect.mTopLeft, tSrcRect.mTopLeft.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[1 * 9], makePosition(tDstRect.mBottomRight.x, tDstRect.mTopLeft.y, tDstRect.mTopLeft.z), tSrcRect.mBottomRight.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[2 * 9], tDstRect.mBottomRight, tSrcRect.mBottomRight.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[3 * 9], makePosition(tDstRect.mTopLeft.x, tDstRect.mBottomRight.y, tDstRect.mTopLeft.z), tSrcRect.mTopLeft.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);

	int stride = sizeof(GLfloat) * 9;
	glBufferData(GL_ARRAY_BUFFER, 4 * stride, vertices, GL_STREAM_DRAW);

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};


	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STREAM_DRAW);

	glBindTexture(GL_TEXTURE_2D, tTextureID);
	glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, 0);


}

static void drawSDLSurface(SDL_Surface* tSurface, GeoRectangle tSrcRect, GeoRectangle tDstRect, DrawingData* tData) {
	GLuint textureID;
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tSurface->w, tSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tSurface->pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture);

	drawOpenGLTexture(textureID, tSrcRect, tDstRect, tData);

	glDeleteTextures(1, &textureID);
}


static void drawPalettedSDLSurface(SDL_Surface* tSurface, int tPaletteID, GeoRectangle tSrcRect, GeoRectangle tDstRect, DrawingData* tData) {

	SDL_SetPaletteColors(tSurface->format->palette, gData.mPalettes[tPaletteID], 0, 256);

	SDL_Surface* surface = SDL_ConvertSurfaceFormat(tSurface, SDL_PIXELFORMAT_RGBA32, 0);
	drawSDLSurface(surface, tSrcRect, tDstRect, tData);
	SDL_FreeSurface(surface);
}

static void drawSortedSprite(DrawListSpriteElement* e) {
	int sizeX = abs(e->mTexturePosition.bottomRight.x - e->mTexturePosition.topLeft.x) + 1;
	int sizeY = abs(e->mTexturePosition.bottomRight.y - e->mTexturePosition.topLeft.y) + 1;


	GeoRectangle srcRect;
	if (e->mTexturePosition.topLeft.x < e->mTexturePosition.bottomRight.x) {
		srcRect.mTopLeft.x = e->mTexturePosition.topLeft.x / (double)(e->mTexture.mTextureSize.x);
		srcRect.mBottomRight.x = (e->mTexturePosition.bottomRight.x + 1) / (double)(e->mTexture.mTextureSize.x);
	}
	else {
		srcRect.mTopLeft.x = (e->mTexturePosition.topLeft.x + 1) / (double)(e->mTexture.mTextureSize.x);
		srcRect.mBottomRight.x = e->mTexturePosition.bottomRight.x / (double)(e->mTexture.mTextureSize.x);
	}

	if (e->mTexturePosition.topLeft.y < e->mTexturePosition.bottomRight.y) {
		srcRect.mTopLeft.y = e->mTexturePosition.topLeft.y / (double)(e->mTexture.mTextureSize.y);
		srcRect.mBottomRight.y = (e->mTexturePosition.bottomRight.y + 1) / (double)(e->mTexture.mTextureSize.y);
	}
	else {
		srcRect.mTopLeft.y = (e->mTexturePosition.topLeft.y + 1) / (double)(e->mTexture.mTextureSize.y);
		srcRect.mBottomRight.y = e->mTexturePosition.bottomRight.y / (double)(e->mTexture.mTextureSize.y);
	}

	GeoRectangle dstRect;
	dstRect.mTopLeft = e->mPos;
	dstRect.mBottomRight = vecAdd(e->mPos, makePosition(sizeX, sizeY, 0));

	Texture texture = (Texture)e->mTexture.mTexture->mData;

	if (e->mData.mBlendType == BLEND_TYPE_ADDITION) {
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	}
	else if (e->mData.mBlendType == BLEND_TYPE_NORMAL) {
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (e->mData.mBlendType == BLEND_TYPE_SUBTRACTION) {
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	}
	else {
		logError("Unimplemented blend type");
		logErrorInteger(e->mData.mBlendType);
		recoverFromError();
	}

	if (e->mTexture.mHasPalette) {
		drawPalettedSDLSurface(texture->mSurface, e->mTexture.mPaletteID, srcRect, dstRect, &e->mData);
	}
	else {
		drawOpenGLTexture(texture->mTexture, srcRect, dstRect, &e->mData);
	}
}

static int isTextPositionEmpty(char tChar) {
	return tChar == ' ';
}


static void drawSortedTruetype(DrawListTruetypeElement* e) {
	int l = strlen(e->mText);
	if (!l) return;

	SDL_Color color;
	color.a = 0xFF;
	color.r = (Uint8)(0xFF * e->mColor.x);
	color.g = (Uint8)(0xFF * e->mColor.y);
	color.b = (Uint8)(0xFF * e->mColor.z);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Position pos = e->mPos;
	int i;
	for (i = 0; i < l;) {
		char text[1024];

		int start = i;
		int previousWordStart = 0;
		int j;
		for (j = 0; j < l - start; j++) {
			int w, h;
			text[j] = e->mText[start + j];
			text[j + 1] = '\0';
			TTF_SizeText(e->mFont, text, &w, &h);
			if (isTextPositionEmpty(text[j])) previousWordStart = j;

			if (w > e->mTextBoxWidth && !isTextPositionEmpty(text[j]) && previousWordStart > 0) {
				text[previousWordStart + 1] = '\0';
				break;
			}
			else if (j == l - start - 1) {
				previousWordStart = j;
			}
		}
		int end = start + previousWordStart;

		SDL_Surface* surface = TTF_RenderText_Blended(e->mFont, text, color);

		GeoRectangle rect;
		rect.mTopLeft = pos;
		rect.mBottomRight = vecAdd(pos, makePosition(surface->w, surface->h, 0));

		GeoRectangle src = makeGeoRectangle(0, 0, surface->w, surface->h);

		pos.y += surface->h;


		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
		drawSDLSurface(formattedSurface, src, rect, &e->mData);

		SDL_FreeSurface(formattedSurface);
		SDL_FreeSurface(surface);

		i = end + 1;
	}
}

static void drawSorted(void* tCaller, void* tData) {
	(void)tCaller;
	DrawListElement* e = (DrawListElement*)tData;

	if (e->mType == DRAW_LIST_ELEMENT_TYPE_SPRITE) {
		drawSortedSprite((DrawListSpriteElement*)e->mData);
	}
	else if (e->mType == DRAW_LIST_ELEMENT_TYPE_TRUETYPE) {
		drawSortedTruetype((DrawListTruetypeElement*)e->mData);
	}
	else {
		logError("Unrecognized draw type");
		logErrorInteger(e->mType);
		recoverFromError();
	}

}

void stopDrawing() {
	vector_sort(&gDrawVector, cmpZ, NULL);
	vector_map(&gDrawVector, drawSorted, NULL);
	clearDrawVector();
	SDL_GL_SwapWindow(gSDLWindow);
}

void waitForScreen() {
	double frameMS = (1.0 / 60) * 1000;
	int frameEndTime = (int)(gBookkeepingData.mFrameStartTime + ceil(frameMS));
	int waitTime = frameEndTime - SDL_GetTicks();
	if (waitTime > 0) {
#ifndef __EMSCRIPTEN__
		SDL_Delay(waitTime);
#endif
	}

	int now = SDL_GetTicks();
	gBookkeepingData.mRealFramerate = 1000.0 / (now - gBookkeepingData.mFrameStartTime);
	gBookkeepingData.mFrameStartTime = now;
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

// TODO: refactor into general drawing code so both have it
static int hasToLinebreak(char* tText, int tCurrent, Position tTopLeft, Position tPos, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize) {

	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;

	char word[1024];
	int positionsRead;
	sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);

	Position delta = makePosition(positionsRead * tFontSize.x + (positionsRead - 1) * tBreakSize.x, 0, 0);
	Position after = vecAdd(tPos, delta);
	Position bottomRight = vecAdd(tTopLeft, tTextBoxSize);

	return (after.x > bottomRight.x);
}

void drawMultilineText(char* tText, char* tFullText, Position tPosition, Vector3D tFontSize, Color tColor, Vector3D tBreakSize, Vector3D tTextBoxSize) {
	int current = 0;

	setDrawingBaseColor(tColor);

	TextureData fontData = getFontTexture();
	Position pos = tPosition;

	while (tText[current] != '\0') {
		FontCharacterData charData = getFontCharacterData(tText[current]);

		Rectangle tTexturePosition;
		tTexturePosition.topLeft.x = (int)(fontData.mTextureSize.x*charData.mFilePositionX1);
		tTexturePosition.topLeft.y = (int)(fontData.mTextureSize.y*charData.mFilePositionY1);
		tTexturePosition.bottomRight.x = (int)(fontData.mTextureSize.x*charData.mFilePositionX2);
		tTexturePosition.bottomRight.y = (int)(fontData.mTextureSize.y*charData.mFilePositionY2);

		double dx = (double)abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x);
		double dy = (double)abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y);
		Vector3D scale = makePosition(1 / dx, 1 / dy, 1);
		scaleDrawing3D(vecScale3D(tFontSize, scale), pos);

		drawSprite(fontData, pos, tTexturePosition);

		pos.x += tFontSize.x + tBreakSize.x;
		current++;

		if (hasToLinebreak(tFullText, current, tPosition, pos, tFontSize, tBreakSize, tTextBoxSize)) {
			pos.x = tPosition.x - (tFontSize.x + tBreakSize.x);
			pos.y += tFontSize.y + tBreakSize.y;
		}
	}

	setDrawingParametersToIdentity();
}

void drawTruetypeText(char * tText, TruetypeFont tFont, Position tPosition, Vector3DI tTextSize, Vector3D tColor, double tTextBoxWidth)
{
	DrawListTruetypeElement* e = (DrawListTruetypeElement*)allocMemory(sizeof(DrawListTruetypeElement));
	strcpy(e->mText, tText);
	e->mFont = (TTF_Font*)tFont;
	e->mPos = tPosition;
	e->mTextSize = tTextSize;
	e->mColor = tColor;
	e->mTextBoxWidth = tTextBoxWidth;
	e->mData = gData;

	DrawListElement* listElement = (DrawListElement*)allocMemory(sizeof(DrawListElement));
	listElement->mType = DRAW_LIST_ELEMENT_TYPE_TRUETYPE;
	listElement->mData = e;
	listElement->mZ = tPosition.z;
	vector_push_back_owned(&gDrawVector, listElement);
}

void scaleDrawing(double tFactor, Position tScalePosition) {
	scaleDrawing3D(makePosition(tFactor, tFactor, 1), tScalePosition);
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition) {
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(tScalePosition));
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createScaleMatrix4D(makePosition(tFactor.x, tFactor.y, tFactor.z)));
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tScalePosition, -1)));
	gData.mIsIdentity = tFactor.x == 1 && tFactor.y == 1;
}

void setDrawingBaseColor(Color tColor) {
	getRGBFromColor(tColor, &gData.r, &gData.g, &gData.b);
}

void setDrawingBaseColorAdvanced(double r, double g, double b) {
	gData.r = r;
	gData.g = g;
	gData.b = b;
}

void setDrawingTransparency(double tAlpha) {
	gData.a = tAlpha;
}

void setDrawingRotationZ(double tAngle, Position tPosition) {
	tAngle = (2 * M_PI - tAngle); // TODO: but why
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(tPosition));
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createRotationZMatrix4D(tAngle));
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tPosition, -1)));
	gData.mIsIdentity = tAngle == 2 * M_PI;
}

void setDrawingParametersToIdentity() {
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	setDrawingBlendType(BLEND_TYPE_NORMAL);

	ScreenSize sz = getScreenSize();
	Vector3D realScreenSize = makePosition(sz.x*gOpenGLData.mScreenScale.x, sz.y*gOpenGLData.mScreenScale.y, 0);
	gData.mTransformationMatrix = createOrthographicProjectionMatrix4D(0, realScreenSize.x, 0, realScreenSize.y, 0, 100);
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(makePosition(0, realScreenSize.y - gOpenGLData.mScreenScale.y*sz.y, 0)));
	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createScaleMatrix4D(makePosition(gOpenGLData.mScreenScale.x, gOpenGLData.mScreenScale.y, 1)));
	gData.mIsIdentity = 1;
}

void setDrawingBlendType(BlendType tBlendType)
{
	gData.mBlendType = tBlendType;
}

typedef struct {
	double mAngle;
	Position mCenter;
} RotationZEffect;

typedef struct {
	Vector3D mTranslation;

} TranslationEffect;

// TODO: check or remove, cause no game uses this stuff
void pushDrawingTranslation(Vector3D tTranslation) {

	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(tTranslation));

	TranslationEffect* e = (TranslationEffect*)allocMemory(sizeof(TranslationEffect));
	e->mTranslation = tTranslation;
	vector_push_back_owned(&gData.mEffectStack, e);
}
void pushDrawingRotationZ(double tAngle, Vector3D tCenter) {
	setDrawingRotationZ(tAngle, tCenter);

	RotationZEffect* e = (RotationZEffect*)allocMemory(sizeof(RotationZEffect));
	e->mAngle = tAngle;
	e->mCenter = tCenter;
	vector_push_back_owned(&gData.mEffectStack, e);
}

void popDrawingRotationZ() {
	int ind = vector_size(&gData.mEffectStack) - 1;
	RotationZEffect* e = (RotationZEffect*)vector_get(&gData.mEffectStack, ind);

	setDrawingRotationZ(-e->mAngle, e->mCenter);

	vector_remove(&gData.mEffectStack, ind);
}
void popDrawingTranslation() {
	int ind = vector_size(&gData.mEffectStack) - 1;
	TranslationEffect* e = (TranslationEffect*)vector_get(&gData.mEffectStack, ind);

	gData.mTransformationMatrix = matMult4D(gData.mTransformationMatrix, createTranslationMatrix4D(vecScale(e->mTranslation, -1)));

	vector_remove(&gData.mEffectStack, ind);
}


static uint32_t* getPixelFromSurface(SDL_Surface* tSurface, int x, int y) {
	uint32_t* pixels = (uint32_t*)tSurface->pixels;
	return &pixels[y*tSurface->w + x];
}

#define PIXEL_BUFFER_SIZE 1000
uint32_t gPixelBuffer[PIXEL_BUFFER_SIZE];

void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget) {
	Texture dst = (Texture)tDst.mTexture->mData;

	double rd, gd, bd;
	getRGBFromColor(tColor, &rd, &gd, &bd);
	uint8_t r = (uint8_t)(rd * 255);
	uint8_t g = (uint8_t)(gd * 255);
	uint8_t b = (uint8_t)(bd * 255);

	int w = tTarget.bottomRight.x - tTarget.topLeft.x + 1;
	int h = tTarget.bottomRight.y - tTarget.topLeft.y + 1;
	if (w * h >= PIXEL_BUFFER_SIZE) {
		logError("Over pixel buffer limit.");
		logErrorInteger(w);
		logErrorInteger(h);
		recoverFromError();
	}

	uint32_t val = SDL_MapRGB(dst->mSurface->format, r, g, b);
	int i;
	for (i = 0; i < w*h; i++) {
		gPixelBuffer[i] = val;
	}


	SDL_Rect rect;
	rect.x = tTarget.topLeft.x;
	rect.y = tTarget.topLeft.y;
	rect.w = w;
	rect.h = h;
	// SDL_UpdateTexture(dst->mTexture, &rect, gPixelBuffer, w*sizeof(uint32_t)); // TODO
}


void disableDrawing() {
	gData.mIsDisabled = 1;
}

void enableDrawing() {
	gData.mIsDisabled = 0;
}


void setDrawingScreenScale(double tScaleX, double tScaleY) {

	gOpenGLData.mScreenScale = makePosition(tScaleX, tScaleY, 1);

	ScreenSize sz = getScreenSize();
	Vector3D realScreenSize = makePosition(sz.x*gOpenGLData.mScreenScale.x, sz.y*gOpenGLData.mScreenScale.y, 0);
	glViewport(0, 0, (GLsizei)realScreenSize.x, (GLsizei)realScreenSize.y);
}

void setPaletteFromARGB256Buffer(int tPaletteID, Buffer tBuffer) {
	assert(tBuffer.mLength == 256 * 4);

	SDL_Color* palette = gData.mPalettes[tPaletteID];

	int amount = tBuffer.mLength / 4;
	int i;
	for (i = 0; i < amount; i++) {
		palette[i].a = ((Uint8*)tBuffer.mData)[4 * i + 0];
		palette[i].r = ((Uint8*)tBuffer.mData)[4 * i + 1];
		palette[i].g = ((Uint8*)tBuffer.mData)[4 * i + 2];
		palette[i].b = ((Uint8*)tBuffer.mData)[4 * i + 3];
	}
}

void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, Buffer tBuffer)
{
	assert(tBuffer.mLength == 256 * 3);

	SDL_Color* palette = gData.mPalettes[tPaletteID];

	int amount = 256;
	int i;
	palette[0].a = palette[0].r = palette[0].g = palette[0].b = 0;
	for (i = 1; i < amount; i++) {
		palette[i].a = 0xFF;
		palette[i].b = ((Uint8*)tBuffer.mData)[3 * i + 0];
		palette[i].g = ((Uint8*)tBuffer.mData)[3 * i + 1];
		palette[i].r = ((Uint8*)tBuffer.mData)[3 * i + 2];
	}
}

SDL_Color* getSDLColorPalette(int tIndex) {
	return gData.mPalettes[tIndex];
}
double getRealFramerate() {
	return gBookkeepingData.mRealFramerate;
}