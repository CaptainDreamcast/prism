#include "prism/memoryhandler.h"

#include "prism/math.h"
#include "prism/log.h"

#include <vita2d.h>

namespace prism {

	int getAvailableTextureMemory() {
		return INF;
		//static const int SAFETY_BUFFER = 5000000;
		//return vglMemFree(VGL_MEM_VRAM) - SAFETY_BUFFER; // might make sense to include proper texture mem check in future
	}

	int getAvailableSoundMemory() {
		return INF;
	}

	// borrowed from the vita2d utils
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))
	static unsigned int getAlignedSize(SceKernelMemBlockType type, unsigned int size)
	{
		switch (type) {
		case SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW:
			return ALIGN(size, 256 * 1024);
		case SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW:
		case SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW:
			return ALIGN(size, 1024 * 1024);
		default:
			return ALIGN(size, 4 * 1024);
		}
	}

	// borrowed from the vita2d utils
	void* vitaGpuAlloc(SceKernelMemBlockType type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID* uid)
	{
		void* mem;
		unsigned int aligned_size = getAlignedSize(type, size);

		*uid = sceKernelAllocMemBlock("gpu_mem", type, aligned_size, NULL);

		// Fallbacking to other mem types if out of mem
		if (*uid < 0) {
			type = type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW ? SCE_KERNEL_MEMBLOCK_TYPE_USER_RW : SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW;
			aligned_size = getAlignedSize(type, size);
			*uid = sceKernelAllocMemBlock("gpu_mem", type, aligned_size, NULL);
			if (*uid < 0) {
				type = SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW;
				aligned_size = getAlignedSize(type, size);
				*uid = sceKernelAllocMemBlock("gpu_mem", type, aligned_size, NULL);
				if (*uid < 0)
					return NULL;
			}
		}

		if (sceKernelGetMemBlockBase(*uid, &mem) < 0)
			return NULL;

		if (sceGxmMapMemory(mem, aligned_size, SceGxmMemoryAttribFlags(attribs)) < 0)
			return NULL;

		return mem;
	}

	// borrowed from the vita2d utils
	void vitaGpuFree(SceUID uid)
	{
		void* mem = NULL;
		if (sceKernelGetMemBlockBase(uid, &mem) < 0)
			return;
		sceGxmUnmapMemory(mem);
		sceKernelFreeMemBlock(uid);
	}

}