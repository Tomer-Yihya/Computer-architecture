#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>



/*******************************************************/
/************* Main Memory sizes setting ***************/
/*******************************************************/

#define BLOCK_SIZE 4            // 4 words in block
//#define MAIN_MEMORY_SIZE 262144 // 2^18 (2^20/4) lines of 4 words block <= to big for prints
#define MAIN_MEMORY_SIZE 256    // 256 lines of 4 words block
#define NUM_OF_BLOCKS (MAIN_MEMORY_SIZE / BLOCK_SIZE) // number of blocks - 64 (2^20/4 = 262144)

/*******************************************************/
/************** Main Memory Structs ********************/
/*******************************************************/

// Memory_block
typedef struct {
    uint32_t tag;   
    uint32_t data[BLOCK_SIZE];  
} memory_block;

typedef struct {
    memory_block blocks[NUM_OF_BLOCKS];
} main_memory;


/*******************************************************/
/************** Main Memory Functions ******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int string_to_int(char* str);

// Initializes the main_memory array in the core structure from the file "memin.txt".
// If the file has fewer lines than MAIN_MEMORY_SIZE, the remaining values are set to 0.
void main_memory_initialization(main_memory* mem);

/*
Returns a copy of the block from the memory array.
Adjusts to memory boundaries so that there is no overflow.
If the array is not initialized, an empty block is returned.
*/
memory_block get_block(main_memory* mem, int tag);

// Writes a word (int) to a specific address in memory (at the appropriate location in the block)
void write_word_to_block(main_memory* mem, uint32_t address, int word);



/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Prints only the non-zero entries in the main_memory array in the core structure.
void print_memory(main_memory* mem);

// Prints the entire main_memory array in the core structure.
void print_all_memory(main_memory* mem);



