
#include <assert.h>
#include <Windows.h>

#include <prism/mugenspritefilereader.h>
#include <prism/compression.h>
#include <prism/wrapper.h>
#include <prism/file.h>
#include <prism/texture.h>

int romdisk_buffer_length = 0;
char romdisk_buffer[2];


typedef struct {
	Buffer mBuffer;

	int group;
	int sprite;
} StoredTexture;

static struct {
	IntMap mStoredTextures;

	uint64_t mBlockAmountPosition;
	uint64_t mBlockHeaderPosition;

	int mBlockAmount;
	int mSpriteAmountInBlock;
} gData;

static TextureData loadTextureARGB16(Buffer b, int w, int h) {
	TextureData ret;
	ret.mHasPalette = 0;
	ret.mTextureSize.x = w;
	ret.mTextureSize.y = h;

	StoredTexture* e = (StoredTexture*)allocMemory(sizeof(StoredTexture));
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

	StoredTexture* e = (StoredTexture*)allocMemory(sizeof(StoredTexture));
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
	Buffer* b = (Buffer*)tCaller;
	Buffer* palette = (Buffer*)tData;

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
	TraversalCaller* caller = (TraversalCaller*)tCaller;
	MugenSpriteFileSubSprite* subsprite = (MugenSpriteFileSubSprite*)tData;

	StoredTexture* texture = (StoredTexture*)subsprite->mTexture.mTexture;
	texture->group = caller->group;
	texture->sprite = caller->sprite;
}



static void traverseSprite(void* tCaller, char* tKey, void* tData) {
	TraversalCaller* caller = (TraversalCaller*)tCaller;
	MugenSpriteFileSprite* sprite = (MugenSpriteFileSprite*)tData;

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
	MugenSpriteFileGroup* group = (MugenSpriteFileGroup*)tData;

	int groupNumber = atoi(tKey);
	
	TraversalCaller caller;
	caller.group = groupNumber;
	string_map_map(&group->mSprites, traverseSprite, &caller);
}

static void traverseSpritesAndAnnotateIndices(MugenSpriteFile* tSprites) {

	string_map_map(&tSprites->mGroups, traverseGroup, NULL);
}

static void annotateSpriteSubSprite(void* tCaller, void* tData) {
	TraversalCaller* caller = (TraversalCaller*)tCaller;
	MugenSpriteFileSubSprite* subsprite = (MugenSpriteFileSubSprite*)tData;

	StoredTexture* texture = (StoredTexture*)subsprite->mTexture.mTexture;
	caller->group = texture->group;
	caller->sprite = texture->sprite;
}

static void writeSpriteSubSprite(void* tCaller, void* tData) {
	TraversalCaller* caller = (TraversalCaller*)tCaller;
	MugenSpriteFileSubSprite* subsprite = (MugenSpriteFileSubSprite*)tData;

	appendBufferInt32(caller->b, subsprite->mOffset.x);
	appendBufferInt32(caller->b, subsprite->mOffset.y);
	appendBufferInt32(caller->b, subsprite->mOffset.z);

	appendBufferInt32(caller->b, subsprite->mTexture.mHasPalette);
	appendBufferInt32(caller->b, subsprite->mTexture.mTextureSize.x);
	appendBufferInt32(caller->b, subsprite->mTexture.mTextureSize.y);

	StoredTexture* texture = (StoredTexture*)subsprite->mTexture.mTexture;
	Buffer* rawBuffer = &texture->mBuffer;
	Buffer dataBuffer = *rawBuffer;
	if (subsprite->mTexture.mHasPalette) {
		compressBufferZSTD(&dataBuffer);
		appendBufferUint32(caller->b, dataBuffer.mLength);
		appendBufferBufferPadded(caller->b, dataBuffer);
	}
	else {
		Buffer twiddledBuffer = twiddleTextureBuffer16(dataBuffer, subsprite->mTexture.mTextureSize.x, subsprite->mTexture.mTextureSize.y);
		compressBufferZSTD(&twiddledBuffer);
		appendBufferUint32(caller->b, twiddledBuffer.mLength);
		appendBufferBufferPadded(caller->b, twiddledBuffer);
	}

	caller->group = texture->group;
	caller->sprite = texture->sprite;
}

static void appendPreloadedBlock(Buffer* b) {
	uint64_t blockSize = (b->mLength - gData.mBlockHeaderPosition) - 8;
	uint32_t* blockHeader = (uint32_t*)&(((char*)b->mData)[gData.mBlockHeaderPosition]);
	*blockHeader = gData.mSpriteAmountInBlock;
	blockHeader++;
	*blockHeader = (uint32_t)blockSize;

	gData.mBlockAmount++;
	gData.mBlockHeaderPosition = b->mLength;
	gData.mSpriteAmountInBlock = 0;
	appendBufferUint32(b, (uint32_t)0);
	appendBufferUint32(b, (uint32_t)0);
}

static void updatePreloadedBlock(Buffer* b) {
	uint64_t blockSize = (b->mLength - gData.mBlockHeaderPosition) - 8;

	if (blockSize < 1024 * 200) return;

	appendPreloadedBlock(b);
}

static void writeSprite(void* tCaller, void* tData) {
	Buffer* b = (Buffer*)tCaller;
	MugenSpriteFileSprite* sprite = (MugenSpriteFileSprite*)tData;

	appendBufferInt32(b, sprite->mIsLinked);
	appendBufferInt32(b, sprite->mIsLinkedTo);
	appendBufferFloat(b, (float)sprite->mAxisOffset.x);
	appendBufferFloat(b, (float)sprite->mAxisOffset.y);
	appendBufferFloat(b, (float)sprite->mAxisOffset.z);

	if (sprite->mIsLinked) {
		appendBufferInt32(b, sprite->mOriginalTextureSize.x);
		appendBufferInt32(b, sprite->mOriginalTextureSize.y);

		appendBufferInt32(b, sprite->mOriginalTextureSize.x);
		appendBufferInt32(b, sprite->mOriginalTextureSize.y);

		appendBufferUint32(b, 0);
		gData.mSpriteAmountInBlock++;
		return;
	}



	TraversalCaller caller;
	caller.b = b;
	caller.group = -1;
	caller.sprite = -1;
	list_map(&sprite->mTextures, annotateSpriteSubSprite, &caller);

	appendBufferInt32(b, caller.group);
	appendBufferInt32(b, caller.sprite);

	appendBufferInt32(b, sprite->mOriginalTextureSize.x);
	appendBufferInt32(b, sprite->mOriginalTextureSize.y);

	appendBufferUint32(b, (uint32_t)list_size(&sprite->mTextures));

	list_map(&sprite->mTextures, writeSpriteSubSprite, &caller);

	gData.mSpriteAmountInBlock++;
	updatePreloadedBlock(b);
}

static void writeSprites(Buffer* b, MugenSpriteFile* tSprites) {
	
	gData.mBlockAmountPosition = b->mLength;
	gData.mBlockAmount = 0;
	appendBufferUint32(b, (uint32_t)0);

	gData.mBlockHeaderPosition = b->mLength;
	gData.mSpriteAmountInBlock = 0;
	appendBufferUint32(b, (uint32_t)0);
	appendBufferUint32(b, (uint32_t)0);

	vector_map(&tSprites->mAllSprites, writeSprite, b);

	if (gData.mSpriteAmountInBlock) {
		appendPreloadedBlock(b);
	}

	b->mLength -= 8;

	uint32_t* blockAmount = (uint32_t*)&(((char*)b->mData)[gData.mBlockAmountPosition]);
	*blockAmount = gData.mBlockAmount;
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

static void PathCombine(WCHAR* tDst, WCHAR* tFolder, WCHAR* tFile) {
	wsprintf(tDst, L"%s\\%s", tFolder, tFile);
}

static void parseSingleSFF(LPCTSTR tPath, int tIsCharacter) {
	char path[MAX_PATH];
	sprintf(path, "%ws", tPath);
	printf("convert %s\n", path);
	if (tIsCharacter) {
		convertPlayerSFF(path);
	}
	else {
		convertNoPlayerSFF(path);
	}
}

static void ConvertSFFRecursively(LPCTSTR lpFolder, LPCTSTR lpFilePattern, int tIsCharacter)
{
	TCHAR szFullPattern[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile;
	// first we are going to process any subdirectories
	PathCombine(szFullPattern, (WCHAR*)lpFolder, L"*");
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!lstrcmpW(L".", FindFileData.cFileName)) continue;
				if (!lstrcmpW(L"..", FindFileData.cFileName)) continue;
				// found a subdirectory; recurse into it
				PathCombine(szFullPattern, (WCHAR*)lpFolder, FindFileData.cFileName);
				
				if (!lstrcmpW(L"story", FindFileData.cFileName)) continue;

				int newIsCharacter = tIsCharacter | !lstrcmpW(L"chars", FindFileData.cFileName);
				ConvertSFFRecursively(szFullPattern, lpFilePattern, newIsCharacter);
			}
		} while (FindNextFile(hFindFile, &FindFileData));
		FindClose(hFindFile);
	}

	// Now we are going to look for the matching files
	PathCombine(szFullPattern, (WCHAR*)lpFolder, (WCHAR*)lpFilePattern);
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// found a file; do something with it
				PathCombine(szFullPattern, (WCHAR*)lpFolder, FindFileData.cFileName);
				parseSingleSFF(szFullPattern, tIsCharacter);
			}
		} while (FindNextFile(hFindFile, &FindFileData));
		FindClose(hFindFile);
	}
}

static void DeletePreloadedRecursively(LPCTSTR lpFolder, LPCTSTR lpFilePattern)
{
	TCHAR szFullPattern[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile;
	// first we are going to process any subdirectories
	PathCombine(szFullPattern, (WCHAR*)lpFolder, L"*");
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!lstrcmpW(L".", FindFileData.cFileName)) continue;
				if (!lstrcmpW(L"..", FindFileData.cFileName)) continue;
				// found a subdirectory; recurse into it
				PathCombine(szFullPattern, (WCHAR*)lpFolder, FindFileData.cFileName);

				DeletePreloadedRecursively(szFullPattern, lpFilePattern);
			}
		} while (FindNextFile(hFindFile, &FindFileData));
		FindClose(hFindFile);
	}

	// Now we are going to look for the matching files
	PathCombine(szFullPattern, (WCHAR*)lpFolder, (WCHAR*)lpFilePattern);
	hFindFile = FindFirstFile(szFullPattern, &FindFileData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// found a file; do something with it
				PathCombine(szFullPattern, (WCHAR*)lpFolder, FindFileData.cFileName);
				char command[MAX_PATH];
				sprintf(command, "del %ws", szFullPattern);
				system(command);
			}
		} while (FindNextFile(hFindFile, &FindFileData));
		FindClose(hFindFile);
	}
}

int main(int argc, char* argv[])
{
	initPrismWrapperWithMugenFlags();

	//DeletePreloadedRecursively(L"C:\\Users\\Legion\\Desktop\\DEV\\MICROSOFT\\WINDOWS\\LIBTARIPORT\\DolmexicaInfinite\\assets", L"*.preloaded");
	//DeletePreloadedRecursively(L"C:\\Users\\Legion\\Desktop\\DEV\\MICROSOFT\\WINDOWS\\LIBTARIPORT\\DolmexicaInfinite\\windows\\debug", L"*.preloaded");
	ConvertSFFRecursively(L"C:\\Users\\Legion\\Desktop\\DEV\\MICROSOFT\\WINDOWS\\LIBTARIPORT\\DolmexicaInfinite\\assets", L"*.sff", 0);
	return 0;
}


