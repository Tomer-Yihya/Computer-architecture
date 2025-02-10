#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sram.h"
#include "core.h"
#include "memory.h"



/*
// check Cache_initialization
int main() {
    Cache cache;
    cache_initialization(&cache);

    // Initial printing of the empty cache
    //printf("=== Initial Cache State (All Blocks) ===\n");
    //print_all_cache(&cache);
    //printf("=== Initial Cache State (Valid Blocks Only) ===\n");
    //print_cache(&cache);

    // Fill the cache with sequential numbers starting from 1
    //printf("\n=== Filling the Cache with Sequential Numbers ===\n");
    for (uint32_t i = 0; i < NUM_BLOCKS; i++) {
        Cache_block new_block = {
            .tag = i,
            .state = MODIFIED
        };
        for (uint32_t j = 0; j < BLOCK_SIZE; j++) {
            new_block.data[j] = i * BLOCK_SIZE + j + 1; // Sequential numbers starting from 1
        }
        insert_block(&cache, i * BLOCK_SIZE, &new_block);
    }

    // Print the cache after filling with sequential numbers
    //printf("\n=== Cache After Filling (All Blocks) ===\n");
    //print_all_cache(&cache);
    printf("=== Cache After Filling (Valid Blocks Only) ===\n");
    print_cache(&cache);

    // Insert 777 into 50 different locations and overwrite old values
    printf("\n=== Overwriting 50 Blocks with Value 777 ===\n");
    for (uint32_t i = 0; i < 50; i++) {
        Cache_block new_block = {
            .tag = i,
            .state = MODIFIED
        };
        for (uint32_t j = 0; j < BLOCK_SIZE; j++) {
            new_block.data[j] = 777;
        }
        insert_block(&cache, i * BLOCK_SIZE, &new_block);
    }

    // Print the cache after overwriting with 777
    //printf("\n=== Cache After Overwriting with 777 (All Blocks) ===\n");
    //print_all_cache(&cache);
    printf("=== Cache After Overwriting with 777 (Valid Blocks Only) ===\n");
    print_cache(&cache);

    // Update the MSI state of 777 blocks to INVALID
    printf("\n=== Updating MSI State of 777 Blocks to INVALID ===\n");
    for (uint32_t i = 0; i < 50; i++) {
        update_state(&cache, i * BLOCK_SIZE, INVALID);
    }

    // Final print of the cache
    //printf("\n=== Final Cache State (All Blocks) ===\n");
    //print_all_cache(&cache);
    printf("=== Final Cache State (Valid Blocks Only) ===\n");
    print_cache(&cache);

    return 0;
}
*/

/*
// check imem_initialization
int main() {
    core cpu0;
    core cpu1;
    core cpu2;
    core cpu3;

    // Initialize the core's instruction memory
    imem_initialization(&cpu0, "imem0.txt");
    imem_initialization(&cpu1, "imem1.txt");
    imem_initialization(&cpu2, "imem2.txt");
    imem_initialization(&cpu3, "imem3.txt");

    // Print the instruction memory
    print_imem(&cpu0);
    print_imem(&cpu1);
    print_imem(&cpu2);
    print_imem(&cpu3);

    return 0;
}
*/

/*
// check main_memory_initialization
int main() {
    
    main_memory memory; // Static allocation
    main_memory_initialization(&memory);
    write_word_to_block(&memory, 2304, 88);
    write_word_to_block(&memory, 2305, 88);
    write_word_to_block(&memory, 2306, 88);
    write_word_to_block(&memory, 2307, 88);
    write_word_to_block(&memory, 2556, 90);
    write_word_to_block(&memory, 2557, 90);
    write_word_to_block(&memory, 2558, 90);
    write_word_to_block(&memory, 2559, 90);
    write_word_to_block(&memory, 2560, -2);
    write_word_to_block(&memory, 2561, -3);
    write_word_to_block(&memory, 2562, -4);
    write_word_to_block(&memory, 2563, -5);
    print_memory(&memory);
    //print_all_memory(&memory);

    return 0;
}
*/



// check the core functions
int main() {
    
    // Step 1: Initialize the core from the file "imem0.txt"
    core* cpu0 = core_initialization(0,"imem0.txt");
    core* cpu1 = core_initialization(1,"imem1.txt");
    if (!cpu0 || !cpu1) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }

    // Step 2: Initialize the main memory from the file "memin.txt"
    main_memory* memory = main_memory_initialization();
    if (!memory) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }

    // Step 3: Initialize and print core status
    //printf("Core status after initialization:\n");
    //print_core_status(cpu);

    // Step 4: Run the core
    run_core(cpu0, memory, "imem0.txt");
    run_core(cpu1, memory, "imem1.txt");
    //run_core(cpu, memory, "imem2.txt");
    //run_core(cpu, memory, "imem3.txt");

    // Step 5: Print the core status after running
    //printf("\nCore status after running:\n");
    //print_core_status(cpu);

    free_main_memory(memory);

    return 0;
}




