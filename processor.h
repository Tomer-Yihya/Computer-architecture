#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sram.h"
#include "core.h"
#include "memory.h"

/*******************************************************/
/**************** Processor sizes setting **************/
/*******************************************************/

#define NUM_OF_CORES 4

/*******************************************************/
/*********************  Structs ************************/
/*******************************************************/

typedef struct
{
    char *sim_str;
    char *imem0_str;
    char *imem1_str;
    char *imem2_str;
    char *imem3_str;
    char *memin_str;
    char *memout_str;
    char *regout0_str;
    char *regout1_str;
    char *regout2_str;
    char *regout3_str;
    char *core0trace_str;
    char *core1trace_str;
    char *core2trace_str;
    char *core3trace_str;
    char *bustrace_str;
    char *dsram0_str;
    char *dsram1_str;
    char *dsram2_str;
    char *dsram3_str;
    char *tsram0_str;
    char *tsram1_str;
    char *tsram2_str;
    char *tsram3_str;
    char *stats0_str;
    char *stats1_str;
    char *stats2_str;
    char *stats3_str;

} filenames;

// Core structures
typedef struct
{
    int cycle;
    core *core0;
    core *core1;
    core *core2;
    core *core3;
    core *round_robin_queue[NUM_OF_CORES];
    instructions *core0_instructions;
    instructions *core1_instructions;
    instructions *core2_instructions;
    instructions *core3_instructions;
    filenames *filenames;

} processor;

/*******************************************************/
/*************** Processor Functions *******************/
/*******************************************************/

// Initializes file names to the argv[] arguments
void set_file_names(filenames *filenames, char *argv[]);

// Initializes file names to the defined default values
void set_default_file_names(filenames *filenames);

/*
 * Initializes the entire processor structure.
 * - Sets pc to 0.
 * - Initializes each core registers and prev_registers to 0.
 * - Initializes each core imem and prev_imem according to to the file instructions.
 * - Initializes each core cache and prev_cache using their respective initialization function.
 */
processor *init_processor();

// Executes the processor run
void run(processor *cpu, main_memory *memory);

// Check if all the cores finished running
bool finish(processor *cpu);

// Frees the cores memory including its cache
void free_processor(processor *cpu);

memory_block *convert_cache_block_to_mem_block(cache_block *c_block);

cache_block *convert_mem_block_to_cache_block(memory_block *m_block);

int search_modified_block(processor *cpu, uint32_t address, bool *about_to_be_overwritten, uint32_t *address_of_overwritten);

void update_cache_stats(cache_block *core0_block, cache_block *core1_block, cache_block *core2_block, cache_block *core3_block, cache_block *mem_block);

/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

void print_bus_status(processor *cpu);

#endif // PROCESSOR_H
