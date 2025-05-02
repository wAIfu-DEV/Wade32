#pragma once

#include "../xstd/xstd_core.h"
#include "../xstd/xstd_alloc.h"

#include "kernel_panic.h"

extern u32 floppy_end; // Points to end of floppy ~1.4MB

typedef struct _page_alloc_block
{
    struct _page_alloc_block* next;
    volatile i8* start;
    u32 pages;
    ibool free;
    ibool valid;
} _PageAllocBlock;

typedef struct
{
    _PageAllocBlock *head;
    u32 blockCount;
} PageAllocState;

#define ALLOC_MEM_START    (volatile i8*)0x100000
#define ALLOC_MEM_END      (volatile i8*)(1024 * 1440)

#define PAGE_SIZE          (u32)4096
#define TOTAL_PAGES        ((((u32)ALLOC_MEM_END) - ((u32)ALLOC_MEM_START)) / (PAGE_SIZE))

#define ALLOC_BLOCK_COUNT  ((TOTAL_PAGES) / (u32)2)
#define ALLOC_BLOCK_SIZE   (sizeof(_PageAllocBlock) * ALLOC_BLOCK_COUNT)
#define ALLOC_BLOCK_START  (((u32)ALLOC_MEM_START) + sizeof(PageAllocState))

#define ALLOC_HEADER_SIZE  (sizeof(PageAllocState) + (ALLOC_BLOCK_SIZE))
#define ALLOC_HEADER_START ((u32)ALLOC_MEM_START)
#define ALLOC_HEADER_END   ((u32)ALLOC_MEM_START + (ALLOC_HEADER_SIZE))
#define ALLOC_HEADER_PAGES (((ALLOC_HEADER_END) - (ALLOC_HEADER_START)) / (PAGE_SIZE))

#define ALLOCABLE_PAGES    ((TOTAL_PAGES) - (ALLOC_HEADER_PAGES))
#define ALLOCABLE_START    (volatile i8*)((u32)ALLOC_MEM_START + (ALLOC_HEADER_PAGES * PAGE_SIZE))

inline u32 __pages_from_size(u32 size)
{
    u32 pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    return pages;
}

inline u32 __size_from_pages(u32 pages)
{
    return pages * PAGE_SIZE;
}

_PageAllocBlock* __page_alloc_seek_block(PageAllocState *state, volatile i8* ptr)
{
    for (u32 i = 0; i < state->blockCount; ++i)
    {
        _PageAllocBlock *block = ((_PageAllocBlock*)ALLOC_BLOCK_START) + i;
        if (block->start == ptr)
        {
            return block;
        }
    }
    return NULL;
}

_PageAllocBlock* __page_alloc_write_block(PageAllocState *state, _PageAllocBlock writeBlock)
{
    // Try overwrite invalid block
    for (u32 i = 0; i < state->blockCount; ++i)
    {
        _PageAllocBlock *block = ((_PageAllocBlock*)ALLOC_BLOCK_START) + i;
        if (!block->valid)
        {
            *block = writeBlock;
            return block;
        }
    }

    // Try append block
    if (state->blockCount < ALLOC_BLOCK_COUNT)
    {
        _PageAllocBlock *block = ((_PageAllocBlock*)ALLOC_BLOCK_START) + state->blockCount;
        *block = writeBlock;
        ++state->blockCount;
        return block;
    }

    return NULL;
}

void* __page_alloc_alloc(Allocator* a, u64 size)
{
    u32 pages = __pages_from_size(size); // Pages required to fit newSize

    PageAllocState* state = a->_internalState;
    _PageAllocBlock *pab = state->head;

    while (pab)
    {
        if (pab->free && pab->pages >= pages) // Is valid block
        {
            if (pab->pages > pages) // Split block
            {
                u32 restPages = pab->pages - pages;

                // Create new free block
                _PageAllocBlock* block = __page_alloc_write_block(state, (_PageAllocBlock){
                    .pages = restPages,
                    .start = pab->start + __size_from_pages(pages),
                    .free = true,
                    .valid = true,
                });

                pab->pages = pages; // Resize current block
                pab->free = false;

                // Insert free block after current
                _PageAllocBlock* temp = pab->next;
                pab->next = block;
                block->next = temp;
            }
            return (void*)pab->start;
        }
        pab = pab->next;
    }
    return NULL;
}

void __page_alloc_free(Allocator* a, void* block)
{
    PageAllocState* state = a->_internalState;
    volatile i8* pagePtr = (volatile i8*)block;

    // There must be a better way, thinking of keeping ptr->block hashmap
    _PageAllocBlock* pageBlock = __page_alloc_seek_block(state, pagePtr);
    if (!pageBlock)
        return;
    
    pageBlock->free = true;

    // Coalesce adjacent free blocks
    _PageAllocBlock *curr = state->head;

    while (curr && curr->next)
    {
        if (curr->free && curr->next->free
            && ((i8 *)curr->start + (curr->pages * PAGE_SIZE) == (i8 *)curr->next->start))
        {
            curr->pages += curr->next->pages;

            // Remove from list
            _PageAllocBlock *temp = curr->next;
            temp->valid = false;
            curr->next = curr->next->next;
        }
        else
        {
            curr = curr->next;
        }
    }
}

void *__page_alloc_realloc(Allocator *this, void *ptr, u64 newSize)
{
    if (!ptr)
        return __page_alloc_alloc(this, newSize);

    if (newSize == 0)
    {
        __page_alloc_free(this, ptr);
        return NULL;
    }

    u32 pages = __pages_from_size(newSize); // Pages required to fit newSize

    volatile i8* pagePtr = (volatile i8*)ptr;
    PageAllocState* state = this->_internalState;
    _PageAllocBlock* pageBlock = __page_alloc_seek_block(state, pagePtr);
    if (!pageBlock)
        return NULL;

    u32 pagesDiff = pages - pageBlock->pages;

    if (pages <= pageBlock->pages) // Should we allow downsizing??
        return ptr;

    // Check next block for enough free pages
    // Best case scenario, no memmove required
    if (pageBlock->next && pageBlock->next->free && pageBlock->next->pages >= pagesDiff)
    {
        _PageAllocBlock* nextFree = pageBlock->next;
        u32 restPages = nextFree->pages - pagesDiff;

        if (restPages > 0)
        {
            nextFree->pages = restPages;
            nextFree->start = nextFree->start + __size_from_pages(pagesDiff);
        }
        else
        {
            pageBlock->next = nextFree->next;
            nextFree->valid = false;
        }

        pageBlock->pages += pagesDiff;
        return ptr;
    }

    void *newPtr = __page_alloc_alloc(this, newSize);
    if (!newPtr)
        return NULL;

    // Very expensive...
    u32 bound = pageBlock->pages * PAGE_SIZE;
    for (u32 i = 0; i < bound; ++i)
        ((i8 *)newPtr)[i] = ((i8 *)ptr)[i];

    __page_alloc_free(this, ptr);
    return newPtr;
}

ResultAllocator kernel_create_page_allocator()
{
    if (ALLOC_MEM_START >= ALLOC_MEM_END)
        kernel_panic(KERR_NO_PAGE_MEMORY, "Not enough allocatable page memory.");

    _PageAllocBlock* initialBlock = (_PageAllocBlock*)ALLOC_BLOCK_START;
    *initialBlock = (_PageAllocBlock){
        .start = ALLOCABLE_START,
        .pages = ALLOCABLE_PAGES,
        .free = true,
        .valid = true,
        .next = NULL,
    };

    PageAllocState *state = (PageAllocState *)ALLOC_HEADER_START;
    *state = (PageAllocState){
        .head = initialBlock,
        .blockCount = 1,
    };

    return (ResultAllocator){
        .value = (Allocator){
            ._internalState = state,
            .alloc = __page_alloc_alloc,
            .free = __page_alloc_free,
            .realloc = __page_alloc_realloc,
        },
        .error = ERR_OK,
    };
}
