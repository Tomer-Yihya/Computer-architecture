#ifndef CORE_H
#define CORE_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sram.h"
#include "memory.h"
#include "core.h"



/*******************************************************/
/****************** Core sizes setting *****************/
/*******************************************************/

#define NUM_OF_REGISTERS 16
#define IMEM_SIZE 1024   // 1024 lines of 32 bits
#define HALT_OPCODE 20
#define STALL_OPCODE 21
#define BUS_DELAY 16  // Delay until the first word is retrieved from memory
#define BLOCK_DELAY 4 // Delay until the entire block is received

/*******************************************************/
/*********************  Structs ************************/
/*******************************************************/

// Assembler structures
typedef struct {
    int pc;
    int opcode;
    int rt;
    int rs;
    int rd;
    int imm;
    int ALU_result;
    int bus_delay;
    int block_delay;
} instruction;


typedef struct {
    instruction* fetch;
    instruction* decode;
    instruction* execute;
    instruction* memory;
    instruction* write_back;
} instructions;


typedef struct {
    int total_cycles;
    int total_instructions;
    int read_hit;
    int write_hit;
    int read_miss;
    int write_miss;
    int num_of_decode_stalls;
    int num_of_mem_stalls;

} stats;


// Core structures
typedef struct {
    int pc;
    int cycle;
    int core_number;
    // this cycle
    int registers[NUM_OF_REGISTERS];
    instruction imem[IMEM_SIZE];
    Cache* cache;
    stats* stats;
    // prev cycle
    int prev_registers[NUM_OF_REGISTERS];
    instruction prev_imem[IMEM_SIZE];
    Cache* prev_cache;
    bool halt_flag; // halt_flag = 1 if we get "halt" opcode - the core know to finish and stop
    bool done;      // true if the core finish the imem instructions

} core;


/*******************************************************/
/**************** Assembler Functions*******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int str_to_int(char* str);

/*
 * Parses an 8-digit string into components and converts them to integers.
 * Parameters:
 * - imm: Pointer to store the integer value of the 3 least significant digits.
 * - rt: Pointer to store the integer value of the fourth least significant digit.
 * - rs: Pointer to store the integer value of the fifth least significant digit.
 * - rd: Pointer to store the integer value of the sixth least significant digit.
 * - opcode: Pointer to store the integer value of the 2 most significant digits.
 * - str: 8-character string representing the input.
 * Returns 1 if parsing and conversion are successful, -1 otherwise.
 */
bool parse_instruction(instruction* instruction, char* str);

// Converts a string line into an instruction structure, Returns 1 if successful, -1 otherwise.
int line_to_instruction(char* line, instruction* inst, int line_index);


/*******************************************************/
/***************** Core Functions **********************/
/*******************************************************/

// Initializes the stats structure
void init_stats(stats** stat);

// Initializes the imem and prev_imem arrays in the core structure from a file.
void init_imem(core* cpu, const char* filename);

/*
 * Initializes the entire core structure.
 * - Sets pc to 0.
 * - Initializes registers and prev_registers to 0.
 * - Initializes imem and prev_imem according to to the file instructions.
 * - Initializes cache and prev_cache using their respective initialization function.
 */
core* core_initialization(int core_num, char* imem_filename);

// Extracts the low 9 bits and returns them as an int - used in jump instructions
int jump_to_pc(int imm);

// Copies one instruction structure to another
void copy_instruction(instruction* dest, instruction* src);

// Initializes the structure with the 5 instructions as stalls
instructions* create_instructions();

// Pipingline stages
// Performing the Fetch phase
void fetch (core* cpu, instruction* instruction);

// Performing the decode phase, Returns true if a jump should be performed and false otherwise
bool decode (core* cpu, instruction* instruction);

// Performing the Execute phase
void execute (core* cpu, instruction* instruction);

// Performing the Mem phase, if the operation was performed, it returns true, otherwise it returns false.
bool mem(core* cpu, instruction* instruction, cache_block data_from_memory, cache_block* data_to_memory, bool bus_ready);

/*
* Returns true if the lw operation is complete and the value is loaded from cache/memory
* if the operation is not complete false will be returned (no progress can be made)
* The function receives: 
* - a core that is currently working with the bus
* - an instruction that is currently in the mem phase with opcode == 16 (lw)
* - cache_block which will be the block that was brought from memory after enough cycles
* - a boolean variable data_valid which says that the block we received is indeed correct (used to simulate waiting for the bus)
* - Pointer to an integer that represents the number of words we received and when it is 0 - we received the entire block
* The function updates all the data in the first cycle the bus transmits the block
* in the next cycles it's waits 4 cycles to simulate receiving the block in parts
*/
bool lw_mem(core* cpu, instruction* instruction, cache_block data_from_memory, cache_block* data_to_memory, bool data_valid);

/*
* Returns true if the sw operation is complete and the value is stored in the cache/memory
* if the operation is not complete false will be returned (no progress can be made)
* The function receives: 
* - a core that is currently working with the bus
* - an instruction that is currently in the mem phase with opcode == 17 (sw)
* - cache_block which will be the block that was brought from memory after enough cycles (if needed)
* - a boolean variable bus_ready which says that we received data from the bus (used to simulate waiting for the bus)
* - Pointer to an integer that represents the number of words we received and when it is 0 - we received the entire block
* The function updates all the data in the first cycle the bus transmits the block
* in the next cycles it's waits 4 cycles to simulate receiving the block in parts
*/
bool sw_mem(core* cpu, instruction* instruction, cache_block data_from_memory, cache_block* data_to_memory, bool bus_ready);

// Performing the WB phase
void write_beck (core* cpu, instruction* instruction);

// performing one step in the core pipeline
// Calculates pipeline delays and updates instructions accordingly
void pipeline_step(FILE* core_trace_file, core* cpu, instructions* instructions, cache_block data_from_memory, cache_block* data_to_memory, bool bus_ready);

// Check if all instructions are stalls
bool done(core* cpu, instructions* instructions);

// Frees the core's memory including its cache
void free_core(core* cpu);

// Frees the structure with the 5 instructions
void free_instructions(instructions* instructions);

// turn instruction to stall
void turn_to_stall(instruction* instruction);

// Writes a line to the coreNUMtrace.txt file after each cycle
void write_line_to_core_trace_file(FILE* coretrace_filename, core* cpu, instructions* instructions);


/*******************************************************/
/*************** Create output files *******************/
/*******************************************************/

void create_output_files(core* cpu, char* regout_filename, char* stats_filename, char* dsram_filename, char* tsram_filename);

// Generates the file "regout.txt"
void create_regout_file(core* cpu, char* filename);

// Generates the file "stats.txt"
void create_stats_file(core* cpu, char* filename);

// Generates the file dsram.txt
void create_dsram_file(core* cpu, char* filename);

// Generates the file tsram.txt
void create_tsram_file(core* cpu, char* filename);

/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Test core with no memory opertions, executes all instructions in the core instruction memory
void run_core(core* cpu, main_memory* memory);

// Receives an opcode as int and returns its representation as a string.
char* opcode_to_string(int opcode);

// Receives an register index as int and returns its representation as a string
char* reg_to_string(int index);

// Receives an imm as int and returns its representation as a string
char* imm_to_string(int imm);

// create stall string
char* stall_string();

// create on line (char*) of the instruction
char* instruction_as_a_string(instruction* instruction);

// Prints the 8 digits of the instruction and the translation we performed 
void print_parse_instruction(instruction* instruction, char* str_instruction);

// Prints the contents of instruction memory
void print_imem(core* cpu);

// Prints the core status at a given moment.
void print_core_status(core* cpu);

// print 5 rows of the pipeline levels in hex to see it contennet
void print_pipline(instructions* instructions);

// Print the cycle, pc, halt flag and registers
void print_core(core* cpu);

// Prints a line in core_trace format (hex)
void print_core_trace_hex(core* cpu, instructions* instructions);




#endif // CORE_H
