#include "prism/saveload.h"

#include <assert.h>

#include "prism/log.h"

void savePrismGameSave(PrismSaveSlot /*tSaveSlot*/, const char* /*tFileName*/, const Buffer& /*tBuffer*/, const char* /*tApplicationName*/, const char* /*tShortDescription*/, const char* /*tLongDescription*/, const Buffer& /*tIconDataBuffer*/, const Buffer& /*tPaletteBuffer*/) {}

Buffer loadPrismGameSave(PrismSaveSlot /*tSaveSlot*/, const char* /*tFileName*/) {
	logError("Loading via prism unimplemented for Win/Emscripten");
	assert(0);
	return makeBuffer(NULL, 0);
}

void deletePrismGameSave(PrismSaveSlot /*tSaveSlot*/, const char* /*tFileName*/) {}
int isPrismSaveSlotActive(PrismSaveSlot /*tSaveSlot*/) { return 0; }
int hasPrismGameSave(PrismSaveSlot /*tSaveSlot*/, const char* /*tFileName*/) { return 0; }
size_t getAvailableSizeForSaveSlot(PrismSaveSlot /*tSaveSlot*/) { return 0; }
size_t getPrismGameSaveSize(const Buffer& /*tBuffer*/, const char* /*tApplicationName*/, const char* /*tShortDescription*/, const char* /*tLongDescription*/, const Buffer& /*tIconDataBuffer*/, const Buffer& /*tPaletteBuffer*/) { return 0; }
void setVMUDisplayIcon(void* /*tBitmap*/, bool) { }
