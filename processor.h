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

// All the files we need
typedef struct {
    FILE* memout;
    FILE* core0trace;
    FILE* core1trace;
    FILE* core2trace;
    FILE* core3trace;
    
} files;


typedef struct {
    char* sim_str;
    char* imem0_str;
    char* imem1_str;
    char* imem2_str;
    char* imem3_str;
    char* memin_str;
    char* memout_str;
    char* regout0_str;
    char* regout1_str;
    char* regout2_str;
    char* regout3_str;
    char* core0trace_str;
    char* core1trace_str;
    char* core2trace_str;
    char* core3trace_str;
    char* bustrace_str;
    char* dsram0_str;
    char* dsram1_str;
    char* dsram2_str;
    char* dsram3_str;
    char* tsram0_str;
    char* tsram1_str;
    char* tsram2_str;
    char* tsram3_str;
    char* stats0_str;
    char* stats1_str;
    char* stats2_str;
    char* stats3_str;

} filesnames;

// Core structures
typedef struct {
    int cycle;
    core* core0;
    core* core1;
    core* core2;
    core* core3;
    instructions* core0_instructions;
    instructions* core1_instructions;
    instructions* core2_instructions;
    instructions* core3_instructions;

} processor;


/*******************************************************/
/*************** Processor Functions *******************/
/*******************************************************/

/*
 * Initializes the entire processor structure.
 * - Sets pc to 0.
 * - Initializes each core registers and prev_registers to 0.
 * - Initializes each core imem and prev_imem according to to the file instructions.
 * - Initializes each core cache and prev_cache using their respective initialization function.
 */
processor* init_processor(filesnames* filesnames);

// Opens a single file and returns an error if not opened.
void open_file(FILE* f, char* filename, char c);

// Opens all files
void open_files(files* f, filesnames* filesnames);

// Closes all open files.
void close_files(files* f);




// Pipingline stages
// Performing the Fetch phase
void fetch (core* cpu, instruction* instruction);

// Performing the decode phase, Returns true if a jump should be performed and false otherwise
bool decode (core* cpu, instruction* instruction);

// Performing the Execute phase
void execute (core* cpu, instruction* instruction);

// Performing the Mem phase
void mem(core* cpu, instruction* instruction, main_memory* memory);

// Performing the WB phase
void write_beck (core* cpu, instruction* instruction);

// performing one step in the core pipeline
// Calculates pipeline delays and updates instructions accordingly
void pipeline_step(filesnames* filesnames, core* cpu, main_memory* memory, instructions* instructions);




// Executes the processor run
void run(filesnames* filesnames, processor* cpu, main_memory* memory);

// Check if all the cores finished running
bool finish(processor* cpu);

// Frees the core's memory including its cache
void free_processor(processor* cpu);


void convert_cache_block_to_mem_block(cache_block* c_block, memory_block* m_block);


void convert_mem_block_to_cache_block(cache_block* c_block, memory_block* m_block);


/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/



#endif // PROCESSOR_H

