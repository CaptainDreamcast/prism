// Adapted from https://github.com/philip-wernersbach/memory-pool-allocator

#include "prism/memorypool.h"

#include <string.h>

typedef struct {
	uint32_t size;
	uint32_t allocated;
	char data;
} pmpa_memory_block;



#define PMPA_MEMORY_BLOCK_HEADER_SIZE ( offsetof(pmpa_memory_block, data) )
#define PMPA_FIRST_VALID_ADDRESS_IN_POOL(pool) pool->master_memory_block
#define PMPA_LAST_VALID_ADDRESS_IN_POOL(pool) (PMPA_FIRST_VALID_ADDRESS_IN_POOL(pool) + pool->master_memory_block_size)
#define PMPA_POINTER_IS_IN_RANGE(a, b, c) ( ((a) < ((b) + (c))) && ((a) >=  (b)) )
#define PMPA_POINTER_IS_IN_POOL(pool, a) PMPA_POINTER_IS_IN_RANGE(a, PMPA_FIRST_VALID_ADDRESS_IN_POOL(pool), pool->master_memory_block_size)


typedef struct {
	pmpa_memory_block *master_memory_block;
	uint32_t master_memory_block_size;
} MemoryPool;

/*
* Internal functions.
*/

static void concat_sequential_blocks(MemoryPool* tPool, pmpa_memory_block *memory_block, uint8_t is_allocated)
{
	pmpa_memory_block *current_memory_block = memory_block;
	pmpa_memory_block *next_memory_block = NULL;

	if (current_memory_block->allocated != is_allocated)
		return;

	while ((next_memory_block = current_memory_block + current_memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE) &&
		PMPA_POINTER_IS_IN_POOL(tPool, next_memory_block + PMPA_MEMORY_BLOCK_HEADER_SIZE) &&
		(next_memory_block->allocated == is_allocated))
		current_memory_block->size += next_memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE;


}

static pmpa_memory_block *find_first_block(MemoryPool* tPool, uint8_t is_allocated, uint32_t min_size)
{
	pmpa_memory_block *memory_block = tPool->master_memory_block;

	while (PMPA_POINTER_IS_IN_POOL(tPool, memory_block + sizeof(pmpa_memory_block))) {
		/* If we're trying to find an block, then defragment the pool as we go along.
		* This incurs a minor speed penalty, but not having to spend time
		* iterating over a fragmented pool makes up for it. */
		if (!is_allocated)
			concat_sequential_blocks(tPool, memory_block, is_allocated);

		if ((memory_block->allocated == is_allocated) && (memory_block->size >= min_size)) {
			return memory_block;
		}
		else {
			memory_block += memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
		}
	}

	return NULL;
}

static void split_block(MemoryPool* tPool, pmpa_memory_block *memory_block, uint32_t size)
{
	pmpa_memory_block *second_memory_block = memory_block + size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
	pmpa_memory_block *original_second_memory_block = memory_block + memory_block->size + PMPA_MEMORY_BLOCK_HEADER_SIZE;
	uint32_t original_memory_block_size = memory_block->size;

	memory_block->allocated = 0;

	/* We can't split this block if there's not enough room to create another one. */
	if (PMPA_POINTER_IS_IN_RANGE((second_memory_block + PMPA_MEMORY_BLOCK_HEADER_SIZE), 0, original_second_memory_block) &&
		(PMPA_POINTER_IS_IN_POOL(tPool, second_memory_block + sizeof(pmpa_memory_block)))) {
		memory_block->size = size;

		second_memory_block->size = original_memory_block_size - (size + PMPA_MEMORY_BLOCK_HEADER_SIZE);
		second_memory_block->allocated = 0;
	}
}

/*
* Externally accessible API functions.
*/

void* createMemoryPool(uint32_t tSize)
{
	MemoryPool* pool = malloc(sizeof(MemoryPool));

	if ((pool->master_memory_block = malloc(tSize))) {
		pool->master_memory_block->size = tSize - PMPA_MEMORY_BLOCK_HEADER_SIZE;
		pool->master_memory_block->allocated = 0;

		pool->master_memory_block_size = tSize;

		return pool;
	}
	else {
		free(pool);
		return NULL;
	}
}

void destroyMemoryPool(void* tPool)
{
	MemoryPool* pool = tPool;
	pool->master_memory_block_size = 0;
	free(pool->master_memory_block);
	free(pool);
}

/*
* Externally accessible C memory functions.
*/

void *allocPoolMemory(void* tPool, size_t size)
{
	MemoryPool* pool = tPool;
	pmpa_memory_block *memory_block = find_first_block(pool, 0, size);

	if (memory_block) {
		split_block(pool, memory_block, size);
		memory_block->allocated = 1;

		return &(memory_block->data);
	}
	else {
		return NULL;
	}
}

void *callocPoolMemory(void* tPool, size_t nelem, size_t elsize)
{
	uint32_t ptr_size = nelem * elsize;
	void *ptr = allocPoolMemory(tPool, ptr_size);

	if (ptr) {
		memset(ptr, 0, ptr_size);

		return ptr;
	}
	else {
		return NULL;
	}
}

void *reallocPoolMemory(void* tPool, void *ptr, size_t size)
{
	MemoryPool* pool = tPool;
	pmpa_memory_block *memory_block = NULL;
	pmpa_memory_block *new_memory_block = NULL;

	uint32_t memory_block_original_size = 0;

	/* If ptr is NULL, realloc() behaves like malloc(). */
	if (!ptr)
		return allocPoolMemory(tPool, size);

	memory_block = (pmpa_memory_block*)((uint32_t)ptr - PMPA_MEMORY_BLOCK_HEADER_SIZE);
	memory_block_original_size = memory_block->size;

	/* Try to cheat by concatenating the current block with contiguous
	* empty blocks after it, and seeing if the new block is big enough. */
	memory_block->allocated = 0;
	concat_sequential_blocks(pool, memory_block, memory_block->allocated);
	memory_block->allocated = 1;

	if (memory_block->size >= size) {
		/* The new block is big enough, split it and use it. */
		split_block(pool, memory_block, size);
		memory_block->allocated = 1;

		return &(memory_block->data);
	}
	else {
		/* The new block is not big enough. */

		/* Restore the memory block's original size. */
		split_block(pool, memory_block, memory_block_original_size);
		memory_block->allocated = 1;

		/* Find another block and try to use that. */
		if (!(new_memory_block = find_first_block(pool, 0, size)))
			return NULL;

		split_block(pool, new_memory_block, size);
		new_memory_block->allocated = 1;

		memcpy(&(new_memory_block->data), &(memory_block->data), memory_block->size);

		freePoolMemory(tPool, &(memory_block->data));
		return &(new_memory_block->data);
	}

	return NULL;
}

void freePoolMemory(void* tPool, void *ptr)
{
	pmpa_memory_block* memory_block = (pmpa_memory_block*)((uint32_t)ptr - PMPA_MEMORY_BLOCK_HEADER_SIZE);

	if (ptr == NULL)
		return;

	memory_block->allocated = 0;
}