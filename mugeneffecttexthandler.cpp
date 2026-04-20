#include "prism/mugeneffecttexthandler.h"

#include <assert.h>
#include <algorithm>
#include <set>
#include <variant>

#include "prism/mugendefreader.h"
#include "prism/mugenanimationhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/drawing.h"
#include "prism/texture.h"
#include "prism/math.h"
#include "prism/stlutil.h"

using namespace std;

namespace prism {

void loadMugenEffectTextHandler()
{
}

typedef struct {
	char mText[1024];
	char mDisplayText[1024];
	Position mPosition;
	MugenFont* mFont;

	Vector3D mBaseColor;
	double mR;
	double mG;
	double mB;
	double mAlpha;

	double mScale;

	MugenTextAlignment mAlignment;
	GeoRectangle2D mRectangle;
	double mTextBoxWidth;
	Duration mBuildupDurationPerLetter;
	Duration mBuildupNow;

	int mIsVisible;
} MugenEffectText;

static struct {
	map<int, MugenEffectText> mHandledTexts;

} gMugenEffectTextHandler;

static void loadMugenEffectTextHandlerActor(void* tData) {
	(void) tData;
	setProfilingSectionMarkerCurrentFunction();
	gMugenEffectTextHandler.mHandledTexts.clear();
}

static void unloadMugenEffectTextHandlerActor(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gMugenEffectTextHandler.mHandledTexts.clear();
}

static void updateSingleTextBuildup(MugenEffectText* e) {
	if (handleDurationAndCheckIfOver(&e->mBuildupNow, e->mBuildupDurationPerLetter)) {
		if (!strcmp(e->mText, e->mDisplayText)) {
			e->mBuildupDurationPerLetter = INF;
			return;
		}

		auto originalLength = strlen(e->mText);
		auto l = strlen(e->mDisplayText);
		if (e->mText[l] == '{') {
			while ((l < originalLength - 1) && e->mText[l] != '}') {
				e->mDisplayText[l] = e->mText[l];
				l++;
			}
		}

		e->mDisplayText[l] = e->mText[l];
		e->mDisplayText[l + 1] = '\0';
	}
}

static void updateSingleText(void* tCaller, MugenEffectText& tData) {
	(void)tCaller;
	(void)tData;
	MugenEffectText* e = &tData;
	updateSingleTextBuildup(e);
}

static void updateMugenEffectTextHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gMugenEffectTextHandler.mHandledTexts, updateSingleText);
}

static double getOriginalMugenFontFactor() {
	const auto sz = getScreenSize();
	return sz.y / 240.0;
}

static double getNewMugenFontFactor() {
	const auto sz = getScreenSize();
	return sz.y / 480.0;
}

typedef struct {
	MugenSpriteFileSprite* mSprite;
	Position mBasePosition;
	MugenEffectText* mText;
	MugenFont* mFont;
} BitmapDrawCaller;

static void drawSingleBitmapSubSprite(MugenSpriteFileSubSprite& tSubSprite, BitmapDrawCaller& tCaller) {
	double factor = getNewMugenFontFactor() * tCaller.mText->mScale;
	Position p = tCaller.mBasePosition + (tSubSprite.mOffset * factor);
	
	int minHeight = 0;
	int maxHeight = tSubSprite.mTexture.mTextureSize.y - 1;
	int upY = max(minHeight, min(maxHeight, (int)(tCaller.mText->mRectangle.mTopLeft.y - p.y)));
	int downY = max(minHeight, min(maxHeight, upY + (int)(tCaller.mText->mRectangle.mBottomRight.y - (p.y + tCaller.mFont->mSize.y))));
	if (upY == downY) return;

	p.y = max(p.y, tCaller.mText->mRectangle.mTopLeft.y);

	scaleDrawing(factor, p);
	drawSprite(tSubSprite.mTexture, p, makeRectangleFromTexture(tSubSprite.mTexture));
	scaleDrawing(1 / factor, p);
}

static set<int> getBitmapTextLinebreaks(char* tText, const Position& tStart, MugenFont* tFont, MugenSpriteFileGroup& tSpriteGroup, double tRightX, double tFactor) {
	if (tRightX >= INF / 2) return set<int>();

	set<int> ret;
	Position p = tStart;
	auto n = int(strlen(tText));
	for (int i = 0; i < n; i++) {
		int positionsRead = 0;
		int doesBreak;
		if (tText[i] == '\n') {
			doesBreak = 1;
		}
		else {
			char word[1024];
			int items = sscanf(tText + i, "%1023s%n", word, &positionsRead);
			if (items != 1) break;
			for (int j = i; j < i + positionsRead; j++) {
				if (stl_map_contains(tSpriteGroup.mSprites, (int)tText[j])) {
					auto& sprite = tSpriteGroup.mSprites[(int)tText[j]];
					p = vecAdd2D(p, vecScale(Vector3D(sprite.mOriginalTextureSize.x, 0, 0), tFactor));
					p = vecAdd2D(p, vecScale(Vector3D(tFont->mSpacing.x, 0, 0), tFactor));
				}
				else {
					p = vecAdd2D(p, vecScale(Vector3D(tFont->mSize.x, 0, 0), tFactor));
					p = vecAdd2D(p, vecScale(Vector3D(tFont->mSpacing.x, 0, 0), tFactor));
				}
			}
			doesBreak = (p.x > tRightX);
			i += std::max(positionsRead - 1, 0);
		}

		if (doesBreak) {
			auto skipIndex = i;
			i -= positionsRead - 1;
			while (i > 0 && i < n && tText[i] == ' ') i++;
			i = max(0, i - 1);
			if (ret.find(i) == ret.end()) {
				ret.insert(i);
				p.x = tStart.x;
			}
			else {
				i = skipIndex;
			}
		}
	}
	return ret;
}

static void drawSingleBitmapText(MugenEffectText* e) {
	MugenFont* font = e->mFont;
	auto& bitmapFont = std::get<MugenBitmapFont>(font->mData);
	auto textLength = int(strlen(e->mDisplayText));
	double factor = getNewMugenFontFactor() * e->mScale;

	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);
	setDrawingTransparency(e->mAlpha);

	auto& spriteGroup = bitmapFont.mSprites.mGroups[0];

	int i;
	Position p = vecAdd2D(e->mPosition, vecScale(Vector3D(font->mOffset.x, font->mOffset.y, 0), factor));
	Position start = p;
	double rightX = p.x + e->mTextBoxWidth;
	const auto breaks = getBitmapTextLinebreaks(e->mText, start, font, spriteGroup, rightX, factor);
	for (i = 0; i < textLength; i++) {

		if (stl_map_contains(spriteGroup.mSprites, (int)e->mDisplayText[i])) {
			BitmapDrawCaller caller;
			caller.mSprite = &spriteGroup.mSprites[(int)e->mDisplayText[i]];
			caller.mBasePosition = p;
			caller.mText = e;
			caller.mFont = font;
			for (auto& texture : caller.mSprite->mTextures)
			{
				drawSingleBitmapSubSprite(texture, caller);
			}

			p = vecAdd2D(p, vecScale(Vector3D(caller.mSprite->mOriginalTextureSize.x, 0, 0), factor));
			p = vecAdd2D(p, vecScale(Vector3D(font->mSpacing.x, 0, 0), factor));
		}
		else {
			p = vecAdd2D(p, vecScale(Vector3D(font->mSize.x, 0, 0), factor));
			p = vecAdd2D(p, vecScale(Vector3D(font->mSpacing.x, 0, 0), factor));
		}

		if (breaks.find(i) != breaks.end()) {
			p.x = start.x;
			p = vecAdd2D(p, vecScale(Vector3D(0, font->mSize.y, 0), factor));
			p = vecAdd2D(p, vecScale(Vector3D(0, font->mSpacing.y, 0), factor));
		}
	}

	setDrawingBaseColorAdvanced(1, 1, 1);
	setDrawingTransparency(1.0);
}

static void drawSingleTruetypeText(MugenEffectText* e) {
	MugenFont* font = e->mFont;
	auto& truetypeFont = std::get<MugenTruetypeFont>(font->mData);
	drawTruetypeText(e->mDisplayText, truetypeFont.mFont, e->mPosition, font->mSize, Vector3D(e->mR, e->mG, e->mB), e->mTextBoxWidth, e->mRectangle);
}

typedef struct {
	MugenElecbyteFontMapEntry* mMapEntry;
	Position mBasePosition;
	double mStartX;
	int mPreviousSubSpriteOffsetY;
	MugenEffectText* mText;
	MugenFont* mFont;
} ElecbyteDrawCaller;

static void drawSingleElecbyteSubSprite(MugenSpriteFileSubSprite& tSubSprite, ElecbyteDrawCaller& tCaller) {
	if (tCaller.mMapEntry->mStartX >= tSubSprite.mOffset.x + tSubSprite.mTexture.mTextureSize.x) return;
	if (tCaller.mMapEntry->mStartX + tCaller.mMapEntry->mWidth - 1 < tSubSprite.mOffset.x) return;

	if (tSubSprite.mOffset.y > tCaller.mPreviousSubSpriteOffsetY) {
		tCaller.mBasePosition.x = tCaller.mStartX;
	}
	tCaller.mPreviousSubSpriteOffsetY = tSubSprite.mOffset.y;

	int minWidth = 0;
	int maxWidth = tSubSprite.mTexture.mTextureSize.x - 1;
	int leftX = max(minWidth, min(maxWidth, tCaller.mMapEntry->mStartX - tSubSprite.mOffset.x));
	int rightX = max(minWidth, min(maxWidth, (tCaller.mMapEntry->mStartX + tCaller.mMapEntry->mWidth - 1) - tSubSprite.mOffset.x));

	double factor = getOriginalMugenFontFactor() * tCaller.mText->mScale;
	Position p = vecAdd2D(tCaller.mBasePosition, vecScale(Vector3D(0.f, tSubSprite.mOffset.y, 0.f), factor));

	int minHeight = 0;
	int maxHeight = tSubSprite.mTexture.mTextureSize.y - 1;
	int upY = max(minHeight, min(maxHeight, (int)(tCaller.mText->mRectangle.mTopLeft.y - p.y)));
	int downY = max(minHeight, min(maxHeight, upY + (int)(tCaller.mText->mRectangle.mBottomRight.y - (p.y + tCaller.mFont->mSize.y))));
	if (upY == downY) return;

	p.y = max(p.y, tCaller.mText->mRectangle.mTopLeft.y);

	scaleDrawing(factor, p);
	drawSprite(tSubSprite.mTexture, p, makeRectangle(leftX, upY, rightX - leftX, downY - upY));
	scaleDrawing(1 / factor, p);

	tCaller.mBasePosition.x += (rightX - leftX + 1) * factor;
}

static void advanceMugenTextEffectPosition(const char* tText, int& i, int len)
{
	if (tText[i] != '{') return;

	int start = i + 1;
	i += 1;
	int j;
	for (j = start; (j < len && tText[j] != '}'); j++)
	{
		i++;
	}
	if (j < len)
	{
		i++;
	}
}

static set<int> getElecbyteTextLinebreaks(char* tText, const Position& tStart, MugenFont* tFont, MugenElecbyteFont& tElecbyteFont, double tRightX, double tFactor) {
	if (tRightX >= INF / 2) return set<int>();

	set<int> ret;
	Position p = tStart;
	auto n = int(strlen(tText));
	for (int i = 0; i < n; i++) {
		int positionsRead = 0;
		int doesBreak;
		if (tText[i] == '\n') {
				doesBreak = 1;
		}
		else {
			char word[1024];
			int items = sscanf(tText + i, "%1023s%n", word, &positionsRead);
			if (items != 1) break;
			for (int j = i; j < i + positionsRead; j++) {
				advanceMugenTextEffectPosition(tText, j, i + positionsRead);
				if (tElecbyteFont.mMap[(uint8_t)tText[j]].mExists) {
					MugenElecbyteFontMapEntry& mapEntry = tElecbyteFont.mMap[(uint8_t)tText[j]];
					p = vecAdd2D(p, vecScale(Vector3D(mapEntry.mWidth, 0, 0), tFactor));
					p = vecAdd2D(p, vecScale(Vector3D(tFont->mSpacing.x, 0, 0), tFactor));
				}
				else {
					p = vecAdd2D(p, vecScale(Vector3D(tFont->mSize.x, 0, 0), tFactor));
					p = vecAdd2D(p, vecScale(Vector3D(tFont->mSpacing.x, 0, 0), tFactor));
				}
			}
			doesBreak = (p.x > tRightX);
			i += std::max(positionsRead - 1, 0);
		}

		if (doesBreak) {
			auto skipIndex = i;
			i -= positionsRead - 1;
			while (i > 0 && i < n && tText[i] == ' ') i++;
			i = max(0, i - 1);
			if (ret.find(i) == ret.end()) {
				ret.insert(i);
				p.x = tStart.x;
			}
			else {
				i = skipIndex;
			}
		}
	}
	return ret;
}

static void setMugenTextEffectRed(MugenEffectText* e)
{
	e->mR = 1.0;
	e->mG = 0.0;
	e->mB = 0.0;
	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);
}

static void setMugenTextEffectColorCancel(MugenEffectText* e)
{
	e->mR = e->mBaseColor.x;
	e->mG = e->mBaseColor.y;
	e->mB = e->mBaseColor.z;
	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);
}

static void setMugenEffectTextToBaseEffect(MugenEffectText* e)
{
	setMugenTextEffectColorCancel(e);
}

static void parseMugenTextEffectCode(MugenEffectText* e, int& i, int len)
{
	if (e->mDisplayText[i] != '{') return;

	int start = i + 1;
	i += 1;
	std::string s;
	int j;
	for (j = start; (j < len && e->mDisplayText[j] != '}'); j++)
	{
		s.push_back(e->mDisplayText[j]);
		i++;
	}
	if (j < len)
	{
		i++;
	}
	
	if (s == "red")
	{
		setMugenTextEffectRed(e);
	}
	else if (s == "/red")
	{
		setMugenTextEffectColorCancel(e);
	}

}

static void drawSingleElecbyteText(MugenEffectText* e) {
	MugenFont* font = e->mFont;
	auto& elecbyteFont = std::get<MugenElecbyteFont>(font->mData);
	auto textLength = int(strlen(e->mDisplayText));
	double factor = getOriginalMugenFontFactor() * e->mScale;

	setMugenEffectTextToBaseEffect(e);
	setDrawingTransparency(e->mAlpha);

	int i;
	Position p = vecAdd2D(e->mPosition, Vector3D(font->mOffset.x, font->mOffset.y, 0));
	Position start = p;
	double rightX = p.x + e->mTextBoxWidth;
	const auto breaks = getElecbyteTextLinebreaks(e->mText, start, font, elecbyteFont, rightX, factor);
	for (i = 0; i < textLength; i++) {
		parseMugenTextEffectCode(e, i, textLength);
		if (elecbyteFont.mMap[(uint8_t)e->mDisplayText[i]].mExists) {
			ElecbyteDrawCaller caller;
			caller.mMapEntry = &elecbyteFont.mMap[(uint8_t)e->mDisplayText[i]];
			caller.mBasePosition = p;
			caller.mStartX = p.x;
			caller.mPreviousSubSpriteOffsetY = INF;
			caller.mText = e;
			caller.mFont = font;
			for (auto& texture : elecbyteFont.mSprite.mTextures)
			{
				drawSingleElecbyteSubSprite(texture, caller);
			}

			p = vecAdd2D(p, vecScale(Vector3D(caller.mMapEntry->mWidth, 0, 0), factor));
			p = vecAdd2D(p, vecScale(Vector3D(font->mSpacing.x, 0, 0), factor));
		}
		else {
			p = vecAdd2D(p, vecScale(Vector3D(font->mSize.x, 0, 0), factor));
			p = vecAdd2D(p, vecScale(Vector3D(font->mSpacing.x, 0, 0), factor));
		}
		if (breaks.find(i) != breaks.end()) {
			p.x = start.x;
			p = vecAdd2D(p, vecScale(Vector3D(0, font->mSize.y, 0), factor));
			p = vecAdd2D(p, vecScale(Vector3D(0, font->mSpacing.y, 0), factor));
		}
	}

	setDrawingBaseColorAdvanced(1, 1, 1);
	setDrawingTransparency(1.0);
}

static void drawSingleText(void* tCaller, MugenEffectText& tData) {
	(void)tCaller;

	MugenEffectText* e = &tData;
	if (!e->mIsVisible) return;

	if (e->mFont->mType == MUGEN_FONT_TYPE_BITMAP) {
		drawSingleBitmapText(e);
	}
	else if (e->mFont->mType == MUGEN_FONT_TYPE_TRUETYPE) {
		drawSingleTruetypeText(e);
	}
	else if (e->mFont->mType == MUGEN_FONT_TYPE_ELECBYTE) {
		drawSingleElecbyteText(e);
	}
	else {
		logError("Unimplemented font type.");
		logErrorInteger(e->mFont->mType);
		recoverFromError();
	}
}

static void drawMugenEffectTextHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gMugenEffectTextHandler.mHandledTexts, drawSingleText);
}

ActorBlueprint getMugenEffectTextHandler() {
	return makeActorBlueprint(loadMugenEffectTextHandlerActor, unloadMugenEffectTextHandlerActor, updateMugenEffectTextHandler, drawMugenEffectTextHandler);
};

void drawMugenEffectText(char* tText, const Position& tPosition, int tFont) {
	MugenEffectText textData;
	strcpy(textData.mText, tText);
	strcpy(textData.mDisplayText, tText);
	textData.mPosition = tPosition;
	textData.mFont = getUsedMugenFontFromAvailable(tFont);
	textData.mR = textData.mG = textData.mB = 1;
	textData.mBaseColor = Vector3D(textData.mR, textData.mG, textData.mB);
	textData.mScale = 1;
	textData.mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	textData.mRectangle = GeoRectangle2D(-INF / 2, -INF / 2, INF, INF);
	textData.mTextBoxWidth = INF;
	textData.mBuildupDurationPerLetter = INF;
	textData.mBuildupNow = 0;

	textData.mIsVisible = 1;
	drawSingleText(NULL, textData);
}

int addMugenEffectText(const char* tText, const Position& tPosition, int tFont)
{
	int id = stl_int_map_push_back(gMugenEffectTextHandler.mHandledTexts, MugenEffectText());

	MugenEffectText& e = gMugenEffectTextHandler.mHandledTexts[id];
	strcpy(e.mText, tText);
	strcpy(e.mDisplayText, tText);

	e.mFont = getUsedMugenFontFromAvailable(tFont);
	e.mScale = 1;
	e.mPosition = vecSub(tPosition, Vector3D(0, e.mFont->mSize.y * e.mScale, 0));
	e.mR = e.mG = e.mB = e.mAlpha = 1;
	e.mBaseColor = Vector3D(e.mR, e.mG, e.mB);
	e.mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	e.mRectangle = GeoRectangle2D(-INF / 2, -INF / 2, INF, INF);
	e.mTextBoxWidth = INF;

	e.mBuildupDurationPerLetter = INF;
	e.mBuildupNow = 0;

	e.mIsVisible = 1;

	return id;
}

int addMugenEffectTextMugenStyle(const char * tText, const Position& tPosition, const Vector3DI& tFont)
{
	int ret = addMugenEffectText(tText, tPosition, tFont.x);
	setMugenEffectTextColor(ret, getMugenEffectTextColorFromMugenEffectTextColorIndex(tFont.y));
	setMugenEffectTextAlignment(ret, getMugenEffectTextAlignmentFromMugenAlignmentIndex(tFont.z));

	return ret;
}

void removeMugenEffectText(int tID)
{
	gMugenEffectTextHandler.mHandledTexts.erase(tID);
}

void setMugenEffectTextFont(int tID, int tFont)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];

	e->mPosition = vecAdd2D(e->mPosition, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
	e->mFont = getUsedMugenFontFromAvailable(tFont);
	e->mPosition = vecSub(e->mPosition, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
}

static double getBitmapTextSize(MugenEffectText* e) {
	MugenFont* font = e->mFont;
	auto& bitmapFont = std::get<MugenBitmapFont>(font->mData);
	auto textLength = int(strlen(e->mText));
	double factor = getOriginalMugenFontFactor() * e->mScale;

	auto& spriteGroup = bitmapFont.mSprites.mGroups[0];

	double sizeX = 0;
	int i;
	for (i = 0; i < textLength; i++) {
		if (stl_map_contains(spriteGroup.mSprites, (int)e->mText[i])) {
			auto& sprite = spriteGroup.mSprites[(int)e->mText[i]];
			sizeX += sprite.mOriginalTextureSize.x * factor;
			sizeX += font->mSpacing.x * factor;
		}
		else {
			sizeX += font->mSize.x * factor;
			sizeX += font->mSpacing.x * factor;
		}
	}

	return sizeX;
}

static double getTruetypeTextSize(MugenEffectText* e) {
	auto textLength = strlen(e->mText);
	return double(e->mFont->mSize.x*textLength);
}

static double getElecbyteTextSize(MugenEffectText* e) {
	MugenFont* font = e->mFont;
	auto& elecbyteFont = std::get<MugenElecbyteFont>(font->mData);
	auto textLength = int(strlen(e->mText));
	double factor = getOriginalMugenFontFactor() * e->mScale;

	int i;
	double sizeX = 0;
	for (i = 0; i < textLength; i++) {
		if (elecbyteFont.mMap[(uint8_t)e->mText[i]].mExists) {
			MugenElecbyteFontMapEntry& mapEntry = elecbyteFont.mMap[(uint8_t)e->mText[i]];
			sizeX += mapEntry.mWidth*factor;
			sizeX += font->mSpacing.x*factor;
		}
		else {
			sizeX += font->mSize.x*factor;
			sizeX += font->mSpacing.x*factor;
		}
	}

	return sizeX;
}

static double getMugenEffectTextSizeXInternal(MugenEffectText* e) {

	if (e->mFont->mType == MUGEN_FONT_TYPE_BITMAP) {
		return getBitmapTextSize(e);
	}
	else if (e->mFont->mType == MUGEN_FONT_TYPE_TRUETYPE) {
		return getTruetypeTextSize(e);
	}
	else if (e->mFont->mType == MUGEN_FONT_TYPE_ELECBYTE) {
		return getElecbyteTextSize(e);
	}
	else {
		logError("Unimplemented font type.");
		logErrorInteger(e->mFont->mType);
		recoverFromError();
		return 0;
	}
}

static double getMugenEffectTextAlignmentOffsetX(MugenEffectText* e, MugenTextAlignment tAlignment) {
	double sizeX = getMugenEffectTextSizeXInternal(e);

	double ret = 0;
	if (e->mAlignment == MUGEN_TEXT_ALIGNMENT_CENTER) {
		ret += sizeX / 2;
	}
	else if (e->mAlignment == MUGEN_TEXT_ALIGNMENT_RIGHT) {
		ret += sizeX;
	}

	if (tAlignment == MUGEN_TEXT_ALIGNMENT_CENTER) {
		ret -= sizeX / 2;
	}
	else if (tAlignment == MUGEN_TEXT_ALIGNMENT_RIGHT) {
		ret -= sizeX;
	}

	return ret;
}

void setMugenEffectTextAlignment(int tID, MugenTextAlignment tAlignment)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mPosition.x += getMugenEffectTextAlignmentOffsetX(e, tAlignment);
	e->mAlignment = tAlignment;
}

void setMugenEffectTextColor(int tID, Color tColor)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	getRGBFromColor(tColor, &e->mR,&e->mG, &e->mB);
	e->mBaseColor = Vector3D(e->mR, e->mG, e->mB);
}

void setMugenEffectTextColorRGB(int tID, double tR, double tG, double tB)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mR = tR;
	e->mG = tG;
	e->mB = tB;
	e->mBaseColor = Vector3D(e->mR, e->mG, e->mB);
}

void setMugenEffectTextRectangle(int tID, const GeoRectangle2D& tRectangle)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mRectangle = tRectangle;
		
}

void setMugenEffectTextPosition(int tID, const Position& tPosition)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mPosition = tPosition;
	e->mPosition = vecSub(e->mPosition, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
	MugenTextAlignment alignment = e->mAlignment;
	e->mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	setMugenEffectTextAlignment(tID, alignment);
}

void addMugenEffectTextPosition(int tID, const Position& tPosition)
{
	auto pos = getMugenEffectTextPositionReference(tID);
	*pos += tPosition;
}

void setMugenEffectTextTextBoxWidth(int tID, double tWidth)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mTextBoxWidth = tWidth;
}

void setMugenEffectTextBuildup(int tID, Duration mBuildUpDurationPerLetter)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mDisplayText[0] = '\0';
	e->mBuildupNow = 0;
	e->mBuildupDurationPerLetter = mBuildUpDurationPerLetter;
}

void setMugenEffectTextBuiltUp(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	strcpy(e->mDisplayText, e->mText);
}

int isMugenEffectTextBuiltUp(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return !strcmp(e->mText, e->mDisplayText);
}

int getMugenEffectTextVisibility(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return e->mIsVisible;
}

void setMugenEffectTextVisibility(int tID, int tIsVisible)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mIsVisible = tIsVisible;
}

double getMugenEffectTextSizeX(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return getMugenEffectTextSizeXInternal(e);
}

void setMugenEffectTextScale(int tID, double tScale)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	Position p = getMugenEffectTextPosition(tID);
	e->mScale = tScale;
	setMugenEffectTextPosition(tID, p);
}

double getMugenEffectTextTransparency(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return e->mAlpha;
}

void setMugenEffectTextTransparency(int tID, double tOpacity)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	e->mAlpha = tOpacity;
}

const char* getMugenEffectTextText(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return e->mText;
}

const char* getMugenEffectTextDisplayedText(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return e->mDisplayText;
}

void changeMugenEffectText(int tID, const char * tText)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	MugenTextAlignment alignment = e->mAlignment;
	setMugenEffectTextAlignment(tID, MUGEN_TEXT_ALIGNMENT_LEFT);
	strcpy(e->mText, tText);
	strcpy(e->mDisplayText, tText);
	setMugenEffectTextAlignment(tID, alignment);
}

Position getMugenEffectTextPosition(int tID) {
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	
	MugenTextAlignment alignment = e->mAlignment;
	setMugenEffectTextAlignment(tID, MUGEN_TEXT_ALIGNMENT_LEFT);
	Position ret = e->mPosition;
	ret = vecAdd(ret, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
	setMugenEffectTextAlignment(tID, alignment);
	return ret;
}

Position * getMugenEffectTextPositionReference(int tID)
{
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tID];
	return &e->mPosition;
}

Vector3D getMugenEffectTextColor(int tIndex) {
	MugenEffectText* e = &gMugenEffectTextHandler.mHandledTexts[tIndex];
	return Vector3D(e->mR, e->mG, e->mB);
}

MugenTextAlignment getMugenEffectTextAlignmentFromMugenAlignmentIndex(int tIndex)
{
	if (tIndex > 0) return MUGEN_TEXT_ALIGNMENT_LEFT;
	else if (tIndex < 0) return MUGEN_TEXT_ALIGNMENT_RIGHT;
	else return MUGEN_TEXT_ALIGNMENT_CENTER;
}

Color getMugenEffectTextColorFromMugenEffectTextColorIndex(int tIndex)
{
	switch (tIndex) {
	case 0:
		return COLOR_WHITE;
	case 1:
		return COLOR_RED;
	case 2:
		return COLOR_GREEN;
	case 3:
		return COLOR_BLUE;
	case 4:
		return COLOR_CYAN;
	case 5:
		return COLOR_YELLOW;
	case 6:
		return COLOR_MAGENTA;
	case 7:
		return COLOR_BLACK;
	case 8:
		return COLOR_LIGHT_GRAY;
	default:
		logError("Unrecognized Mugen text color.");
		logErrorInteger(tIndex);
		recoverFromError();
		return COLOR_WHITE;
	}
}

}