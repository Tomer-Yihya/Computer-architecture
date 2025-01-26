#include <stdio.h>
#include <stdint.h>

/*******************************************************/
/****************** Cache sizes setting ****************/
/*******************************************************/

#define CACHE_SIZE 256       // 256 words in the cache
#define CACHE_BLOCK_SIZE 4   // 4 words in block
#define NUM_BLOCKS (CACHE_SIZE / CACHE_BLOCK_SIZE) // number of blocks - 64 (256/4 = 64)

/*******************************************************/
/****************** Cashe Structs **********************/
/*******************************************************/

// MESI - states
typedef enum {
    INVALID = 0,
    SHARED,
    EXCLUSIVE,
    MODIFIED
} MESI_state;

// Cache_block
typedef struct {
    uint32_t tag;
    MESI_state state;           
    uint32_t data[CACHE_BLOCK_SIZE];  
} Cache_block;

// Cache
typedef struct {
    Cache_block blocks[NUM_BLOCKS]; 
} Cache;


/*******************************************************/
/**************** cashe functions **********************/
/*******************************************************/

void cache_initialization(Cache *cache);


int find_word(Cache *cache, uint32_t address, uint32_t *data);


int insert_block(Cache *cache, uint32_t address, Cache_block *new_block);


void copy_cache(Cache *source, Cache *destination);


int update_state(Cache *cache, uint32_t address, MESI_state new_state);


/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

void print_all_cache(Cache *cache); 


void print_cache(Cache *cache);

