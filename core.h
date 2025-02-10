#ifndef CORE_H
#define CORE_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sram.h"
#include "memory.h"



/*******************************************************/
/****************** Core sizes setting *****************/
/*******************************************************/

#define NUM_OF_REGISTERS 16
#define IMEM_SIZE 1024   // 1024 lines of 32 bits
#define HALT_OPCODE 20
#define STALL_OPCODE 21

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
} instruction;


// Core structures
typedef struct {
    int pc;
    int cycle;
    int core_number;
    bool halt_flag; // halt_flag = 1 if we get "halt" opcode - the core know to finish and stop
    // this cycle
    int registers[NUM_OF_REGISTERS];
    instruction imem[IMEM_SIZE];
    Cache* cache;
    // prev cycle
    int prev_registers[NUM_OF_REGISTERS];
    instruction prev_imem[IMEM_SIZE];
    Cache* prev_cache;

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

// Initializes the imem and prev_imem arrays in the core structure from a file.
void imem_initialization(core* cpu, const char* filename);

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

// Pipingline stages
// Performing the Fetch phase
void fetch (core* cpu, instruction* f_instruction);
//void fetch (core* cpu, instruction* instruction);

// Performing the decode phase, Returns true if a jump should be performed and false otherwise
bool decode (core* cpu, instruction* instruction);
// void decode (core* cpu, instruction* instruction);

// Performing the Execute phase
void execute (core* cpu, instruction* instruction);

// Performing the Mem phase
void mem(core* cpu, instruction* instruction, main_memory* memory);

// Performing the WB phase
void write_beck (core* cpu, instruction* instruction);

// performing one step in the core pipeline
// Calculates pipeline delays and updates instructions accordingly
void pipeline_step(FILE* core_trace_file, core* cpu, main_memory* memory, instruction* f_instruction, instruction* d_instruction,
                    instruction* exe_instruction, instruction* mem_instruction, instruction* wb_instruction);

// Executes all instructions in the core instruction memory
void run_core(core* cpu, main_memory* memory, char* filename);

// Check if all instructions are stalls
bool done(instruction* inst1, instruction* inst2, instruction* inst3, instruction* inst4, instruction* inst5);

// Frees the core's memory including its cache
void free_core(core* cpu);

// turn instruction to stall
void turn_to_stall(instruction* instruction);

// Writes a line to the coreNUMtrace.txt file after each cycle
void write_line_to_core_trace_file(FILE* coretrace_filename, core* cpu, instruction* f, instruction* d, instruction* e, instruction* m, instruction* w);

// Generates the file regout.txt with the register values ​​at the end of the run
void create_regout(core* cpu);



/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Receives an opcode as int and returns its representation as a string.
char* opcode_to_string(int opcode);

// Receives an register index as int and returns its representation as a string
char* reg_to_string(int index);

// Receives an imm as int and returns its representation as a string
char* imm_to_string(int imm);

// create on line (char*) of the instruction
char* instruction_as_a_string(instruction* instruction);

// Prints the 8 digits of the instruction and the translation we performed 
void print_parse_instruction(instruction* instruction, char* str_instruction);

// Prints the contents of instruction memory
void print_imem(core* cpu);

// Prints the core status at a given moment.
void print_core_status(core* cpu);

// print 5 rows of the pipeline levels in hex to see it contennet
void print_pipline(instruction* fetch_inst, instruction* decode_inst, instruction* exe_inst, instruction* mem_inst, instruction* wb_inst);

// Print the cycle, pc, halt flag and registers
void print_core(core* cpu);

// Prints a line in core_trace format (hex)
void print_core_trace_hex(core* cpu, instruction* f, instruction* d, instruction* e, instruction* m, instruction* w);




#endif // CORE_H
