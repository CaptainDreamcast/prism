#pragma once

#include "tari/file.h"

fup void initRomdisks();
fup FileHandler fileOpenRomdisk(char* tPath, int tFlags);
fup int fileCloseRomdisk(FileHandler tHandler);
fup size_t fileReadRomdisk(FileHandler tHandler, void* tBuffer, size_t tCount);
fup size_t fileSeekRomdisk(FileHandler tHandler, size_t tOffset, int tWhence);
fup size_t fileTellRomdisk(FileHandler tHandler);
fup size_t fileTotalRomdisk(FileHandler tHandler);

fup void mountRomdiskWindows(char* tFilePath, char* tMountPath);
fup void unmountRomdiskWindows(char* tMountPath);
fup int isRomdiskPath(char* tPath);
fup int isRomdiskFileHandler(FileHandler tHandler);