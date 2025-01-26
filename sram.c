#include <stdio.h>
#include <stdint.h>
#include "sram.h"


/*******************************************************/
/**************** cashe functions **********************/
/*******************************************************/

/* 
The function initializes the cache with:
each tag iniital as 0
each state iniital as "invalid"
each data iniital as 0 
*/
void cache_initialization(Cache *cache) 
{
    for (int i = 0; i < NUM_BLOCKS; i++) {
        cache->blocks[i].tag = 0;               // tag iniital as 0
        cache->blocks[i].state = INVALID;       // invalid
        for (int j = 0; j < CACHE_BLOCK_SIZE; j++) {
            cache->blocks[i].data[j] = 0;       // initial data = 0
        }
    }
}


/*
The function searches for the block in the cache
if found data will be assigned to it and if not data = 0.
returns 1 on success or -1 on failure
*/
int find_word(Cache *cache, uint32_t address, uint32_t *data) {
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS; // extracting the index
    uint32_t tag = address / (CACHE_BLOCK_SIZE * NUM_BLOCKS);   // extracting the tag
    uint32_t offset = address % CACHE_BLOCK_SIZE;               // extracting the offset
    // get the block from the cache
    Cache_block *block = &cache->blocks[index]; 
    // hit
    if (block->state != INVALID && block->tag == tag) {
        *data = block->data[offset];
        return 1;
    }
    // miss
    *data = 0;
    return -1;
}


/*
 * The function inserts a block into the cache.
 * If a block exists at the target index, it will be overwritten.
 * Returns 1 on success, -1 on failure.
 */
int insert_block(Cache *cache, uint32_t address, Cache_block *new_block) {
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS; // extracting the index
    uint32_t tag = address / (CACHE_BLOCK_SIZE * NUM_BLOCKS);   // extracting the index

    // Check if the index is within bounds
    if (index >= NUM_BLOCKS) {
        return -1; // Failure: index out of bounds
    }

    new_block->tag = tag; // Update the block's tag
    cache->blocks[index] = *new_block;

    return 1; // Success
}


/*
 * The function copies the content of one SRAM (source) to another (destination).
 * All blocks, tags, states, and data are copied.
 */
void copy_cache(Cache *source, Cache *destination) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        destination->blocks[i] = source->blocks[i];
    }
}


/*
 * The function updates the MESI state of a block in the cache if it exists.
 * If the block is found, updates its state and returns 1.
 * If the block is not found, returns -1.
 */
int update_state(Cache *cache, uint32_t address, MESI_state new_state) {
    
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS; // Extract the index
    uint32_t tag = address / (CACHE_BLOCK_SIZE * NUM_BLOCKS);   // Extract the tag

    Cache_block *block = &cache->blocks[index];

    // Check if the block is valid and the tag matches
    if (block->state != INVALID && block->tag == tag) {
        block->state = new_state; // Update the state
        return 1; // Success
    }
    return -1; // Block not found
}






/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

/*
 * Prints the entire cache, including all blocks regardless of their MESI state.
 */
void print_all_cache(Cache *cache) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        Cache_block *block = &cache->blocks[i];
        printf("block(%d): tag = %u, data = {", i, block->tag);
        for (int j = 0; j < CACHE_BLOCK_SIZE; j++) {
            printf("%u", block->data[j]);
            if (j < CACHE_BLOCK_SIZE - 1) {
                printf(", ");
            }
        }
        printf("}, MESI state = ");
        switch (block->state) {
            case MODIFIED:
                printf("M");
                break;
            case SHARED:
                printf("S");
                break;
            case EXCLUSIVE:
                printf("E");
                break;
            case INVALID:
                printf("I");
                break;
        }
        printf("\n");
    }
}

/*
 * Prints only the valid blocks of the cache (blocks with MESI state not INVALID).
 */
void print_cache(Cache *cache) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        Cache_block *block = &cache->blocks[i];
        if (block->state != INVALID) {
            printf("block(%d): tag = %u, data = {", i, block->tag);
            for (int j = 0; j < CACHE_BLOCK_SIZE; j++) {
                printf("%u", block->data[j]);
                if (j < CACHE_BLOCK_SIZE - 1) {
                    printf(", ");
                }
            }
            printf("}, MESI state = ");
            switch (block->state) {
                case MODIFIED:
                    printf("M");
                    break;
                case SHARED:
                    printf("S");
                    break;
                case EXCLUSIVE:
                    printf("E");
                    break;
                case INVALID:
                    // This case won't occur because we skip INVALID states
                    break;
            }
            printf("\n");
        }
    }
}
