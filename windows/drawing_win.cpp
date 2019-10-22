#include "../include/prism/drawing.h"

#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <thread>

#include <SDL.h>
#include <GL/glew.h>
#ifdef __EMSCRIPTEN__
#include <SDL_image.h>
#include <SDL_ttf.h>
#elif defined _WIN32
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include "prism/log.h"
#include "prism/system.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/math.h"
#include "prism/stlutil.h"

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
"	gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
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

struct DrawListSpriteElement{
	TextureData mTexture;
	Position mPos;
	Rectangle mTexturePosition;

	DrawingData mData;
	double mZ;
} ;

struct DrawListTruetypeElement{
	char mText[1024];
	TTF_Font* mFont;
	Position mPos;
	Vector3DI mTextSize;
	Vector3D mColor;
	double mTextBoxWidth;
	GeoRectangle mDrawRectangle;

	DrawingData mData;
	double mZ;

};

class DrawListElement {
public:
	enum Type {
		DRAW_LIST_ELEMENT_TYPE_SPRITE,
		DRAW_LIST_ELEMENT_TYPE_TRUETYPE,
	};

	DrawListElement(DrawListSpriteElement e) {
		impl_.mSprite = e;
		mType = Type::DRAW_LIST_ELEMENT_TYPE_SPRITE;
	}

	DrawListElement(DrawListTruetypeElement e) {
		impl_.mTrueType = e;
		mType = Type::DRAW_LIST_ELEMENT_TYPE_TRUETYPE;
	}

	~DrawListElement() {
		if (mType == DRAW_LIST_ELEMENT_TYPE_SPRITE) {
			impl_.mSprite.~DrawListSpriteElement();
		}
		else {
			impl_.mTrueType.~DrawListTruetypeElement();
		}
	}

	double getZ() const {
		if (mType == DRAW_LIST_ELEMENT_TYPE_SPRITE) {
			return impl_.mSprite.mZ;
		}
		else {
			return impl_.mTrueType.mZ;
		}
	}

	operator DrawListSpriteElement() const {
		return impl_.mSprite;
	}

	DrawListSpriteElement* asSpriteElement() {
		return &impl_.mSprite;
	}

	operator DrawListTruetypeElement() const {
		return impl_.mTrueType;
	}

	DrawListTruetypeElement* asTruetypeElement() {
		return &impl_.mTrueType;
	}

	Type mType;

private:
	union Impl {
		DrawListSpriteElement mSprite;
		DrawListTruetypeElement mTrueType;
		Impl() {}
	} impl_;
};

enum GraphicsCardType {
	INTEL,
	NVIDIA,
	AMD,
	UNKNOWN
};

static struct {
	int          mShaderHandle, mVertHandle, mFragHandle;
	int          mAttribLocationTex, mAttribLocationProjMtx;
	int          mAttribLocationPosition, mAttribLocationUV, mAttribLocationColor;
	unsigned int mVboHandle, mElementsHandle;

	GraphicsCardType mCardType;
	Vector3D mScreenScale;

	uint32_t mSubtractionEquation;
} gOpenGLData;

static struct {
	double mFrameStartTime;
	double mRealFramerate = 60;
} gBookkeepingData;

static vector<DrawListElement> gDrawVector;
static DrawingData gPrismWindowsData;

extern SDL_Window* gSDLWindow;

static void detectOpenGLCardType() {
	auto vendorString = std::string((char*)glGetString(GL_VENDOR));
	turnStringLowercase(vendorString);
	const auto vendorStringsSplit = splitStringBySeparator(vendorString, ' ');

	if (std::find(vendorStringsSplit.begin(), vendorStringsSplit.end(), "intel") != vendorStringsSplit.end()) {
		gOpenGLData.mCardType = GraphicsCardType::INTEL;
	}
	else if (std::find(vendorStringsSplit.begin(), vendorStringsSplit.end(), "nvidia") != vendorStringsSplit.end()) {
		gOpenGLData.mCardType = GraphicsCardType::NVIDIA;
	} 
	else if (std::find(vendorStringsSplit.begin(), vendorStringsSplit.end(), "ati") != vendorStringsSplit.end() || std::find(vendorStringsSplit.begin(), vendorStringsSplit.end(), "amd") != vendorStringsSplit.end()) {
		gOpenGLData.mCardType = GraphicsCardType::AMD;
	}
	else {
		logWarningFormat("Unable to detect GPU properly: %s %s", (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER));
		gOpenGLData.mCardType = GraphicsCardType::UNKNOWN;
	}
}

static void setupCardSpecificRendering() {
	if (gOpenGLData.mCardType == GraphicsCardType::INTEL) {
		gOpenGLData.mSubtractionEquation = GL_FUNC_ADD;
	}
	else {
		gOpenGLData.mSubtractionEquation = GL_FUNC_REVERSE_SUBTRACT;
	}
}

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
	int stride = sizeof(GLfloat) * 8;
	glVertexAttribPointer(gOpenGLData.mAttribLocationPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
	glVertexAttribPointer(gOpenGLData.mAttribLocationUV, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 2));
	glVertexAttribPointer(gOpenGLData.mAttribLocationColor, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 4));
	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gOpenGLData.mElementsHandle);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	GLuint elements[] = {
	0, 1, 2,
	2, 3, 0
	};
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STREAM_DRAW);

	glViewport(0, 0, 640, 480);
	glClearColor(0, 0, 0, 1);

	detectOpenGLCardType();
	setupCardSpecificRendering();
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

	gDrawVector.clear();
	gBookkeepingData.mFrameStartTime = 0;

	gPrismWindowsData.mEffectStack = new_vector();

	int i;
	for (i = 0; i < 4; i++) {
		createEmptySDLPalette(gPrismWindowsData.mPalettes[i]);
	}

	gPrismWindowsData.mIsDisabled = 0;

	initOpenGL();
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {
	if (gPrismWindowsData.mIsDisabled) return;
	
	if (gPrismWindowsData.mIsIdentity) {
		double sx = tTexture.mTextureSize.x;
		double sy = tTexture.mTextureSize.y;
		double maxDelta = max(sx, sy);
		ScreenSize sz = getScreenSize(); // TODO: fix (https://dev.azure.com/captdc/DogmaRnDA/_workitems/edit/372)
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


	DrawListSpriteElement e;
	e.mTexture = tTexture;
	e.mPos = tPos;
	e.mTexturePosition = tTexturePosition;
	e.mData = gPrismWindowsData;
	e.mZ = tPos.z;
	gDrawVector.push_back(DrawListElement(e));
}

static void clearDrawVector() {
	gDrawVector.clear();
}

void startDrawing() {
	glClear(GL_COLOR_BUFFER_BIT);

	clearDrawVector();
}

static bool cmpZ(const DrawListElement& tData1, const DrawListElement& tData2) {
	double z1 = tData1.getZ();
	double z2 = tData2.getZ();

	return z1 < z2;
}

static void setSingleVertex(GLfloat* tDst, Position tPosition, double tU, double tV, Position tColor, double tAlpha) {
	tDst[0] = (GLfloat)tPosition.x;
	tDst[1] = (GLfloat)tPosition.y;
	tDst[2] = (GLfloat)tU;
	tDst[3] = (GLfloat)tV;
	tDst[4] = (GLfloat)tColor.x;
	tDst[5] = (GLfloat)tColor.y;
	tDst[6] = (GLfloat)tColor.z;
	tDst[7] = (GLfloat)tAlpha;
}

// tSrcRect in relative coords to texturesize, tDstRect in pixels
static void drawOpenGLTexture(GLuint tTextureID, const GeoRectangle& tSrcRect, const GeoRectangle& tDstRect, DrawingData* tData) {
	Matrix4D* finalMatrix = &tData->mTransformationMatrix;

	float matrix[4][4];
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			matrix[y][x] = (float)finalMatrix->m[y][x];
		}
	}

	glUniformMatrix4fv(gOpenGLData.mAttribLocationProjMtx, 1, GL_FALSE, &matrix[0][0]);

	GLfloat vertices[4 * 8];
	setSingleVertex(&vertices[0 * 8], tDstRect.mTopLeft, tSrcRect.mTopLeft.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[1 * 8], makePosition(tDstRect.mBottomRight.x, tDstRect.mTopLeft.y, tDstRect.mTopLeft.z), tSrcRect.mBottomRight.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[2 * 8], tDstRect.mBottomRight, tSrcRect.mBottomRight.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[3 * 8], makePosition(tDstRect.mTopLeft.x, tDstRect.mBottomRight.y, tDstRect.mTopLeft.z), tSrcRect.mTopLeft.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);

	int stride = sizeof(GLfloat) * 8;
	glBufferData(GL_ARRAY_BUFFER, 4 * stride, vertices, GL_STREAM_DRAW);

	glBindTexture(GL_TEXTURE_2D, tTextureID);
	glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, 0);
}

static void drawPalettedOpenGLTexture(int tTextureID, int tPaletteID, GeoRectangle tSrcRect, GeoRectangle tDstRect, DrawingData* tData) {
	(void)tTextureID;
	(void)tPaletteID;
	(void)tSrcRect;
	(void)tDstRect;
	(void)tData;
	// TODO: implement as proper shader (https://dev.azure.com/captdc/DogmaRnDA/_workitems/edit/222)
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
		glBlendEquation(gOpenGLData.mSubtractionEquation);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	}
	else {
		logError("Unimplemented blend type");
		logErrorInteger(e->mData.mBlendType);
		recoverFromError();
	}

	if (e->mTexture.mHasPalette) {
		drawPalettedOpenGLTexture(texture->mTexture, e->mTexture.mPaletteID, srcRect, dstRect, &e->mData);
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

		GeoRectangle noCulled;
		noCulled.mTopLeft = pos;
		noCulled.mBottomRight = vecAdd(pos, makePosition(surface->w, surface->h, 0));

		GeoRectangle rect;
		rect.mTopLeft = clampPositionToGeoRectangle(noCulled.mTopLeft, e->mDrawRectangle);
		rect.mBottomRight = clampPositionToGeoRectangle(noCulled.mBottomRight, e->mDrawRectangle);

		const Position topLeftSrc = makePosition((rect.mTopLeft.x - noCulled.mTopLeft.x) / surface->w, (rect.mTopLeft.y - noCulled.mTopLeft.y) / surface->h, 0.0);
		const Position bottomRightSrc = makePosition((rect.mBottomRight.x - noCulled.mTopLeft.x) / surface->w, (rect.mBottomRight.y - noCulled.mTopLeft.y) / surface->h, 0.0);

		GeoRectangle src = makeGeoRectangle(topLeftSrc.x, topLeftSrc.y, bottomRightSrc.x - topLeftSrc.x, bottomRightSrc.y - topLeftSrc.y);
		pos.y += surface->h;

		// must be converted because otherwise WebGL freezes up
		auto convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
		SDL_FreeSurface(surface);

		GLint last_texture;
		GLuint texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, convertedSurface->w, convertedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedSurface->pixels);
		glBindTexture(GL_TEXTURE_2D, last_texture);
		SDL_FreeSurface(convertedSurface);

		drawOpenGLTexture(texture, src, rect, &e->mData);
		glDeleteTextures(1, &texture);
		i = end + 1;
	}
}

static void drawSorted(void* tCaller, DrawListElement& tData) {
	(void)tCaller;
	DrawListElement* e = &tData;

	if (e->mType == DrawListElement::Type::DRAW_LIST_ELEMENT_TYPE_SPRITE) {
		auto sprite = e->asSpriteElement();
		drawSortedSprite(sprite);
	}
	else if (e->mType == DrawListElement::Type::DRAW_LIST_ELEMENT_TYPE_TRUETYPE) {
		auto sprite = e->asTruetypeElement();
		drawSortedTruetype(sprite);
	}
	else {
		logError("Unrecognized draw type");
		logErrorInteger(e->mType);
		recoverFromError();
	}

}

void stopDrawing() {
	sort(gDrawVector.begin(), gDrawVector.end(), cmpZ);
	stl_vector_map(gDrawVector, drawSorted);
	clearDrawVector();
	SDL_GL_SwapWindow(gSDLWindow);
}

#ifndef __EMSCRIPTEN__
#define Rectangle Rectangle2
#include <Windows.h>
#undef Rectangle
#endif

void waitForScreen() {
#ifndef __EMSCRIPTEN__
	LARGE_INTEGER counter;

	QueryPerformanceFrequency(&counter);
	double freq = counter.QuadPart / 1000.0;
	double frameMS = (1.0 / 60) * 1000;
	double frameEndTime = gBookkeepingData.mFrameStartTime + frameMS;
	QueryPerformanceCounter(&counter);
	double waitTime = frameEndTime - (counter.QuadPart / freq);
	while (waitTime > 0) {
		QueryPerformanceCounter(&counter);
		waitTime = frameEndTime - (counter.QuadPart / freq);
	}

	QueryPerformanceCounter(&counter);
	double now = (counter.QuadPart / freq);
	gBookkeepingData.mRealFramerate = 1000.0 / (now - gBookkeepingData.mFrameStartTime);
	gBookkeepingData.mFrameStartTime = now;
#else
	gBookkeepingData.mRealFramerate = 60;
#endif
}

extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

void drawMultilineText(const char* tText, const char* tFullText, Position tPosition, Vector3D tFontSize, Color tColor, Vector3D tBreakSize, Vector3D tTextBoxSize) {
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

void drawTruetypeText(const char * tText, TruetypeFont tFont, Position tPosition, Vector3DI tTextSize, Vector3D tColor, double tTextBoxWidth, GeoRectangle tDrawRectangle)
{
	DrawListTruetypeElement e;
	strcpy(e.mText, tText);
	e.mFont = (TTF_Font*)tFont;
	e.mPos = tPosition;
	e.mTextSize = tTextSize;
	e.mColor = tColor;
	e.mTextBoxWidth = tTextBoxWidth;
	e.mDrawRectangle = tDrawRectangle;
	e.mData = gPrismWindowsData;
	e.mZ = tPosition.z;

	gDrawVector.push_back(DrawListElement(e));
}

void scaleDrawing(double tFactor, Position tScalePosition) {
	scaleDrawing3D(makePosition(tFactor, tFactor, 1), tScalePosition);
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition) {
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(tScalePosition));
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createScaleMatrix4D(makePosition(tFactor.x, tFactor.y, tFactor.z)));
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tScalePosition, -1)));
	gPrismWindowsData.mIsIdentity = gPrismWindowsData.mIsIdentity && tFactor.x == 1 && tFactor.y == 1;
}

void setDrawingBaseColor(Color tColor) {
	getRGBFromColor(tColor, &gPrismWindowsData.r, &gPrismWindowsData.g, &gPrismWindowsData.b);
}

void setDrawingBaseColorAdvanced(double r, double g, double b) {
	gPrismWindowsData.r = r;
	gPrismWindowsData.g = g;
	gPrismWindowsData.b = b;
}

void setDrawingTransparency(double tAlpha) {
	gPrismWindowsData.a = tAlpha;
}

void setDrawingRotationZ(double tAngle, Position tPosition) {
	tAngle = (2 * M_PI - tAngle);
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(tPosition));
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createRotationZMatrix4D(tAngle));
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tPosition, -1)));
	gPrismWindowsData.mIsIdentity = gPrismWindowsData.mIsIdentity && tAngle == 2 * M_PI;
}

void setDrawingParametersToIdentity() {
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	setDrawingBlendType(BLEND_TYPE_NORMAL);

	ScreenSize sz = getScreenSize();
	Vector3D realScreenSize = makePosition(sz.x*gOpenGLData.mScreenScale.x, sz.y*gOpenGLData.mScreenScale.y, 0);
	gPrismWindowsData.mTransformationMatrix = createOrthographicProjectionMatrix4D(0, realScreenSize.x, 0, realScreenSize.y, 0, 100);
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(makePosition(0, realScreenSize.y - gOpenGLData.mScreenScale.y*sz.y, 0)));
	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createScaleMatrix4D(makePosition(gOpenGLData.mScreenScale.x, gOpenGLData.mScreenScale.y, 1)));
	gPrismWindowsData.mIsIdentity = 1;
}

void setDrawingBlendType(BlendType tBlendType)
{
	gPrismWindowsData.mBlendType = tBlendType;
}

typedef struct {
	double mAngle;
	Position mCenter;
} RotationZEffect;

typedef struct {
	Vector3D mTranslation;

} TranslationEffect;

void pushDrawingTranslation(Vector3D tTranslation) {

	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(tTranslation));

	TranslationEffect* e = (TranslationEffect*)allocMemory(sizeof(TranslationEffect));
	e->mTranslation = tTranslation;
	vector_push_back_owned(&gPrismWindowsData.mEffectStack, e);
}
void pushDrawingRotationZ(double tAngle, Vector3D tCenter) {
	setDrawingRotationZ(tAngle, tCenter);

	RotationZEffect* e = (RotationZEffect*)allocMemory(sizeof(RotationZEffect));
	e->mAngle = tAngle;
	e->mCenter = tCenter;
	vector_push_back_owned(&gPrismWindowsData.mEffectStack, e);
}

void popDrawingRotationZ() {
	int ind = vector_size(&gPrismWindowsData.mEffectStack) - 1;
	RotationZEffect* e = (RotationZEffect*)vector_get(&gPrismWindowsData.mEffectStack, ind);

	setDrawingRotationZ(-e->mAngle, e->mCenter);

	vector_remove(&gPrismWindowsData.mEffectStack, ind);
}
void popDrawingTranslation() {
	int ind = vector_size(&gPrismWindowsData.mEffectStack) - 1;
	TranslationEffect* e = (TranslationEffect*)vector_get(&gPrismWindowsData.mEffectStack, ind);

	gPrismWindowsData.mTransformationMatrix = matMult4D(gPrismWindowsData.mTransformationMatrix, createTranslationMatrix4D(vecScale(e->mTranslation, -1)));

	vector_remove(&gPrismWindowsData.mEffectStack, ind);
}

#define PIXEL_BUFFER_SIZE 1000
uint32_t gPixelBuffer[PIXEL_BUFFER_SIZE];

void drawColoredRectangleToTexture(TextureData tDst, Color tColor, Rectangle tTarget) {
	(void)tDst;
	(void)tColor;
	(void)tTarget;
	// TODO: readd (https://dev.azure.com/captdc/DogmaRnDA/_workitems/edit/221)
}


void disableDrawing() {
	gPrismWindowsData.mIsDisabled = 1;
}

void enableDrawing() {
	gPrismWindowsData.mIsDisabled = 0;
}


void setDrawingScreenScale(double tScaleX, double tScaleY) {

	gOpenGLData.mScreenScale = makePosition(tScaleX, tScaleY, 1);

	ScreenSize sz = getScreenSize();
	Vector3D realScreenSize = makePosition(sz.x*gOpenGLData.mScreenScale.x, sz.y*gOpenGLData.mScreenScale.y, 0);
	glViewport(0, 0, (GLsizei)realScreenSize.x, (GLsizei)realScreenSize.y);
}

void setPaletteFromARGB256Buffer(int tPaletteID, Buffer tBuffer) {
	assert(tBuffer.mLength == 256 * 4);

	SDL_Color* palette = gPrismWindowsData.mPalettes[tPaletteID];

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

	SDL_Color* palette = gPrismWindowsData.mPalettes[tPaletteID];

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
	return gPrismWindowsData.mPalettes[tIndex];
}
double getRealFramerate() {
	return gBookkeepingData.mRealFramerate;
}