#include "prism/filereader.h"

#include "prism/memoryhandler.h"

typedef struct {
	Buffer b;
	BufferPointer p;

	int mIsOver;
} MugenSpriteFileBufferReaderData;

static void initBufferReader(FileReader* tReader, const char* tPath) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)allocMemory(sizeof(MugenSpriteFileBufferReaderData));
	data->b = fileToBuffer(tPath);
	data->p = getBufferPointer(data->b);
	data->mIsOver = 0;
	tReader->mData = data;
}

static void deleteBufferReader(FileReader* tReader) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	freeBuffer(data->b);
	freeMemory(data);
}

static void readBufferReader(FileReader* tReader, void* tDst, uint32_t tSize) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	readFromBufferPointer(tDst, &data->p, tSize);
}

static void seekBufferReader(FileReader* tReader, uint32_t tPosition) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	data->p = ((char*)data->b.mData) + tPosition;

	if (data->p >= ((char*)data->b.mData) + data->b.mLength) {
		tReader->mSetOver(tReader);
	}
}

static Buffer readBufferReaderBuffer(FileReader* tReader, uint32_t tSize) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	return makeBuffer(data->p, tSize);
}

static uint32_t getCurrentBufferReaderOffset(FileReader* tReader) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	return (uint32_t)(data->p - ((char*)data->b.mData));
}

static void setBufferReaderOver(FileReader* tReader) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	data->p = NULL;
	data->mIsOver = 1;
}

static int isBufferReaderOver(FileReader* tReader) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)tReader->mData;
	return data->mIsOver;
}

FileReader getBufferFileReader() {
	FileReader ret;
	ret.mType = FILE_READER_TYPE_BUFFER;
	ret.mInit = initBufferReader;
	ret.mRead = readBufferReader;
	ret.mReadBufferReadOnly = readBufferReaderBuffer;
	ret.mSeek = seekBufferReader;
	ret.mGetCurrentOffset = getCurrentBufferReaderOffset;
	ret.mDelete = deleteBufferReader;
	ret.mSetOver = setBufferReaderOver;
	ret.mIsOver = isBufferReaderOver;
	return ret;
}

typedef struct {
	FileHandler mFile;
	uint32_t mFileSize;
	int mIsOver;
} MugenSpriteFileFileReaderData;

static void initFileReader(FileReader* tReader, const char* tPath) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)allocMemory(sizeof(MugenSpriteFileFileReaderData));
	data->mFile = fileOpen(tPath, O_RDONLY);
	data->mFileSize = fileTotal(data->mFile);
	data->mIsOver = 0;
	tReader->mData = data;
}

static void deleteFileReader(FileReader* tReader) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)tReader->mData;
	fileClose(data->mFile);
	freeMemory(data);
}

static void readFileReader(FileReader* tReader, void* tDst, uint32_t tSize) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)tReader->mData;

	fileRead(data->mFile, tDst, tSize);
}

static void seekFileReader(FileReader* tReader, uint32_t tPosition) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)tReader->mData;

	fileSeek(data->mFile, tPosition, SEEK_SET);
	uint32_t position = tReader->mGetCurrentOffset(tReader);

	if (position >= data->mFileSize) {
		tReader->mSetOver(tReader);
	}
}

static Buffer readFileReaderBuffer(FileReader* tReader, uint32_t tSize) {
	(void)tReader;
	uint32_t originalPosition = tReader->mGetCurrentOffset(tReader);
	char* dst = (char*)allocMemory(tSize + 10);
	tReader->mRead(tReader, dst, tSize);

	tReader->mSeek(tReader, originalPosition);

	return makeBufferOwned(dst, tSize);
}

static uint32_t getCurrentFileReaderOffset(FileReader* tReader) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)tReader->mData;

	return (uint32_t)fileTell(data->mFile);
}

static void setFileReaderOver(FileReader* tReader) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)tReader->mData;
	data->mIsOver = 1;
}

static int isFileReaderOver(FileReader* tReader) {
	MugenSpriteFileFileReaderData* data = (MugenSpriteFileFileReaderData*)tReader->mData;
	return data->mIsOver;
}

FileReader getFileFileReader() {
	FileReader ret;
	ret.mType = FILE_READER_TYPE_FILE;
	ret.mInit = initFileReader;
	ret.mRead = readFileReader;
	ret.mReadBufferReadOnly = readFileReaderBuffer;
	ret.mSeek = seekFileReader;
	ret.mGetCurrentOffset = getCurrentFileReaderOffset;
	ret.mDelete = deleteFileReader;
	ret.mSetOver = setFileReaderOver;
	ret.mIsOver = isFileReaderOver;
	return ret;
}

void initBufferFileReaderReadOnlyBuffer(FileReader* tReader, Buffer tBuffer) {
	MugenSpriteFileBufferReaderData* data = (MugenSpriteFileBufferReaderData*)allocMemory(sizeof(MugenSpriteFileBufferReaderData));
	data->b = tBuffer;
	data->p = getBufferPointer(data->b);
	data->mIsOver = 0;
	tReader->mData = data;
}