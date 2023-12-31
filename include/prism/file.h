#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string>

#ifndef SEEK_SET
#define 	SEEK_SET   0
#define 	SEEK_CUR   1
#define 	SEEK_END   2
#endif

#ifdef DREAMCAST

typedef int FileHandler;

#elif defined _WIN32 || defined __EMSCRIPTEN__ || defined VITA

#define O_RDONLY	0x1
#define O_WRONLY	0x2

#define FILEHND_INVALID	0

typedef FILE* FileHandler;

#endif

typedef struct {
  int mIsOwned;
  uint32_t mLength;
  void* mData;
} Buffer;

typedef char* BufferPointer;

FileHandler fileOpen(const char* tPath, int tFlags);
int fileClose(FileHandler tHandler);
size_t fileRead(FileHandler tHandler, void* tBuffer, size_t tCount);
size_t fileWrite(FileHandler tHandler, const void* tBuffer, size_t tCount);
size_t fileSeek(FileHandler tHandler, size_t tOffset, int tWhence);
size_t fileTell(FileHandler tHandler);
size_t fileTotal(FileHandler tHandler);
void fileFlush(FileHandler tHandler);
int fileUnlink(const char* tPath);
void* fileMemoryMap(FileHandler tHandler);

int isFile(const char* tPath);
int isFile(const std::string& tPath);
int isDirectory(const char* tPath);

void createDirectory(const char* tPath);

Buffer makeBuffer(void* tData, uint32_t tLength);
Buffer makeBufferOwned(void* tData, uint32_t tLength);
Buffer makeBufferEmptyOwned();
Buffer copyBuffer(const Buffer& tBuffer);
Buffer makeBufferOwnedIfNecessary(const Buffer& tBuffer);
Buffer fileToBuffer(const char* path);
Buffer copyStringToBuffer(const std::string& tString);
void bufferToFile(const char* tPath, const Buffer& tBuffer);
void freeBuffer(Buffer& buffer);
void appendTerminationSymbolToBuffer(Buffer* tBuffer);
void fileToMemory(void* tDst, int tSize, const char* tPath);
BufferPointer getBufferPointer(const Buffer& tBuffer);
void readFromBufferPointer(void* tDst, BufferPointer* tPointer, uint32_t tSize);
int readIntegerFromTextStreamBufferPointer(BufferPointer* tPointer);
double readFloatFromTextStreamBufferPointer(BufferPointer* tPointer);
int hasStringFromTextStreamBufferPointer(BufferPointer tPointer);
std::string readStringFromTextStreamBufferPointer(BufferPointer* tPointer);
std::string readLineFromTextStreamBufferPointer(BufferPointer* tPointer);
std::string readLineOrEOFFromTextStreamBufferPointer(BufferPointer* tPointer, const Buffer& tBuffer);
int isBufferPointerOver(BufferPointer tPointer, const Buffer& tBuffer);

void appendBufferChar(Buffer* tBuffer, char tChar);
void appendBufferUint32(Buffer* tBuffer, uint32_t tInteger);
void appendBufferInt32(Buffer* tBuffer, int32_t tInteger);
void appendBufferInteger(Buffer* tBuffer, int tInteger);
void appendBufferFloat(Buffer* tBuffer, float tFloat);
void appendBufferString(Buffer* tBuffer, const char* tString, int tLength);
void appendBufferBuffer(Buffer* tBuffer, const Buffer& tInputBuffer);

void setActiveFileSystemOnStartup();
void initFileSystem();
void shutdownFileSystem();
void setFileSystem(const char* path);
void setWorkingDirectory(const char* path);
const char* getFileSystem();
const char* getWorkingDirectory();

void mountRomdiskFromBuffer(const Buffer& b, const char* tMountPath);
void mountRomdisk(const char* tFilePath, const char* tMountPath);
void unmountRomdisk(const char* tMountPath);

const char* getPureFileName(const char* path);
int hasFileExtension(const char* tPath);
const char* getFileExtension(const char* tPath);
char* getFileExtension(char* tPath);
size_t getFileSize(const char* tPath);
void getPathWithoutFileExtension(char* tDest, const char* tPath);
void  getPathWithNumberAffixedFromAssetPath(char* tDest, const char* tSrc, int i);
void cleanPathSlashes(char* tDest, const char* tPath);
void cleanPathSlashes(char* tPath);
void cleanPathSlashes(std::string& tPath);
void getFullPath(char* tDest, const char* tPath);
void getPathToFile(char* tDest, const char* tPath);
void getPathToFile(std::string& oDest, const char* tPath);

void printDirectory(const char* tPath);

std::string sanitizeFileNameWithInvalidCharacters(const std::string& tPathWithInvalidCharacters);
int isDebugMinusCheckEnabled();
void setDebugMinusCheckEnabled(int tIsEnabled);

#ifdef _WIN32
void imguiFileGeneral();
void imguiFileHardware();
void imguiBuffer(const std::string_view& tName, const Buffer& tBuffer);
#endif