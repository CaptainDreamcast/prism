#pragma once

#include <stdint.h>

#include "file.h"

enum class PrismSaveSlot : int32_t {
	AUTOMATIC = -1,
	A1,
	A2,
	B1,
	B2,
	C1,
	C2,
	D1,
	D2,
	AMOUNT
};

#define DREAMCAST_SAVE_ICON_PALETTE_SIZE	32
#define DREAMCAST_SAVE_ICON_BUFFER_SIZE		512

void savePrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName, const Buffer& tBuffer, const char* tApplicationName, const char* tShortDescription, const char* tLongDescription, const Buffer& tIconDataBuffer, const Buffer& tPaletteBuffer);
Buffer loadPrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName);
void deletePrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName);
int isPrismSaveSlotActive(PrismSaveSlot tSaveSlot);
int hasPrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName);
size_t getAvailableSizeForSaveSlot(PrismSaveSlot tSaveSlot);
size_t getPrismGameSaveSize(const Buffer& tBuffer, const char* tApplicationName, const char* tShortDescription, const char* tLongDescription, const Buffer& tIconDataBuffer, const Buffer& tPaletteBuffer);

void setVMUDisplayIcon(void* tBitmap, bool invertOnTheFly = false);