
#include <assert.h>

#include <prism/mugenspritefilereader.h>
#include <prism/compression.h>
#include <prism/wrapper.h>
#include <prism/file.h>

int romdisk_buffer_length = 0;
char romdisk_buffer[2];


typedef struct {
	Buffer mBuffer;

	int group;
	int sprite;
} StoredTexture;

static struct {
	IntMap mStoredTextures;
} gData;

static TextureData loadTextureARGB16(Buffer b, int w, int h) {
	TextureData ret;
	ret.mHasPalette = 0;
	ret.mTextureSize.x = w;
	ret.mTextureSize.y = h;

	StoredTexture* e = allocMemory(sizeof(StoredTexture));
	e->mBuffer = copyBuffer(b);

	ret.mTexture = (TextureMemory)e;

	return ret;
}

static TextureData loadTextureARGB32(Buffer b, int w, int h) {
	assert(0);
	TextureData ret;
	ret.mHasPalette = 0;
	return ret;
}

static TextureData loadPalettedTexture(Buffer b, int tID, int w, int h) {
	(void)tID;
	TextureData ret;
	ret.mHasPalette = 1;
	ret.mTextureSize.x = w;
	ret.mTextureSize.y = h;

	StoredTexture* e = allocMemory(sizeof(StoredTexture));
	e->mBuffer = copyBuffer(b);

	ret.mTexture = (TextureMemory)e;

	return ret;
}


static void writeSharedHeader(Buffer* b) {
	char signature[12];
	sprintf(signature, "DolmSpr");
	appendBufferString(b, signature, 12);

	char version[4];
	version[0] = 0;
	version[1] = 0;
	version[2] = 0;
	version[3] = 100;
	appendBufferString(b, version, 4);
}

static void appendBufferBufferPadded(Buffer* b, Buffer otherBuffer) {
	appendBufferBuffer(b, otherBuffer);

	int pad = (4 - (otherBuffer.mLength % 4)) % 4;

	char padString[4];
	padString[0] = padString[1] = padString[2] = padString[3] = 0;
	appendBufferString(b, padString, pad);
}

static void writeSinglePalette(void* tCaller, void* tData) {
	Buffer* b = tCaller;
	Buffer* palette = tData;

	appendBufferUint32(b, palette->mLength);
	appendBufferBufferPadded(b, *palette);
}

static void writePalettes(Buffer* b, MugenSpriteFile* tSprites) {

	appendBufferUint32(b, (uint32_t)vector_size(&tSprites->mPalettes));
	vector_map(&tSprites->mPalettes, writeSinglePalette, b);
}

typedef struct {
	int group;
	int sprite;
	Buffer* b;
} TraversalCaller;

static void traverseSpriteSubSprite(void* tCaller, void* tData) {
	TraversalCaller* caller = tCaller;
	MugenSpriteFileSubSprite* subsprite = tData;

	StoredTexture* texture = (StoredTexture*)subsprite->mTexture.mTexture;
	texture->group = caller->group;
	texture->sprite = caller->sprite;
}



static void traverseSprite(void* tCaller, char* tKey, void* tData) {
	TraversalCaller* caller = tCaller;
	MugenSpriteFileSprite* sprite = tData;

	int spriteNumber = atoi(tKey);
	caller->sprite = spriteNumber;

	if (sprite->mIsLinked) {
		sprite->mOriginalTextureSize.x = caller->group;
		sprite->mOriginalTextureSize.y = caller->sprite;
		return;
	}

	list_map(&sprite->mTextures, traverseSpriteSubSprite, caller);
}

static void traverseGroup(void* tCaller, char* tKey, void* tData) {
	MugenSpriteFileGroup* group = tData;

	int groupNumber = atoi(tKey);
	
	TraversalCaller caller;
	caller.group = groupNumber;
	string_map_map(&group->mSprites, traverseSprite, &caller);
}

static void traverseSpritesAndAnnotateIndices(MugenSpriteFile* tSprites) {

	string_map_map(&tSprites->mGroups, traverseGroup, NULL);
}

static void writeSpriteSubSprite(void* tCaller, void* tData) {
	TraversalCaller* caller = tCaller;
	MugenSpriteFileSubSprite* subsprite = tData;

	appendBufferInteger(caller->b, subsprite->mOffset.x);
	appendBufferInteger(caller->b, subsprite->mOffset.y);
	appendBufferInteger(caller->b, subsprite->mOffset.z);

	appendBufferInteger(caller->b, subsprite->mTexture.mHasPalette);
	appendBufferInteger(caller->b, subsprite->mTexture.mTextureSize.x);
	appendBufferInteger(caller->b, subsprite->mTexture.mTextureSize.y);

	StoredTexture* texture = (StoredTexture*)subsprite->mTexture.mTexture;
	Buffer* rawBuffer = &texture->mBuffer;
	Buffer dataBuffer =  *rawBuffer;
	compressBuffer(&dataBuffer);
	appendBufferUint32(caller->b, dataBuffer.mLength);
	appendBufferBufferPadded(caller->b, dataBuffer);

	caller->group = texture->group;
	caller->sprite = texture->sprite;
}

static void writeSprite(void* tCaller, void* tData) {
	Buffer* b = tCaller;
	MugenSpriteFileSprite* sprite = tData;

	appendBufferInteger(b, sprite->mIsLinked);
	appendBufferInteger(b, sprite->mIsLinkedTo);
	appendBufferFloat(b, sprite->mAxisOffset.x);
	appendBufferFloat(b, sprite->mAxisOffset.y);
	appendBufferFloat(b, sprite->mAxisOffset.z);

	if (sprite->mIsLinked) {
		appendBufferInteger(b, sprite->mOriginalTextureSize.x);
		appendBufferInteger(b, sprite->mOriginalTextureSize.y);
		return;
	}
	appendBufferInteger(b, sprite->mOriginalTextureSize.x);
	appendBufferInteger(b, sprite->mOriginalTextureSize.y);

	appendBufferUint32(b, (uint32_t)list_size(&sprite->mTextures));

	TraversalCaller caller;
	caller.b = b;
	caller.group = -1;
	caller.sprite = -1;
	list_map(&sprite->mTextures, writeSpriteSubSprite, &caller);

	appendBufferInteger(b, caller.group);
	appendBufferInteger(b, caller.sprite);
}

static void writeSprites(Buffer* b, MugenSpriteFile* tSprites) {
	appendBufferUint32(b, (uint32_t)vector_size(&tSprites->mAllSprites));

	vector_map(&tSprites->mAllSprites, writeSprite, b);
}

static Buffer convertMugenSpriteFileToPreloaded(MugenSpriteFile* tSprites) {
	Buffer b = makeBufferEmptyOwned();
	writeSharedHeader(&b);
	writePalettes(&b, tSprites);
	traverseSpritesAndAnnotateIndices(tSprites);
	writeSprites(&b, tSprites);

	return b;
}

static void convertPlayerSFF(char* tPath) {
	pushMemoryStack();

	char outputPath[1024];
	sprintf(outputPath, "%s.preloaded", tPath);

	gData.mStoredTextures = new_int_map();
	setMugenSpriteFileReaderCustomFunctionsAndForceARGB16(loadTextureARGB16, loadTextureARGB32, loadPalettedTexture);
	setMugenSpriteFileReaderToUsePalette(-1);
	MugenSpriteFile sprites = loadMugenSpriteFile(tPath, 1, 1, "dummy.act");
	setMugenSpriteFileReaderToNotUsePalette();
	Buffer b = convertMugenSpriteFileToPreloaded(&sprites);
	bufferToFile(outputPath, b);

	popMemoryStack();
}

static void convertNoPlayerSFF(char* tPath) {
	pushMemoryStack();

	char outputPath[1024];
	sprintf(outputPath, "%s.preloaded", tPath);

	gData.mStoredTextures = new_int_map();
	setMugenSpriteFileReaderCustomFunctionsAndForceARGB16(loadTextureARGB16, loadTextureARGB32, loadPalettedTexture);
	setMugenSpriteFileReaderToNotUsePalette();
	MugenSpriteFile sprites = loadMugenSpriteFileWithoutPalette(tPath);
	Buffer b = convertMugenSpriteFileToPreloaded(&sprites);
	bufferToFile(outputPath, b);

	popMemoryStack();
}


int main(int argc, char* argv[])
{
	initPrismWrapperWithMugenFlags();

	convertPlayerSFF("kfm.sff");

	return 0;
}


