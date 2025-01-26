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
void main_memory_initialization(main_memory* mem) {
    // Initialize all blocks and their data to 0
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        mem->blocks[i].tag = i; // Set the tag for each block based on its index
        for (int j = 0; j < BLOCK_SIZE; j++) {
            mem->blocks[i].data[j] = 0; // Initialize data to 0
        }
    }
    FILE* file = fopen("memin.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
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
            mem->blocks[block_index].data[block_offset] = value & 0xFFFFF; // Mask to 20 bits
        }
        line_number++;
        // Stop if all blocks are filled
        if (block_index >= NUM_OF_BLOCKS) {
            break;
        }
    }

    fclose(file);
}

/*
Returns a copy of the block from the memory array.
Adjusts to memory boundaries so that there is no overflow.
If the array is not initialized, an empty block is returned.
*/
memory_block get_block(main_memory* mem, int tag) {
    if (!mem) {
        printf("Error: Memory is not initialized.\n");
        memory_block empty_block = {0}; // Return an empty block in case of error
        return empty_block;
    }
    // Adjust tag using modulo to ensure it is within bounds
    int adjusted_index = tag % NUM_OF_BLOCKS;
    if (adjusted_index < 0) {
        adjusted_index += NUM_OF_BLOCKS; // Handle negative indices
    }
    // Return a copy of the appropriate block
    return mem->blocks[adjusted_index];
}

// Writes a word (int) to a specific address in memory (at the appropriate location in the block)
void write_word_to_block(main_memory* mem, uint32_t address, int word) {
    if (!mem) {
        printf("Error: Memory is not initialized.\n");
        return;
    }
    // Extract offset, index, and tag from the address
    uint32_t offset = address % BLOCK_SIZE;                      // Offset within the block
    uint32_t index = (address / BLOCK_SIZE) % NUM_OF_BLOCKS;     // Index of the block
    uint32_t tag = address / (BLOCK_SIZE * NUM_OF_BLOCKS);       // Tag of the block
    // Update the block in memory
    mem->blocks[index].data[offset] = word; // Write the word at the correct offset
    mem->blocks[index].tag = tag;           // Update the tag for the block
}


/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Prints only the non-zero entries in the main_memory array in the core structure.
void print_memory(main_memory* mem) {
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




