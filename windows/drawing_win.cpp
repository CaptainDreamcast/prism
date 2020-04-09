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
#include "prism/debug.h"

static const GLchar *gVertexShader =
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

static const GLchar* gFragmentShader =
#ifdef __EMSCRIPTEN__
// WebGL requires precision specifiers but OpenGL 2.1 disallows
// them, so I define the shader without it and then add it here.
"precision mediump float;\n"
#endif
"uniform sampler2D Texture;\n"
"uniform sampler2D BG;\n"
"uniform ivec3 ScreenSizeBlendStyle;\n"
"uniform float DestinationAlpha;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	if(ScreenSizeBlendStyle.z == 0) {\n"
"		gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV);\n"
"	} else {\n"
"		vec4 dstColor = texture2D(BG, gl_FragCoord.xy / vec2(ScreenSizeBlendStyle.xy));\n"
"		vec4 srcColor = Frag_Color * texture2D(Texture, Frag_UV);\n"
"		float blendFactor = (ScreenSizeBlendStyle.z == 1) ? 1.0 : -1.0;\n"
"		gl_FragColor = vec4(dstColor.xyz * DestinationAlpha + blendFactor * srcColor.xyz * srcColor.w, dstColor.w);\n"
"	}\n"
"}\n";

static const GLchar* gFragmentShaderSolidColor =
#ifdef __EMSCRIPTEN__
// WebGL requires precision specifiers but OpenGL 2.1 disallows
// them, so I define the shader without it and then add it here.
"precision mediump float;\n"
#endif
"uniform sampler2D Texture;\n"
"uniform sampler2D BG;\n"
"uniform ivec3 ScreenSizeBlendStyle;\n"
"uniform float DestinationAlpha;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	if(ScreenSizeBlendStyle.z == 0) {\n"
"		gl_FragColor = vec4(Frag_Color.xyz, Frag_Color.w * texture2D(Texture, Frag_UV).w);\n"
"	} else {\n"
"		vec4 dstColor = texture2D(BG, gl_FragCoord.xy / vec2(ScreenSizeBlendStyle.xy));\n"
"		vec4 srcColor = vec4(Frag_Color.xyz, Frag_Color.w * texture2D(Texture, Frag_UV).w);\n"
"		float blendFactor = (ScreenSizeBlendStyle.z == 1) ? 1.0 : -1.0;\n"
"		gl_FragColor = vec4(dstColor.xyz * DestinationAlpha + blendFactor * srcColor.xyz * srcColor.w, dstColor.w);\n"
"	}\n"
"}\n";

static const GLchar* gFragmentShaderPalette =
#ifdef __EMSCRIPTEN__
// WebGL requires precision specifiers but OpenGL 2.1 disallows
// them, so I define the shader without it and then add it here.
"precision mediump float;\n"
#endif
"uniform sampler2D Texture;\n"
"uniform sampler2D Palette;\n"
"uniform sampler2D BG;\n"
"uniform ivec3 ScreenSizeBlendStyle;\n"
"uniform float DestinationAlpha;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	vec4 index = texture2D(Texture, Frag_UV);\n"
"	if(ScreenSizeBlendStyle.z == 0) {\n"
"		gl_FragColor = Frag_Color * texture2D(Palette, vec2(index.w, 0.0));\n"
"	} else {\n"
"		vec4 dstColor = texture2D(BG, gl_FragCoord.xy / vec2(ScreenSizeBlendStyle.xy));\n"
"		vec4 srcColor = Frag_Color * texture2D(Palette, vec2(index.w, 0.0));\n"
"		float blendFactor = (ScreenSizeBlendStyle.z == 1) ? 1.0 : -1.0;\n"
"		gl_FragColor = vec4(dstColor.xyz * DestinationAlpha + blendFactor * srcColor.xyz * srcColor.w, dstColor.w);\n"
"	}\n"
"}\n";

static const GLchar* gFragmentShaderPaletteSolidColor =
#ifdef __EMSCRIPTEN__
// WebGL requires precision specifiers but OpenGL 2.1 disallows
// them, so I define the shader without it and then add it here.
"precision mediump float;\n"
#endif
"uniform sampler2D Texture;\n"
"uniform sampler2D Palette;\n"
"uniform sampler2D BG;\n"
"uniform ivec3 ScreenSizeBlendStyle;\n"
"uniform float DestinationAlpha;\n"
"varying vec2 Frag_UV;\n"
"varying vec4 Frag_Color;\n"
"void main()\n"
"{\n"
"	vec4 index = texture2D(Texture, Frag_UV);\n"
"	if(ScreenSizeBlendStyle.z == 0) {\n"
"		gl_FragColor = vec4(Frag_Color.xyz, Frag_Color.w * texture2D(Palette, vec2(index.w, 0)).w);\n"
"	} else {\n"
"		vec4 dstColor = texture2D(BG, gl_FragCoord.xy / vec2(ScreenSizeBlendStyle.xy));\n"
"		vec4 srcColor = vec4(Frag_Color.xyz, Frag_Color.w * texture2D(Palette, vec2(index.w, 0)).w);\n"
"		float blendFactor = (ScreenSizeBlendStyle.z == 1) ? 1.0 : -1.0;\n"
"		gl_FragColor = vec4(dstColor.xyz * DestinationAlpha + blendFactor * srcColor.xyz * srcColor.w, dstColor.w);\n"
"	}\n"
"}\n";

using namespace std;

typedef struct {

	double a;
	double r;
	double g;
	double b;

	Matrix4D mTransformationMatrix;

	Vector mEffectStack;

	int mIsDisabled;

	BlendType mBlendType;
	int mIsColorSolid;
	double mDestAlpha;

	GLuint mPalettes[4];
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
	WEBGL,
	UNKNOWN
};

enum ActiveShaderFlags {
	ACTIVE_SHADER_NONE = 0,
	ACTIVE_SHADER_REGULAR = (1 << 0),
	ACTIVE_SHADER_PALETTE = (1 << 1),
	ACTIVE_SHADER_SOLID_COLOR = (1 << 2),
};

enum ShaderBlendType {
	SHADER_BLEND_TYPE_NORMAL = 0,
	SHADER_BLEND_TYPE_ADDITION = 1,
	SHADER_BLEND_TYPE_SUBTRACTION = 2,
};

typedef struct {
	int mShaderHandle, mVertHandle, mFragHandle;
	int mAttribLocationTex, mAttribLocationBG, mAttribLocationProjMtx, mAttribLocationScreenSizeBlendStyle, mAttribLocationDestinationAlpha;
	int mAttribLocationPosition, mAttribLocationUV, mAttribLocationColor;
} RegularShader;

typedef struct {
	int mShaderHandle, mVertHandle, mFragHandle;
	int mAttribLocationTex, mAttribLocationPal, mAttribLocationBG, mAttribLocationProjMtx, mAttribLocationScreenSizeBlendStyle, mAttribLocationDestinationAlpha;
	int mAttribLocationPosition, mAttribLocationUV, mAttribLocationColor;
} PaletteShader;

static struct {
	unsigned int mVboHandle, mElementsHandle, mFBO, mFBOColorAttachment;
	RegularShader mRegularShader;
	RegularShader mRegularSolidColorShader;
	PaletteShader mPaletteShader;
	PaletteShader mPaletteSolidColorShader;

	GraphicsCardType mCardType;
	Vector3D mScreenScale;
	Vector3D mRealScreenSize;

	uint32_t mSubtractionEquation;
	uint32_t mActiveShaderFlags;
} gOpenGLData;

static struct {
	double mFrequency;
	double mFrameStartTime;
	double mRealFramerate = 60;
} gBookkeepingData;

static vector<DrawListElement> gDrawVector;
static DrawingData gPrismWindowsDrawingData;

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
	else if (isOnWeb()) {
		gOpenGLData.mCardType = GraphicsCardType::WEBGL;
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

static void initRegularShaderGeneral(RegularShader& tShader, const GLchar* tFragmentShader) {
	tShader.mShaderHandle = glCreateProgram();
	tShader.mVertHandle = glCreateShader(GL_VERTEX_SHADER);
	tShader.mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(tShader.mVertHandle, 1, &gVertexShader, NULL);
	glShaderSource(tShader.mFragHandle, 1, &tFragmentShader, 0);
	glCompileShader(tShader.mVertHandle);
	glCompileShader(tShader.mFragHandle);
	glAttachShader(tShader.mShaderHandle, tShader.mVertHandle);
	glAttachShader(tShader.mShaderHandle, tShader.mFragHandle);
	glLinkProgram(tShader.mShaderHandle);

	tShader.mAttribLocationTex = glGetUniformLocation(tShader.mShaderHandle, "Texture");
	tShader.mAttribLocationBG = glGetUniformLocation(tShader.mShaderHandle, "BG");
	tShader.mAttribLocationProjMtx = glGetUniformLocation(tShader.mShaderHandle, "ProjMtx");
	tShader.mAttribLocationScreenSizeBlendStyle = glGetUniformLocation(tShader.mShaderHandle, "ScreenSizeBlendStyle");
	tShader.mAttribLocationDestinationAlpha = glGetUniformLocation(tShader.mShaderHandle, "DestinationAlpha");
	tShader.mAttribLocationPosition = glGetAttribLocation(tShader.mShaderHandle, "Position");
	tShader.mAttribLocationUV = glGetAttribLocation(tShader.mShaderHandle, "UV");
	tShader.mAttribLocationColor = glGetAttribLocation(tShader.mShaderHandle, "Color");
}

static void initRegularShader() {
	initRegularShaderGeneral(gOpenGLData.mRegularShader, gFragmentShader);
}

static void initRegularSolidColorShader() {
	initRegularShaderGeneral(gOpenGLData.mRegularSolidColorShader, gFragmentShaderSolidColor);
}

static void initPaletteShaderGeneral(PaletteShader& tShader, const GLchar* tFragmentShader) {
	tShader.mShaderHandle = glCreateProgram();
	tShader.mVertHandle = glCreateShader(GL_VERTEX_SHADER);
	tShader.mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(tShader.mVertHandle, 1, &gVertexShader, NULL);
	glShaderSource(tShader.mFragHandle, 1, &tFragmentShader, 0);
	glCompileShader(tShader.mVertHandle);
	glCompileShader(tShader.mFragHandle);
	glAttachShader(tShader.mShaderHandle, tShader.mVertHandle);
	glAttachShader(tShader.mShaderHandle, tShader.mFragHandle);
	glLinkProgram(tShader.mShaderHandle);

	tShader.mAttribLocationTex = glGetUniformLocation(tShader.mShaderHandle, "Texture");
	tShader.mAttribLocationPal = glGetUniformLocation(tShader.mShaderHandle, "Palette");
	tShader.mAttribLocationBG = glGetUniformLocation(tShader.mShaderHandle, "BG");
	tShader.mAttribLocationProjMtx = glGetUniformLocation(tShader.mShaderHandle, "ProjMtx");
	tShader.mAttribLocationScreenSizeBlendStyle = glGetUniformLocation(tShader.mShaderHandle, "ScreenSizeBlendStyle");
	tShader.mAttribLocationDestinationAlpha = glGetUniformLocation(tShader.mShaderHandle, "DestinationAlpha");
	tShader.mAttribLocationPosition = glGetAttribLocation(tShader.mShaderHandle, "Position");
	tShader.mAttribLocationUV = glGetAttribLocation(tShader.mShaderHandle, "UV");
	tShader.mAttribLocationColor = glGetAttribLocation(tShader.mShaderHandle, "Color");
}

static void initPaletteShader() {
	initPaletteShaderGeneral(gOpenGLData.mPaletteShader, gFragmentShaderPalette);
}

static void initPaletteSolidColorShader() {
	initPaletteShaderGeneral(gOpenGLData.mPaletteSolidColorShader, gFragmentShaderPaletteSolidColor);
}

static void initShaders() {
	initRegularShader();
	initRegularSolidColorShader();
	initPaletteShader();
	initPaletteSolidColorShader();
}

static void useShaderGeneral() {
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

	glViewport(0, 0, (GLsizei)gOpenGLData.mRealScreenSize.x, (GLsizei)gOpenGLData.mRealScreenSize.y);
	glClearColor(0, 0, 0, 1);
}

static void useRegularShaderGeneral(const RegularShader& tShader) {
	glUseProgram(tShader.mShaderHandle);
	glUniform1i(tShader.mAttribLocationTex, 0);
	glUniform1i(tShader.mAttribLocationBG, 1);

	// Render command lists
	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glEnableVertexAttribArray(tShader.mAttribLocationPosition);
	glEnableVertexAttribArray(tShader.mAttribLocationUV);
	glEnableVertexAttribArray(tShader.mAttribLocationColor);
	int stride = sizeof(GLfloat) * 8;
	glVertexAttribPointer(tShader.mAttribLocationPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
	glVertexAttribPointer(tShader.mAttribLocationUV, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 2));
	glVertexAttribPointer(tShader.mAttribLocationColor, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 4));

	useShaderGeneral();
}

static void useRegularShader() {
	useRegularShaderGeneral(gOpenGLData.mRegularShader);
}

static void useRegularSolidColorShader() {
	useRegularShaderGeneral(gOpenGLData.mRegularSolidColorShader);
}

static void usePaletteShaderGeneral(const PaletteShader& tShader) {
	glUseProgram(tShader.mShaderHandle);
	glUniform1i(tShader.mAttribLocationTex, 0);
	glUniform1i(tShader.mAttribLocationPal, 1);
	glUniform1i(tShader.mAttribLocationBG, 2);

	// Render command lists
	glBindBuffer(GL_ARRAY_BUFFER, gOpenGLData.mVboHandle);
	glEnableVertexAttribArray(tShader.mAttribLocationPosition);
	glEnableVertexAttribArray(tShader.mAttribLocationUV);
	glEnableVertexAttribArray(tShader.mAttribLocationColor);
	int stride = sizeof(GLfloat) * 8;
	glVertexAttribPointer(tShader.mAttribLocationPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
	glVertexAttribPointer(tShader.mAttribLocationUV, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 2));
	glVertexAttribPointer(tShader.mAttribLocationColor, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat) * 4));

	useShaderGeneral();
}

static void usePaletteShader() {
	usePaletteShaderGeneral(gOpenGLData.mPaletteShader);
}

static void usePaletteSolidColorShader() {
	usePaletteShaderGeneral(gOpenGLData.mPaletteSolidColorShader);
}

#ifndef __EMSCRIPTEN__
static void createFBO(GLuint* tTarget, GLuint* tColorBuffer) {
	glGenFramebuffers(1, tTarget);
	glBindFramebuffer(GL_FRAMEBUFFER, *tTarget);

	glGenTextures(1, tColorBuffer);
	glBindTexture(GL_TEXTURE_2D, *tColorBuffer);
	debugFormat("Creating fbo with size %d %d\n", (GLsizei)gOpenGLData.mRealScreenSize.x, (GLsizei)gOpenGLData.mRealScreenSize.y);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)gOpenGLData.mRealScreenSize.x, (GLsizei)gOpenGLData.mRealScreenSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tColorBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		logError("Framebuffer is not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void destroyFBO(GLuint* tTarget, GLuint* tColorBuffer) {
	glDeleteTextures(1, tColorBuffer);
	glDeleteFramebuffers(1, tTarget);
}

static void initFBOs() {
	createFBO(&gOpenGLData.mFBO, &gOpenGLData.mFBOColorAttachment);
}

static void unloadFBOs() {
	destroyFBO(&gOpenGLData.mFBO, &gOpenGLData.mFBOColorAttachment);
}

static void recreateFBOs() {
	unloadFBOs();
	initFBOs();
}
#endif

#ifndef __EMSCRIPTEN__
static void GLAPIENTRY
MessageCallback(GLenum /*source*/,
	GLenum type,
	GLuint /*id*/,
	GLenum severity,
	GLsizei /*length*/,
	const GLchar* message,
	const void* /*userParam*/)
{
	if (severity == 0x826b) return;
	logErrorFormat("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}
#endif

static void initOpenGL() {
#ifndef __EMSCRIPTEN__
	if (isInDevelopMode()) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
	}
#endif
	initShaders();

	glGenBuffers(1, &gOpenGLData.mVboHandle);
	glGenBuffers(1, &gOpenGLData.mElementsHandle);
#ifndef __EMSCRIPTEN__
	initFBOs();
#endif

	gOpenGLData.mActiveShaderFlags = ACTIVE_SHADER_NONE;

	for (int i = 0; i < 4; i++) {
		glGenTextures(1, &gPrismWindowsDrawingData.mPalettes[i]);
	}

	detectOpenGLCardType();
	setupCardSpecificRendering();
}

void setDrawingScreenScale(double tScaleX, double tScaleY);

#ifndef __EMSCRIPTEN__
#define Rectangle Rectangle2
#include <Windows.h>
#undef Rectangle
#endif

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

#ifndef __EMSCRIPTEN__
	LARGE_INTEGER counter;
	QueryPerformanceFrequency(&counter);
	gBookkeepingData.mFrequency = counter.QuadPart / 1000.0;
#endif
	gBookkeepingData.mFrameStartTime = 0;

	gPrismWindowsDrawingData.mEffectStack = new_vector();

	gPrismWindowsDrawingData.mIsDisabled = 0;

	initOpenGL();
}

static int isCulledOutsideScreen(const Position& tPos, const Rectangle& tTexturePosition) {
	const auto sizeX = abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x) + 1;
	const auto sizeY = abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y) + 1;
	std::vector<Position> corners;
	corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos));
	corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos + makePosition(sizeX, 0, 0)));
	corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos + makePosition(0, sizeY, 0)));
	corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos + makePosition(sizeX, sizeY, 0)));
	const auto minX = min(corners[0].x, min(corners[1].x, min(corners[2].x, corners[3].x)));
	const auto maxX = max(corners[0].x, max(corners[1].x, max(corners[2].x, corners[3].x)));
	const auto minY = min(corners[0].y, min(corners[1].y, min(corners[2].y, corners[3].y)));
	const auto maxY = max(corners[0].y, max(corners[1].y, max(corners[2].y, corners[3].y)));
	static const auto CULL_EPSILON = 1e-5;
	if (maxX < -1 - CULL_EPSILON) return 1;
	if (minX > 1 + CULL_EPSILON) return 1;
	if (maxY < -1 - CULL_EPSILON) return 1;
	if (minY > 1 + CULL_EPSILON) return 1;
	return 0;
}

void drawSprite(TextureData tTexture, Position tPos, Rectangle tTexturePosition) {
	if (gPrismWindowsDrawingData.mIsDisabled) return;
	if (isCulledOutsideScreen(tPos, tTexturePosition)) return;

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
	e.mData = gPrismWindowsDrawingData;
	e.mZ = tPos.z;
	gDrawVector.push_back(DrawListElement(e));
}

static void clearDrawVector() {
	gDrawVector.clear();
}

void startDrawing() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
#ifndef __EMSCRIPTEN__
	glBindFramebuffer(GL_FRAMEBUFFER, gOpenGLData.mFBO);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	clearDrawVector();
}

static const RegularShader& getActiveRegularShaderReference() {
	if (gOpenGLData.mActiveShaderFlags == ACTIVE_SHADER_REGULAR) {
		return gOpenGLData.mRegularShader;
	}
	else {
		return gOpenGLData.mRegularSolidColorShader;
	}
}

static const PaletteShader& getActivePaletteShaderReference() {
	if (gOpenGLData.mActiveShaderFlags == ACTIVE_SHADER_PALETTE) {
		return gOpenGLData.mPaletteShader;
	}
	else {
		return gOpenGLData.mPaletteSolidColorShader;
	}
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
static void drawOpenGLTexture(GLuint tTextureID, const GeoRectangle& tSrcRect, const GeoRectangle& tDstRect, DrawingData* tData, ShaderBlendType tShaderBlendType) {
	const auto& shader = getActiveRegularShaderReference();
	Matrix4D* finalMatrix = &tData->mTransformationMatrix;

	float matrix[4][4];
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			matrix[y][x] = (float)finalMatrix->m[y][x];
		}
	}

#ifndef __EMSCRIPTEN__
	if (tShaderBlendType != SHADER_BLEND_TYPE_NORMAL) {
		glMemoryBarrier(GL_ALL_BARRIER_BITS); // wait or we can't be sure fbo is up-to-date
	}
#endif

	glUniformMatrix4fv(shader.mAttribLocationProjMtx, 1, GL_FALSE, &matrix[0][0]);
	glUniform3i(shader.mAttribLocationScreenSizeBlendStyle, (GLsizei)gOpenGLData.mRealScreenSize.x, (GLsizei)gOpenGLData.mRealScreenSize.y, int(tShaderBlendType));
	glUniform1f(shader.mAttribLocationDestinationAlpha, GLfloat(tData->mDestAlpha));

	GLfloat vertices[4 * 8];
	setSingleVertex(&vertices[0 * 8], tDstRect.mTopLeft, tSrcRect.mTopLeft.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[1 * 8], makePosition(tDstRect.mBottomRight.x, tDstRect.mTopLeft.y, tDstRect.mTopLeft.z), tSrcRect.mBottomRight.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[2 * 8], tDstRect.mBottomRight, tSrcRect.mBottomRight.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[3 * 8], makePosition(tDstRect.mTopLeft.x, tDstRect.mBottomRight.y, tDstRect.mTopLeft.z), tSrcRect.mTopLeft.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);

	int stride = sizeof(GLfloat) * 8;
	glBufferData(GL_ARRAY_BUFFER, 4 * stride, vertices, GL_STREAM_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tTextureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gOpenGLData.mFBOColorAttachment);
	glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, 0);
	glActiveTexture(GL_TEXTURE0);

}

static void drawPalettedOpenGLTexture(int tTextureID, int tPaletteID, GeoRectangle tSrcRect, GeoRectangle tDstRect, DrawingData* tData, ShaderBlendType tShaderBlendType) {
	const auto& shader = getActivePaletteShaderReference();
	Matrix4D* finalMatrix = &tData->mTransformationMatrix;

	float matrix[4][4];
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			matrix[y][x] = (float)finalMatrix->m[y][x];
		}
	}

#ifndef __EMSCRIPTEN__
	if (tShaderBlendType != SHADER_BLEND_TYPE_NORMAL) {
		glMemoryBarrier(GL_ALL_BARRIER_BITS); // wait or we can't be sure fbo is up-to-date
	}
#endif

	glUniformMatrix4fv(shader.mAttribLocationProjMtx, 1, GL_FALSE, &matrix[0][0]);
	glUniform3i(shader.mAttribLocationScreenSizeBlendStyle, (GLsizei)gOpenGLData.mRealScreenSize.x, (GLsizei)gOpenGLData.mRealScreenSize.y, int(tShaderBlendType));	
	glUniform1f(shader.mAttribLocationDestinationAlpha, GLfloat(tData->mDestAlpha));

	GLfloat vertices[4 * 8];
	setSingleVertex(&vertices[0 * 8], tDstRect.mTopLeft, tSrcRect.mTopLeft.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[1 * 8], makePosition(tDstRect.mBottomRight.x, tDstRect.mTopLeft.y, tDstRect.mTopLeft.z), tSrcRect.mBottomRight.x, tSrcRect.mTopLeft.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[2 * 8], tDstRect.mBottomRight, tSrcRect.mBottomRight.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);
	setSingleVertex(&vertices[3 * 8], makePosition(tDstRect.mTopLeft.x, tDstRect.mBottomRight.y, tDstRect.mTopLeft.z), tSrcRect.mTopLeft.x, tSrcRect.mBottomRight.y, makePosition(tData->r, tData->g, tData->b), tData->a);

	int stride = sizeof(GLfloat) * 8;
	glBufferData(GL_ARRAY_BUFFER, 4 * stride, vertices, GL_STREAM_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tTextureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gPrismWindowsDrawingData.mPalettes[tPaletteID]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gOpenGLData.mFBOColorAttachment);	
	glDrawElements(GL_TRIANGLES, (GLsizei)6, GL_UNSIGNED_INT, 0);
	glActiveTexture(GL_TEXTURE0);
}

static void updateActiveShader(uint32_t tNewShader) {
	if (tNewShader == gOpenGLData.mActiveShaderFlags) return;

	switch (tNewShader) {
	case ACTIVE_SHADER_REGULAR:
		useRegularShader();
		break;
	case ACTIVE_SHADER_REGULAR | ACTIVE_SHADER_SOLID_COLOR:
		useRegularSolidColorShader();
		break;
	case ACTIVE_SHADER_PALETTE:
		usePaletteShader();
		break;
	case ACTIVE_SHADER_PALETTE | ACTIVE_SHADER_SOLID_COLOR:
		usePaletteSolidColorShader();
		break;
	default:
		break;
	}
	gOpenGLData.mActiveShaderFlags = tNewShader;
}

static void drawSortedSprite(DrawListSpriteElement* e) {
	int sizeX = abs(e->mTexturePosition.bottomRight.x - e->mTexturePosition.topLeft.x) + 1;
	int sizeY = abs(e->mTexturePosition.bottomRight.y - e->mTexturePosition.topLeft.y) + 1;

	updateActiveShader((e->mTexture.mHasPalette ? ACTIVE_SHADER_PALETTE : ACTIVE_SHADER_REGULAR) | (e->mData.mIsColorSolid * ACTIVE_SHADER_SOLID_COLOR));

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
	auto blendType = e->mData.mBlendType;
	ShaderBlendType shaderBlendType = SHADER_BLEND_TYPE_NORMAL;
#ifndef __EMSCRIPTEN__
	if (blendType == BLEND_TYPE_ADDITION) {
		blendType = BLEND_TYPE_NORMAL;
		shaderBlendType = SHADER_BLEND_TYPE_ADDITION;
	} else if(blendType == BLEND_TYPE_SUBTRACTION) {
		blendType = BLEND_TYPE_NORMAL;
		shaderBlendType = SHADER_BLEND_TYPE_SUBTRACTION;
	}
#endif

	switch (blendType) {
	case BLEND_TYPE_ADDITION:
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		break;
	case BLEND_TYPE_NORMAL:
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_TYPE_SUBTRACTION:
		glBlendEquation(gOpenGLData.mSubtractionEquation);
		glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
		break;
	case BLEND_TYPE_ONE:
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	default:
		logError("Unimplemented blend type");
		logErrorInteger(e->mData.mBlendType);
		recoverFromError();
		break;
	}

	if (e->mTexture.mHasPalette) {
		drawPalettedOpenGLTexture(texture->mTexture, e->mTexture.mPaletteID, srcRect, dstRect, &e->mData, shaderBlendType);
	}
	else {
		drawOpenGLTexture(texture->mTexture, srcRect, dstRect, &e->mData, shaderBlendType);
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

		drawOpenGLTexture(texture, src, rect, &e->mData, SHADER_BLEND_TYPE_NORMAL);
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

#ifndef __EMSCRIPTEN__
static void drawFBOToScreen() {
	updateActiveShader(ACTIVE_SHADER_REGULAR);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	const auto srcRect = makeGeoRectangle(0, 1, 1, -1);
	const auto sz = getScreenSize();
	const auto dstRect = makeGeoRectangle(0, 0, sz.x, sz.y);
	setDrawingParametersToIdentity();
	drawOpenGLTexture(gOpenGLData.mFBOColorAttachment, srcRect, dstRect, &gPrismWindowsDrawingData, SHADER_BLEND_TYPE_NORMAL);
	glBindFramebuffer(GL_FRAMEBUFFER, gOpenGLData.mFBO);
}
#endif

void stopDrawing() {
	sort(gDrawVector.begin(), gDrawVector.end(), cmpZ);
	stl_vector_map(gDrawVector, drawSorted);
	clearDrawVector();

#ifndef __EMSCRIPTEN__
	drawFBOToScreen();
#endif

	SDL_GL_SwapWindow(gSDLWindow);
}

void waitForScreen() {
#ifndef __EMSCRIPTEN__
	LARGE_INTEGER counter;
	static const auto frameMS = (1.0 / 60) * 1000;
	const auto frameEndTime = gBookkeepingData.mFrameStartTime + frameMS;
	QueryPerformanceCounter(&counter);
	auto waitTime = frameEndTime - (counter.QuadPart / gBookkeepingData.mFrequency);
	while (waitTime > 0) {
		QueryPerformanceCounter(&counter);
		waitTime = frameEndTime - (counter.QuadPart / gBookkeepingData.mFrequency);
	}

	QueryPerformanceCounter(&counter);
	const auto now = (counter.QuadPart / gBookkeepingData.mFrequency);
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
	e.mData = gPrismWindowsDrawingData;
	e.mZ = tPosition.z;

	gDrawVector.push_back(DrawListElement(e));
}

void scaleDrawing(double tFactor, Position tScalePosition) {
	scaleDrawing3D(makePosition(tFactor, tFactor, 1), tScalePosition);
}

void scaleDrawing3D(Vector3D tFactor, Position tScalePosition) {
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(tScalePosition));
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createScaleMatrix4D(makePosition(tFactor.x, tFactor.y, tFactor.z)));
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tScalePosition, -1)));
}

void setDrawingBaseColor(Color tColor) {
	getRGBFromColor(tColor, &gPrismWindowsDrawingData.r, &gPrismWindowsDrawingData.g, &gPrismWindowsDrawingData.b);
}

void setDrawingBaseColorAdvanced(double r, double g, double b) {
	gPrismWindowsDrawingData.r = r;
	gPrismWindowsDrawingData.g = g;
	gPrismWindowsDrawingData.b = b;
}

void setDrawingColorSolidity(int tIsSolid)
{
	gPrismWindowsDrawingData.mIsColorSolid = tIsSolid;
}

void setDrawingTransparency(double tAlpha) {
	gPrismWindowsDrawingData.a = tAlpha;
}

void setDrawingDestinationTransparency(double tAlpha) {
	gPrismWindowsDrawingData.mDestAlpha = tAlpha;
}

void setDrawingRotationZ(double tAngle, Position tPosition) {
	tAngle = (2 * M_PI - tAngle);
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(tPosition));
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createRotationZMatrix4D(tAngle));
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tPosition, -1)));
}

void setDrawingParametersToIdentity() {
	setDrawingBaseColor(COLOR_WHITE);
	setDrawingTransparency(1.0);
	setDrawingDestinationTransparency(1.0);
	setDrawingBlendType(BLEND_TYPE_NORMAL);
	setDrawingColorSolidity(0);

	ScreenSize sz = getScreenSize();
	Vector3D realScreenSize = makePosition(sz.x*gOpenGLData.mScreenScale.x, sz.y*gOpenGLData.mScreenScale.y, 0);
	gPrismWindowsDrawingData.mTransformationMatrix = createOrthographicProjectionMatrix4D(0, realScreenSize.x, 0, realScreenSize.y, 0, 100);
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(makePosition(0, realScreenSize.y - gOpenGLData.mScreenScale.y*sz.y, 0)));
	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createScaleMatrix4D(makePosition(gOpenGLData.mScreenScale.x, gOpenGLData.mScreenScale.y, 1)));
}

void setDrawingBlendType(BlendType tBlendType)
{
	gPrismWindowsDrawingData.mBlendType = tBlendType;
}

typedef struct {
	double mAngle;
	Position mCenter;
} RotationZEffect;

typedef struct {
	Vector3D mTranslation;

} TranslationEffect;

void pushDrawingTranslation(Vector3D tTranslation) {

	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(tTranslation));

	TranslationEffect* e = (TranslationEffect*)allocMemory(sizeof(TranslationEffect));
	e->mTranslation = tTranslation;
	vector_push_back_owned(&gPrismWindowsDrawingData.mEffectStack, e);
}
void pushDrawingRotationZ(double tAngle, Vector3D tCenter) {
	setDrawingRotationZ(tAngle, tCenter);

	RotationZEffect* e = (RotationZEffect*)allocMemory(sizeof(RotationZEffect));
	e->mAngle = tAngle;
	e->mCenter = tCenter;
	vector_push_back_owned(&gPrismWindowsDrawingData.mEffectStack, e);
}

void popDrawingRotationZ() {
	int ind = vector_size(&gPrismWindowsDrawingData.mEffectStack) - 1;
	RotationZEffect* e = (RotationZEffect*)vector_get(&gPrismWindowsDrawingData.mEffectStack, ind);

	setDrawingRotationZ(-e->mAngle, e->mCenter);

	vector_remove(&gPrismWindowsDrawingData.mEffectStack, ind);
}
void popDrawingTranslation() {
	int ind = vector_size(&gPrismWindowsDrawingData.mEffectStack) - 1;
	TranslationEffect* e = (TranslationEffect*)vector_get(&gPrismWindowsDrawingData.mEffectStack, ind);

	gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(vecScale(e->mTranslation, -1)));

	vector_remove(&gPrismWindowsDrawingData.mEffectStack, ind);
}

void disableDrawing() {
	gPrismWindowsDrawingData.mIsDisabled = 1;
}

void enableDrawing() {
	gPrismWindowsDrawingData.mIsDisabled = 0;
}

void setDrawingScreenScale(double tScaleX, double tScaleY) {

	gOpenGLData.mScreenScale = makePosition(tScaleX, tScaleY, 1);

	ScreenSize sz = getScreenSize();
	gOpenGLData.mRealScreenSize = makePosition(sz.x*gOpenGLData.mScreenScale.x, sz.y*gOpenGLData.mScreenScale.y, 0);
	glViewport(0, 0, (GLsizei)gOpenGLData.mRealScreenSize.x, (GLsizei)gOpenGLData.mRealScreenSize.y);
#ifndef __EMSCRIPTEN__
	recreateFBOs();
#endif
}

void setPaletteFromARGB256Buffer(int tPaletteID, Buffer tBuffer) {
	assert(tBuffer.mLength == 256 * 4);

	uint8_t* src = (uint8_t*)tBuffer.mData;
	std::vector<uint8_t> finalBuffer(256 * 4);
	for (int i = 0; i < 256; i++) {
		finalBuffer[4 * i + 0] = src[4 * i + 1];
		finalBuffer[4 * i + 1] = src[4 * i + 2];
		finalBuffer[4 * i + 2] = src[4 * i + 3];
		finalBuffer[4 * i + 3] = src[4 * i + 0];
	}

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, gPrismWindowsDrawingData.mPalettes[tPaletteID]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalBuffer.data());
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, Buffer tBuffer)
{
	assert(tBuffer.mLength == 256 * 3);

	uint8_t* src = (uint8_t*)tBuffer.mData;
	std::vector<uint8_t> finalBuffer(256 * 4);
	for (int i = 0; i < 256; i++) {
		finalBuffer[4 * i + 0] = src[3 * i + 0];
		finalBuffer[4 * i + 1] = src[3 * i + 1];
		finalBuffer[4 * i + 2] = src[3 * i + 2];
		finalBuffer[4 * i + 3] = i == 0 ? 0 : 255;
	}

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, gPrismWindowsDrawingData.mPalettes[tPaletteID]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, finalBuffer.data());
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

double getRealFramerate() {
	return gBookkeepingData.mRealFramerate;
}