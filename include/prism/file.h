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

#elif defined _WIN32 || defined __EMSCRIPTEN__

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
int fileUnlink(char* tPath);
void* fileMemoryMap(FileHandler tHandler);

int isFile(char* tPath);
int isDirectory(char* tPath);

Buffer makeBuffer(void* tData, uint32_t tLength);
Buffer makeBufferOwned(void* tData, uint32_t tLength);
Buffer makeBufferEmptyOwned();
Buffer copyBuffer(Buffer tBuffer);
Buffer fileToBuffer(const char* path);
void bufferToFile(char* tPath, Buffer tBuffer);
void freeBuffer(Buffer buffer);
void appendTerminationSymbolToBuffer(Buffer* tBuffer);
void fileToMemory(void* tDst, int tSize, char* tPath);
BufferPointer getBufferPointer(Buffer tBuffer);
void readFromBufferPointer(void* tDst, BufferPointer* tPointer, uint32_t tSize);
int readIntegerFromTextStreamBufferPointer(BufferPointer* tPointer);
double readFloatFromTextStreamBufferPointer(BufferPointer* tPointer);
std::string readStringFromTextStreamBufferPointer(BufferPointer* tPointer);
std::string readLineFromTextStreamBufferPointer(BufferPointer* tPointer);
std::string readLineOrEOFFromTextStreamBufferPointer(BufferPointer* tPointer, Buffer tBuffer);
int isBufferPointerOver(BufferPointer tPointer, Buffer tBuffer);

void appendBufferChar(Buffer* tBuffer, char tChar);
void appendBufferUint32(Buffer* tBuffer, uint32_t tInteger);
void appendBufferInt32(Buffer* tBuffer, int32_t tInteger);
void appendBufferInteger(Buffer* tBuffer, int tInteger);
void appendBufferFloat(Buffer* tBuffer, float tFloat);
void appendBufferString(Buffer* tBuffer, char* tString, int tLength);
void appendBufferBuffer(Buffer* tBuffer, Buffer tInputBuffer);

void initFileSystem();
void setFileSystem(char* path);
void setWorkingDirectory(char* path);
const char* getFileSystem();
const char* getWorkingDirectory();

void mountRomdiskFromBuffer(Buffer b, char* tMountPath);
void mountRomdisk(char* tFilePath, char* tMountPath);
void unmountRomdisk(char* tMountPath);

char* getPureFileName(char* path);
const char* getFileExtension(const char* tPath);
char* getFileExtension(char* tPath);
void getPathWithoutFileExtension(char* tDest, char* tPath);
void  getPathWithNumberAffixedFromAssetPath(char* tDest, const char* tSrc, int i);
void getFullPath(char* tDest, const char* tPath);
void getPathToFile(char* tDest, char* tPath);

void printDirectory(char* tPath);

