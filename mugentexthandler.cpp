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
	MUGEN_BITMAP_FONT_BANK_TYPE_PALETTE,

} MugenBitmapFontBankType;

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
	MugenBitmapFontBankType mBankType;
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

	char* path = getAllocatedMugenDefStringVariable(tScript, "Def", "file");
	e->mSprites = loadMugenSpriteFileWithoutPalette(path);
	freeMemory(path);

	// TODO: banktype

	tFont->mData = e;
}

static void loadMugenTruetypeFont(MugenDefScript* tScript, MugenFont* tFont) {
	MugenTruetypeFont* e = (MugenTruetypeFont*)allocMemory(sizeof(MugenTruetypeFont));
	char* name = getAllocatedMugenDefStringVariable(tScript, "Def", "file");
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
		(void)items;
		assert(items == 3);
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
		sscanf(e->mString, "%i", &keyValue);
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

	char* text = getAllocatedMugenDefStringVariable(tScript, "Def", "type");
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

static void loadMugenElecbyteFont(MugenDefScript* tScript, Buffer tTextureBuffer, MugenFont* tFont) {
	MugenElecbyteFont* e = (MugenElecbyteFont*)allocMemory(sizeof(MugenElecbyteFont));

	e->mType = getMugenElecbyteFontType(tScript);

	for (int i = 0; i < 0xFF; i++) e->mMap[i].mExists = 0;

	MugenDefScriptGroup* group = &tScript->mGroups["Map"];

	ElecbyteMapParseCaller caller;
	caller.mFont = tFont;
	caller.mElecbyteFont = e;
	caller.i = 0;
	list_map(&group->mOrderedElementList, parseSingleMapElement, &caller);

	e->mSprite = loadSingleTextureFromPCXBuffer(tTextureBuffer);

	tFont->mData = e;
}

static MugenFontType getMugenFontTypeFromScript(MugenDefScript* tScript) {
	char* text = getAllocatedMugenDefStringVariable(tScript, "Def", "type");
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

static void setMugenFontDirectory(char* tPath) {

	(void)tPath;
	setWorkingDirectory("/"); // TODO
}

static void setMugenFontDirectory2(char* tPath) {
	char path[1024];
	getPathToFile(path, tPath);
	setWorkingDirectory(path);
}

static void resetMugenFontDirectory() {
	setWorkingDirectory("/");
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

static void unloadElecbyteFont(MugenFont* tFont); // TODO: remove

static void addMugenFont1(int tKey, char* tPath) {
	setMugenFontDirectory(tPath);

	MugenFontHeader header;

	Buffer b = fileToBuffer(tPath);
	BufferPointer p = getBufferPointer(b);
	readFromBufferPointer(&header, &p, sizeof(MugenFontHeader));

	Buffer textureBuffer = makeBuffer((void*)(((uint32_t)b.mData) + header.mTextureOffset), header.mTextureLength);
	Buffer textBuffer = makeBuffer((void*)(((uint32_t)b.mData) + header.mTextOffset), header.mTextLength);

	MugenDefScript script; 
	loadMugenDefScriptFromBufferAndFreeBuffer(&script, textBuffer);

	MugenFont& e = gMugenFontData.mFonts[tKey];
	e.mSize = getMugenDefVectorIOrDefault(&script, "Def", "size", makeVector3DI(1, 1, 0));
	e.mSpacing = getMugenDefVectorIOrDefault(&script, "Def", "spacing", makeVector3DI(0, 0, 0));
	e.mOffset = getMugenDefVectorIOrDefault(&script, "Def", "offset", makeVector3DI(0, 0, 0));

	e.mType = MUGEN_FONT_TYPE_ELECBYTE;
	loadMugenElecbyteFont(&script, textureBuffer, &e);

	unloadMugenDefScript(script);

	freeBuffer(b);
	resetMugenFontDirectory();
}

static void addMugenFont2(int tKey, char* tPath) {
	MugenDefScript script; 
	loadMugenDefScript(&script, tPath);

	setMugenFontDirectory2(tPath);

	MugenFont& e = gMugenFontData.mFonts[tKey];
	e.mType = getMugenFontTypeFromScript(&script);

	e.mSize = getMugenDefVectorIOrDefault(&script, "Def", "size", makeVector3DI(1, 1, 0));
	e.mSpacing = getMugenDefVectorIOrDefault(&script, "Def", "spacing", makeVector3DI(0, 0, 0));
	e.mOffset = getMugenDefVectorIOrDefault(&script, "Def", "offset", makeVector3DI(0, 0, 0));

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

	unloadMugenDefScript(script);

	resetMugenFontDirectory();
}

void addMugenFont(int tKey, const char* tPath) {
	char path[1024];

    // TODO: fix when assets is dropped from Dolmexica
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
	
	const char* ending = getFileExtension(path);

	if (!strcmp("def", ending)) {
		addMugenFont2(tKey, path);
	}
	else if (!strcmp("fnt", ending)) {
		addMugenFont1(tKey, path);
	}
	else {
		logError("Unrecognized font file type.");
		logErrorString(tPath);
		recoverFromError();
	}

}

static void loadMugenFonts(MugenDefScript* tScript) {


	addMugenFont(-1, "font/f4x6.fnt");
	
	int i;
	for (i = 0; i < 100; i++) {
		char name[100];
		sprintf(name, "font%d", i);
		if (isMugenDefStringVariable(tScript, "Files", name)) {
			char* path = getAllocatedMugenDefStringVariable(tScript, "Files", name);
			addMugenFont(i, path);
			freeMemory(path);
		}
		else if (i != 0) {
			break;
		}

	}
}


void loadMugenTextHandler()
{
	gMugenFontData.mFonts.clear();
}

void loadMugenSystemFonts() {
	MugenDefScript script; 
	loadMugenDefScript(&script, "assets/data/system.def");
	loadMugenFonts(&script);
	unloadMugenDefScript(script);
}

void loadMugenFightFonts()
{
	MugenDefScript script;
	loadMugenDefScript(&script, "assets/data/fight.def");
	loadMugenFonts(&script);
	unloadMugenDefScript(script);
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

	MugenTextAlignment mAlignment;
	GeoRectangle mRectangle;
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
	gMugenTextHandler.mHandledTexts.clear();
}

static void unloadMugenTextHandlerActor(void* tData) {
	(void)tData;
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
	stl_int_map_map(gMugenTextHandler.mHandledTexts, updateSingleText);
}

typedef struct {
	MugenSpriteFileSprite* mSprite;
	Position mBasePosition;
} BitmapDrawCaller;

// TODO: rectangle + color
static void drawSingleBitmapSubSprite(void* tCaller, void* tData) {
	BitmapDrawCaller* caller = (BitmapDrawCaller*)tCaller;
	MugenSpriteFileSubSprite* subSprite = (MugenSpriteFileSubSprite*)tData;

	double factor = 0.5; // TODO: 640p
	Position p = vecAdd2D(caller->mBasePosition, vecScale(makePosition(subSprite->mOffset.x, subSprite->mOffset.y, subSprite->mOffset.z), factor));
	scaleDrawing(factor, p);
	drawSprite(subSprite->mTexture, p, makeRectangleFromTexture(subSprite->mTexture));
	setDrawingParametersToIdentity();
}

static int hasBitmapTextToLinebreak(char* tText, int tCurrent, Position p, MugenFont* tFont, MugenSpriteFileGroup* tSpriteGroup, double tRightX, double tFactor) {

	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;

	char word[1024];
	int positionsRead;
	sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);

	int i;
	for (i = tCurrent; i < tCurrent+positionsRead; i++) {
		if (int_map_contains(&tSpriteGroup->mSprites, (int)tText[i])) {
			MugenSpriteFileSprite* sprite = (MugenSpriteFileSprite*)int_map_get(&tSpriteGroup->mSprites, (int)tText[i]);
			
			p = vecAdd2D(p, vecScale(makePosition(sprite->mOriginalTextureSize.x, 0, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSpacing.x, 0, 0), tFactor));
		}
		else {
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSize.x, 0, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSpacing.x, 0, 0), tFactor));
		}
	}

	return (p.x > tRightX);
}

static void drawSingleBitmapText(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenBitmapFont* bitmapFont = (MugenBitmapFont*)font->mData;
	int textLength = strlen(e->mDisplayText);
	double factor = 0.5; // TODO: 640p

	MugenSpriteFileGroup* spriteGroup = (MugenSpriteFileGroup*)int_map_get(&bitmapFont->mSprites.mGroups, 0);

	int i;
	Position p = vecAdd2D(e->mPosition, vecScale(makePosition(font->mOffset.x, font->mOffset.y, 0), factor));
	Position start = p;
	double rightX = p.x + e->mTextBoxWidth;
	for (i = 0; i < textLength; i++) {

		if (int_map_contains(&spriteGroup->mSprites, (int)e->mDisplayText[i])) {
			BitmapDrawCaller caller;
			caller.mSprite = (MugenSpriteFileSprite*)int_map_get(&spriteGroup->mSprites, (int)e->mDisplayText[i]);
			caller.mBasePosition = p;
			list_map(&caller.mSprite->mTextures, drawSingleBitmapSubSprite, &caller);

			p = vecAdd2D(p, vecScale(makePosition(caller.mSprite->mOriginalTextureSize.x, 0, 0), factor));
			p = vecAdd2D(p, vecScale(makePosition(font->mSpacing.x, 0, 0), factor));
		}
		else {
			p = vecAdd2D(p, vecScale(makePosition(font->mSize.x, 0, 0), factor));
			p = vecAdd2D(p, vecScale(makePosition(font->mSpacing.x, 0, 0), factor));
		}

		if (hasBitmapTextToLinebreak(e->mText, i, p, font, spriteGroup, rightX, factor)) {
			p.x = start.x;
			p = vecAdd2D(p, vecScale(makePosition(0, font->mSize.y, 0), factor));
			p = vecAdd2D(p, vecScale(makePosition(0, font->mSpacing.y, 0), factor));
		}
	}
}

// TODO: color + rectangle
static void drawSingleTruetypeText(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenTruetypeFont* truetypeFont = (MugenTruetypeFont*)font->mData;

	drawTruetypeText(e->mDisplayText, truetypeFont->mFont, e->mPosition, font->mSize, makePosition(e->mR, e->mG, e->mB), e->mTextBoxWidth);
}

typedef struct {
	MugenElecbyteFontMapEntry* mMapEntry;
	Position mBasePosition;
	MugenText* mText;
	MugenFont* mFont;
} ElecbyteDrawCaller;

static void drawSingleElecbyteSubSprite(void* tCaller, void* tData) {
	ElecbyteDrawCaller* caller = (ElecbyteDrawCaller*)tCaller;
	MugenSpriteFileSubSprite* subSprite = (MugenSpriteFileSubSprite*)tData;

	int minWidth = 0;
	int maxWidth = subSprite->mTexture.mTextureSize.x - 1;
	int leftX = max(minWidth, min(maxWidth, caller->mMapEntry->mStartX - subSprite->mOffset.x));
	int rightX = max(minWidth, min(maxWidth, (caller->mMapEntry->mStartX + caller->mMapEntry->mWidth - 1) - subSprite->mOffset.x));

	if (caller->mMapEntry->mWidth > 1 && leftX == rightX) return;
	if (caller->mMapEntry->mWidth == 1 && (caller->mMapEntry->mStartX < subSprite->mOffset.x || caller->mMapEntry->mStartX >= subSprite->mOffset.x + subSprite->mTexture.mTextureSize.x)) return;
	
	Position p = caller->mBasePosition;

	int minHeight = 0;
	int maxHeight = subSprite->mTexture.mTextureSize.y - 1;
	int upY = max(minHeight, min(maxHeight, (int)(caller->mText->mRectangle.mTopLeft.y - p.y)));
	int downY = max(minHeight, min(maxHeight, upY + (int)(caller->mText->mRectangle.mBottomRight.y - (p.y + caller->mFont->mSize.y))));
	if (upY == downY) return;

	p.y = max(p.y, caller->mText->mRectangle.mTopLeft.y);

	double factor = 1; // TODO: remove

	// scaleDrawing(factor, p); // TODO: remove
	drawSprite(subSprite->mTexture, p, makeRectangle(leftX, upY, rightX - leftX, downY - upY));
	// setDrawingParametersToIdentity();

	caller->mBasePosition.x += (rightX - leftX + 1) * factor;
}

static set<int> getElecbyteTextLinebreaks(char* tText, Position tStart, MugenFont* tFont, MugenElecbyteFont* tElecbyteFont, double tRightX, double tFactor) { // TODO: fix
	
	set<int> ret;
	Position p = tStart;
	int n = strlen(tText);
	for (int i = 0; i < n; i++) {
		if (tElecbyteFont->mMap[(int)tText[i]].mExists) {
			MugenElecbyteFontMapEntry& mapEntry = tElecbyteFont->mMap[(int)tText[i]];
			p = vecAdd2D(p, vecScale(makePosition(mapEntry.mWidth, 0, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSpacing.x, 0, 0), tFactor));
		}
		else {
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSize.x, 0, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSpacing.x, 0, 0), tFactor));
		}

		if (p.x > tRightX) {
			p.x = tStart.x;
			p = vecAdd2D(p, vecScale(makePosition(0, tFont->mSize.y, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(0, tFont->mSpacing.y, 0), tFactor));
			ret.insert(i);
		}
	}

	return ret;
}

static int hasElecbyteTextToLinebreak(char* tText, int tCurrent, Position p, MugenFont* tFont, MugenElecbyteFont* tElecbyteFont, double tRightX, double tFactor) {

	if (tText[0] == ' ') return 0;
	if (tText[0] == '\n') return 1;

	char word[1024];
	int positionsRead;
	sscanf(tText + tCurrent, "%1023s%n", word, &positionsRead);

	int i;
	for (i = tCurrent; i < tCurrent + positionsRead; i++) {
		if (tElecbyteFont->mMap[(int)tText[i]].mExists) {
			MugenElecbyteFontMapEntry& mapEntry = tElecbyteFont->mMap[(int)tText[i]];
			p = vecAdd2D(p, vecScale(makePosition(mapEntry.mWidth, 0, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSpacing.x, 0, 0), tFactor));
		}
		else {
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSize.x, 0, 0), tFactor));
			p = vecAdd2D(p, vecScale(makePosition(tFont->mSpacing.x, 0, 0), tFactor));
		}
	}

	return (p.x > tRightX);
}

static void drawSingleElecbyteText(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenElecbyteFont* elecbyteFont = (MugenElecbyteFont*)font->mData;
	int textLength = strlen(e->mDisplayText);
	double factor = 1; // TODO

	setDrawingBaseColorAdvanced(e->mR, e->mG, e->mB);

	//printf("draw %s\n", e->mText);
	int i;
	Position p = vecAdd2D(e->mPosition, makePosition(font->mOffset.x, font->mOffset.y, 0));
	Position start = p;
	double rightX = p.x + e->mTextBoxWidth;
	set<int> breaks = getElecbyteTextLinebreaks(e->mText, start, font, elecbyteFont, rightX, factor);
	for (i = 0; i < textLength; i++) {

		if (elecbyteFont->mMap[(int)e->mDisplayText[i]].mExists) {
			ElecbyteDrawCaller caller;
			caller.mMapEntry = &elecbyteFont->mMap[(int)e->mDisplayText[i]];
			caller.mBasePosition = p;
			caller.mText = e;
			caller.mFont = font;
			list_map(&elecbyteFont->mSprite->mTextures, drawSingleElecbyteSubSprite, &caller);

			p = vecAdd2D(p, vecScale(makePosition(caller.mMapEntry->mWidth, 0, 0), factor));
			p = vecAdd2D(p, vecScale(makePosition(font->mSpacing.x, 0, 0), factor));
		}
		else {
			p = vecAdd2D(p, vecScale(makePosition(font->mSize.x, 0, 0), factor));
			p = vecAdd2D(p, vecScale(makePosition(font->mSpacing.x, 0, 0), factor));
		}
		if (hasElecbyteTextToLinebreak(e->mText, i, p, font, elecbyteFont, rightX, factor)) {
			p.x = start.x;
			p = vecAdd2D(p, vecScale(makePosition(0, font->mSize.y, 0), factor));
			p = vecAdd2D(p, vecScale(makePosition(0, font->mSpacing.y, 0), factor));
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
	stl_int_map_map(gMugenTextHandler.mHandledTexts, drawSingleText);
}

ActorBlueprint getMugenTextHandler() {
	return makeActorBlueprint(loadMugenTextHandlerActor, unloadMugenTextHandlerActor, updateMugenTextHandler, drawMugenTextHandler);
};

void drawMugenText(char* tText, Position tPosition, int tFont) {
	MugenText textData;
	strcpy(textData.mText, tText);
	strcpy(textData.mDisplayText, tText);
	textData.mPosition = tPosition;
	textData.mFont = &gMugenFontData.mFonts[tFont];
	textData.mR = textData.mG = textData.mB = 1;
	textData.mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	textData.mRectangle = makeGeoRectangle(-INF / 2, -INF / 2, INF, INF);
	textData.mTextBoxWidth = INF;

	textData.mBuildupDurationPerLetter = INF;
	textData.mBuildupNow = 0;

	textData.mIsVisible = 1;
	drawSingleText(NULL, textData);
}

int addMugenText(const char * tText, Position tPosition, int tFont)
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
	e.mPosition = vecSub(tPosition, makePosition(0, e.mFont->mSize.y, 0));
	e.mR = e.mG = e.mB = 1;
	e.mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	e.mRectangle = makeGeoRectangle(-INF / 2, -INF / 2, INF, INF);
	e.mTextBoxWidth = INF;

	e.mBuildupDurationPerLetter = INF;
	e.mBuildupNow = 0;

	e.mIsVisible = 1;

	return id;
}

int addMugenTextMugenStyle(const char * tText, Position tPosition, Vector3DI tFont)
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

	e->mPosition = vecAdd2D(e->mPosition, makePosition(0, e->mFont->mSize.y, 0));
	if (stl_map_contains(gMugenFontData.mFonts, tFont)) {
		e->mFont = &gMugenFontData.mFonts[tFont];
	}
	else {
		e->mFont = &gMugenFontData.mFonts[1];
	}

	e->mPosition = vecSub(e->mPosition, makePosition(0, e->mFont->mSize.y, 0));
}

static double getBitmapTextSize(MugenText* e) {
	MugenFont* font = e->mFont;
	MugenBitmapFont* bitmapFont = (MugenBitmapFont*)font->mData;
	int textLength = strlen(e->mText);
	double factor = 0.5; // TODO: 640p

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
	double factor = 1; // TODO: 640p

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

void setMugenTextRectangle(int tID, GeoRectangle tRectangle)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mRectangle = tRectangle;
		
}

void setMugenTextPosition(int tID, Position tPosition)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mPosition = tPosition;
	e->mPosition = vecSub(e->mPosition, makePosition(0, e->mFont->mSize.y, 0));
	MugenTextAlignment alignment = e->mAlignment;
	e->mAlignment = MUGEN_TEXT_ALIGNMENT_LEFT;
	setMugenTextAlignment(tID, alignment);
}

void setMugenTextTextBoxWidth(int tID, double tWidth)
{
	MugenText* e = &gMugenTextHandler.mHandledTexts[tID];
	e->mTextBoxWidth = tWidth; // TODO: implement for all types
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
	ret = vecAdd(ret, makePosition(0, e->mFont->mSize.y, 0));
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
	return makePosition(e->mR, e->mG, e->mB);
}

MugenTextAlignment getMugenTextAlignmentFromMugenAlignmentIndex(int tIndex)
{
	if (tIndex > 0) return MUGEN_TEXT_ALIGNMENT_LEFT;
	else if (tIndex < 0) return MUGEN_TEXT_ALIGNMENT_RIGHT;
	else return MUGEN_TEXT_ALIGNMENT_CENTER;
}

Color getMugenTextColorFromMugenTextColorIndex(int tIndex)
{
	// TODO: refactor
	if (tIndex == 0) {
		return COLOR_WHITE;
	}
	else if (tIndex == 1) {
		return COLOR_RED;
	}
	else if (tIndex == 2) {
		return COLOR_GREEN;
	}
	else if (tIndex == 3) {
		return COLOR_BLUE;
	}
	else if (tIndex == 4) {
		return COLOR_CYAN;
	}
	else if (tIndex == 5) {
		return COLOR_YELLOW;
	}
	else if (tIndex == 6) {
		return COLOR_MAGENTA;
	}
	else if (tIndex == 7) {
		return COLOR_BLACK;
	}
	else if (tIndex == 8) {
		return COLOR_LIGHT_GRAY; // TODO: new index(?)
	}
	else {
		logError("Unrecognized Mugen text color.");
		logErrorInteger(tIndex);
		recoverFromError();
		return COLOR_WHITE;
	}
}
