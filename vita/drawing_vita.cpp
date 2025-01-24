#include "../include/prism/drawing.h"

#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <thread>

#include <vita2d.h>

#include "prism/log.h"
#include "prism/system.h"
#include "prism/datastructures.h"
#include "prism/memoryhandler.h"
#include "prism/math.h"
#include "prism/stlutil.h"
#include "prism/debug.h"
#include "prism/geometry.h"

namespace prism {

	using namespace std;

	struct VitaPalette {
		void* mBuffer;
		SceUID mID;
	};

	typedef struct {

		double a;
		double r;
		double g;
		double b;
		double rOffset;
		double gOffset;
		double bOffset;

		Matrix4D mTransformationMatrix;

		Vector mEffectStack;

		int mIsDisabled;

		BlendType mBlendType;
		int mIsColorSolid;
		int mIsColorInversed;
		double mDestAlpha;
		double mColorFactor;

		VitaPalette mPalettes[4];
	} DrawingData;

	struct DrawListSpriteElement {
		TextureData mTexture;
		Position2D mTopLeft;
		Position2D mTopRight;
		Position2D mBottomLeft;
		Position2D mBottomRight;
		PrismRectangle mTexturePosition;

		DrawingData mData;
		double mZ;
	};

	struct DrawListTruetypeElement {
		char mText[1024];
		//TTF_Font* mFont;
		Position2D mPos;
		Vector3DI mTextSize;
		Vector3D mColor;
		double mTextBoxWidth;
		GeoRectangle2D mDrawRectangle;

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

		const DrawListSpriteElement* asSpriteElement() const {
			return &impl_.mSprite;
		}

		operator DrawListTruetypeElement() const {
			return impl_.mTrueType;
		}

		const DrawListTruetypeElement* asTruetypeElement() const {
			return &impl_.mTrueType;
		}

		bool operator<(const DrawListElement& other) const {
			return getZ() < other.getZ();
		}

		Type mType;

	private:
		union Impl {
			DrawListSpriteElement mSprite;
			DrawListTruetypeElement mTrueType;
			Impl() {}
		} impl_;
	};

	enum ShaderBlendType {
		SHADER_BLEND_TYPE_NORMAL = 0,
		SHADER_BLEND_TYPE_ADDITION = 1,
		SHADER_BLEND_TYPE_SUBTRACTION = 2,
	};

	static struct {
		Vector3D mScreenScale;
		Vector3D mRealScreenSize;

		uint32_t mActiveShaderFlags;
	} gVitaDrawingHeaderData;

	static struct {
		double mFrequency;
		double mFrameStartTime;
		double mRealFramerate = 60;
	} gBookkeepingData;

	// We still need ordered drawing or alpha blending will stop working properly, it's not like on the dreamcast where rasterization is deferred
	static multiset<DrawListElement> gDrawVector;
	static DrawingData gPrismWindowsDrawingData;

	void setDrawingScreenScale(double tScaleX, double tScaleY);

	static void initPalettes()
	{
		for (int i = 0; i < 4; i++)
		{
			const int paletteSize = 256 * sizeof(uint32_t);

			gPrismWindowsDrawingData.mPalettes[i].mBuffer = vitaGpuAlloc(
				SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
				paletteSize,
				SCE_GXM_PALETTE_ALIGNMENT,
				SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
				&gPrismWindowsDrawingData.mPalettes[i].mID);

			if (!gPrismWindowsDrawingData.mPalettes[i].mBuffer) {
				logErrorFormat("Unable to allocate palette %d", i);
				abortSystem();
			}

			memset(gPrismWindowsDrawingData.mPalettes[i].mBuffer, 0, paletteSize);
		}
	}

	void initDrawing() {

		vita2d_init();
		vita2d_set_clear_color(RGBA8(0x0, 0x0, 0x0, 0xFF));

		initPalettes();

		ScreenSize sz = getScreenSize();
		setDrawingScreenScale((960.0 / sz.x), (544.0 / sz.y));
		setDrawingParametersToIdentity();

		gDrawVector.clear();

		gBookkeepingData.mFrameStartTime = 0;

		gPrismWindowsDrawingData.mEffectStack = new_vector();

		gPrismWindowsDrawingData.mIsDisabled = 0;

	}

	static int isCulledOutsideScreen(const Position& tPos, const PrismRectangle& tTexturePosition) {
		setProfilingSectionMarkerCurrentFunction();

		const auto sizeX = abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x) + 1;
		const auto sizeY = abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y) + 1;
		std::vector<Position> corners;
		corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos));
		corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos + Vector3D(sizeX, 0, 0)));
		corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos + Vector3D(0, sizeY, 0)));
		corners.push_back(rotateScaleTranslatePositionByMatrix4D(gPrismWindowsDrawingData.mTransformationMatrix, tPos + Vector3D(sizeX, sizeY, 0)));
		const auto minX = min(corners[0].x, min(corners[1].x, min(corners[2].x, corners[3].x)));
		const auto maxX = max(corners[0].x, max(corners[1].x, max(corners[2].x, corners[3].x)));
		const auto minY = min(corners[0].y, min(corners[1].y, min(corners[2].y, corners[3].y)));
		const auto maxY = max(corners[0].y, max(corners[1].y, max(corners[2].y, corners[3].y)));
		const auto vitaScreenSize = getDisplayedScreenSize();
		static const auto CULL_EPSILON = 1e-5;
		if (maxX < 0 - CULL_EPSILON) return 1;
		if (minX > vitaScreenSize.x + CULL_EPSILON) return 1;
		if (maxY < 0 - CULL_EPSILON) return 1;
		if (minY > vitaScreenSize.y + CULL_EPSILON) return 1;
		return 0;
	}

	void drawSprite(const TextureData& tTexture, const Position& tPos, const PrismRectangle& tTexturePosition) {
		setProfilingSectionMarkerCurrentFunction();
		if (gPrismWindowsDrawingData.mIsDisabled) return;
		if (isCulledOutsideScreen(tPos, tTexturePosition)) return;

		const auto sizeX = abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x) + 1;
		const auto sizeY = abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y) + 1;
		drawSpriteNoRectangle(tTexture, tPos, Vector3D(tPos.x + sizeX, tPos.y, tPos.z), Vector3D(tPos.x, tPos.y + sizeY, tPos.z), Vector3D(tPos.x + sizeX, tPos.y + sizeY, tPos.z), tTexturePosition);
	}


	void drawSpriteNoRectangle(const TextureData& tTexture, const Position& tTopLeft, const Position& tTopRight, const Position& tBottomLeft, const Position& tBottomRight, const PrismRectangle& tTexturePosition)
	{
		setProfilingSectionMarkerCurrentFunction();
		if (gPrismWindowsDrawingData.mIsDisabled) return;

		if (tTexture.mTextureSize.x < 0 || tTexture.mTextureSize.y < 0) {
			logError("Called with invalid textureSize");
			logErrorInteger(tTexture.mTextureSize.x);
			logErrorInteger(tTexture.mTextureSize.y);

			return;
		}

		DrawListSpriteElement e;
		e.mTexture = tTexture;
		e.mTopLeft = tTopLeft.xy();
		e.mTopRight = tTopRight.xy();
		e.mBottomLeft = tBottomLeft.xy();
		e.mBottomRight = tBottomRight.xy();
		e.mTexturePosition = tTexturePosition;
		e.mData = gPrismWindowsDrawingData;
		e.mZ = tTopLeft.z;
		gDrawVector.insert(DrawListElement(e));
	}

	static void clearDrawVector() {
		gDrawVector.clear();
	}

	void startDrawing() {
		setProfilingSectionMarkerCurrentFunction();

		vita2d_start_drawing();
		vita2d_clear_screen();

		clearDrawVector();
	}

	static void forceSingleValueToInteger(double* tVal) {
		*tVal = floor(*tVal);
	}

	static void applyDrawingMatrixAndSetVertex(vita2d_texture_vertex* vertex, const Vector3D& tPos, const Matrix4D* tMatrix) {
		auto finalPos = rotateScaleTranslatePositionByMatrix4D(*tMatrix, tPos);
		forceSingleValueToInteger(&finalPos.x);
		forceSingleValueToInteger(&finalPos.y);
		vertex->x = finalPos.x;
		vertex->y = finalPos.y;
		vertex->z = tPos.z / 100.f;
	}

	// tSrcRect in relative coords to texturesize, tDstRect in pixels
	static void drawVitaTextureUniversal(vita2d_texture* tTexture, int tPaletteID, const GeoRectangle2D& tSrcRect, const Position2D& tTopLeft, const Position2D& tTopRight, const Position2D& tBottomLeft, const Position2D& tBottomRight, const DrawingData* tData, ShaderBlendType tShaderBlendType, int tHasPalette, double tZ) {
		const Matrix4D* finalMatrix = &tData->mTransformationMatrix;

		const auto vertexCount = 4;
		vita2d_texture_vertex* vertices = (vita2d_texture_vertex*)vita2d_pool_memalign(
			vertexCount * sizeof(vita2d_texture_vertex),
			sizeof(vita2d_texture_vertex));

		applyDrawingMatrixAndSetVertex(&vertices[0], tTopLeft.xyz(tZ), finalMatrix);
		applyDrawingMatrixAndSetVertex(&vertices[1], tTopRight.xyz(tZ), finalMatrix);
		applyDrawingMatrixAndSetVertex(&vertices[3], tBottomLeft.xyz(tZ), finalMatrix);
		applyDrawingMatrixAndSetVertex(&vertices[2], tBottomRight.xyz(tZ), finalMatrix);

		vertices[0].u = tSrcRect.mTopLeft.x;
		vertices[0].v = tSrcRect.mTopLeft.y;
		vertices[1].u = tSrcRect.mBottomRight.x;
		vertices[1].v = tSrcRect.mTopLeft.y;
		vertices[3].u = tSrcRect.mTopLeft.x;
		vertices[3].v = tSrcRect.mBottomRight.y;
		vertices[2].u = tSrcRect.mBottomRight.x;
		vertices[2].v = tSrcRect.mBottomRight.y;

		if (tHasPalette)
		{
			sceGxmTextureSetPalette(&tTexture->gxm_tex, gPrismWindowsDrawingData.mPalettes[tPaletteID].mBuffer);
		}

		vita2d_set_blend_mode_add((int)tData->mBlendType);
		vita2d_draw_array_textured(tTexture, SCE_GXM_PRIMITIVE_TRIANGLE_FAN, vertices, vertexCount, RGBA8(uint8_t(0xFF * tData->r), uint8_t(0xFF * tData->g), uint8_t(0xFF * tData->b), uint8_t(0xFF * tData->a)));
	}

	static void drawSortedSprite(const DrawListSpriteElement* e) {
		GeoRectangle2D srcRect;
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

		Texture texture = (Texture)e->mTexture.mTexture->mData;
		ShaderBlendType shaderBlendType = SHADER_BLEND_TYPE_NORMAL;

		drawVitaTextureUniversal(texture->mTexture, e->mTexture.mPaletteID, srcRect, e->mTopLeft, e->mTopRight, e->mBottomLeft, e->mBottomRight, &e->mData, shaderBlendType, e->mTexture.mHasPalette, e->mZ);
	}

	static int isTextPositionEmpty(char tChar) {
		return tChar == ' ';
	}

	static void drawSorted(const DrawListElement& tData) {
		const DrawListElement* e = &tData;
		if (e->mType == DrawListElement::Type::DRAW_LIST_ELEMENT_TYPE_SPRITE) {
			auto sprite = e->asSpriteElement();
			drawSortedSprite(sprite);
		}
		else if (e->mType == DrawListElement::Type::DRAW_LIST_ELEMENT_TYPE_TRUETYPE) {
			auto sprite = e->asTruetypeElement();
			//drawSortedTruetype(sprite);
		}
		else {
			logError("Unrecognized draw type");
			logErrorInteger(e->mType);
			recoverFromError();
		}
	}

	void stopDrawing() {
		for (auto& drawElement : gDrawVector) {
			drawSorted(drawElement);
		}
		clearDrawVector();

		vita2d_end_drawing();
		vita2d_swap_buffers();
	}

	void waitForRendering() {
		vita2d_wait_rendering_done();
	}

	void waitForScreen() {
		setProfilingSectionMarkerCurrentFunction();
		gBookkeepingData.mRealFramerate = 60;
	}

	bool isSkippingDrawing()
	{
		return false;
	}

	void setDrawingFrameSkippingEnabled(bool /*tIsEnabled*/) {

	}

	void resetDrawingFrameStartTime() {

	}

	extern void getRGBFromColor(Color tColor, double* tR, double* tG, double* tB);

	void drawMultilineText(const char* tText, const char* tFullText, const Position& tPosition, const Vector3D& tFontSize, Color tColor, const Vector3D& tBreakSize, const Vector3D& tTextBoxSize) {
		int current = 0;

		setDrawingBaseColor(tColor);

		TextureData fontData = getFontTexture();
		Position pos = tPosition;

		while (tText[current] != '\0') {
			FontCharacterData charData = getFontCharacterData(tText[current]);

			PrismRectangle tTexturePosition;
			tTexturePosition.topLeft.x = (int)(fontData.mTextureSize.x * charData.mFilePositionX1);
			tTexturePosition.topLeft.y = (int)(fontData.mTextureSize.y * charData.mFilePositionY1);
			tTexturePosition.bottomRight.x = (int)(fontData.mTextureSize.x * charData.mFilePositionX2);
			tTexturePosition.bottomRight.y = (int)(fontData.mTextureSize.y * charData.mFilePositionY2);

			double dx = (double)abs(tTexturePosition.bottomRight.x - tTexturePosition.topLeft.x);
			double dy = (double)abs(tTexturePosition.bottomRight.y - tTexturePosition.topLeft.y);
			Vector3D scale = Vector3D(1 / dx, 1 / dy, 1);
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

	void drawTruetypeText(const char* tText, TruetypeFont tFont, const Position& tPosition, const Vector3DI& tTextSize, const Vector3D& tColor, double tTextBoxWidth, const GeoRectangle2D& tDrawRectangle)
	{
		/*
		DrawListTruetypeElement e;
		strcpy(e.mText, tText);
		e.mFont = (TTF_Font*)tFont;
		e.mPos = tPosition.xy();
		e.mTextSize = tTextSize;
		e.mColor = tColor;
		e.mTextBoxWidth = tTextBoxWidth;
		e.mDrawRectangle = tDrawRectangle;
		e.mData = gPrismWindowsDrawingData;
		e.mZ = tPosition.z;

		gDrawVector.insert(DrawListElement(e));
		*/
	}

	void scaleDrawing(double tFactor, const Position& tScalePosition) {
		scaleDrawing3D(Vector3D(tFactor, tFactor, 1), tScalePosition);
	}

	void scaleDrawing2D(const Vector2D& tFactor, const Position2D& tScalePosition) {
		setProfilingSectionMarkerCurrentFunction();
		scaleDrawing3D(tFactor.xyz(1.0), tScalePosition.xyz(0.0));
	}

	void scaleDrawing3D(const Vector3D& tFactor, const Position& tScalePosition) {
		setProfilingSectionMarkerCurrentFunction();
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(tScalePosition));
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createScaleMatrix4D(Vector3D(tFactor.x, tFactor.y, tFactor.z)));
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tScalePosition, -1)));
	}

	void setDrawingBaseColorOffsetAdvanced(double r, double g, double b) {
		gPrismWindowsDrawingData.rOffset = r;
		gPrismWindowsDrawingData.gOffset = g;
		gPrismWindowsDrawingData.bOffset = b;
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

	void setDrawingColorInversed(int tIsInversed)
	{
		gPrismWindowsDrawingData.mIsColorInversed = tIsInversed;
	}

	void setDrawingColorFactor(double tColorFactor) {
		gPrismWindowsDrawingData.mColorFactor = tColorFactor;
	}

	void setDrawingTransparency(double tAlpha) {
		gPrismWindowsDrawingData.a = tAlpha;
	}

	void setDrawingDestinationTransparency(double tAlpha) {
		gPrismWindowsDrawingData.mDestAlpha = tAlpha;
	}

	void setDrawingRotationZ(double tAngle, const Position2D& tPosition) {
		setProfilingSectionMarkerCurrentFunction();
		setDrawingRotationZ(tAngle, tPosition.xyz(0.0));
	}

	void setDrawingRotationZ(double tAngle, const Position& tPosition) {
		setProfilingSectionMarkerCurrentFunction();
		tAngle = (2 * M_PI - tAngle);
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(tPosition));
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createRotationZMatrix4D(tAngle));
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(vecScale(tPosition, -1)));
	}

	void setDrawingParametersToIdentity() {
		setProfilingSectionMarkerCurrentFunction();
		setDrawingBaseColor(COLOR_WHITE);
		setDrawingTransparency(1.0);
		setDrawingDestinationTransparency(1.0);
		setDrawingBlendType(BLEND_TYPE_NORMAL);
		setDrawingColorSolidity(0);
		setDrawingColorInversed(0);
		setDrawingColorFactor(1.0);

		ScreenSize sz = getScreenSize();
		Vector3D realScreenSize = Vector3D(sz.x * gVitaDrawingHeaderData.mScreenScale.x, sz.y * gVitaDrawingHeaderData.mScreenScale.y, 0);
		gPrismWindowsDrawingData.mTransformationMatrix = makeIdentityMatrix4D();
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(Vector3D(0, realScreenSize.y - gVitaDrawingHeaderData.mScreenScale.y * sz.y, 0)));
		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createScaleMatrix4D(Vector3D(gVitaDrawingHeaderData.mScreenScale.x, gVitaDrawingHeaderData.mScreenScale.y, 1)));
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

	void pushDrawingTranslation(const Vector3D& tTranslation) {

		gPrismWindowsDrawingData.mTransformationMatrix = matMult4D(gPrismWindowsDrawingData.mTransformationMatrix, createTranslationMatrix4D(tTranslation));

		TranslationEffect* e = (TranslationEffect*)allocMemory(sizeof(TranslationEffect));
		e->mTranslation = tTranslation;
		vector_push_back_owned(&gPrismWindowsDrawingData.mEffectStack, e);
	}
	void pushDrawingRotationZ(double tAngle, const Vector3D& tCenter) {
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

		gVitaDrawingHeaderData.mScreenScale = Vector3D(tScaleX, tScaleY, 1);

		ScreenSize sz = getScreenSize();
		gVitaDrawingHeaderData.mRealScreenSize = Vector3D(sz.x * gVitaDrawingHeaderData.mScreenScale.x, sz.y * gVitaDrawingHeaderData.mScreenScale.y, 0);
	}

	void setPaletteFromARGB256Buffer(int tPaletteID, const Buffer& tBuffer) {
		assert(tBuffer.mLength == 256 * 4);

		uint8_t* src = (uint8_t*)tBuffer.mData;
		uint8_t* dst = (uint8_t*)gPrismWindowsDrawingData.mPalettes[tPaletteID].mBuffer;
		memcpy(dst, src, tBuffer.mLength);
	}

	void setPaletteFromBGR256WithFirstValueTransparentBuffer(int tPaletteID, const Buffer& tBuffer)
	{
		assert(tBuffer.mLength == 256 * 3);

		uint8_t* src = (uint8_t*)tBuffer.mData;
		uint8_t* dst = (uint8_t*)gPrismWindowsDrawingData.mPalettes[tPaletteID].mBuffer;
		for (int i = 0; i < 256; i++) {
			dst[4 * i + 0] = i == 0 ? 0 : 255;
			dst[4 * i + 1] = src[3 * i + 2];
			dst[4 * i + 2] = src[3 * i + 1];
			dst[4 * i + 3] = src[3 * i + 0];
		}
	}

	double getRealFramerate() {
		return gBookkeepingData.mRealFramerate;
	}
}