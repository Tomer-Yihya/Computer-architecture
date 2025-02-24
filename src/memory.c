#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sram.h"
#include "memory.h"




/*******************************************************/
/************** Main Memory Functions ******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int string_to_int(char* str) 
{
    return (int)strtol(str, NULL, 16); // Convert hex string to int
}

// Initializes the main_memory array in the core structure from the file "memin.txt".
// If the file has fewer lines than MAIN_MEMORY_SIZE, the remaining values are set to 0.
main_memory* init_main_memory(char* filename) 
{
    // Allocate memory for the main memory structure
    main_memory* mem = malloc(sizeof(main_memory));
    if (!mem) {
        perror("Failed to allocate memory for main memory");
        exit(EXIT_FAILURE);
    }
    // Initialize all blocks
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        mem->blocks[i].tag = (uint32_t)i/64;  // Ensure tag starts at 0
        for (int j = 0; j < BLOCK_SIZE; j++) {
            mem->blocks[i].data[j] = 0;
        }
    }
    // Open memory input file
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        free(mem);
        return NULL;
    }
    char line[20];
    int line_number = 0;
    int block_index = 0;
    int block_offset = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) { 
            line_number++;
            continue;
        }
        uint32_t value = (uint32_t)strtol(line, NULL, 16);
        block_index = line_number / BLOCK_SIZE;
        block_offset = line_number % BLOCK_SIZE;
        if (block_index < NUM_OF_BLOCKS) {
            mem->blocks[block_index].data[block_offset] = value;
        }
        line_number++;
    }
    while (line_number < MAIN_MEMORY_SIZE) {
        block_index = line_number / BLOCK_SIZE;
        block_offset = line_number % BLOCK_SIZE;
        if (block_index < NUM_OF_BLOCKS) {
            mem->blocks[block_index].data[block_offset] = 0;
        }
        line_number++;
    }
    fclose(file);
    return mem;
}

/*
Returns a copy of the block from the memory array.
Adjusts to memory boundaries so that there is no overflow.
If the array is not initialized, an empty block is returned.
*/
memory_block* get_block(main_memory* mem, uint32_t address) 
{
    if (!mem) {
        printf("Error: Memory is not initialized.\n");
        return NULL;
    }
    memory_block* mem_block = malloc(sizeof(memory_block));
    if (!mem_block) {
        perror("Failed to allocate memory for memory block");
        exit(EXIT_FAILURE);
    }
    int index = get_index(address);
    // Return a copy of the appropriate block
    mem_block->tag = mem->blocks[index].tag;
    for(int i = 0; i < BLOCK_SIZE; i++){
        mem_block->data[i] = mem->blocks[index].data[i];
    }
    return mem_block;
}


void insert_block_to_memory(main_memory* mem, uint32_t address, memory_block new_block) 
{
    if (!mem) {
        printf("Error: Memory pointer is NULL in write_block_to_memory.\n");
        return;
    }
    // Compute the index in memory based on the tag
    uint32_t index = get_index(address);
    uint32_t tag = get_tag(address);
    // Replace the old block with the new block
    mem->blocks[index] = new_block;
    // Update the tag of the new block
    mem->blocks[index].tag = tag;
}


void free_main_memory(main_memory* memory) 
{
    if (!memory) {
        return;
    }
    // Free the main memory itself
    free(memory);
}


/*******************************************************/
/*************** Create output files *******************/
/*******************************************************/

void create_memout_file(main_memory* mem, char* filename)
{
    FILE* file = fopen(filename, "w");
    if (!file || !mem) {
        perror("Error opening memout file or Memory structure is NULL");
        return;
    }
    // Find the last nonzero element in memory
    int last_nonzero_index = 0;
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            if (mem->blocks[i].data[j] != 0) {
                last_nonzero_index = i * BLOCK_SIZE + j; // Store absolute index of last nonzero element
            }
        }
    }
    // Print memory contents up to the last nonzero element
    int current_index = 0;
    for (int i = 0; i < NUM_OF_BLOCKS; i++) {
        for (int j = 0; j < BLOCK_SIZE; j++) {
            // Ensure we are within valid memory bounds
            fprintf(file, "%08X\n", mem->blocks[i].data[j]);
            if (current_index == last_nonzero_index) {
                fclose(file);
                return;
            }
            current_index++;
        }
    }
    fclose(file);
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



