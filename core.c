#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sram.h"
#include "core.h"
#include "memory.h"



/*******************************************************/
/*************** Assembler Functions *******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int str_to_int(char* str) 
{
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
bool parse_instruction(instruction* instruction, char* str) 
{
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
    instruction->imm = str_to_int(str_imm);
    instruction->rt = str_to_int(str_rt);
    instruction->rs = str_to_int(str_rs);
    instruction->rd = str_to_int(str_rd);
    instruction->opcode = str_to_int(str_opcode);

    return 1; // Success
}

// Converts a string line into an instruction structure, Returns 1 if successful, -1 otherwise.
int line_to_instruction(char* line, instruction* inst, int line_index) {
    inst->pc = line_index;
    return parse_instruction(inst, line);
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
    while (fgets(buffer, sizeof(buffer), file) && line_index < IMEM_SIZE - 1) {
        // Remove newline character
        buffer[strcspn(buffer, "\n")] = '\0';
        // Skip empty lines or lines with only whitespace
        if (strlen(buffer) == 0 || strspn(buffer, " \t") == strlen(buffer)) {
            continue;
        }
        // Convert the line to an instruction and store it in imem
        if (line_to_instruction(buffer, &cpu->imem[line_index], line_index) == 1) {
            // Copy the same instruction to prev_imem
            cpu->prev_imem[line_index] = cpu->imem[line_index];
            line_index++;
        } else {
            printf("Failed to parse line: %s\n", buffer);
        }
    }
    fclose(file);
    // Add halt instruction to the last line of imem
    if (line_index < IMEM_SIZE) {
        cpu->imem[line_index].opcode = HALT_OPCODE;  // halt opcode == 20
        cpu->imem[line_index].rt = 0;
        cpu->imem[line_index].rs = 0;
        cpu->imem[line_index].rd = 0;
        cpu->imem[line_index].imm = 0;
        cpu->imem[line_index].pc = -1;
        cpu->imem[line_index].ALU_result = 0;
        // Copy the halt instruction to prev_imem as well
        cpu->prev_imem[line_index] = cpu->imem[line_index];
    }
    int i = 5;
    line_index++;
    while(line_index < IMEM_SIZE && i > 0) {
        cpu->imem[line_index].opcode = STALL_OPCODE;  // stall opcode == 21
        cpu->imem[line_index].rt = 0;
        cpu->imem[line_index].rs = 0;
        cpu->imem[line_index].rd = 0;
        cpu->imem[line_index].imm = 0;
        cpu->imem[line_index].pc = -1;
        cpu->imem[line_index].ALU_result = 0;
        // Copy the halt instruction to prev_imem as well
        cpu->prev_imem[line_index] = cpu->imem[line_index];
        line_index++;
        i--;
    }

}

/*
 * Initializes the entire core structure.
 * - Sets pc to 0.
 * - Initializes registers and prev_registers to 0.
 * - Initializes imem and prev_imem according to to the file instructions.
 * - Initializes cache and prev_cache using their respective initialization function.
 */
core* core_initialization(int core_num, char* imem_filename) 
{
    // Initialize the Program Counter (PC)
    core* cpu = malloc(sizeof(core));
    if (!cpu) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }
    cpu->pc = 0;
    cpu->cycle = 0;
    cpu->core_number = core_num;
    cpu->halt_flag = false;
    // Initialize all registers to 0
    for (int i = 0; i < NUM_OF_REGISTERS; i++) {
        cpu->registers[i] = 0;
        cpu->prev_registers[i] = 0;
    }
    // Initialize imem and prev_imem with empty instructions
    for (int i = 0; i < IMEM_SIZE; i++) {
        cpu->imem[i] = (instruction){0, 0, 0, 0, 0, 0, 0};
        cpu->prev_imem[i] = (instruction){0, 0, 0, 0, 0, 0, 0};
    }

    // Allocate and initialize SRAM
    cpu->cache = (Cache*)malloc(sizeof(Cache));
    cpu->prev_cache = (Cache*)malloc(sizeof(Cache));
    if (cpu->cache && cpu->prev_cache) {
        cache_initialization(cpu->cache);
        cache_initialization(cpu->prev_cache);
    } 
    else {
        perror("Failed to allocate memory for SRAM");
        exit(EXIT_FAILURE);
    }
    // Initialize the instruction memory (imem) using the provided file
    imem_initialization(cpu, imem_filename);
    // Copy imem to prev_imem to match initial state
    for (int i = 0; i < IMEM_SIZE; i++) {
        cpu->prev_imem[i] = cpu->imem[i];
    }
    return cpu;
}

// Extracts the low 9 bits and returns them as an int - used in jump instructions
int jump_to_pc(int imm) 
{
    int result = imm & 0x1FF; // 0x1FF = 0000000111111111 in binary
    return result;
}

// Copies one instruction structure to another
void copy_instruction(instruction* dest, instruction* src)
{
    if(!dest || !src){
        return;
    }
    dest->pc = src->pc;
    dest->opcode = src->opcode;
    dest->rt = src->rt;
    dest->rs = src->rs;
    dest->rd = src->rd;
    dest->imm = src->imm;
    dest->ALU_result = src->ALU_result;
}

// Pipingline stages
// Performing the Fetch phase
void fetch (core* cpu, instruction* instruction) 
{
    if(instruction->opcode == STALL_OPCODE && cpu->pc != 0){
    //if(instruction->opcode == HALT_OPCODE || (instruction->opcode == STALL_OPCODE && cpu->pc != 0)){
        cpu->pc++;
        return;
    }
    if(cpu->pc < IMEM_SIZE) {
        *instruction = cpu->imem[cpu->pc];
        instruction->ALU_result = 0;
    }
    else{
        turn_to_stall(instruction);
    }
    cpu->pc++;
}

// Performing the decode phase, Returns true if a jump should be performed and false otherwise
bool decode (core* cpu, instruction* instruction)
{
    if(cpu->halt_flag || instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) { 
        return false; 
    }
    int rt = instruction->rt;
    int rs = instruction->rs;
    int rd = instruction->rd;
    // One of the register values ​​is invalid, change the opcode to stall so that nothing is executed in the following steps
    if(0 > rd || 16 < rd || 0 > rs || 16 < rs || 0 > rt || 16 < rt) { 
        turn_to_stall(instruction);
        return false; 
    }
    // Make sure the imm is indeed up to 12 bits in size
    instruction->imm = instruction->imm & 0xFFF;
    // update register $imm to the imm value (just for this calc, we will restore it after)
    int imm = cpu->registers[1];
    cpu->registers[1] = instruction->imm;
    // branch resolution (if needed)
    int opcode = instruction->opcode;
    if(opcode >= 9 && opcode <= 15){
        // new_pc = R[rd][9:0] (used in case we jump)
        int new_pc = jump_to_pc(cpu->registers[rd]); // new_pc = R[rd][9:0]
        // update register $imm to the imm value (just for this calc, we will restore it after)
        cpu->registers[1] = instruction->imm;
        switch (opcode) {
            // beq:  if(R[rs] == R[rt]) pc = R[rd][low bits 9:0]
            case 9:  if(cpu->registers[rs] == cpu->registers[rt]) { cpu->pc = new_pc; } break;
            // bne:  if(R[rs] != R[rt]) pc = R[rd] [low bits 9:0]
            case 10: if(cpu->registers[rs] != cpu->registers[rt]) { cpu->pc = new_pc; } break;
            // blt:  if(R[rs] < R[rt])  pc = R[rd] [low bits 9:0]
            case 11: if(cpu->registers[rs] < cpu->registers[rt])  { cpu->pc = new_pc; } break; 
            // bgt:  if(R[rs] > R[rt])  pc = R[rd] [low bits 9:0]
            case 12: if(cpu->registers[rs] > cpu->registers[rt])  { cpu->pc = new_pc; } break; 
            // ble:  if(R[rs] <= R[rt]) pc = R[rd] [low bits 9:0]
            case 13: if(cpu->registers[rs] <= cpu->registers[rt]) { cpu->pc = new_pc; } break; 
            // ble:  if(R[rs] >= R[rt]) pc = R[rd] [low bits 9:0]
            case 14: if(cpu->registers[rs] >= cpu->registers[rt]) { cpu->pc = new_pc; } break; 
            // jal:  R[15] = next instruction address, pc = R[rd][9:0]  
            case 15: cpu->registers[15] = (cpu->pc + 1);  cpu->pc = new_pc; break;
            default:  
                break;
        }
        // restore the $r1 value
        cpu->registers[1] = imm;
        return true;
    }
    // restore the $r1 value
    cpu->registers[1] = imm;
    return false;
}

// Performing the Execute phase
void execute (core* cpu, instruction* instruction)
{   
    if(cpu->halt_flag || instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) { 
        //turn_to_stall(instruction);
        return; 
    }
    int opcode = instruction->opcode;
    int rt = instruction->rt;
    int rs = instruction->rs;
    // update register $imm to the imm value (just for this calc, we will restore it after)
    int imm = cpu->registers[1];
    cpu->registers[1] = instruction->imm;
    switch (opcode) {
        case 0:  instruction->ALU_result = cpu->registers[rs] + cpu->registers[rt]; return;  // add:  R[rd] = R[rs] + R[rt]
        case 1:  instruction->ALU_result = cpu->registers[rs] - cpu->registers[rt]; return;  // sub:  R[rd] = R[rs] - R[rt]
        case 2:  instruction->ALU_result = cpu->registers[rs] & cpu->registers[rt]; return;  // and:  R[rd] = R[rs] & R[rt]
        case 3:  instruction->ALU_result = cpu->registers[rs] | cpu->registers[rt]; return;  //  or:  R[rd] = R[rs] | R[rt]
        case 4:  instruction->ALU_result = cpu->registers[rs] ^ cpu->registers[rt]; return;  // xor:  R[rd] = R[rs] ^ R[rt]
        case 5:  instruction->ALU_result = cpu->registers[rs] * cpu->registers[rt]; return;  // mul:  R[rd] = R[rs] * R[rt]
        case 6:  instruction->ALU_result = cpu->registers[rs] << cpu->registers[rt]; return; // sll:  R[rd] = R[rs] << R[rt]
        case 7:  instruction->ALU_result = cpu->registers[rs] >> cpu->registers[rt]; return; // sra:  R[rd] = R[rs] >> R[rt]
        case 8:  instruction->ALU_result = (uint32_t)cpu->registers[rs] >> cpu->registers[rt]; return; // srl: R[rd] = R[rs] >> R[rt] (Logical shift)
        case 9:  return; // beq:  Handled in the Decode phase
        case 10: return; // bne:  Handled in the Decode phase
        case 11: return; // blt:  Handled in the Decode phase
        case 12: return; // bgt:  Handled in the Decode phase
        case 13: return; // ble:  Handled in the Decode phase
        case 14: return; // ble:  Handled in the Decode phase
        case 15: return; // jal:  Handled in the Decode phase  
        case 16: instruction->ALU_result = cpu->registers[rs] + cpu->registers[rt]; return; // lw: Prepares the result (to the MEM phase)
        case 17: instruction->ALU_result = cpu->registers[rs] + cpu->registers[rt]; return; // sw: Prepares the result (to the MEM phase)
        case 20: cpu->halt_flag = true; return;
        default: // opcode = stall or invalid opcode 
            return;
    }
    // restore the $r1 value
    cpu->registers[1] = imm;
}

// Performing the Mem phase
void mem(core* cpu, instruction* instruction, main_memory* memory) {
    
    if (!cpu || !instruction || !memory) {
        printf("Error: Null pointer passed to mem\n");
        return;
    }
    // No memory operation needed
    if ((instruction->opcode != 16 && instruction->opcode != 17) 
        || instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) {
        return; 
    }
    int data = cpu->registers[instruction->rt] + cpu->registers[instruction->rs];
    uint32_t address = (uint32_t)data;
    uint32_t offset = address % BLOCK_SIZE;
    uint32_t tag = address / (BLOCK_SIZE * NUM_OF_BLOCKS);
    // Blocks for search
    cache_block cache_block;
    memory_block mem_block;
    // update register $imm to the imm value (just for this calc, we will restore it after)
    cpu->registers[1] = instruction->imm;
    // lw: R[rd] = MEM[R[rs]+R[rt]]
    if (instruction->opcode == 16) { 
        // Cache hit
        if (search_block(cpu->cache, address, &cache_block)) {
            cpu->registers[instruction->rd] = cache_block.data[offset];
        } 
        // Cache miss
        else {
            mem_block = get_block(memory, tag);
            cpu->registers[instruction->rd] = mem_block.data[offset];
            // create block to insert the cache
            cache_block.data[offset] = cpu->registers[instruction->rd];
            cache_block.state = SHARED;
            cache_block.tag = tag;
            insert_block(cpu->cache, address, &cache_block);
        }
    }
    // sw: MEM[R[rs]+R[rt]] = R[rd]
    if (instruction->opcode == 17) {
        if (search_block(cpu->cache, address, &cache_block)) {
            cache_block.data[offset] = cpu->registers[instruction->rd];
            cache_block.state = MODIFIED;
            insert_block(cpu->cache, address, &cache_block);
        } 
        else {
            mem_block = get_block(memory, tag);
            // create block to insert the cache
            cache_block.state = SHARED;
            cache_block.tag = tag;
            // copy the data
            for (int i = 0; i < BLOCK_SIZE; i++) {
                cache_block.data[i] = mem_block.data[i];
            }
            // insert the block to the cache
            insert_block(cpu->cache, address, &cache_block);
        }
    }
}

// Performing the WB phase
void write_beck (core* cpu, instruction* instruction)
{
    if(instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) {
        return;
    }
    cpu->registers[1] = instruction->imm;
    // Do not perform an R-type (arithmetic operation) into the $ziro register.
    int opcode = instruction->opcode;
    int rd = instruction->rd;
    if((0 <= opcode && opcode <= 8  && (rd == 0 || rd == 1)) || opcode > 8){
        return;
    }
    // update register $imm to the imm value
    // perform only R-type (arithmetic operation), if opcode isn't R-type do nothing
    if(opcode >= 0 && opcode <= 8) {
        cpu->registers[rd] = instruction->ALU_result;
    }
    // sw - The value fetched from memory in the Mem phase is written to the register.
    if(opcode == 17) {
        cpu->registers[rd] = instruction->ALU_result;
    }
    // Update PC address
    //cpu->registers[15] = cpu->pc;
}

// performing one step in the core pipeline
// Calculates pipeline delays and updates instructions accordingly
void pipeline_step(FILE* core_trace_file, core* cpu, main_memory* memory, instruction* f_instruction, instruction* d_instruction,
    instruction* exe_instruction, instruction* mem_instruction, instruction* wb_instruction) {
    
    bool forward_fetch = true;
    bool forward_decode = true;
    bool forward_execute = true;
    bool forward_memory = true;
    // We have reached the halt command. We will continue until the pipeline is emptied.
    if(exe_instruction->opcode == HALT_OPCODE) {
        turn_to_stall(f_instruction);
        turn_to_stall(d_instruction);
    }
    // Data Hazard: EXE `$rd` is used as `$rs` or `$rt` in Decode → Insert stall
    bool data_hazard_decode_and_exe = (exe_instruction->rd == d_instruction->rs || exe_instruction->rd == d_instruction->rt);
    // Data Hazard: MEM `$rd` is used as `$rs` or `$rt` in Decode → Insert stall
    bool data_hazard_decode_and_mem = (mem_instruction->rd == d_instruction->rs || mem_instruction->rd == d_instruction->rt);
    // Data Hazard: WB isn't finish and `$rd` is used as `$rs` or `$rt` in Decode → Insert stall
    bool data_hazard_decode_and_wb = ( wb_instruction->rd == d_instruction->rs || wb_instruction->rd == d_instruction->rt);
    if(mem_instruction->opcode < 9){
        if(data_hazard_decode_and_exe || data_hazard_decode_and_mem || data_hazard_decode_and_wb){
            forward_fetch = false;
            forward_decode = false;
        }
    }
   // Load Hazard: `LW` in EXE, `$rd` used in Decode/Execute/WB before WB → Insert stall
    if (exe_instruction->opcode == 16){
        if(data_hazard_decode_and_exe || data_hazard_decode_and_mem || data_hazard_decode_and_wb) {
            forward_fetch = false;
            forward_decode = false;
            forward_execute = false;
        }
    }
    // Memory Hazard (cache miss)  → Insert 16 stalls
    if (mem_instruction->opcode == 16) {
        forward_fetch = false;
        forward_decode = false;
        forward_execute = false;
        forward_memory = false;
    }
    // Performing the actions
    fetch(cpu, f_instruction);
    int prev_pc = cpu->pc;
    bool jump_taken = decode(cpu, d_instruction);
    execute(cpu, exe_instruction);
    mem(cpu, mem_instruction, memory);
    //print_core_trace_hex(cpu, f_instruction, d_instruction, exe_instruction, mem_instruction, wb_instruction);
    write_line_to_core_trace_file(core_trace_file, cpu, f_instruction, d_instruction, exe_instruction, mem_instruction, wb_instruction);
    write_beck(cpu, wb_instruction);
    cpu->cycle++;
    // Advancing the stages in the core pipeline
    if(forward_memory) { copy_instruction(wb_instruction, mem_instruction);  }
    else {  
        turn_to_stall(wb_instruction); 
    }
    if(forward_execute){ copy_instruction(mem_instruction, exe_instruction); }
    else {  
        turn_to_stall(mem_instruction); 
    }
    if(forward_decode) { copy_instruction(exe_instruction, d_instruction);   }
    else { // fetch the same instruction again and hold decode in the same place
        if(jump_taken) { 
            cpu->pc = prev_pc; 
        }
        turn_to_stall(exe_instruction); // insert stall
    }
    if(forward_fetch)  { copy_instruction(d_instruction, f_instruction);     }
    else{
        cpu->pc--;
        copy_instruction(f_instruction, d_instruction);
    }
}

// Executes all instructions in the core instruction memory
void run_core(core* cpu, main_memory* memory, char* filename) {

    FILE* coretrace_file;
    if(cpu->core_number == 0) { coretrace_file = fopen("core0trace.txt", "w"); }
    if(cpu->core_number == 1) { coretrace_file = fopen("core1trace.txt", "w"); }
    if(cpu->core_number == 2) { coretrace_file = fopen("core2trace.txt", "w"); }
    if(cpu->core_number == 3) { coretrace_file = fopen("core3trace.txt", "w"); }
    if (!coretrace_file) {
        printf("Error: Failed to open core%dtrace.txt for writing.\n", cpu->core_number);
        return;
    }
    // Allocate memory for the instructions
    instruction* f_instruction = malloc(sizeof(instruction));
    instruction* d_instruction = malloc(sizeof(instruction));
    instruction* exe_instruction = malloc(sizeof(instruction));
    instruction* mem_instruction = malloc(sizeof(instruction));
    instruction* wb_instruction = malloc(sizeof(instruction));
    if (!f_instruction || !d_instruction || !exe_instruction || !mem_instruction || !wb_instruction) {
        perror("Failed to allocate memory for instructions");
        exit(EXIT_FAILURE);
    }
    // Initialize the instructions as bubbles
    turn_to_stall(f_instruction);
    turn_to_stall(d_instruction);
    turn_to_stall(exe_instruction);
    turn_to_stall(mem_instruction);
    turn_to_stall(wb_instruction);
    
    while(!done(f_instruction, d_instruction, exe_instruction, mem_instruction, wb_instruction) || cpu->pc == 0) {
        // Performing the actions
        pipeline_step(coretrace_file, cpu, memory, f_instruction, d_instruction, exe_instruction, mem_instruction, wb_instruction);
        //print_core_trace_hex(cpu, f_instruction, d_instruction, exe_instruction, mem_instruction, wb_instruction);
        //print_pipline(f_instruction, d_instruction, exe_instruction, mem_instruction, wb_instruction);
        //print_core(cpu);
    }
    // create the regout.txt file
    create_regout(cpu);
    // Ensure to free allocated memory at the end of the function
    free_core(cpu);
    free(f_instruction);
    free(d_instruction);
    free(exe_instruction);
    free(mem_instruction);
    free(wb_instruction);
}

// Check if all instructions are stalls
bool done(instruction* inst1, instruction* inst2, instruction* inst3, instruction* inst4, instruction* inst5) {
    if (!inst1 || !inst2 || !inst3 || !inst4 || !inst5) {
        return false;
    }
    bool b1 = (inst1->opcode == STALL_OPCODE);
    bool b2 = (inst2->opcode == STALL_OPCODE);
    bool b3 = (inst3->opcode == STALL_OPCODE);
    bool b4 = (inst4->opcode == STALL_OPCODE);
    bool b5 = (inst5->opcode == STALL_OPCODE);

    return (b1 && b2 && b3 && b4 && b5);
}

// Frees the core's memory including its cache
void free_core(core* cpu) {
    if (!cpu) {
        printf("Error: Null pointer passed to free_core.\n");
        return;
    }
    // Free the cache if it exists
    if (cpu->cache) {
        free(cpu->cache);
        cpu->cache = NULL;
    }
    // Free the core itself
    free(cpu);
}

// turn instruction to stall
void turn_to_stall(instruction* instruction)
{
    instruction->opcode = STALL_OPCODE;
    instruction->pc = -1;
    instruction->rt = 0;
    instruction->rs = 0;
    instruction->rd = 0;
    instruction->imm = 0;
    instruction->ALU_result = 0;
}

// Writes a line to the coreNUMtrace.txt file after each cycle
void write_line_to_core_trace_file(FILE* coretrace_filename, core* cpu, instruction* f, 
    instruction* d, instruction* e, instruction* m, instruction* w) 
{
    if (!coretrace_filename || !cpu || !cpu->cache) {
        printf("Error: Invalid file pointer or uninitialized core/cache.\n");
        return;
    }
    // Write the clock cycle number
    fprintf(coretrace_filename, "%d ", cpu->cycle);
    // Write the PC values for each pipeline stage
    if(f->pc != -1) { fprintf(coretrace_filename, "%03X ", f->pc); }
    else{ fprintf(coretrace_filename, "--- "); }

    if(d->pc != -1) { fprintf(coretrace_filename, "%03X ", d->pc); }
    else{ fprintf(coretrace_filename, "--- "); }

    if(e->pc != -1) { fprintf(coretrace_filename, "%03X ", e->pc); }
    else{ fprintf(coretrace_filename, "--- "); }

    if(m->pc != -1) { fprintf(coretrace_filename, "%03X ", m->pc); }
    else{ fprintf(coretrace_filename, "--- "); }
    
    if(w->pc != -1) { fprintf(coretrace_filename, "%03X ", w->pc); }
    else{ fprintf(coretrace_filename, "--- "); }
    
    // Write the register values (starting from R2)
    for (int i = 2; i < NUM_OF_REGISTERS; i++) {
        fprintf(coretrace_filename, "%08X ", cpu->registers[i]);
    }
    // End the line
    fprintf(coretrace_filename, "\n");
    if(done(f, d, e, m, w)){
        fclose(coretrace_filename);
    }
}

// Generates the file regout.txt with the register values ​​at the end of the run
void create_regout(core* cpu) 
{
    FILE* regout_filename;
    if(cpu->core_number == 0) { regout_filename = fopen("regout0.txt", "w"); }
    if(cpu->core_number == 1) { regout_filename = fopen("regout1.txt", "w"); }
    if(cpu->core_number == 2) { regout_filename = fopen("regout2.txt", "w"); }
    if(cpu->core_number == 3) { regout_filename = fopen("regout3.txt", "w"); }
    if (!regout_filename) {
        printf("Error: Failed to open regout%d.txt for writing.\n",cpu->core_number);
        return;
    }
    // Write the register values (starting from R2)
    for (int i = 2; i < NUM_OF_REGISTERS; i++) {
        fprintf(regout_filename, "%08X\n", cpu->registers[i]);
    }
    // Close the file
    fclose(regout_filename);
}



/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Receives an opcode as int and returns its representation as a string.
char* opcode_to_string(int opcode)
{
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
char* reg_to_string(int index)
{
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
char* imm_to_string(int imm) 
{
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

// create on line (char*) of the instruction
char* instruction_as_a_string(instruction* instruction)
{
    // Convert opcode, registers, and immediate value to strings
    const char* opcode_str = opcode_to_string(instruction->opcode);
    const char* rt_str = reg_to_string(instruction->rt);
    const char* rs_str = reg_to_string(instruction->rs);
    const char* rd_str = reg_to_string(instruction->rd);
    char* imm_str = imm_to_string(instruction->imm);

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
    snprintf(result, size, "pc = %d: %s %s, %s, %s, %s", instruction->pc, opcode_str, rd_str, rs_str, rt_str, imm_str);
    // Free allocated memory
    free(imm_str);

    return result;
}

// Prints the 8 digits of the instruction and the translation we performed 
void print_parse_instruction(instruction* instruction, char* str_instruction) 
{
    // Call parse_instruction
    if(parse_instruction(instruction, str_instruction)) { 
        printf("Error: Failed to parse instruction \"%s\".\n", str_instruction);
        return;
    }
    // Print the parsed values
    // Convert parsed instruction to a string
    char* parse_instruction = instruction_as_a_string(instruction);
    if (parse_instruction) {
        printf("instruction: \"%s\" ==> %s\n", str_instruction, parse_instruction);
        free(parse_instruction); // Free the allocated memory
    } 
    else {
        printf("Error: Failed to create parsed instruction string.\n");
    }
}

// Prints the contents of instruction memory
void print_imem(core* cpu) 
{
    printf("Instruction Memory:\n");

    for (int i = 0; i < IMEM_SIZE; i++) {
        instruction* inst = &cpu->imem[i];
        // Check if the instruction is non-empty
        if (inst->opcode != 0 || inst->rt != 0 || inst->rs != 0 || inst->rd != 0 || inst->imm != 0) {
            char* inst_str = instruction_as_a_string(inst);
            if (inst_str) {
                printf("%s\n", inst_str);
                free(inst_str);
            } else {
                printf("Error creating string for instruction at index %d.\n", i);
            }
        }
    }

    printf("End of Instruction Memory.\n\n");
}

// Prints the core status at a given moment.
void print_core_status(core* cpu) 
{
    if (!cpu || !cpu->cache) {
        printf("Error: Core or Cache is not initialized.\n");
        return;
    }

    // Print the cycle, pc, and halt flag
    printf("cycle = %d, pc = %d, halt_flag = %s\n", 
           cpu->cycle, 
           cpu->pc, 
           cpu->halt_flag ? "true" : "false");

    // Print the registers
    printf("registers = { ");
    for (int i = 0; i < NUM_OF_REGISTERS; i++) {
        printf("%d", cpu->registers[i]);
        if (i < NUM_OF_REGISTERS - 1) {
            printf(", ");
        }
    }
    printf(" }\n");

    // Print non-zero blocks in the cache
    printf("Cache Content (Non-Zero Blocks):\n");
    print_cache(cpu->cache);
    printf("\n");
}

// print 5 rows of the pipeline levels in hex to see it contennet
void print_pipline(instruction* fetch_inst, instruction* decode_inst, instruction* exe_inst, instruction* mem_inst, instruction* wb_inst) 
{
    // Array of pointers to the instructions
    instruction* pipeline_stages[5] = {fetch_inst, decode_inst, exe_inst, mem_inst, wb_inst};
    const char* stage_names[5] = {"Fetch: ", "Decode: ", "EXE: ", "MEM: ", "WB: "};

    for (int i = 0; i < 5; i++) {
        if(i != 1){
            printf("%s\t", stage_names[i]);
        }
        else{
            printf("%s", stage_names[i]);
        }
        if (pipeline_stages[i]->opcode == STALL_OPCODE) {
            printf("stall\n");
        } else {
            char* inst_str = instruction_as_a_string(pipeline_stages[i]);

            if (inst_str) {
                printf("%s\n", inst_str);
                free(inst_str); // Free the allocated memory
            } else {
                printf("Error converting instruction to string\n");
            }
        }
    }
}

// Print the cycle, pc, halt flag and registers
void print_core(core* cpu) 
{
    if (!cpu || !cpu->cache) {
        printf("Error: Core or Cache is not initialized.\n");
        return;
    }

    // Print the cycle, pc, and halt flag
    printf("cycle = %d, pc = %d, halt_flag = %s, ", 
           cpu->cycle, 
           cpu->pc, 
           cpu->halt_flag ? "true" : "false");

    // Print the registers
    printf("registers = { ");
    for (int i = 0; i < NUM_OF_REGISTERS; i++) {
        printf("%d", cpu->registers[i]);
        if (i < NUM_OF_REGISTERS - 1) {
            printf(", ");
        }
    }
    printf(" }\n");
}

// Prints a line in core_trace format (hex)
void print_core_trace_hex(core* cpu, instruction* f, instruction* d, instruction* e, instruction* m, instruction* w) 
{
    if (!cpu || !cpu->cache) {
        printf("Error: Core or Cache is not initialized.\n");
        return;
    }
    // Print the cycle, and instructions pc
    printf("%d ", cpu->cycle);
    f->pc != -1 ? printf("%03X ", f->pc) : printf("--- ");
    d->pc != -1 ? printf("%03X ", d->pc) : printf("--- ");
    e->pc != -1 ? printf("%03X ", e->pc) : printf("--- ");
    m->pc != -1 ? printf("%03X ", m->pc) : printf("--- ");
    w->pc != -1 ? printf("%03X ", w->pc) : printf("--- "); 
    // Print the registers
    for (int i = 2; i < NUM_OF_REGISTERS; i++) {
        printf("%08X ", cpu->registers[i]);
    }
    printf("\n");
}

