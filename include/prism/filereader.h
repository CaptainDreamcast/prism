#pragma once

#include "file.h"

enum FileReaderType {
	FILE_READER_TYPE_NONE = 0,
	FILE_READER_TYPE_BUFFER = 1,
	FILE_READER_TYPE_FILE = 2,
};

struct FileReader {
	FileReaderType mType;
	void(*mInit)(FileReader* tReader, const char* tPath);
	void(*mRead)(FileReader* tReader, void* tDst, uint32_t tSize);
	Buffer(*mReadBufferReadOnly)(FileReader* tReader, uint32_t tSize);
	void(*mSeek)(FileReader* tReader, uint32_t tPosition);
	uint32_t(*mGetCurrentOffset)(FileReader* tReader);
	void(*mDelete)(FileReader* tReader);
	void(*mSetOver)(FileReader* tReader);
	int(*mIsOver)(FileReader* tReader);
	void* mData;
};

FileReader getBufferFileReader();
FileReader getFileFileReader();

void initBufferFileReaderReadOnlyBuffer(FileReader* tReader, const Buffer& tBuffer);