#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "memory.h"




/*******************************************************/
/************** Main Memory Functions ******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int string_to_int(char* str) {
    return (int)strtol(str, NULL, 16); // Convert hex string to int
}

// Initializes the main_memory array in the core structure from the file "memin.txt".
// If the file has fewer lines than MAIN_MEMORY_SIZE, the remaining values are set to 0.
main_memory* init_main_memory(char* filename) {
    
    // Initialize all blocks and their data to 0
    main_memory* mem = malloc(sizeof(main_memory));
    if (!mem) {
        perror("Failed to allocate memory for main memory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        mem->blocks[i].tag = i; // Set the tag for each block based on its index
        for (int j = 0; j < BLOCK_SIZE; j++) {
            mem->blocks[i].data[j] = 0; // Initialize data to 0
        }
    }
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    char line[20];
    int line_number = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines
        if (strlen(line) == 0) {
            line_number++;
            continue;
        }
        // Convert the line from hex to int
        uint32_t value = (uint32_t)strtol(line, NULL, 16);
        // Determine block index and offset within the block
        int block_index = line_number / BLOCK_SIZE;
        int block_offset = line_number % BLOCK_SIZE;
        // Store value in the appropriate block and offset
        if (block_index < NUM_OF_BLOCKS) {
            mem->blocks[block_index].data[block_offset] = (int)(value & 0xFFFFF); // Mask to 20 bits
        }
        line_number++;
        // Stop if all blocks are filled
        if (block_index >= NUM_OF_BLOCKS) {
            break;
        }
    }
    fclose(file);
    return mem;
}

/*
Returns a copy of the block from the memory array.
Adjusts to memory boundaries so that there is no overflow.
If the array is not initialized, an empty block is returned.
*/
memory_block* get_block(main_memory* mem, int tag) {
    if (!mem) {
        printf("Error: Memory is not initialized.\n");
        return NULL;
    }
    if (tag < 0) {
        printf("Error: Invalid tag value in get_block.\n");
        return NULL;
    }
    memory_block* mem_block = malloc(sizeof(memory_block));
    if (!mem_block) {
        perror("Failed to allocate memory for memory block");
        exit(EXIT_FAILURE);
    }
    int adjusted_index = tag % NUM_OF_BLOCKS;
    // Return a copy of the appropriate block
    mem_block->tag = mem->blocks[adjusted_index].tag;
    for(int i = 0; i < BLOCK_SIZE; i++){
        mem_block->data[i] = mem->blocks[adjusted_index].data[i];
    }
    return mem_block;
}

// Writes a word (int) to a specific address in memory (at the appropriate location in the block)
void write_word_to_block(main_memory* mem, uint32_t address, int word) {
    if (!mem) {
        printf("Error: Memory pointer is NULL in write_word_to_block.\n");
        return;
    }
    // Extract offset, index, and tag from the address
    if (address >= MAIN_MEMORY_SIZE * BLOCK_SIZE) {
        printf("Error: Address out of bounds in write_word_to_block.\n");
        return;
    }
    uint32_t offset = address % BLOCK_SIZE;                  // Offset within the block
    uint32_t index = (address / BLOCK_SIZE) % NUM_OF_BLOCKS; // Index of the block
    uint32_t tag = address / (BLOCK_SIZE * NUM_OF_BLOCKS);   // Tag of the block
    // Update the block in memory
    mem->blocks[index].data[offset] = word; // Write the word at the correct offset
    mem->blocks[index].tag = tag;           // Update the tag for the block
}


void insert_block_to_memory(main_memory* mem, int tag, memory_block new_block) 
{
    if (!mem) {
        printf("Error: Memory pointer is NULL in write_block_to_memory.\n");
        return;
    }
    if (tag < 0) {
        printf("Error: Invalid tag value in write_block_to_memory.\n");
        return;
    }
    // Compute the index in memory based on the tag
    int index = tag % NUM_OF_BLOCKS;
    // Replace the old block with the new block
    mem->blocks[index] = new_block;
    // Update the tag of the new block
    mem->blocks[index].tag = tag;
}



void free_main_memory(main_memory* memory) {
    if (!memory) {
        return;
    }
    // free each block
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        free(memory->blocks[i].data);
    }
    // Free the main memory itself
    free(memory);
}



/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Prints only the non-zero entries in the main_memory array in the core structure.
void print_memory(main_memory* mem) {
    if (!mem) {
        printf("Error: Memory pointer is NULL in print_memory.\n");
        return;
    }
    printf("Main Memory (Non-Zero Entries):\n");
    int count = 0;
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        int is_non_zero_block = 0; // Flag to check if the block has non-zero data
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (mem->blocks[i].data[j] != 0) {
                is_non_zero_block = 1;
                break;
            }
        }
        if (is_non_zero_block) {
            printf("Block[%d]: { ", i);
            for (int j = 0; j < BLOCK_SIZE; j++) {
                printf("%d", mem->blocks[i].data[j]);
                if (j < BLOCK_SIZE - 1) {
                    printf(", ");
                }
            }
            printf(" } - Tag: %u\n", mem->blocks[i].tag);
            count++;
        }
    }
    if (count == 0) {
        printf("All memory entries are zero.\n");
    }
    printf("End of Non-Zero Memory.\n");
}


// Prints the entire main_memory array in the core structure.
void print_all_memory(main_memory* mem) {
    if (!mem) {
        printf("Error: Memory pointer is NULL in print_all_memory.\n");
        return;
    }
    printf("Main Memory (All Entries):\n");
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        printf("Block[%d]: { ", i);
        for (int j = 0; j < BLOCK_SIZE; j++) {
            printf("%d", mem->blocks[i].data[j]);
            if (j < BLOCK_SIZE - 1) {
                printf(", ");
            }
        }
        printf(" } - Tag: %u\n", mem->blocks[i].tag);
    }

    printf("End of Main Memory.\n");
}



