#include "prism/mugentexthandler.h"

#include <assert.h>
#include <algorithm>
#include <set>

#include "prism/mugendefreader.h"
#include "prism/mugenanimationhandler.h"
#include "prism/log.h"
#include "prism/system.h"
#include "prism/drawing.h"
#include "prism/texture.h"
#include "prism/math.h"
#include "prism/stlutil.h"

using namespace std;

typedef enum {
	MUGEN_FONT_TYPE_BITMAP,
	MUGEN_FONT_TYPE_TRUETYPE,
	MUGEN_FONT_TYPE_ELECBYTE,
} MugenFontType;

typedef enum {
	MUGEN_ELECBYTE_FONT_TYPE_VARIABLE,
	MUGEN_ELECBYTE_FONT_TYPE_FIXED,
} MugenElecbyteFontType;

typedef struct {
	int mExists;
	int mStartX;
	int mWidth;

} MugenElecbyteFontMapEntry;

typedef struct {
	MugenElecbyteFontMapEntry mMap[0xFF];
	MugenSpriteFileSprite* mSprite;

	MugenElecbyteFontType mType;
} MugenElecbyteFont;

typedef struct {
	TruetypeFont mFont;
} MugenTruetypeFont;

typedef struct {
	MugenSpriteFile mSprites;
} MugenBitmapFont;

typedef struct {
	MugenFontType mType;
	void* mData;

	Vector3DI mSize;
	Vector3DI mSpacing;
	Vector3DI mOffset;
} MugenFont;

static struct {
	map<int, MugenFont> mFonts;
} gMugenFontData;

static void loadBitmapFont(MugenDefScript* tScript, MugenFont* tFont) {
	MugenBitmapFont* e = (MugenBitmapFont*)allocMemory(sizeof(MugenBitmapFont));

	char* path = getAllocatedMugenDefStringVariable(tScript, "def", "file");
	e->mSprites = loadMugenSpriteFileWithoutPalette(path);
	freeMemory(path);

	tFont->mData = e;
}

static void loadMugenTruetypeFont(MugenDefScript* tScript, MugenFont* tFont) {
	MugenTruetypeFont* e = (MugenTruetypeFont*)allocMemory(sizeof(MugenTruetypeFont));
	char* name = getAllocatedMugenDefStringVariable(tScript, "def", "file");
	e->mFont = loadTruetypeFont(name, tFont->mSize.y);
	freeMemory(name);
	tFont->mData = e;
}

typedef struct {
	MugenFont* mFont;
	MugenElecbyteFont* mElecbyteFont;
	int i;
} ElecbyteMapParseCaller;

static void parseSingleMapElement(void* tCaller, void* tData) {
	ElecbyteMapParseCaller* caller = (ElecbyteMapParseCaller*)tCaller;
	MugenDefScriptGroupElement* element = (MugenDefScriptGroupElement*)tData;
	assert(element->mType == MUGEN_DEF_SCRIPT_GROUP_STRING_ELEMENT);
	MugenDefScriptStringElement* e = (MugenDefScriptStringElement*)element->mData;

	MugenElecbyteFontMapEntry entry;

	char key[100];
	if (caller->mElecbyteFont->mType == MUGEN_ELECBYTE_FONT_TYPE_VARIABLE) {
		int items = sscanf(e->mString, "%s %d %d", key, &entry.mStartX, &entry.mWidth);
		if (items != 3) {
			logWarningFormat("Unable to parse entry from string: %s", e->mString);
			caller->i++;
			return;
		}
	}
	else if (caller->mElecbyteFont->mType == MUGEN_ELECBYTE_FONT_TYPE_FIXED) {
		int items = sscanf(e->mString, "%s", key);
		(void)items;
		assert(items == 1);
		entry.mStartX = caller->i * caller->mFont->mSize.x;
		entry.mWidth = caller->mFont->mSize.x;
	}

	int keyValue;
	if (strlen(key) == 1) {
		keyValue = key[0];
	}
	else if (key[0] == '0' && key[1] == 'x') {
		int items = sscanf(e->mString, "%i", &keyValue);
		if (items != 1) {
			logWarningFormat("Unable to parse keyValue from string: %s", e->mString);
			keyValue = 0;
		}
	}
	else {
		logError("Unrecognized map key.");
		logErrorString(key);
		recoverFromError();
		keyValue = -1;
	}

	entry.mExists = 1;
	caller->mElecbyteFont->mMap[keyValue % 0xFF] = entry;

	caller->i++;
}

static MugenElecbyteFontType getMugenElecbyteFontType(MugenDefScript* tScript) {
	MugenElecbyteFontType ret;

	char* text = getAllocatedMugenDefStringVariable(tScript, "def", "type");
	turnStringLowercase(text);

	if (!strcmp("fixed", text)) {
		ret = MUGEN_ELECBYTE_FONT_TYPE_FIXED;
	}
	else if (!strcmp("variable", text)) {
		ret = MUGEN_ELECBYTE_FONT_TYPE_VARIABLE;
	}
	else {
		logError("Unable to determine font type.");
		logErrorString(text);
		recoverFromError();
		ret = MUGEN_ELECBYTE_FONT_TYPE_VARIABLE;
	}

	freeMemory(text);

	return ret;
}

static void loadMugenElecbyteFont(MugenDefScript* tScript, const Buffer& tTextureBuffer, MugenFont* tFont) {
	MugenElecbyteFont* e = (MugenElecbyteFont*)allocMemory(sizeof(MugenElecbyteFont));

	e->mType = getMugenElecbyteFontType(tScript);

	for (int i = 0; i < 0xFF; i++) e->mMap[i].mExists = 0;

	MugenDefScriptGroup* group = &tScript->mGroups["map"];

	ElecbyteMapParseCaller caller;
	caller.mFont = tFont;
	caller.mElecbyteFont = e;
	caller.i = 0;
	list_map(&group->mOrderedElementList, parseSingleMapElement, &caller);

	e->mSprite = loadSingleTextureFromPCXBuffer(tTextureBuffer);

	tFont->mData = e;
}

static MugenFontType getMugenFontTypeFromScript(MugenDefScript* tScript) {
	char* text = getAllocatedMugenDefStringVariable(tScript, "def", "type");
	turnStringLowercase(text);

	MugenFontType ret;
	if (!strcmp("bitmap", text)) {
		ret = MUGEN_FONT_TYPE_BITMAP;
	}
	else if (!strcmp("truetype", text)) {
		ret = MUGEN_FONT_TYPE_TRUETYPE;
	}
	else {
		ret = MUGEN_FONT_TYPE_BITMAP;
		logError("Unable to determine font type");
		logErrorString(text);
		recoverFromError();
	}

	freeMemory(text);

	return ret;
}

static void setMugenFontDirectory(const char* tPath) {

	(void)tPath;
	setWorkingDirectory("/");
}

static void setMugenFontDirectory2(const char* tPath) {
	char path[1024];
	getPathToFile(path, tPath);
	setWorkingDirectory(path);
}

static void resetMugenFontDirectory() {
	setWorkingDirectory("/");
}

int hasMugenFont(int tKey)
{
	return stl_map_contains(gMugenFontData.mFonts, tKey);
}

typedef struct {
	uint8_t mMagic[12];
	uint16_t mVersionHigh;
	uint16_t mVersionLow;

	uint32_t mTextureOffset;
	uint32_t mTextureLength;

	uint32_t mTextOffset;
	uint32_t mTextLength;

	uint8_t mComment[40];
} MugenFontHeader;

static void addMugenFont1(int tKey, const char* tPath) {
	setMugenFontDirectory(tPath);

	MugenFontHeader header;

	Buffer b = fileToBuffer(tPath);
	BufferPointer p = getBufferPointer(b);
	readFromBufferPointer(&header, &p, sizeof(MugenFontHeader));

	Buffer textureBuffer = makeBuffer((void*)(((uintptr_t)b.mData) + header.mTextureOffset), header.mTextureLength);
	Buffer textBuffer = makeBuffer((void*)(((uintptr_t)b.mData) + header.mTextOffset), (b.mLength - header.mTextOffset));
	MugenDefScript script; 
	loadMugenDefScriptFromBufferAndFreeBuffer(&script, textBuffer);

	MugenFont& e = gMugenFontData.mFonts[tKey];
	e.mSize = getMugenDefVectorIOrDefault(&script, "def", "size", Vector3DI(1, 1, 0));
	e.mSpacing = getMugenDefVectorIOrDefault(&script, "def", "spacing", Vector3DI(0, 0, 0));
	e.mOffset = getMugenDefVectorIOrDefault(&script, "def", "offset", Vector3DI(0, 0, 0));

	e.mType = MUGEN_FONT_TYPE_ELECBYTE;
	loadMugenElecbyteFont(&script, textureBuffer, &e);

	unloadMugenDefScript(&script);

	freeBuffer(b);
	resetMugenFontDirectory();
}

static void addMugenFont2(int tKey, const char* tPath) {
	MugenDefScript script; 
	loadMugenDefScript(&script, tPath);

	setMugenFontDirectory2(tPath);

	MugenFont& e = gMugenFontData.mFonts[tKey];
	e.mType = getMugenFontTypeFromScript(&script);

	e.mSize = getMugenDefVectorIOrDefault(&script, "def", "size", Vector3DI(1, 1, 0));
	e.mSpacing = getMugenDefVectorIOrDefault(&script, "def", "spacing", Vector3DI(0, 0, 0));
	e.mOffset = getMugenDefVectorIOrDefault(&script, "def", "offset", Vector3DI(0, 0, 0));

	if (e.mType == MUGEN_FONT_TYPE_BITMAP) {
		loadBitmapFont(&script, &e);
	}
	else if (e.mType == MUGEN_FONT_TYPE_TRUETYPE) {
		loadMugenTruetypeFont(&script, &e);
	}
	else {
		logError("Unimplemented font type.");
		logErrorInteger(e.mType);
		recoverFromError();
	}

	unloadMugenDefScript(&script);

	resetMugenFontDirectory();
}

void addMugenFont(int tKey, const char* tPath) {
	char path[1024];
	if (strchr(tPath, '/')) {
		sprintf(path, "assets/%s", tPath);
		if (!isFile(path)) {
			sprintf(path, "%s", tPath);
		}
	}
	else {
		sprintf(path, "assets/font/%s", tPath);
		if (!isFile(path)) {
			sprintf(path, "font/%s", tPath);
		}
	}
	
	if (!isFile(path)) {
		logWarningFormat("Unable to open font %s. Ignoring.", tPath);
		return;
	}

	std::string ending = getFileExtension(path);
	turnStringLowercase(ending);

	if (ending == "def") {
		addMugenFont2(tKey, path);
	}
	else if (ending == "fnt") {
		addMugenFont1(tKey, path);
	}
	else {
		logError("Unrecognized font file type.");
		logErrorString(tPath);
		recoverFromError();
	}

}

void loadMugenTextHandler()
{
	gMugenFontData.mFonts.clear();
}

static void unloadBitmapFont(MugenFont* tFont) {
	MugenBitmapFont* bitmapFont = (MugenBitmapFont*)tFont->mData;
	unloadMugenSpriteFile(&bitmapFont->mSprites);
	freeMemory(bitmapFont);
}

static void unloadElecbyteFont(MugenFont* tFont) {
	MugenElecbyteFont* elecbyteFont = (MugenElecbyteFont*)tFont->mData;
	unloadMugenSpriteFileSprite(elecbyteFont->mSprite);
	freeMemory(elecbyteFont->mSprite);
	freeMemory(elecbyteFont);
}

static void unloadMugenTruetypeFont(MugenFont* tFont) {
	MugenTruetypeFont* truetypeFont = (MugenTruetypeFont*)tFont->mData;
	unloadTruetypeFont(truetypeFont->mFont);
	freeMemory(truetypeFont);
}


static int unloadSingleFont(void* tCaller, MugenFont& tData) {
	(void)tCaller;
	MugenFont* font = &tData;

	if (font->mType == MUGEN_FONT_TYPE_BITMAP) {
		unloadBitmapFont(font);
	} else if (font->mType == MUGEN_FONT_TYPE_ELECBYTE) {
		unloadElecbyteFont(font);
	} else if (font->mType == MUGEN_FONT_TYPE_TRUETYPE) {
		unloadMugenTruetypeFont(font);
	}
	else {
		logError("Unimplemented font type.");
		logErrorInteger(font->mType);
		recoverFromError();
	}

	return 1;
}

void unloadMugenFonts()
{
	stl_int_map_remove_predicate(gMugenFontData.mFonts, unloadSingleFont);
}

int getMugenFontSizeY(int tKey)
{
	MugenFont& font = gMugenFontData.mFonts[tKey];
	return font.mSize.y;
}

int getMugenFontSpacingY(int tKey)
{
	MugenFont& font = gMugenFontData.mFonts[tKey];
	return font.mSpacing.y;
}


typedef struct {
	char mText[1024];
	char mDisplayText[1024];
	Position mPosition;
	MugenFont* mFont;

	double mR;
	double mG;
	double mB;

	double mScale;

	MugenTextAlignment mAlignment;
	GeoRectangle2D mRectangle;
	double mTextBoxWidth;
	Duration mBuildupDurationPerLetter;
	Duration mBuildupNow;

	int mIsVisible;
} MugenText;

static struct {
	map<int, MugenText> mHandledTexts;

} gMugenTextHandler;

static void loadMugenTextHandlerActor(void* tData) {
	(void) tData;
	setProfilingSectionMarkerCurrentFunction();
	gMugenTextHandler.mHandledTexts.clear();
}

static void unloadMugenTextHandlerActor(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	gMugenTextHandler.mHandledTexts.clear();
}

static void updateSingleTextBuildup(MugenText* e) {
	if (handleDurationAndCheckIfOver(&e->mBuildupNow, e->mBuildupDurationPerLetter)) {
		if (!strcmp(e->mText, e->mDisplayText)) {
			e->mBuildupDurationPerLetter = INF;
			return;
		}

	
		int l = strlen(e->mDisplayText);
		e->mDisplayText[l] = e->mText[l];
		e->mDisplayText[l + 1] = '\0';
	}
}

static void updateSingleText(void* tCaller, MugenText& tData) {
	(void)tCaller;
	(void)tData;
	MugenText* e = &tData;
	updateSingleTextBuildup(e);
}

static void updateMugenTextHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gMugenTextHandler.mHandledTexts, updateSingleText);
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
	MugenText* mText;
	MugenFont* mFont;
} BitmapDrawCaller;

static void drawSingleBitmapSubSprite(void* tCaller, void* tData) {
	BitmapDrawCaller* caller = (BitmapDrawCaller*)tCaller;
	MugenSpriteFileSubSprite* subSprite = (MugenSpriteFileSubSprite*)tData;

	double factor = getNewMugenFontFactor() * caller->mText->mScale;
	Position p = caller->mBasePosition + (subSprite->mOffset * factor);	
	
	int minHeight = 0;
	int maxHeight = subSprite->mTexture.mTextureSize.y - 1;
	int upY = max(minHeight, min(maxHeight, (int)(caller->mText->mRectangle.mTopLeft.y - p.y)));
	int downY = max(minHeight, min(maxHeight, upY + (int)(caller->mText->mRectangle.mBottomRight.y - (p.y + caller->mFont->mSize.y))));
	if (upY == downY) return;

	p.y = max(p.y, caller->mText->mRectangle.mTopLeft.y);

	scaleDrawing(factor, p);
	drawSprite(subSprite->mTexture, p, makeRectangleFromTexture(subSprite->mTexture));
	scaleDrawing(1 / factor, p);
}

static set<int> getBitmapTextLinebreaks(char* tText, const Position& tStart, MugenFont* tFont, MugenSpriteFileGroup* tSpriteGroup, double tRightX, double tFactor) {
	if (tRightX >= INF / 2) return set<int>();

	set<int> ret;
	Position p = tStart;
	int n = strlen(tText);
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
				if (int_map_contains(&tSpriteGroup->mSprites, (int)tText[j])) {
					MugenSpriteFileSprite* sprite = (MugenSpriteFileSprite*)int_map_get(&tSpriteGroup->mSprites, (int)tText[j]);
					p = vecAdd2D(p, vecScale(Vector3D(sprite->mOriginalTextureSize.x, 0, 0), tFactor));
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

static void drawSingleBitmapText(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenBitmapFont* bitmapFont = (MugenBitmapFont*)font->mData;
	int textLength = strlen(e->mDisplayText);
	double factor = getNewMugenFontFactor() * e->mScale;

	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);

	MugenSpriteFileGroup* spriteGroup = (MugenSpriteFileGroup*)int_map_get(&bitmapFont->mSprites.mGroups, 0);

	int i;
	Position p = vecAdd2D(e->mPosition, vecScale(Vector3D(font->mOffset.x, font->mOffset.y, 0), factor));
	Position start = p;
	double rightX = p.x + e->mTextBoxWidth;
	const auto breaks = getBitmapTextLinebreaks(e->mText, start, font, spriteGroup, rightX, factor);
	for (i = 0; i < textLength; i++) {

		if (int_map_contains(&spriteGroup->mSprites, (int)e->mDisplayText[i])) {
			BitmapDrawCaller caller;
			caller.mSprite = (MugenSpriteFileSprite*)int_map_get(&spriteGroup->mSprites, (int)e->mDisplayText[i]);
			caller.mBasePosition = p;
			caller.mText = e;
			caller.mFont = font;
			list_map(&caller.mSprite->mTextures, drawSingleBitmapSubSprite, &caller);

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
}

static void drawSingleTruetypeText(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenTruetypeFont* truetypeFont = (MugenTruetypeFont*)font->mData;
	drawTruetypeText(e->mDisplayText, truetypeFont->mFont, e->mPosition, font->mSize, Vector3D(e->mR, e->mG, e->mB), e->mTextBoxWidth, e->mRectangle);
}

typedef struct {
	MugenElecbyteFontMapEntry* mMapEntry;
	Position mBasePosition;
	double mStartX;
	int mPreviousSubSpriteOffsetY;
	MugenText* mText;
	MugenFont* mFont;
} ElecbyteDrawCaller;

static void drawSingleElecbyteSubSprite(void* tCaller, void* tData) {
	ElecbyteDrawCaller* caller = (ElecbyteDrawCaller*)tCaller;
	MugenSpriteFileSubSprite* subSprite = (MugenSpriteFileSubSprite*)tData;

	if (caller->mMapEntry->mStartX >= subSprite->mOffset.x + subSprite->mTexture.mTextureSize.x) return;
	if (caller->mMapEntry->mStartX + caller->mMapEntry->mWidth - 1 < subSprite->mOffset.x) return;

	if (subSprite->mOffset.y > caller->mPreviousSubSpriteOffsetY) {
		caller->mBasePosition.x = caller->mStartX;
	}
	caller->mPreviousSubSpriteOffsetY = subSprite->mOffset.y;

	int minWidth = 0;
	int maxWidth = subSprite->mTexture.mTextureSize.x - 1;
	int leftX = max(minWidth, min(maxWidth, caller->mMapEntry->mStartX - subSprite->mOffset.x));
	int rightX = max(minWidth, min(maxWidth, (caller->mMapEntry->mStartX + caller->mMapEntry->mWidth - 1) - subSprite->mOffset.x));

	double factor = getOriginalMugenFontFactor() * caller->mText->mScale;
	Position p = vecAdd2D(caller->mBasePosition, vecScale(Vector3D(0.f, subSprite->mOffset.y, 0.f), factor));

	int minHeight = 0;
	int maxHeight = subSprite->mTexture.mTextureSize.y - 1;
	int upY = max(minHeight, min(maxHeight, (int)(caller->mText->mRectangle.mTopLeft.y - p.y)));
	int downY = max(minHeight, min(maxHeight, upY + (int)(caller->mText->mRectangle.mBottomRight.y - (p.y + caller->mFont->mSize.y))));
	if (upY == downY) return;

	p.y = max(p.y, caller->mText->mRectangle.mTopLeft.y);

	scaleDrawing(factor, p);
	drawSprite(subSprite->mTexture, p, makeRectangle(leftX, upY, rightX - leftX, downY - upY));
	scaleDrawing(1 / factor, p);

	caller->mBasePosition.x += (rightX - leftX + 1) * factor;
}

static set<int> getElecbyteTextLinebreaks(char* tText, const Position& tStart, MugenFont* tFont, MugenElecbyteFont* tElecbyteFont, double tRightX, double tFactor) {
	if (tRightX >= INF / 2) return set<int>();

	set<int> ret;
	Position p = tStart;
	int n = strlen(tText);
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
				if (tElecbyteFont->mMap[(int)tText[j]].mExists) {
					MugenElecbyteFontMapEntry& mapEntry = tElecbyteFont->mMap[(int)tText[j]];
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

static void drawSingleElecbyteText(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenElecbyteFont* elecbyteFont = (MugenElecbyteFont*)font->mData;
	int textLength = strlen(e->mDisplayText);
	double factor = getOriginalMugenFontFactor() * e->mScale;

	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);
	int i;
	Position p = vecAdd2D(e->mPosition, Vector3D(font->mOffset.x, font->mOffset.y, 0));
	Position start = p;
	double rightX = p.x + e->mTextBoxWidth;
	const auto breaks = getElecbyteTextLinebreaks(e->mText, start, font, elecbyteFont, rightX, factor);
	for (i = 0; i < textLength; i++) {

		if (elecbyteFont->mMap[(int)e->mDisplayText[i]].mExists) {
			ElecbyteDrawCaller caller;
			caller.mMapEntry = &elecbyteFont->mMap[(int)e->mDisplayText[i]];
			caller.mBasePosition = p;
			caller.mStartX = p.x;
			caller.mPreviousSubSpriteOffsetY = INF;
			caller.mText = e;
			caller.mFont = font;
			list_map(&elecbyteFont->mSprite->mTextures, drawSingleElecbyteSubSprite, &caller);

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
}

static void drawSingleText(void* tCaller, MugenText& tData) {
	(void)tCaller;

	MugenText* e = &tData;
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

static void drawMugenTextHandler(void* tData) {
	(void)tData;
	setProfilingSectionMarkerCurrentFunction();
	stl_int_map_map(gMugenTextHandler.mHandledTexts, drawSingleText);
}

ActorBlueprint getMugenTextHandler() {
	return makeActorBlueprint(loadMugenTextHandlerActor, unloadMugenTextHandlerActor, updateMugenTextHandler, drawMugenTextHandler);
};

void drawMugenText(char* tText, const Position& tPosition, int tFont) {
	MugenText textData;
	strcpy(textData.mText, tText);
	strcpy(textData.mDisplayText, tText);
	textData.mPosition = tPosition;
	textData.mFont = &gMugenFontData.mFonts[tFont];
	textData.mR = textData.mG = textData.mB = 1;
	textData.mScale = 1;
	textData.mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	textData.mRectangle = GeoRectangle2D(-INF / 2, -INF / 2, INF, INF);
	textData.mTextBoxWidth = INF;
	textData.mBuildupDurationPerLetter = INF;
	textData.mBuildupNow = 0;

	textData.mIsVisible = 1;
	drawSingleText(NULL, textData);
}

int addMugenText(const char * tText, const Position& tPosition, int tFont)
{
	int id = stl_int_map_push_back(gMugenTextHandler.mHandledTexts, MugenText());

	MugenText& e = gMugenTextHandler.mHandledTexts[id];
	strcpy(e.mText, tText);
	strcpy(e.mDisplayText, tText);

	if (stl_map_contains(gMugenFontData.mFonts, tFont)) {
		e.mFont = &gMugenFontData.mFonts[tFont];
	}
	else {
		e.mFont = &gMugenFontData.mFonts[1];
	}
	e.mScale = 1;
	e.mPosition = vecSub(tPosition, Vector3D(0, e.mFont->mSize.y * e.mScale, 0));
	e.mR = e.mG = e.mB = 1;
	e.mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	e.mRectangle = GeoRectangle2D(-INF / 2, -INF / 2, INF, INF);
	e.mTextBoxWidth = INF;

	e.mBuildupDurationPerLetter = INF;
	e.mBuildupNow = 0;

	e.mIsVisible = 1;

	return id;
}

int addMugenTextMugenStyle(const char * tText, const Position& tPosition, const Vector3DI& tFont)
{
	int ret = addMugenText(tText, tPosition, tFont.x);
	setMugenTextColor(ret, getMugenTextColorFromMugenTextColorIndex(tFont.y));
	setMugenTextAlignment(ret, getMugenTextAlignmentFromMugenAlignmentIndex(tFont.z));

	return ret;
}

void removeMugenText(int tID)
{
	gMugenTextHandler.mHandledTexts.erase(tID);
}

void setMugenTextFont(int tID, int tFont)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];

	e->mPosition = vecAdd2D(e->mPosition, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
	if (stl_map_contains(gMugenFontData.mFonts, tFont)) {
		e->mFont = &gMugenFontData.mFonts[tFont];
	}
	else {
		e->mFont = &gMugenFontData.mFonts[1];
	}

	e->mPosition = vecSub(e->mPosition, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
}

static double getBitmapTextSize(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenBitmapFont* bitmapFont = (MugenBitmapFont*)font->mData;
	int textLength = strlen(e->mText);
	double factor = getOriginalMugenFontFactor() * e->mScale;

	MugenSpriteFileGroup* spriteGroup = (MugenSpriteFileGroup*)int_map_get(&bitmapFont->mSprites.mGroups, 0);

	double sizeX = 0;
	int i;
	for (i = 0; i < textLength; i++) {
		if (int_map_contains(&spriteGroup->mSprites, (int)e->mText[i])) {
			MugenSpriteFileSprite* sprite = (MugenSpriteFileSprite*)int_map_get(&spriteGroup->mSprites, (int)e->mText[i]);
			sizeX += sprite->mOriginalTextureSize.x * factor;
			sizeX += font->mSpacing.x * factor;
		}
		else {
			sizeX += font->mSize.x * factor;
			sizeX += font->mSpacing.x * factor;
		}
	}

	return sizeX;
}

static double getTruetypeTextSize(MugenText* e) {
	int textLength = strlen(e->mText);
	return e->mFont->mSize.x*textLength;
}

static double getElecbyteTextSize(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenElecbyteFont* elecbyteFont = (MugenElecbyteFont*)font->mData;
	int textLength = strlen(e->mText);
	double factor = getOriginalMugenFontFactor() * e->mScale;

	int i;
	double sizeX = 0;
	for (i = 0; i < textLength; i++) {
		if (elecbyteFont->mMap[(int)e->mText[i]].mExists) {
			MugenElecbyteFontMapEntry& mapEntry = elecbyteFont->mMap[(int)e->mText[i]];
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

static double getMugenTextSizeXInternal(MugenText* e) {

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

static double getMugenTextAlignmentOffsetX(MugenText* e, MugenTextAlignment tAlignment) {
	double sizeX = getMugenTextSizeXInternal(e);

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

void setMugenTextAlignment(int tID, MugenTextAlignment tAlignment)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mPosition.x += getMugenTextAlignmentOffsetX(e, tAlignment);
	e->mAlignment = tAlignment;
}

void setMugenTextColor(int tID, Color tColor)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	getRGBFromColor(tColor, &e->mR,&e->mG, &e->mB);
}

void setMugenTextColorRGB(int tID, double tR, double tG, double tB)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mR = tR;
	e->mG = tG;
	e->mB = tB;
}

void setMugenTextRectangle(int tID, const GeoRectangle2D& tRectangle)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mRectangle = tRectangle;
		
}

void setMugenTextPosition(int tID, const Position& tPosition)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mPosition = tPosition;
	e->mPosition = vecSub(e->mPosition, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
	MugenTextAlignment alignment = e->mAlignment;
	e->mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	setMugenTextAlignment(tID, alignment);
}

void addMugenTextPosition(int tID, const Position& tPosition)
{
	auto pos = getMugenTextPositionReference(tID);
	*pos += tPosition;
}

void setMugenTextTextBoxWidth(int tID, double tWidth)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mTextBoxWidth = tWidth;
}

void setMugenTextBuildup(int tID, Duration mBuildUpDurationPerLetter)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mDisplayText[0] = '\0';
	e->mBuildupNow = 0;
	e->mBuildupDurationPerLetter = mBuildUpDurationPerLetter;
}

void setMugenTextBuiltUp(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	strcpy(e->mDisplayText, e->mText);
}

int isMugenTextBuiltUp(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	return !strcmp(e->mText, e->mDisplayText);
}

int getMugenTextVisibility(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	return e->mIsVisible;
}

void setMugenTextVisibility(int tID, int tIsVisible)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mIsVisible = tIsVisible;
}

double getMugenTextSizeX(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	return getMugenTextSizeXInternal(e);
}

void setMugenTextScale(int tID, double tScale)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	Position p = getMugenTextPosition(tID);
	e->mScale = tScale;
	setMugenTextPosition(tID, p);
}

const char* getMugenTextText(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	return e->mText;
}

const char* getMugenTextDisplayedText(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	return e->mDisplayText;
}

void changeMugenText(int tID, const char * tText)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	MugenTextAlignment alignment = e->mAlignment;
	setMugenTextAlignment(tID, MUGEN_TEXT_ALIGNMENT_LEFT);
	strcpy(e->mText, tText);
	strcpy(e->mDisplayText, tText);
	setMugenTextAlignment(tID, alignment);
}

Position getMugenTextPosition(int tID) {
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	
	MugenTextAlignment alignment = e->mAlignment;
	setMugenTextAlignment(tID, MUGEN_TEXT_ALIGNMENT_LEFT);
	Position ret = e->mPosition;
	ret = vecAdd(ret, Vector3D(0, e->mFont->mSize.y * e->mScale, 0));
	setMugenTextAlignment(tID, alignment);
	return ret;
}

Position * getMugenTextPositionReference(int tID)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	return &e->mPosition;
}

Vector3D getMugenTextColor(int tIndex) {
	MugenText* e = &gMugenTextHandler.mHandledTexts[tIndex];
	return Vector3D(e->mR, e->mG, e->mB);
}

MugenTextAlignment getMugenTextAlignmentFromMugenAlignmentIndex(int tIndex)
{
	if (tIndex > 0) return MUGEN_TEXT_ALIGNMENT_LEFT;
	else if (tIndex < 0) return MUGEN_TEXT_ALIGNMENT_RIGHT;
	else return MUGEN_TEXT_ALIGNMENT_CENTER;
}

Color getMugenTextColorFromMugenTextColorIndex(int tIndex)
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
