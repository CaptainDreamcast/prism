#include "prism/saveload.h"

#include <kos.h>
#include <assert.h>

#include "prism/log.h"
#include "prism/compression.h"

static const auto MEM_BLOCK_SIZE = 512;

static maple_device_t* getVMUDeviceOrNull(PrismSaveSlot& tSaveSlot) {
	if (tSaveSlot == PrismSaveSlot::AUTOMATIC) {
		for (size_t index = 0; index < size_t(PrismSaveSlot::AMOUNT); index++) {
			PrismSaveSlot tempSlot = PrismSaveSlot(index);
			auto dev = getVMUDeviceOrNull(tempSlot);
			if (!dev) continue;
			tSaveSlot = tempSlot;
			return dev;
		}
		return NULL;
	}
	else {
		const auto index = int(tSaveSlot);
		const auto controllerNumber = index / 2;
		const auto deviceNumber = (index % 2) + 1;
		auto dev = maple_enum_dev(controllerNumber, deviceNumber);
		return (dev && (dev->info.functions & MAPLE_FUNC_MEMCARD)) ? dev : NULL;
	}
}

static vmu_pkg_t packVMUPackageHeader(Buffer& tBuffer, const char* tApplicationName, const char* tShortDescription, const char* tLongDescription, Buffer tIconDataBuffer, Buffer tPaletteBuffer) {
	tBuffer = copyBuffer(tBuffer);
	compressBufferZSTD(&tBuffer);

	assert(tIconDataBuffer.mLength == 512);
	assert(tPaletteBuffer.mLength == 32);

	vmu_pkg_t pkg;
	strcpy(pkg.app_id, tApplicationName);
	strcpy(pkg.desc_short, tShortDescription);
	strcpy(pkg.desc_long, tLongDescription);
	pkg.icon_cnt = 1;
	memcpy(pkg.icon_pal, tPaletteBuffer.mData, tPaletteBuffer.mLength);
	pkg.icon_data = (uint8_t*)tIconDataBuffer.mData;
	pkg.icon_anim_speed = 0;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data_len = tBuffer.mLength;
	pkg.data = (uint8_t*)tBuffer.mData;
	return pkg;
}

static std::string getSaveSlotPath(PrismSaveSlot tSaveSlot) {
	assert(int(tSaveSlot) >= 0 && int(tSaveSlot) < int(PrismSaveSlot::AMOUNT));
	static const char* SAVE_SLOT_PATHS[] = { "a1", "a2", "b1", "b2", "c1", "c2", "d1", "d2" };
	return std::string(SAVE_SLOT_PATHS[int(tSaveSlot)]);
}

void savePrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName, Buffer tBuffer, const char* tApplicationName, const char* tShortDescription, const char* tLongDescription, Buffer tIconDataBuffer, Buffer tPaletteBuffer) {
	auto dev = getVMUDeviceOrNull(tSaveSlot);
	if (!dev) {
		logErrorFormat("Writing VMU file %s to save slot %d failed, no device found. Aborting.\n", tFileName, int(tSaveSlot));
		return;
	}
	
	auto pkg = packVMUPackageHeader(tBuffer, tApplicationName, tShortDescription, tLongDescription, tIconDataBuffer, tPaletteBuffer);

	uint8* pkg_out;
	int pkg_size;
	vmu_pkg_build(&pkg, &pkg_out, &pkg_size);
	
	const auto path = ("$/vmu/" + getSaveSlotPath(tSaveSlot) + "/" + tFileName);
	if (isFile(path)) {
		fileUnlink(path.c_str());
	}
	auto fileHandler = fileOpen(path.c_str(), O_WRONLY);
	if (fileHandler == FILEHND_INVALID) {
		logErrorFormat("Error writing VMU file %s. Aborting.\n", path.c_str());
		free(pkg_out);
		return;
	}
	fileWrite(fileHandler, pkg_out, pkg_size);
	fileClose(fileHandler);

	free(pkg_out);
	freeBuffer(tBuffer);

	logFormat("Successfully wrote save to path %s", path.c_str());
}

Buffer loadPrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName) {
	auto dev = getVMUDeviceOrNull(tSaveSlot);
	if (!dev) {
		logErrorFormat("Error loading VMU file %s from save slot %d. Aborting.\n", tFileName, int(tSaveSlot));
		return makeBuffer(NULL, 0);
	}

	const auto path = ("$/vmu/" + getSaveSlotPath(tSaveSlot) + "/" + tFileName);
	if (!isFile(path)) {
		logErrorFormat("Error loading VMU file %s. Aborting.\n", path.c_str());
		return makeBuffer(NULL, 0);
	}
	
	auto fileHandler = fileOpen(path.c_str(), O_RDONLY);
	if (fileHandler == FILEHND_INVALID) {
		logErrorFormat("Error reading VMU file %s. Aborting.\n", path.c_str());
		return makeBuffer(NULL, 0);
	}
	vmu_pkg_t pkg;
	vmu_pkg_parse((uint8_t*)fileMemoryMap(fileHandler), &pkg);
	auto ret = copyBuffer(makeBuffer((void*)pkg.data, pkg.data_len));
	decompressBufferZSTD(&ret);
	fileClose(fileHandler);

	logFormat("Successfully loaded save from path %s", path.c_str());
	return ret;
}

void deletePrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName) {
	const auto dev = getVMUDeviceOrNull(tSaveSlot);
	if (!dev) {
		return;
	}

	const auto path = ("$/vmu/" + getSaveSlotPath(tSaveSlot) + "/" + tFileName);
	if (isFile(path)) {
		fileUnlink(path.c_str());
	}
}

int isPrismSaveSlotActive(PrismSaveSlot tSaveSlot) {
	const auto dev = getVMUDeviceOrNull(tSaveSlot);
	return dev != NULL;
}

int hasPrismGameSave(PrismSaveSlot tSaveSlot, const char* tFileName) {
	auto dev = getVMUDeviceOrNull(tSaveSlot);
	if (!dev) {
		return 0;
	}

	const auto path = ("$/vmu/" + getSaveSlotPath(tSaveSlot) + "/" + tFileName);
	return isFile(path);
}

size_t getAvailableSizeForSaveSlot(PrismSaveSlot tSaveSlot) {
	auto dev = getVMUDeviceOrNull(tSaveSlot);
	if (!dev) return 0;
	return size_t(vmufs_free_blocks(dev) * MEM_BLOCK_SIZE);
}

size_t getPrismGameSaveSize(Buffer tBuffer, const char* tApplicationName, const char* tShortDescription, const char* tLongDescription, Buffer tIconDataBuffer, Buffer tPaletteBuffer) {
	auto pkg = packVMUPackageHeader(tBuffer, tApplicationName, tShortDescription, tLongDescription, tIconDataBuffer, tPaletteBuffer);

	uint8* pkg_out;
	int pkg_size;
	vmu_pkg_build(&pkg, &pkg_out, &pkg_size);
	free(pkg_out);
	freeBuffer(tBuffer);

	return size_t(((pkg_size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE);
}