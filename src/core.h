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
#define BUS_DELAY 17  // Delay until the first word is retrieved from memory (16 + 1)
#define BLOCK_DELAY 4 // Delay until the entire block is received
#define EXTRA_DELAY 4 // Delay until the entire block from the cache moves to memory


/*******************************************************/
/*********************  Structs ************************/
/*******************************************************/

// Structure of a single instruction
typedef struct {
    int pc;
    int opcode;
    int rt;
    int rs;
    int rd;
    int imm;
    int ALU_result;  // store the ALU calculation value from the EXE phase
    int bus_delay;   // in the case of a memory operation, it's the number of cycles it will wait for the bus
    int block_delay; // in the case of a memory operation, it's the number of cycles it will wait to receive the entire block
    int extra_delay; // in the case of a memory operation where a block is moved from cache to the memory, 
                     // this is the additional number of cycles that the instruction will wait
} instruction;

// A set of 5 instructions currently in the pipeline
typedef struct {
    instruction* fetch;
    instruction* decode;
    instruction* execute;
    instruction* memory;
    instruction* write_back;
} instructions;

// Structure of core statistics - for the stats file
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
    int registers[NUM_OF_REGISTERS];
    instruction imem[IMEM_SIZE];
    Cache* cache;
    stats* stats;
    // flags
    bool done;         // if true, signals to the processor that this core finish the imem instructions
    bool need_the_bus; // if true, signals to the processor that this core needs the bus
    bool hold_the_bus; // if true, Signals to the processor that this core currently owns the bus
    // files names the core need to create
    char* imem_filename;
    char* coretrace_filename;
    char* regout_filename;
    char* stats_filename;
    char* dsram_filename;
    char* tsram_filename;
    // files 
    FILE* coretrace_file; // the only file the core need to update each step

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

// Converts a hax string line into an instruction structure, Returns 1 if successful, -1 otherwise.
int line_to_instruction(char* line, instruction* inst, int line_index);


/*******************************************************/
/***************** Core Functions **********************/
/*******************************************************/

// Initializes the stats structure
void init_stats(stats** stat);

// Initializes the imem array in the core structure, take the data from the imem file
void init_imem(core* cpu);

/*
 * Initializes the entire core structure:
 * - Sets pc and cycle to 0.
 * - Initializes output filenames.
 * - Initializes all registers to zeros.
 * - Initializes the cache with zeros and INVALID state.
 * - Initializes the instruction memory from the imem file.
 * - Initializes the stats struct.
 */
core* init_core(int core_num, char* imem_str, char* coretrace_str, char* regout_str, char* stats_str, char* dsram_str, char* tsram_str);

// Extracts the low 9 bits and returns them as an int - used in jump instructions
int jump_to_pc(int imm);

// Copies one instruction structure to another
void copy_instruction(instruction* dest, instruction* src);

// Creates a structure of 5 instructions and returns a pointer to it (used by the pipeline)
instructions* create_instructions();

// Performing the Fetch phase
void fetch (core* cpu, instruction* instruction);

// Performing the decode phase, Returns true if a jump should be performed and false otherwise
bool decode (core* cpu, instruction* instruction);

// Performing the Execute phase
void execute (core* cpu, instruction* instruction);

// Performing the Mem phase, do nothing until the last cycle of the sum of the delays in the delay fields
bool mem(core* cpu, instruction* instruction, cache_block* data_from_memory, uint32_t* address, bool* extra_delay);

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
bool lw(core* cpu, instruction* instruction, cache_block* data_from_memory, uint32_t* address, bool* extra_delay);

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
bool sw(core *cpu, instruction *instruction, cache_block *data_from_memory, uint32_t *address, bool *extra_delay);

// Performing the WB phase
void write_beck (core* cpu, instruction* instruction);

// performing one step in the core pipeline, Calculates pipeline delays and updates instructions accordingly
cache_block* pipeline_step(core* cpu, instructions* instructions, cache_block* data_from_memory, uint32_t* address, bool* extra_delay);

// Check if all instructions are stalls (the core finish running)
bool done(core* cpu, instructions* instructions);

// Frees the core's memory including its cache and the stats
void free_core(core* cpu);

// Frees the structure with the 5 instructions
void free_instructions(instructions* instructions);

// turn instruction to stall
void turn_to_stall(instruction* instruction);

// turn instruction to halt
void turn_to_halt(instruction* instruction);


/*******************************************************/
/*************** Create output files *******************/
/*******************************************************/

// Writes a line to the coretrace.txt file after each cycle
void write_line_to_core_trace_file(core* cpu, instructions* instructions);

// Generates all the output files (Except of coretrace.txt) at once
void create_output_files(core* cpu);

// Generates the file "regout.txt"
void create_regout_file(core* cpu);

// Generates the file "stats.txt"
void create_stats_file(core* cpu);

// Generates the file dsram.txt
void create_dsram_file(core* cpu);

// Generates the file tsram.txt
void create_tsram_file(core* cpu);

// Opens a single file and returns an error if not opened.
void open_file(FILE** f, char* filename, char* c);


/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Test single core, executes all instructions in the core instruction memory
void run_core(core* cpu, main_memory* memory);

// create instruction as a string
char* get_instruction_as_a_string(instruction* instr);

// Prints the contents of instruction memory
void print_imem(core* cpu);

// Prints the core status at a given moment.
void print_core_status(core* cpu);

// Prints a line in core_trace format (hex)
void print_core_trace_hex(core* cpu, instructions* instructions);

// print 5 rows of the pipeline levels in hex to see it contennet
void print_pipline(instructions* instructions);




#endif // CORE_H
