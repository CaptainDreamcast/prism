#include "prism/drawing.h"

#include "prism/log.h"

namespace prism {

	static struct {
		float mFramerate = 60.0;
	} gDrawingLinuxData;

	void initDrawing() {}

	void drawSprite(const TextureData& /*tTexture*/, const Position& /*tPos*/, const PrismRectangle& /*tTexturePosition*/) {}
	void drawSpriteNoRectangle(const TextureData& /*tTexture*/, const Position& /*tTopLeft*/, const Position& /*tTopRight*/, const Position& /*tBottomLeft*/, const Position& /*tBottomRight*/, const PrismRectangle& /*tTexturePosition*/) {}
	void drawMultilineText(const char* /*tText*/, const char* /*tFullText*/, const Position& /*tPosition*/, const Vector3D& /*tFontSize*/, Color /*tColor*/, const Vector3D& /*tBreakSize*/, const Vector3D& /*tTextBoxSize*/) {}
	void drawTruetypeText(const char* /*tText*/, TruetypeFont /*tFont*/, const Position& /*tPosition*/, const Vector3DI& /*tTextSize*/, const Vector3D& /*tColor*/, float /*tTextBoxWidth*/, const GeoRectangle2D& /*tDrawRectangle*/) {}

	void waitForScreen() {}
	void startDrawing() {}
	void stopDrawing() {}
	void waitForRendering() {}

	bool isSkippingDrawing() { return true; }
	void setDrawingFrameSkippingEnabled(bool /*tIsEnabled*/) {}
	void resetDrawingFrameStartTime() {}
	void disableDrawing() {}
	void enableDrawing() {}

	void scaleDrawing(float /*tFactor*/, const Position& /*tScalePosition*/) {}
	void scaleDrawing2D(const Vector2D& /*tFactor*/, const Position2D& /*tScalePosition*/) {}
	void scaleDrawing3D(const Vector3D& /*tFactor*/, const Position& /*tScalePosition*/) {}

	void setDrawingBaseColorOffsetAdvanced(float /*r*/, float /*g*/, float /*b*/) {}
	void setDrawingBaseColor(Color /*tColor*/) {}
	void setDrawingBaseColorAdvanced(float /*r*/, float /*g*/, float /*b*/) {}
	void setDrawingColorSolidity(int /*tIsSolid*/) {}
	void setDrawingColorInversed(int /*tIsInversed*/) {}
	void setDrawingColorFactor(float /*tColorFactor*/) {}
	void setDrawingTransparency(float /*tAlpha*/) {}
	void setDrawingDestinationTransparency(float /*tAlpha*/) {}
	void setDrawingRotationZ(float /*tAngle*/, const Position2D& /*tPosition*/) {}
	void setDrawingRotationZ(float /*tAngle*/, const Position& /*tPosition*/) {}
	void setDrawingParametersToIdentity() {}
	void setDrawingBlendType(BlendType /*tBlendType*/) {}

	void setPaletteFromARGB256Buffer(int /*tPaletteID*/, const Buffer& /*tBuffer*/) {}
	void setPaletteFromBGR256WithFirstValueTransparentBuffer(int /*tPaletteID*/, const Buffer& /*tBuffer*/) {}

	float getRealFramerate() { return gDrawingLinuxData.mFramerate; }

}
