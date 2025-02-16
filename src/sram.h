#ifndef SRAM_H
#define SRAM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/*******************************************************/
/****************** Cache sizes setting ****************/
/*******************************************************/

#define CACHE_SIZE 256                             // 256 words in the cache
#define CACHE_BLOCK_SIZE 4                         // 4 words in block
#define NUM_BLOCKS (CACHE_SIZE / CACHE_BLOCK_SIZE) // number of blocks - 64 (256/4 = 64)

/*******************************************************/
/****************** Cashe Structs **********************/
/*******************************************************/

// MESI - states
typedef enum
{
    INVALID,
    SHARED,
    EXCLUSIVE,
    MODIFIED
} MESI_state;

// cache_block
typedef struct
{
    uint32_t tag;
    MESI_state state;
    int cycle;
    int data[CACHE_BLOCK_SIZE];
} cache_block;

// Cache
typedef struct
{
    cache_block blocks[NUM_BLOCKS];
} Cache;

/*******************************************************/
/**************** cashe functions **********************/
/*******************************************************/

/*
 * The function initializes the cache with:
 * each tag iniital as 0
 * each state iniital as "invalid"
 * each data iniital as 0
 */
void cache_initialization(Cache *cache);

/*
 * The function looks for the block in the cache, if it is found it returns true and otherwise it returns false.
 * In addition, the function returns a copy of the block via the pointer it received as an argument
 */
bool search_block(Cache *cache, uint32_t address);

// return pointer to copy of the cache block
cache_block *get_cache_block(Cache *cache, uint32_t address);

/*
 * The function inserts a block into the cache.
 * If a block exists at the target index, it will be overwritten.
 * Returns true on success and false in case of a failure.
 */
bool insert_block(Cache *cache, uint32_t address, cache_block *new_block, int cycle);

/*
 * The function copies the content of one SRAM (source) to another (destination).
 * All blocks, tags, states, and data are copied.
 */
void copy_cache(Cache *source, Cache *destination);

/*
 * The function updates the MESI state of a block in the cache if it exists.
 * If the block is found, updates its state and true, if not found, returns false.
 */
bool update_state(Cache *cache, uint32_t address, MESI_state new_state);

void free_cache(Cache *cache);

/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Prints the entire cache, including all blocks regardless of their MESI state.
void print_all_cache(Cache *cache);

// Prints only the valid blocks of the cache (blocks with MESI state not INVALID).
void print_cache(Cache *cache);

#endif // SRAM_H
