#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sram.h"
#include "core.h"
#include "memory.h"


/*******************************************************/
/*************** Assembler Functions *******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int str_to_int(char* str) {
    return (int)strtol(str, NULL, 16); // Convert hex string to int
}

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
int parse_instruction(int* imm, int* rt, int* rs, int* rd, int* opcode, char* str) {
    if (strlen(str) != 8) {
        return -1;
    }
    // Allocate buffers for the components
    char str_imm[4];        // 3 digits + null terminator
    char str_rt[2];         // 1 digit + null terminator
    char str_rs[2];         // 1 digit + null terminator
    char str_rd[2];         // 1 digit + null terminator
    char str_opcode[3];     // 2 digits + null terminator

    // Extract substrings
    strncpy(str_imm, &str[5], 3);
    str_imm[3] = '\0';

    strncpy(str_rt, &str[4], 1);
    str_rt[1] = '\0';

    strncpy(str_rs, &str[3], 1);
    str_rs[1] = '\0';

    strncpy(str_rd, &str[2], 1);
    str_rd[1] = '\0';

    strncpy(str_opcode, &str[0], 2);
    str_opcode[2] = '\0';

    // Convert strings to integers
    *imm = str_to_int(str_imm);
    *rt = str_to_int(str_rt);
    *rs = str_to_int(str_rs);
    *rd = str_to_int(str_rd);
    *opcode = str_to_int(str_opcode);

    return 1; // Success
}

// Converts a string line into an instruction structure. 
// Returns 1 if successful, -1 otherwise.
int line_to_instruction(char* line, instruction* inst) {
    return parse_instruction(&inst->imm, &inst->rt, &inst->rs, &inst->rd, &inst->opcode, line);
}




/*******************************************************/
/***************** Core Functions **********************/
/*******************************************************/

// Initializes the imem and prev_imem arrays in the core structure from a file.
void imem_initialization(core* cpu, const char* filename) 
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char buffer[1024]; // Buffer for reading lines
    int line_index = 0;

    // Read lines from the file and convert them to instructions
    while (fgets(buffer, sizeof(buffer), file) && line_index < IMEM_SIZE) {
        // Remove newline character
        buffer[strcspn(buffer, "\n")] = '\0';

        // Skip empty lines or lines with only whitespace
        if (strlen(buffer) == 0 || strspn(buffer, " \t") == strlen(buffer)) {
            continue;
        }

        // Convert the line to an instruction and store it in imem
        if (line_to_instruction(buffer, &cpu->imem[line_index]) == 1) {
            // Copy the same instruction to prev_imem
            cpu->prev_imem[line_index] = cpu->imem[line_index];
            line_index++;
        } else {
            printf("Failed to parse line: %s\n", buffer);
        }
    }

    fclose(file);
}

/*
 * Initializes the entire core structure.
 * - Sets pc to 0.
 * - Initializes registers and prev_registers to 0.
 * - Initializes imem and prev_imem according to to the file instructions.
 * - Initializes sram and prev_sram using their respective initialization function.
 */
void core_initialization(core* cpu, const char* filename) 
{
    // Initialize the Program Counter (PC)
    cpu->pc = 0;

    // Initialize all registers to 0
    for (int i = 0; i < NUM_OF_REGISTERS; i++) {
        cpu->registers[i] = 0;
        cpu->prev_registers[i] = 0;
    }

    // Initialize imem and prev_imem with empty instructions
    for (int i = 0; i < IMEM_SIZE; i++) {
        cpu->imem[i] = (instruction){0, 0, 0, 0, 0};
        cpu->prev_imem[i] = (instruction){0, 0, 0, 0, 0};
    }

    // Allocate and initialize SRAM
    cpu->sram = (Cache*)malloc(sizeof(Cache));
    cpu->prev_sram = (Cache*)malloc(sizeof(Cache));
    if (cpu->sram && cpu->prev_sram) {
        cache_initialization(cpu->sram);
        cache_initialization(cpu->prev_sram);
    } else {
        perror("Failed to allocate memory for SRAM");
        exit(EXIT_FAILURE);
    }

    // Initialize the instruction memory (imem) using the provided file
    imem_initialization(cpu, filename);

    // Copy imem to prev_imem to match initial state
    for (int i = 0; i < IMEM_SIZE; i++) {
        cpu->prev_imem[i] = cpu->imem[i];
    }
}

// Extracts the low 9 bits and returns them as an int - used in jump instructions
int jump_to_pc(int imm) 
{
    int result = imm & 0x1FF; // 0x1FF = 0000000111111111 in binary
    return result;
}
/*
// Pipingline stages
void fetch (core* cpu, instruction* instruction) 
{
    if(cpu->halt_flag) { 
        return; 
    } 
    *instruction = cpu->imem[cpu->pc];
}

void decode (core* cpu, instruction* instruction, int* opcode, int* rt, int* rs, int* rd, int* imm)
{
    if(cpu->halt_flag) { 
        return; 
    }
    opcode = instruction->opcode;
    rt = instruction->rt;
    rs = instruction->rs;
    rd = instruction->rd;
    imm = instruction->imm;
}

void execute (core* cpu, main_memory* mem, Cache* cache, int opcode, int rt, int rs, int rd, int imm)
{   
    if(cpu->halt_flag) { 
        return; 
    }
    switch (opcode) {
        case 0:  return; // cpu->registers[rd] = cpu->registers[rs] + cpu->registers[rt];  // add:  R[rd] = R[rs] + R[rt]
        case 1:  return; // cpu->registers[rd] = cpu->registers[rs] - cpu->registers[rt];  // sub:  R[rd] = R[rs] - R[rt]
        case 2:  return; // cpu->registers[rd] = cpu->registers[rs] & cpu->registers[rt];  // and:  R[rd] = R[rs] & R[rt]
        case 3:  return; // cpu->registers[rd] = cpu->registers[rs] | cpu->registers[rt];  //  or:  R[rd] = R[rs] | R[rt]
        case 4:  return; // cpu->registers[rd] = cpu->registers[rs] ^ cpu->registers[rt];  // xor:  R[rd] = R[rs] ^ R[rt]
        case 5:  return; // cpu->registers[rd] = cpu->registers[rs] * cpu->registers[rt];  // mul:  R[rd] = R[rs] * R[rt]
        case 6:  return; // cpu->registers[rd] = cpu->registers[rs] << cpu->registers[rt]; // sll:  R[rd] = R[rs] << R[rt]
        case 7:  return; // cpu->registers[rd] = cpu->registers[rs] >> cpu->registers[rt]; // sra:  R[rd] = R[rs] << R[rt]
        case 8:  return; // cpu->registers[rd] = cpu->registers[rs] << cpu->registers[rt]; // srl:  R[rd] = R[rs] << R[rt] -  check this
        
        case 9:  if(cpu->registers[rs] == cpu->registers[rt]) { cpu->pc = jump_to_pc(cpu->registers[rd]); } // beq:  if(R[rs] == R[rt]) pc = R[rd][low bits 9:0]
        case 10: if(cpu->registers[rs] != cpu->registers[rt]) { cpu->pc = jump_to_pc(cpu->registers[rd]); } // bne:  if(R[rs] != R[rt]) pc = R[rd] [low bits 9:0]
        case 11: if(cpu->registers[rs] < cpu->registers[rt]) { cpu->pc = jump_to_pc(cpu->registers[rd]); }  // blt:  if(R[rs] < R[rt])  pc = R[rd] [low bits 9:0]
        case 12: if(cpu->registers[rs] > cpu->registers[rt]) { cpu->pc = jump_to_pc(cpu->registers[rd]); }  // bgt:  if(R[rs] > R[rt])  pc = R[rd] [low bits 9:0]
        case 13: if(cpu->registers[rs] <= cpu->registers[rt]) { cpu->pc = jump_to_pc(cpu->registers[rd]); } // ble:  if(R[rs] <= R[rt]) pc = R[rd] [low bits 9:0]
        case 14: if(cpu->registers[rs] >= cpu->registers[rt]) { cpu->pc = jump_to_pc(cpu->registers[rd]); } // ble:  if(R[rs] >= R[rt]) pc = R[rd] [low bits 9:0]
        case 15: cpu->registers[15] = (cpu->pc + 1);  cpu->pc = jump_to_pc(cpu->registers[rd]);             // lal:  R[15] = next instruction address, pc = R[rd][9:0]
        
        case 16: return; // lw: We'll deal with that later (in the MEM function).
        case 17: return; // sw: We'll deal with that later (in the MEM function).
        case 20: cpu->halt_flag = true; return;
        default: // opcode = stall or invalid opcode 
            return "unknown";
    }
}

void mem (core* cpu, main_memory* mem, Cache* cache, int opcode, int rt, int rs, int rd)
{
    
    if(opcode == 16) { // lw:  R[rd] = MEM[R[rs]+R[rt]]

    }
    if(opcode == 17) { // sw:  MEM[R[rs]+R[rt]] = R[rd]

    } 
}


void write_beck (core* cpu, int opcode, int rt, int rs, int rd, int imm)
{
    switch (opcode) {
        case 0:  cpu->registers[rd] = cpu->registers[rs] + cpu->registers[rt];  // add:  R[rd] = R[rs] + R[rt]
        case 1:  cpu->registers[rd] = cpu->registers[rs] - cpu->registers[rt];  // sub:  R[rd] = R[rs] - R[rt]
        case 2:  cpu->registers[rd] = cpu->registers[rs] & cpu->registers[rt];  // and:  R[rd] = R[rs] & R[rt]
        case 3:  cpu->registers[rd] = cpu->registers[rs] | cpu->registers[rt];  //  or:  R[rd] = R[rs] | R[rt]
        case 4:  cpu->registers[rd] = cpu->registers[rs] ^ cpu->registers[rt];  // xor:  R[rd] = R[rs] ^ R[rt]
        case 5:  cpu->registers[rd] = cpu->registers[rs] * cpu->registers[rt];  // mul:  R[rd] = R[rs] * R[rt]
        case 6:  cpu->registers[rd] = cpu->registers[rs] << cpu->registers[rt]; // sll:  R[rd] = R[rs] << R[rt]
        case 7:  cpu->registers[rd] = cpu->registers[rs] >> cpu->registers[rt]; // sra:  R[rd] = R[rs] << R[rt]
        case 8:  cpu->registers[rd] = cpu->registers[rs] << cpu->registers[rt]; // srl:  R[rd] = R[rs] << R[rt] -  check this
        case 9:  return; // beq: We'll already deal with that (in the EXE function).
        case 10: return; // bne: We'll already deal with that (in the EXE function).
        case 11: return; // blt: We'll already with that (in the EXE function).
        case 12: return; // bgt: We'll already with that (in the EXE function).
        case 13: return; // ble: We'll already with that (in the EXE function).
        case 14: return; // ble: We'll already with that (in the EXE function).
        case 15: return; // jal: We'll already with that (in the EXE function).  
        case 16: return; // lw: We'll already with that (in the MEM function).
        case 17: return; // sw: We'll already with that (in the MEM function).
        case 20: cpu->halt_flag = true; return;
        default: // opcode = stall or invalid opcode 
            return "unknown";
    }
}
*/

/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Receives an opcode as int and returns its representation as a string.
const char* opcode_to_string(int opcode) {
    switch (opcode) {
        case 0: return "add";
        case 1: return "sub";
        case 2: return "and";
        case 3: return "or";
        case 4: return "xor";
        case 5: return "mul";
        case 6: return "sll";
        case 7: return "sra";
        case 8: return "srl";
        case 9: return "beq";
        case 10: return "bne";
        case 11: return "blt";
        case 12: return "bgt";
        case 13: return "ble";
        case 14: return "bge";
        case 15: return "jal";
        case 16: return "lw";
        case 17: return "sw";
        case 20: return "halt";
        default:
            return "unknown"; // For invalid opcode values
    }
}

// Receives an register index as int and returns its representation as a string
const char* reg_to_string(int index) {
    switch (index) {
        case 0: return "$zero";
        case 1: return "$imm";
        case 2: return "$r2";
        case 3: return "$r3";
        case 4: return "$r4";
        case 5: return "$r5";
        case 6: return "$r6";
        case 7: return "$r7";
        case 8: return "$r8";
        case 9: return "$r9";
        case 10: return "$r10";
        case 11: return "$r11";
        case 12: return "$r12";
        case 13: return "$r13";
        case 14: return "$r14";
        case 15: return "$r15";
        default:
            return "unknown"; // For invalid register index
    }
}

// Receives an imm as int and returns its representation as a string
char* imm_to_string(int imm) {
    // Allocate a buffer large enough to hold the largest integer as a string
    // Maximum size for a 32-bit integer as a string is 11 characters (-2147483648) + null terminator
    char* result = (char*)malloc(12 * sizeof(char));
    if (!result) {
        perror("Failed to allocate memory for imm_to_string");
        return NULL;
    }

    // Convert the integer to a string
    snprintf(result, 12, "%d", imm);
    return result;
}

// create on line of the instruction
char* instruction_as_a_string(int opcode, int rt, int rs, int rd, int imm) {
    // Convert opcode, registers, and immediate value to strings
    const char* opcode_str = opcode_to_string(opcode);
    const char* rt_str = reg_to_string(rt);
    const char* rs_str = reg_to_string(rs);
    const char* rd_str = reg_to_string(rd);
    char* imm_str = imm_to_string(imm);

    if (!imm_str) {
        perror("Failed to allocate memory for immediate value");
        return NULL;
    }

    // Calculate the size of the final string
    size_t size = strlen(opcode_str) + strlen(rt_str) + strlen(rs_str) +
                  strlen(rd_str) + strlen(imm_str) + 16; // Extra space for formatting

    // Allocate memory for the final string
    char* result = (char*)malloc(size * sizeof(char));
    if (!result) {
        perror("Failed to allocate memory for instruction string");
        free(imm_str);
        return NULL;
    }

    // Format the instruction string
    snprintf(result, size, "%s %s, %s, %s, %s", opcode_str, rd_str, rs_str, rt_str, imm_str);

    // Free allocated memory
    free(imm_str);

    return result;
}

// Prints the 8 digits of the instruction and the translation we performed 
void print_parse_instruction(char* instruction, int* imm, int* rt, int* rs, int* rd, int* opcode) {
    // Call parse_instruction
    int result = parse_instruction(imm, rt, rs, rd, opcode, instruction);

    // Check if parsing was successful
    if (result == 1) {
        // Print the parsed values
        // Convert parsed instruction to a string
        char* parse_instruction = instruction_as_a_string(*opcode, *rt, *rs, *rd, *imm);
        if (parse_instruction) {
            printf("instruction: \"%s\" ==> %s\n", instruction, parse_instruction);
            free(parse_instruction); // Free the allocated memory
        } else {
            printf("Error: Failed to create parsed instruction string.\n");
        }
    } else {
        // Print an error message if parsing failed
        printf("Error: Failed to parse instruction \"%s\".\n", instruction);
    }
}


void print_imem(const core* cpu) {
    printf("Instruction Memory:\n");

    for (int i = 0; i < IMEM_SIZE; i++) {
        const instruction* inst = &cpu->imem[i]; // Use const

        // Check if the instruction is non-empty
        if (inst->opcode != 0 || inst->rt != 0 || inst->rs != 0 || inst->rd != 0 || inst->imm != 0) {
            char* inst_str = instruction_as_a_string(inst->opcode, inst->rt, inst->rs, inst->rd, inst->imm);
            if (inst_str) {
                printf("PC %d: %s\n", i, inst_str);
                free(inst_str);
            } else {
                printf("Error creating string for instruction at index %d.\n", i);
            }
        }
    }

    printf("End of Instruction Memory.\n\n");
}




