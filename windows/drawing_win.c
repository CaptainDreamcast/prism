#include "../include/tari/drawing.h"

#include <stdlib.h>
#include <assert.h>

#include <SDL.h>
#include <GL/glew.h>
#ifdef __EMSCRIPTEN__
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#elif defined _WIN32
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include "tari/log.h"
#include "tari/system.h"
#include "tari/datastructures.h"
#include "tari/memoryhandler.h"
#include "tari/math.h"



static const GLchar *vertex_shader =
"uniform mat4 ProjMtx;\n"
"attribute vec2 Position;\n"
"attribute vec2 UV;\n"
"attribute vec4 Color;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	Frag_UV = UV;\n"
"	Frag_Color = Color;\n"
"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
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


typedef struct {

	double a;
	double r;
	double g;
	double b;

	Vector3D mTranslation;
	Vector3D mScale;
	Vector3D mAngle;
	Position mScaleEffectCenter;
	Position mRotationEffectCenter;
	int mIsScaleEffectCenterAbsolute;
	int mIsRotationEffectCenterAbsolute;

	int mFrameStartTime;

	Vector mEffectStack;

	int mIsDisabled;

	BlendType mBlendType;
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

} gOpenGLData;

static Vector gDrawVector;
static DrawingData gData;

extern SDL_Window* gSDLWindow;

static void initOpenGL() {
	GLint last_texture, last_array_buffer;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

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
	
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
}

void initDrawing() {
	setDrawingParametersToIdentity();

	if (gSDLWindow == NULL) {
		logError("Unable to init drawing without SDL window.");
		abortSystem();
	}

	ScreenSize sz = getScreenSize();
	// SDL_RenderSetScale(gRenderer, (float)(640.0 / sz.x), (float)(480.0 / sz.y)); // TODO: set rendering scale 

	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	gDrawVector = new_vector();
	gData.mFrameStartTime = 0;

	gData.mTranslation = makePosition(0, 0, 0);
	gData.mEffectStack = new_vector();
	gData.mIsScaleEffectCenterAbsolute = 1;
	gData.mIsRotationEffectCenterAbsolute = 1;

	gData.mIsDisabled = 0;

	initOpenGL();
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {
	if (gData.mIsDisabled) return;

  debugLog("Draw Sprite");
  debugInteger(tTexture.mTextureSize.x);
  debugInteger(tTexture.mTextureSize.y);

  if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0) {
    logError("Called with invalid textureSize");
    logErrorInteger(tTexture.mTextureSize.x);
    logErrorInteger(tTexture.mTextureSize.y);

    return;
  }


  DrawListSpriteElement* e = allocMemory(sizeof(DrawListSpriteElement));
  e->mTexture = tTexture;
  e->mPos = tPos;
  e->mPos = vecAdd(e->mPos, gData.mTranslation);
  e->mTexturePosition = tTexturePosition;
  e->mData = gData;

  DrawListElement* listElement = allocMemory(sizeof(DrawListElement));
  listElement->mType = DRAW_LIST_ELEMENT_TYPE_SPRITE;
  listElement->mData = e;
  listElement->mZ = tPos.z;
  vector_push_back_owned(&gDrawVector, listElement);
}

static void clearSingleDrawElement(void* tCaller, void* tData) {
	(void)tCaller;
	DrawListElement* e = tData;
	freeMemory(e->mData);
}

static void clearDrawVector() {
	vector_map(&gDrawVector, clearSingleDrawElement, NULL);
	vector_empty(&gDrawVector);
}

void startDrawing() {

	glViewport(0, 0, 640, 480);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	clearDrawVector();
}

static int cmpZ(void* tCaller, void* tData1, void* tData2) {
	(void)tCaller;
	DrawListElement* e1 = tData1;
	DrawListElement* e2 = tData2;
	
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


static SDL_Rect scaleSDLRect(SDL_Rect tRect, Vector3D tScale, Position tCenter) {
	Rectangle rect = makeRectangleFromSDLRect(tRect);

	rect = translateRectangle(rect, vecScale(tCenter, -1));
	rect = scaleRectangle(rect, tScale);
	rect = translateRectangle(rect, tCenter);

	return makeSDLRectFromRectangle(rect);
}

static void setSingleVertex(GLfloat* tDst, double tX, double tY, double tU, double tV) {
	tDst[0] = tX;
	tDst[1] = tY;
	tDst[2] = tU;
	tDst[3] = tV;
	tDst[4] = tDst[5] = tDst[6] = tDst[7] = 1;
}

static void drawOpenGLTexture(GLuint tTextureID, SDL_Rect tSrcRect, SDL_Rect tDstRect) {
	GLint last_program, last_texture, last_array_buffer, last_element_array_buffer;
	glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);


	// Setup orthographic projection matrix
	const float ortho_projection[4][4] =
	{
		{ 2.0f / 640, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 2.0f / -480, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -1.0f, 0.0f },
		{ -1.0f, 1.0f, 0.0f, 1.0f },
	};
	glUseProgram(gOpenGLData.mShaderHandle);
	glUniform1i(gOpenGLData.mAttribLocationTex, 0);
	glUniformMatrix4fv(gOpenGLData.mAttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

	// Render command lists
	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glEnableVertexAttribArray(gOpenGLData.mAttribLocationPosition);
	glEnableVertexAttribArray(gOpenGLData.mAttribLocationUV);
	glEnableVertexAttribArray(gOpenGLData.mAttribLocationColor);


	int stride = sizeof(GLfloat) * 8;
	glVertexAttribPointer(gOpenGLData.mAttribLocationPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
	glVertexAttribPointer(gOpenGLData.mAttribLocationUV, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 2));
	glVertexAttribPointer(gOpenGLData.mAttribLocationColor, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 4));

	GLfloat vertices[4 * 8];
	setSingleVertex(&vertices[0 * 8], tDstRect.x, tDstRect.y, 0, 0);
	setSingleVertex(&vertices[1 * 8], tDstRect.x + tDstRect.w, tDstRect.y, 1, 0);
	setSingleVertex(&vertices[2 * 8], tDstRect.x + tDstRect.w, tDstRect.y + tDstRect.h, 1, 1);
	setSingleVertex(&vertices[3 * 8], tDstRect.x, tDstRect.y + tDstRect.h, 0, 1);

	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glBufferData(GL_ARRAY_BUFFER, 4 * stride, vertices, GL_STREAM_DRAW);

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gOpenGLData.mElementsHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STREAM_DRAW);

	glBindTexture(GL_TEXTURE_2D, tTextureID);
	// glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
	glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, 0);

	// Restore modified state
	glDisableVertexAttribArray(gOpenGLData.mAttribLocationPosition);
	glDisableVertexAttribArray(gOpenGLData.mAttribLocationUV);
	glDisableVertexAttribArray(gOpenGLData.mAttribLocationColor);
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
	glDisable(GL_SCISSOR_TEST);
}

static void drawSDLSurface(SDL_Surface* tSurface, SDL_Rect tSrcRect, SDL_Rect tDstRect) {
	GLuint textureID;
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tSurface->w, tSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tSurface->pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture);

	drawOpenGLTexture(textureID, tSrcRect, tDstRect);

	glDeleteTextures(1, &textureID);
}

static void drawSortedSprite(DrawListSpriteElement* e) {
	int sizeX = abs(e->mTexturePosition.bottomRight.x - e->mTexturePosition.topLeft.x) + 1;
	int sizeY = abs(e->mTexturePosition.bottomRight.y - e->mTexturePosition.topLeft.y) + 1;

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

	dstRect = scaleSDLRect(dstRect, e->mData.mScale, e->mData.mScaleEffectCenter);

	Position realEffectPos;
	if (e->mData.mIsScaleEffectCenterAbsolute) {
		realEffectPos = vecAdd(e->mData.mScaleEffectCenter, vecScale(e->mPos, -1));
	}
	else {
		realEffectPos = e->mData.mScaleEffectCenter;
	}


	if (e->mData.mIsRotationEffectCenterAbsolute) {
		realEffectPos = vecAdd(e->mData.mRotationEffectCenter, vecScale(e->mPos, -1));
	}
	else {
		realEffectPos = e->mData.mRotationEffectCenter;
	}
	realEffectPos = vecScale3D(realEffectPos, e->mData.mScale);


	SDL_Point effectCenter;
	effectCenter.x = (int)realEffectPos.x;
	effectCenter.y = (int)realEffectPos.y;

	int flip = 0;
	if (e->mTexturePosition.bottomRight.x < e->mTexturePosition.topLeft.x) flip |= SDL_FLIP_HORIZONTAL;
	if (e->mTexturePosition.bottomRight.y < e->mTexturePosition.topLeft.y) flip |= SDL_FLIP_VERTICAL;

	double angleDegrees = 360 - ((e->mData.mAngle.z * 180) / M_PI);

	Texture texture = e->mTexture.mTexture->mData;

	glBlendColor((GLclampf)e->mData.r, (GLclampf)e->mData.g, (GLclampf)e->mData.b, (GLclampf)e->mData.a);
	
	
	if (e->mData.mBlendType == BLEND_TYPE_ADDITION) {
		glBlendEquation(GL_FUNC_ADD);
	}
	else if(e->mData.mBlendType == BLEND_TYPE_NORMAL){
		glBlendEquation(GL_FUNC_ADD);
	} else if (e->mData.mBlendType == BLEND_TYPE_SUBTRACTION) {
		glBlendEquation(GL_FUNC_SUBTRACT);
	}
	else {
		logError("Unimplemented blend type");
		logErrorInteger(e->mData.mBlendType);
		abortSystem();
	}

	// TODO: angle, flip, scale, effect center
	drawOpenGLTexture(texture->mTexture, srcRect, dstRect);
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

	
	Position pos = e->mPos;
	int i;
	for (i = 0; i < l;) {
		char text[1024];

		double size = 0;
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

		SDL_Rect rect;
		rect.x = (int)pos.x;
		rect.y = (int)pos.y;
		rect.w = surface->w;
		rect.h = surface->h;

		SDL_Rect src;
		src.x = 0;
		src.y = 0;
		src.w = surface->w;
		src.h = surface->h;

		pos.y += surface->h;
		

		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
		drawSDLSurface(formattedSurface, src, rect);
		
		SDL_FreeSurface(formattedSurface);
		SDL_FreeSurface(surface);
		
		i = end + 1;
	}
}

static void drawSorted(void* tCaller, void* tData) {
	(void)tCaller;
	DrawListElement* e = tData;

	if (e->mType == DRAW_LIST_ELEMENT_TYPE_SPRITE) {
		drawSortedSprite(e->mData);
	} else if (e->mType == DRAW_LIST_ELEMENT_TYPE_TRUETYPE) {
		drawSortedTruetype(e->mData);
	}
	else {
		logError("Unrecognized draw type");
		logErrorInteger(e->mType);
		abortSystem();
	}
	
}

void stopDrawing() {
	vector_sort(&gDrawVector,cmpZ, NULL);
	vector_map(&gDrawVector, drawSorted, NULL);
	clearDrawVector();
	SDL_GL_SwapWindow(gSDLWindow);
}

void waitForScreen() {
	double frameMS = (1.0 / 60) * 1000;
	int frameEndTime = (int)(gData.mFrameStartTime + ceil(frameMS));
	int waitTime = frameEndTime-SDL_GetTicks();

	if (waitTime > 0) {
#ifndef __EMSCRIPTEN__
		SDL_Delay(waitTime);
#endif
	}

	gData.mFrameStartTime = SDL_GetTicks();
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

static void drawTextInternal() {

}

// TODO: refactor into general drawing code so both have it
static int hasToLinebreak(char* tText, int tCurrent, Position tTopLeft, Position tPos, Vector3D tFontSize, Vector3D tBreakSize, Vector3D tTextBoxSize) {
	
	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;
	
	char word[1024];
	int positionsRead;
	sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);

	Position delta = makePosition(positionsRead * tFontSize.x + (positionsRead-1) * tBreakSize.x, 0, 0);
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

		double dx = fabs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x);
		double dy = fabs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y);
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
	DrawListTruetypeElement* e = allocMemory(sizeof(DrawListTruetypeElement));
	strcpy(e->mText, tText);
	e->mFont = tFont;
	e->mPos = tPosition;
	e->mTextSize = tTextSize;
	e->mColor = tColor;
	e->mTextBoxWidth = tTextBoxWidth;

	DrawListElement* listElement = allocMemory(sizeof(DrawListElement));
	listElement->mType = DRAW_LIST_ELEMENT_TYPE_TRUETYPE;
	listElement->mData = e;
	listElement->mZ = tPosition.z;
	vector_push_back_owned(&gDrawVector, listElement);
}

void scaleDrawing(double tFactor, Position tScalePosition){
	gData.mScale = makePosition(tFactor,tFactor,1);
	gData.mScaleEffectCenter = tScalePosition;
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition){
	gData.mScale = tFactor;
	gData.mScaleEffectCenter = tScalePosition;
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
	gData.mRotationEffectCenter = tPosition;
}

void setDrawingParametersToIdentity(){
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	scaleDrawing(1, makePosition(0, 0, 0));
	setDrawingRotationZ(0, makePosition(0,0,0));
	setDrawingBlendType(BLEND_TYPE_NORMAL);
}

void setDrawingBlendType(BlendType tBlendType)
{
	gData.mBlendType = tBlendType;
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
	gData.mRotationEffectCenter = tCenter;
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


static uint32_t* getPixelFromSurface(SDL_Surface* tSurface, int x, int y) {
	uint32_t* pixels = tSurface->pixels;
	return &pixels[y*tSurface->w + x];
}

#define PIXEL_BUFFER_SIZE 1000
uint32_t gPixelBuffer[PIXEL_BUFFER_SIZE];

void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget) {
	Texture dst = tDst.mTexture->mData;

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
		abortSystem();
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



