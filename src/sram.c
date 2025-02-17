#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "sram.h"



/*******************************************************/
/**************** cashe functions **********************/
/*******************************************************/

/* 
* The function initializes the cache with:
* each tag iniital as 0
* each state iniital as "invalid"
* each data iniital as 0 
*/
void cache_initialization(Cache *cache) 
{
    for (int i = 0; i < NUM_BLOCKS; i++) {
        cache->blocks[i].tag = 0;               // tag iniital as 0
        cache->blocks[i].cycle = 0;
        cache->blocks[i].state = INVALID;       // invalid
        for (int j = 0; j < CACHE_BLOCK_SIZE; j++) {
            cache->blocks[i].data[j] = 0;       // initial data = 0
        }
    }
}


// The function looks for the block in the cache, if it is found it returns true and otherwise it returns false.
bool search_block(Cache *cache, uint32_t address) {
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS; // extracting the index
    uint32_t tag = address / (CACHE_BLOCK_SIZE * NUM_BLOCKS);   // extracting the tag
    // uint32_t offset = address % CACHE_BLOCK_SIZE;            // extracting the offset
    
    // get the block from the cache
    cache_block* cache_block_ptr = &cache->blocks[index];
    // hit
    //if (cache_block_ptr->state != INVALID && cache_block_ptr->tag == tag) {
    if (cache_block_ptr->state != INVALID && cache_block_ptr->tag == tag) {
        return true;
    }
    // miss
    return false;
}

// return pointer to copy of the cache block
cache_block* get_cache_block(Cache *cache, uint32_t address)
{
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS;
    cache_block block = cache->blocks[index];
    cache_block* c_block = (cache_block*)malloc(sizeof(cache_block));
    if (!c_block) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    c_block->tag = block.tag;
    c_block->state = block.state;
    for(int i = 0; i < CACHE_BLOCK_SIZE; i++){
        c_block->data[i] = block.data[i];
    }
    return &cache->blocks[index];
}


/*
 * The function inserts a block into the cache.
 * If a block exists at the target index, it will be overwritten.
 * Returns true on success and false in case of a failure.
 */
bool insert_block(Cache *cache, uint32_t address, cache_block *new_block, int cycle) {
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS; // extracting the index
    uint32_t tag = address / (CACHE_BLOCK_SIZE * NUM_BLOCKS);   // extracting the index

    // Check if the index is within bounds
    if (index >= NUM_BLOCKS) {
        return false;; // Failure: index out of bounds
    }
    new_block->tag = tag; // Update the block's tag
    new_block->cycle = cycle;
    cache->blocks[index] = *new_block;

    return true;; // Success
}


/*
 * The function updates the MESI state of a block in the cache if it exists.
 * If the block is found, updates its state and true, if not found, returns false.
 */
bool update_state(Cache *cache, uint32_t address, MESI_state new_state) {
    
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS; // Extract the index
    uint32_t tag = address / (CACHE_BLOCK_SIZE * NUM_BLOCKS);   // Extract the tag

    cache_block *block = &cache->blocks[index];

    // Check if the block is valid and the tag matches
    if (block->state != INVALID && block->tag == tag) {
        block->state = new_state; // Update the state
        return true; // Success
    }
    return false; // Block not found
}



void free_cache(Cache* cache) {
    if (!cache) return;
    free(cache);
}


/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

/*
 * Prints the entire cache, including all blocks regardless of their MESI state.
 */
void print_all_cache(Cache *cache) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        cache_block *block = &cache->blocks[i];
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
        cache_block *block = &cache->blocks[i];
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
