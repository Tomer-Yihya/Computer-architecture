#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



/*******************************************************/
/****************** Core sizes setting *****************/
/*******************************************************/

#define NUM_OF_REGISTERS 16
#define IMEM_SIZE 1024   // 1024 lines of 32 bits

/*******************************************************/
/*********************  Structs ************************/
/*******************************************************/

// Assembler structures
typedef struct {
    int opcode;
    int rt;
    int rs;
    int rd;
    int imm;
} instruction;


// Core structures
typedef struct {
    int pc;
    int cycle;
    bool halt_flag; // halt_flag = 1 if we get "halt" opcode - the core know to finish and stop
    // this cycle
    int registers[NUM_OF_REGISTERS];
    instruction imem[IMEM_SIZE];
    Cache* sram;
    // prev cycle
    int prev_registers[NUM_OF_REGISTERS];
    instruction prev_imem[IMEM_SIZE];
    Cache* prev_sram;

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
int parse_instruction(int* imm, int* rt, int* rs, int* rd, int* opcode, char* str);


/*******************************************************/
/***************** Core Functions **********************/
/*******************************************************/

// Initializes the imem and prev_imem arrays in the core structure from a file.
void imem_initialization(core* cpu, const char* filename);







/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

const char* opcode_to_string(int opcode);

const char* reg_to_string(int index);

char* imm_to_string(int imm);

char* instruction_as_a_string(int opcode, int rt, int rs, int rd, int imm);

void print_parse_instruction(char* instruction, int* imm, int* rt, int* rs, int* rd, int* opcode);

void print_imem(const core* cpu);