#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sram.h"
#include "core.h"
#include "memory.h"
#include "bus.h"


/*********************** Debug *************************/

#define CORE_DEBUG false // print the coretrace of each step
#define CORE_NUM 2      // the core we are debugging

/*******************************************************/
/*************** Assembler Functions *******************/
/*******************************************************/

// Converts a hexadecimal string to an integer and returns the integer value.
int str_to_int(char *str)
{
    return (int)strtol(str, NULL, 16); // Convert hex string to int
}

// Converts a hex string to an instruction and returns true if successed
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
    instruction->ALU_result = 0;
    instruction->bus_delay = BUS_DELAY;
    instruction->block_delay = BLOCK_DELAY;
    instruction->extra_delay = EXTRA_DELAY;
    return 1; // Success
}

// Converts a hax string line into an instruction structure, Returns 1 if successful, -1 otherwise
int line_to_instruction(char* line, instruction* inst, int line_index) 
{
    inst->pc = line_index;
    return parse_instruction(inst, line);
}


/*******************************************************/
/***************** Core Functions **********************/
/*******************************************************/

// Initializes the stats structure
void init_stats(stats** stat) 
{
    *stat = (stats*)malloc(sizeof(stats));
    if (!*stat) {
        perror("Failed to allocate memory for stats");
        exit(EXIT_FAILURE);
    }
    (*stat)->total_cycles = 0;
    (*stat)->total_instructions = 0;
    (*stat)->read_hit = 0;
    (*stat)->write_hit = 0;
    (*stat)->read_miss = 0;
    (*stat)->write_miss = 0;
    (*stat)->num_of_decode_stalls = 0;
    (*stat)->num_of_mem_stalls = 0;
}

// Initializes the imem array in the core structure, take the data from the file
void init_imem(core* cpu) 
{
    FILE* file = fopen(cpu->imem_filename, "r");
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
            line_index++;
        }
        else {
            printf("Failed to parse line: %s\n", buffer);
        }
    }
    fclose(file);
    // Add halt instruction to the last line of imem
    if (line_index < IMEM_SIZE) {
        turn_to_halt(&cpu->imem[line_index]);
    }
    // Below the halt instruction add 5 stalls if there is room
    int i = 5;
    line_index++;
    while(line_index < IMEM_SIZE && i > 0) {
        turn_to_stall(&cpu->imem[line_index]);
        line_index++;
        i--;
    }
}

/*
 * Initializes the entire core structure:
 * - Sets pc and cycle to 0.
 * - Initializes output filenames.
 * - Initializes all registers to zeros.
 * - Initializes the cache with zeros and INVALID state.
 * - Initializes the instruction memory from the imem file.
 * - Initializes the stats struct.
 */
core* init_core(int core_num, char* imem_str, char* coretrace_str, char* regout_str, char* stats_str, char* dsram_str, char* tsram_str)
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
    cpu->done = false;
    cpu->need_the_bus = false;
    cpu->hold_the_bus = false;
    cpu->stats = NULL;
    cpu->imem_filename = imem_str;
    cpu->coretrace_filename = coretrace_str;
    cpu->regout_filename = regout_str;
    cpu->stats_filename = stats_str;
    cpu->dsram_filename = dsram_str;
    cpu->tsram_filename = tsram_str;
    // Initializing the stats fields
    init_stats(&(cpu->stats));
    // Initialize all registers to 0
    for (int i = 0; i < NUM_OF_REGISTERS; i++) {
        cpu->registers[i] = 0;
    }
    // Allocate and initialize the Cache
    cpu->cache = (Cache*)malloc(sizeof(Cache));
    if (cpu->cache) {
        cache_initialization(cpu->cache);
    }
    else {
        perror("Failed to allocate memory for the Cache");
        exit(EXIT_FAILURE);
    }
    // Initialize the instruction memory (imem) using the provided file
    init_imem(cpu);
    open_file(&cpu->coretrace_file, cpu->coretrace_filename, "w");
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
    dest->bus_delay = src->bus_delay;
    dest->block_delay = src->block_delay;
}

// Creates a structure of 5 instructions and returns a pointer to it (used by the pipeline)
instructions* create_instructions() 
{
    instructions* instr = (instructions*)malloc(sizeof(instructions));
    if (!instr) {
        perror("Failed to allocate memory for instructions");
        exit(EXIT_FAILURE);
    }
    instr->fetch = (instruction*)malloc(sizeof(instruction));
    instr->decode = (instruction*)malloc(sizeof(instruction));
    instr->execute = (instruction*)malloc(sizeof(instruction));
    instr->memory = (instruction*)malloc(sizeof(instruction));
    instr->write_back = (instruction*)malloc(sizeof(instruction));
    if (!instr->fetch || !instr->decode || !instr->execute || !instr->memory || !instr->write_back) {
        perror("Failed to allocate memory for instructions stages");
        free_instructions(instr);
        exit(EXIT_FAILURE);
    }
    turn_to_stall(instr->fetch);
    turn_to_stall(instr->decode);
    turn_to_stall(instr->execute);
    turn_to_stall(instr->memory);
    turn_to_stall(instr->write_back);
    return instr;
}

// Performing the Fetch phase
void fetch (core* cpu, instruction* instruction) 
{
    if(instruction->opcode == STALL_OPCODE && cpu->pc != 0){
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
    if(instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) { 
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
    int opcode = instruction->opcode;
    // branch resolution (if needed)
    if(opcode >= 9 && opcode <= 15){
        // new_pc = R[rd][9:0] (used in case we jump)
        int new_pc = jump_to_pc(cpu->registers[rd]); // new_pc = R[rd][9:0]
        // update register $imm to the imm value (just for this calc, we will restore it after)
        cpu->registers[1] = instruction->imm;
        switch (opcode) {
            case 9:  if(cpu->registers[rs] == cpu->registers[rt]) { cpu->pc = new_pc; } break; // beq:  if(R[rs] == R[rt]) pc = R[rd][low bits 9:0]
            case 10: if(cpu->registers[rs] != cpu->registers[rt]) { cpu->pc = new_pc; } break; // bne:  if(R[rs] != R[rt]) pc = R[rd] [low bits 9:0]
            case 11: if(cpu->registers[rs] < cpu->registers[rt])  { cpu->pc = new_pc; } break; // blt:  if(R[rs] < R[rt])  pc = R[rd] [low bits 9:0] 
            case 12: if(cpu->registers[rs] > cpu->registers[rt])  { cpu->pc = new_pc; } break; // bgt:  if(R[rs] > R[rt])  pc = R[rd] [low bits 9:0]
            case 13: if(cpu->registers[rs] <= cpu->registers[rt]) { cpu->pc = new_pc; } break; // ble:  if(R[rs] <= R[rt]) pc = R[rd] [low bits 9:0]
            case 14: if(cpu->registers[rs] >= cpu->registers[rt]) { cpu->pc = new_pc; } break; // ble:  if(R[rs] >= R[rt]) pc = R[rd] [low bits 9:0]
            case 15: cpu->registers[15] = (cpu->pc + 1);  cpu->pc = new_pc; break; // jal:  R[15] = next instruction address, pc = R[rd][9:0]  
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

// Performing the Execute phase, if needed store the result in ALU-result in the instruction struct
void execute (core* cpu, instruction* instruction)
{
    // Do nothing if it is not an arithmetic operation or a memory operation.
    if((instruction->opcode > 8 && instruction->opcode < 16) || instruction->opcode > 17
     || instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) { 
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
        case 16: instruction->ALU_result = cpu->registers[rs] + cpu->registers[rt]; return; // lw: Prepares the result (to the MEM phase)
        case 17: instruction->ALU_result = cpu->registers[rs] + cpu->registers[rt]; return; // sw: Prepares the result (to the MEM phase)
        default: // opcode = stall or invalid opcode 
        return;
    }
    // restore the $r1 value
    cpu->registers[1] = imm;
}

// Performing the Mem phase, do nothing until the last cycle of the sum of the delays in the delay fields
bool mem(core* cpu, instruction* instruction, cache_block* data_from_memory, uint32_t* address, bool* extra_delay)
{
    // No memory operation needed
    if (instruction->opcode != 16 && instruction->opcode != 17) {
        return true;
    }
    // The operation cannot be completed until the bus is received
    else if (!cpu->hold_the_bus && !search_block(cpu->cache, (uint32_t)instruction->ALU_result))
    {
        return false;
    }

    // lw: R[rd] = MEM[R[rs]+R[rt]]
    if (instruction->opcode == 16)
    {
        return lw(cpu, instruction, data_from_memory, address, extra_delay);
    }
    // sw: MEM[R[rs]+R[rt]] = R[rd]
    // if (instruction->opcode == 17)
    else
    {
        return sw(cpu, instruction, data_from_memory, address, extra_delay);
    }
}

/*
 * Returns true if the lw operation is complete and the value is loaded from cache/memory
 * if the operation is not complete false will be returned (no progress can be made)
 * The function receives:
 * - a core that is currently working with the bus
 * - an instruction that is currently in the mem phase with opcode == 16 (lw)
 * - cache_block which will be the block that was brought from memory after enough cycles (if needed)
 * - a boolean variable bus_ready which says that we received data from the bus (used to simulate waiting for the bus)
 * - Pointer to an integer that represents the number of words we received and when it is 0 - we received the entire block
 * The function updates all the data in the first cycle the bus transmits the block
 * in the next cycles it's waits 4 cycles to simulate receiving the block in parts
 */
bool lw(core* cpu, instruction* instruction, cache_block* data_from_memory, uint32_t* address, bool* extra_delay) 
{
    // lw: R[rd] = MEM[R[rs]+R[rt]] = MEM[ALU_result]
    // break data to address, offset and tag
    *address = (uint32_t)instruction->ALU_result;
    address_done = false;
    uint32_t offset = *address % BLOCK_SIZE;
    // block for the search
    cache_block* c_block = NULL;
    bool found = search_block(cpu->cache, *address);
    // Cache hit
    if (found) { // search_block returns a pointer to the block if it exists.
        c_block = get_cache_block(cpu->cache, *address);
        instruction->ALU_result = c_block->data[offset];
        cpu->stats->read_hit++;
        address_done = true;
        return true;
    }
    // Cache miss
    else
    {
        // waiting for the whole block from the bus
        if (*extra_delay && instruction->extra_delay > 0)
        {
            instruction->extra_delay--;
            return false;
        }
        // waiting for the first word from the bus
        if(instruction->bus_delay > 0){
            instruction->bus_delay--;
            return false;
        }
        // waiting for the whole block from the bus
        else if(instruction->block_delay > 0){
            instruction->block_delay--;
            return false;
        }
        // finished waiting for the whole block, update the cache and move forward (return true)
        // else if (instruction->bus_delay == 0 && instruction->block_delay == 0 && (!*extra_delay || instruction->extra_delay == 0))
        else
        {
            *extra_delay = false;
            // create block to insert the cache
            c_block = (cache_block*)malloc(sizeof(cache_block));
            if (!c_block) {
                printf("Memory allocation failed!\n");
                return false;
            }
            c_block->tag = data_from_memory->tag;
            c_block->state = EXCLUSIVE;
            c_block->cycle = cpu->cycle;
            for(int i = 0; i < CACHE_BLOCK_SIZE; i++){
                c_block->data[i] = data_from_memory->data[i];
            }
            // load the word we want
            instruction->ALU_result = data_from_memory->data[offset];
            insert_block(cpu->cache, *address, c_block, cpu->cycle); // Overwrite the old block with the new block
            // release the bus
            cpu->need_the_bus = false;
            cpu->hold_the_bus = false;
            cpu->stats->read_miss++;
            address_done = true;
            return true;
        }
    }
}

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
bool sw(core* cpu, instruction* instruction, cache_block* data_from_memory, uint32_t *address, bool* extra_delay)
{
    // sw: MEM[R[rs]+R[rt]] = R[rd]
    int data = instruction->ALU_result;
    address_done = false;
    *address = (uint32_t)data;
    uint32_t offset = *address % BLOCK_SIZE;
    //uint32_t tag = *address / (BLOCK_SIZE * NUM_OF_BLOCKS);
    // block for the search
    bool found = search_block(cpu->cache, *address);
    cache_block *c_block;
    // cache hit
    if (found)
    {
        c_block = get_cache_block(cpu->cache, *address);
        c_block->data[offset] = cpu->registers[instruction->rd];
        c_block->state = MODIFIED;
        cpu->stats->write_hit++;
        address_done = true;
        return true;
    }
    // Cache miss
    else
    {
        // waiting for the whole block from the bus
        if (*extra_delay && instruction->extra_delay > 0)
        {
            instruction->extra_delay--;
            return false;
        }
        // waiting for the first word from the bus
        if(instruction->bus_delay > 0){
            instruction->bus_delay--;
            return false;
        }
        // waiting for the whole block from the bus
        else if(instruction->block_delay > 0){
            instruction->block_delay--;
            return false;
        }
        // finished waiting for the whole block, update the cache and move forward (return true)
        //else if (instruction->bus_delay == 0 && instruction->block_delay == 0 && (!*extra_delay || instruction->extra_delay == 0))
        else
        {
            *extra_delay = false;
            // create block to insert the cache
            c_block = (cache_block*)malloc(sizeof(cache_block));
            if (!c_block) {
                printf("Memory allocation failed!\n");
                return false;
            }
            c_block->tag = data_from_memory->tag;
            c_block->state = MODIFIED;
            c_block->cycle = cpu->cycle;
            for(int i = 0; i < CACHE_BLOCK_SIZE; i++){
                c_block->data[i] = data_from_memory->data[i];
            }
            c_block->data[offset] = cpu->registers[instruction->rd];
            insert_block(cpu->cache, *address, c_block, cpu->cycle); // Overwrite the old block with the new block
            cpu->stats->write_miss++;                               // count the miss just one
            // release the bus
            cpu->need_the_bus = false;
            cpu->hold_the_bus = false;
            address_done = true;
            return true;
        }
    }
}

// Performing the WB phase
void write_back (core* cpu, instruction* instruction)
{
    if(instruction->opcode == STALL_OPCODE || instruction->opcode == HALT_OPCODE) {
        return;
    }
    cpu->registers[1] = instruction->imm;
    // Do not perform an R-type (arithmetic operation) into the $ziro register.
    int opcode = instruction->opcode;
    int rd = instruction->rd;
    // if not write back to reg opertion
    if((0 <= opcode && opcode <= 8  && (rd == 0 || rd == 1)) || (opcode > 8 && opcode != 16)){
        return;
    }
    // perform only R-type (arithmetic operation), if opcode isn't R-type do nothing
    if(opcode >= 0 && opcode <= 8) {
        cpu->registers[rd] = instruction->ALU_result;
    }
    // lw - The value fetched from memory in the Mem phase is written to the register.
    if(opcode == 16) {
        cpu->registers[rd] = instruction->ALU_result;
    }
    cpu->stats->total_instructions++;
    // Update PC address
    //cpu->registers[15] = cpu->pc;
}

// performing one step in the core pipeline
// Calculates pipeline delays and updates instructions accordingly
cache_block* pipeline_step(core* cpu, instructions* instructions, cache_block* data_from_memory, uint32_t* address, bool* extra_delay) 
{
    if (!cpu) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }

    cache_block* c_block = NULL;
    if (*address != -1 && search_block(cpu->cache, *address))
    {
        c_block = get_cache_block(cpu->cache, *address);
    }

    if(cpu->done) { return c_block; } // The core has finished executing all instructions.

    
    
    

    bool forward_fetch = true;
    bool forward_decode = true;
    bool forward_execute = true;
    bool forward_memory = true;

    // Preparation before calculations (for convenience)
    int decode_rt = instructions->decode->rt;
    int decode_rs = instructions->decode->rs;
    int decode_rd = instructions->decode->rd;
    int exe_rd = instructions->execute->rd;
    int mem_rd = instructions->memory->rd;
    int wb_rd = instructions->write_back->rd;

    // Data Hazard: EXE $rd is used as $rs or $rt or $rd in Decode → Insert stall
    bool data_hazard_decode_and_exe = ((exe_rd == decode_rd || exe_rd == decode_rs || exe_rd == decode_rt) && exe_rd != 0 && exe_rd != 1);
    // Data Hazard: MEM $rd is used as $rs or $rt or $rd in Decode → Insert stall
    bool data_hazard_decode_and_mem = ((mem_rd == decode_rd || mem_rd == decode_rs || mem_rd == decode_rt) && mem_rd != 0 && mem_rd != 1);
    // Data Hazard: WB isn't finish and $rd is used as $rs or $rt or $rd in Decode → Insert stall
    bool write_to_reg = (((instructions->write_back->opcode >= 0) && (instructions->write_back->opcode < 9)) || (instructions->write_back->opcode == 16));
    bool data_hazard_decode_and_wb = (((wb_rd == decode_rd) || (wb_rd == decode_rs) || (wb_rd == decode_rt)) && write_to_reg);
    
    // if there is at least one data hazard
    bool data_hazard = (data_hazard_decode_and_exe || data_hazard_decode_and_mem || data_hazard_decode_and_wb);
    if (cpu->cycle > 1 && (instructions->decode->opcode != HALT_OPCODE) && data_hazard)
    {
        forward_fetch = false;
        forward_decode = false;
    }
    // Performing the actions
    fetch(cpu, instructions->fetch);
    int prev_pc = cpu->pc;
    bool jump_taken = decode(cpu, instructions->decode);
    execute(cpu, instructions->execute);
    bool mem_hazard = !mem(cpu, instructions->memory, data_from_memory, address, extra_delay);
    // Memory Hazard (cache miss)  → Insert 16 stalls
    if (mem_hazard)
    {
        forward_fetch = false;
        forward_decode = false;
        forward_execute = false;
        forward_memory = false;
        cpu->stats->num_of_mem_stalls++;
    }
    if(CORE_DEBUG && cpu->core_number == CORE_NUM) print_core_trace_hex(cpu, instructions);
    write_line_to_core_trace_file(cpu, instructions);
    write_back(cpu, instructions->write_back);
    cpu->cycle++;
    // Advancing the stages in the core pipeline
    if(forward_memory) { copy_instruction(instructions->write_back, instructions->memory);  }
    else { turn_to_stall(instructions->write_back);  } // mem Not finished - insert stall
    if(forward_execute){ copy_instruction(instructions->memory, instructions->execute); }
    else {  // exe need to wait
        if(forward_memory){ // if mem is getting forward and exe not - insert stall
            turn_to_stall(instructions->memory);
        }
    }
    if(forward_decode) { 
        copy_instruction(instructions->execute, instructions->decode);
        // We have reached the halt command. We will continue until the pipeline is emptied but turn fetch and decode to stalls.
        if(instructions->execute->opcode == HALT_OPCODE) {
            turn_to_stall(instructions->fetch);
            turn_to_stall(instructions->decode);
        }
    }
    else { // fetch the same instruction again and hold decode in the same place
        if(jump_taken) { 
            cpu->pc = prev_pc;
        }
        if(forward_execute) {
            turn_to_stall(instructions->execute); // insert stall
        }
    }
    if(forward_fetch)  { copy_instruction(instructions->decode, instructions->fetch);     }
    else{
        cpu->pc--;
        copy_instruction(instructions->fetch, instructions->decode);
    }
    // Count all stalls that complete the wb phase
    if(instructions->write_back->opcode == STALL_OPCODE){
        cpu->stats->num_of_decode_stalls++;
    }
    if(done(cpu, instructions)) {
        cpu->stats->total_cycles = cpu->cycle;
        // The number of instructions executed is total cycles - total stalls
        cpu->stats->total_instructions = (cpu->cycle -  cpu->stats->num_of_decode_stalls);
        // decode stalls = total stalls - mem_stalls + 4 (the number of stalls for filling the pipeline)
        cpu->stats->num_of_decode_stalls = (cpu->stats->num_of_decode_stalls - (cpu->stats->num_of_mem_stalls + 4));
        cpu->done = true;
        fclose(cpu->coretrace_file);
    }
    return c_block;
}

// Check if all instructions are stalls (the core finish running)
bool done(core* cpu, instructions* instructions)
{
    bool b1 = (instructions->fetch->opcode == STALL_OPCODE);
    bool b2 = (instructions->decode->opcode == STALL_OPCODE);
    bool b3 = (instructions->execute->opcode == STALL_OPCODE);
    bool b4 = (instructions->memory->opcode == STALL_OPCODE);
    bool b5 = (instructions->write_back->opcode == STALL_OPCODE);
    bool just_stalls = (b1 && b2 && b3 && b4 && b5);

    cpu->done = ((just_stalls && cpu->cycle > 0) || (instructions->fetch->pc == IMEM_SIZE-1));
    return cpu->done;
}

// Frees the core's memory including its cache and the stats
void free_core(core* cpu)
{
    if (!cpu) {
        return;
    }
    // Free the cache if it exists
    if(cpu->cache){
        free_cache(cpu->cache);
    }
    // Free the stats struct
    if (cpu->stats) {
        free(cpu->stats);
    }
    // Free the core itself
    free(cpu);
}

// Frees the structure with the 5 instructions
void free_instructions(instructions* instructions)
{
    if (!instructions) {
        return;
    }
    free(instructions->fetch);
    free(instructions->decode);
    free(instructions->execute);
    free(instructions->memory);
    free(instructions->write_back);
    free(instructions);
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
    instruction->bus_delay = 0;
    instruction->block_delay = 0;
}

// turn instruction to halt
void turn_to_halt(instruction* instruction)
{
    instruction->opcode = HALT_OPCODE;
    instruction->pc = -1;
    instruction->rt = 0;
    instruction->rs = 0;
    instruction->rd = 0;
    instruction->imm = 0;
    instruction->ALU_result = 0;
    instruction->bus_delay = 0;
    instruction->block_delay = 0;
}


/*******************************************************/
/*************** Create output files *******************/
/*******************************************************/

// Writes a line to the coretrace.txt file after each cycle
void write_line_to_core_trace_file(core* cpu, instructions* instructions) 
{
    if (!cpu) {
        printf("Error: Invalid file pointer or uninitialized core/cache.\n");
        return;
    }
    // Write the clock cycle number
    fprintf(cpu->coretrace_file, "%d ", cpu->cycle);
    // Write the PC values for each pipeline stage
    if(instructions->fetch->pc != -1) { fprintf(cpu->coretrace_file, "%03X ", instructions->fetch->pc); }
    else{ fprintf(cpu->coretrace_file, "--- "); }

    if(instructions->decode->pc != -1) { fprintf(cpu->coretrace_file, "%03X ", instructions->decode->pc); }
    else{ fprintf(cpu->coretrace_file, "--- "); }

    if(instructions->execute->pc != -1) { fprintf(cpu->coretrace_file, "%03X ", instructions->execute->pc); }
    else{ fprintf(cpu->coretrace_file, "--- "); }

    if(instructions->memory->pc != -1) { fprintf(cpu->coretrace_file, "%03X ", instructions->memory->pc); }
    else{ fprintf(cpu->coretrace_file, "--- "); }
    
    if(instructions->write_back->pc != -1) { fprintf(cpu->coretrace_file, "%03X ", instructions->write_back->pc); }
    else{ fprintf(cpu->coretrace_file, "--- "); }

    // Write the register values (starting from R2)
    for (int i = 2; i < NUM_OF_REGISTERS; i++) {
        fprintf(cpu->coretrace_file, "%08X ", cpu->registers[i]);
    }
    // End the line
    fprintf(cpu->coretrace_file, "\n");
    if(done(cpu, instructions)){
        fclose(cpu->coretrace_file);
    }
}

// Generates all the output files (Except of coretrace) at once
void create_output_files(core* cpu)
{
    create_regout_file(cpu);
    create_stats_file(cpu);
    create_dsram_file(cpu);
    create_tsram_file(cpu);
}

// Generates the file regout.txt with the register values ​​at the end of the run
void create_regout_file(core* cpu) 
{
    FILE* regout_file = NULL;
    open_file(&regout_file, cpu->regout_filename, "w");
    // Write the register values (starting from R2)
    for (int i = 2; i < NUM_OF_REGISTERS; i++) {
        fprintf(regout_file, "%08X\n", cpu->registers[i]);
    }
    // Close the file
    fclose(regout_file);
}

// Generates the file stats.txt
void create_stats_file(core* cpu) 
{
    FILE* file = NULL;
    open_file(&file, cpu->stats_filename, "w");

    // Write the values
    fprintf(file, "cycles %d\n", cpu->stats->total_cycles);
    fprintf(file, "instructions %d\n", cpu->stats->total_instructions);
    fprintf(file, "read_hit %d\n", cpu->stats->read_hit);
    fprintf(file, "write_hit %d\n", cpu->stats->write_hit);
    fprintf(file, "read_miss %d\n", cpu->stats->read_miss);
    fprintf(file, "write_miss %d\n", cpu->stats->write_miss);
    fprintf(file, "decode_stall %d\n", cpu->stats->num_of_decode_stalls);
    fprintf(file, "mem_stall %d\n", cpu->stats->num_of_mem_stalls);

    // Close the file
    fclose(file);
}

// Generates the file dsram.txt
void create_dsram_file(core* cpu)
{
    FILE* file = NULL;
    open_file(&file, cpu->dsram_filename, "w");
    if (!file) {
        perror("Error opening dsram file");
        return;
    }
    for (int i = 0; i < NUM_BLOCKS; i++) {
        for (int j = 0; j < CACHE_BLOCK_SIZE; j++) {
            fprintf(file, "%08X\n", cpu->cache->blocks[i].data[j]);
        }
    }
    fclose(file);
}

// Generates the file tsram.txt
void create_tsram_file(core* cpu)
{
    FILE* file = NULL;
    open_file(&file, cpu->tsram_filename, "w");
    for (int i = 0; i < NUM_BLOCKS; i++) {
        uint32_t tag = cpu->cache->blocks[i].tag;
        MESI_state state = cpu->cache->blocks[i].state;
        uint32_t tsram_entry = (tag << 2) | state; // Tag is 12 bits, state is 2 bits
        fprintf(file, "%08X\n", tsram_entry);
    }
    fclose(file);
}

// Opens a single file and returns an error if not opened.
void open_file(FILE** f, char* filename, char* c)
{
    *f = fopen(filename, c);
    // Checking if all file opened successfully
    if (!f) { 
        printf("Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
}


/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

// Test single core, executes all instructions in the core instruction memory
void run_core(core* cpu, main_memory* memory)
{
    // Allocate memory for the instructions
    instructions* instructions = create_instructions();
    cache_block* data_from_memory = NULL;
    bool extra_delay = false;
    uint32_t address = 0;
    while(!done(cpu, instructions) || cpu->pc == 0) {
        // Performing the actions
        pipeline_step(cpu, instructions, data_from_memory, &address, &extra_delay);
    }
    // create the regout.txt file
    create_output_files(cpu);
    // Ensure to free allocated memory at the end of the function
    free_instructions(instructions);
    free_core(cpu);
}

// create instruction as a string
char* get_instruction_as_a_string(instruction* instr)
{    
    // opcodes list
    const char* opcodes[] = {
        "add", "sub", "and", "or", "xor", "mul", "sll", "sra", "srl",
        "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", 
        " ", " ", "halt"
    };
    // registers list
    const char* registers[] = {
        "$zero", "$imm", "$r2", "$r3", "$r4", "$r5", "$r6", "$r7",
        "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14", "$r15"
    };
    // Preparing the instruction parts
    const char* opcode_str = (instr->opcode >= 0 && instr->opcode <= 20) ? opcodes[instr->opcode] : "unknown";
    const char* rt_str = (instr->rt >= 0 && instr->rt <= 15) ? registers[instr->rt] : "unknown";
    const char* rs_str = (instr->rs >= 0 && instr->rs <= 15) ? registers[instr->rs] : "unknown";
    const char* rd_str = (instr->rd >= 0 && instr->rd <= 15) ? registers[instr->rd] : "unknown";
    char imm_str[12];
    snprintf(imm_str, sizeof(imm_str), "%d", instr->imm);
    // Calculating the size of the final string
    size_t size = strlen(opcode_str) + strlen(rt_str) + strlen(rs_str) + strlen(rd_str) + strlen(imm_str) + 32;
    char* result = (char*)malloc(size * sizeof(char));
    if (!result) {
        perror("Failed to allocate memory for instruction string");
        return NULL;
    }
    // stall case
    if (instr->opcode == STALL_OPCODE) {
        snprintf(result, size, "# PC = %d:\tstall", instr->pc);
    } else {
        snprintf(result, size, "# PC = %d:\t%s %s, %s, %s, %s",
                 instr->pc, opcode_str, rd_str, rs_str, rt_str, imm_str);
    }
    return result;
}

// Prints the contents of instruction memory
void print_imem(core* cpu) 
{
    printf("Instruction Memory:\n");

    for (int i = 0; i < IMEM_SIZE; i++) {
        instruction* inst = &cpu->imem[i];
        // Check if the instruction is non-empty
        if (inst->opcode != 0 || inst->rt != 0 || inst->rs != 0 || inst->rd != 0 || inst->imm != 0) {
            char* inst_str = get_instruction_as_a_string(inst);
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
    // Print the cycle and pc
    printf("cycle = %d, pc = %d, ", cpu->cycle, cpu->pc);
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
    //printf("Cache Content (Non-Zero Blocks):\n");
    //print_cache(cpu->cache);
    //printf("\n");
}

// Prints a line in core_trace format (hex)
void print_core_trace_hex(core* cpu, instructions* instructions) 
{
    if (!cpu || !cpu->cache) {
        printf("Error: Core or Cache is not initialized.\n");
        return;
    }
    // Print the cycle, and instructions pc
    printf("%d ", cpu->cycle);
    instructions->fetch->pc != -1 ? printf("%03X ", instructions->fetch->pc) : printf("--- ");
    instructions->decode->pc != -1 ? printf("%03X ", instructions->decode->pc) : printf("--- ");
    instructions->execute->pc != -1 ? printf("%03X ", instructions->execute->pc) : printf("--- ");
    instructions->memory->pc != -1 ? printf("%03X ", instructions->memory->pc) : printf("--- ");
    instructions->write_back->pc != -1 ? printf("%03X ", instructions->write_back->pc) : printf("--- "); 
    // Print the registers
    for (int i = 2; i < NUM_OF_REGISTERS; i++) {
        printf("%08X ", cpu->registers[i]);
    }
    printf("\n");
}

// print 5 rows of the pipeline levels in hex to see it contennet
void print_pipline(instructions* instructions) 
{
    // Array of pointers to the instructions
    instruction* pipeline_stages[5] = {instructions->fetch, instructions->decode, instructions->execute, instructions->memory, instructions->write_back};
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
            char* inst_str = get_instruction_as_a_string(pipeline_stages[i]);

            if (inst_str) {
                printf("%s\n", inst_str);
                free(inst_str); // Free the allocated memory
            } else {
                printf("Error converting instruction to string\n");
            }
        }
    }
}

