#pragma once

#include "prism/file.h"

void initRomdisks();
void shutdownRomdisks();
FileHandler fileOpenRomdisk(char* tPath, int tFlags);
int fileCloseRomdisk(FileHandler tHandler);
size_t fileReadRomdisk(FileHandler tHandler, void* tBuffer, size_t tCount);
size_t fileSeekRomdisk(FileHandler tHandler, size_t tOffset, int tWhence);
size_t fileTellRomdisk(FileHandler tHandler);
size_t fileTotalRomdisk(FileHandler tHandler);

void mountRomdiskWindowsFromBuffer(Buffer b, const char* tMountPath);
void mountRomdiskWindows(const char* tFilePath, const char* tMountPath);
void unmountRomdiskWindows(const char* tMountPath);
int isRomdiskPath(const char* tPath);
int isRomdiskFileHandler(FileHandler tHandler);