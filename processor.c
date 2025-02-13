#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"
#include "processor.h"



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
processor* init_processor(filesnames* filesnames)
{
    processor* cpu = malloc(sizeof(processor));
    if (!cpu) {
        perror("Failed to allocate memory for the processor");
        exit(EXIT_FAILURE);
    }
    core_initialization(0, filesnames->imem0_str);
    core_initialization(1, filesnames->imem1_str);
    core_initialization(2, filesnames->imem2_str);
    core_initialization(3, filesnames->imem3_str);
    if (!cpu->core0 || !cpu->core1 || !cpu->core2 || !cpu->core3
        || !cpu->core0->cache || !cpu->core1->cache || !cpu->core2->cache || !cpu->core3->cache) {
        perror("Failed to allocate memory for the processor cores");
        exit(EXIT_FAILURE);
    }
    cpu->core0_instructions = create_instructions();
    cpu->core1_instructions = create_instructions();
    cpu->core2_instructions = create_instructions();
    cpu->core3_instructions = create_instructions();
    if (!cpu->core0_instructions || !cpu->core1_instructions || !cpu->core2_instructions || !cpu->core3_instructions) {
        perror("Failed to allocate memory for the cores instructions");
        exit(EXIT_FAILURE);
    }
    cpu->cycle = 0;
}

// Opens a single file and returns an error if not opened.
void open_file(FILE* f, char* filename, char c) {
    f = fopen(filename, c);
    // Checking if all file opened successfully
    if (!f) { 
        printf("Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

// Opens all files
void open_files(files* f, filesnames* filesnames) {
    open_file(f->imem0, filesnames->imem0_str, 'r');
    open_file(f->imem1, filesnames->imem1_str, 'r');
    open_file(f->imem2, filesnames->imem2_str, 'r');
    open_file(f->imem3, filesnames->imem3_str, 'r');
    open_file(f->imem3, filesnames->imem3_str, 'r');
    open_file(f->memin, filesnames->memin_str, 'r');
    open_file(f->memout, filesnames->memout_str, 'w');
    open_file(f->regout0, filesnames->regout0_str, 'w');
    open_file(f->regout1, filesnames->regout1_str, 'w');
    open_file(f->regout2, filesnames->regout2_str, 'w');
    open_file(f->regout3, filesnames->regout3_str, 'w');
    open_file(f->core0trace, filesnames->core0trace_str, 'w');
    open_file(f->core1trace, filesnames->core1trace_str, 'w');
    open_file(f->core2trace, filesnames->core2trace_str, 'w');
    open_file(f->core3trace, filesnames->core3trace_str, 'w');
    open_file(f->dsram0, filesnames->dsram0_str, 'w');
    open_file(f->dsram1, filesnames->dsram1_str, 'w');
    open_file(f->dsram2, filesnames->dsram2_str, 'w');
    open_file(f->dsram3, filesnames->dsram3_str, 'w');
    open_file(f->tsram0, filesnames->tsram0_str, 'w');
    open_file(f->tsram1, filesnames->tsram1_str, 'w');
    open_file(f->tsram2, filesnames->tsram2_str, 'w');
    open_file(f->tsram3, filesnames->tsram3_str, 'w');
    open_file(f->stats0, filesnames->stats0_str, 'w');
    open_file(f->stats1, filesnames->stats1_str, 'w');
    open_file(f->stats2, filesnames->stats2_str, 'w');
    open_file(f->stats3, filesnames->stats3_str, 'w');
}

// Closes all open files.
void close_files(files* f) 
{
    fclose(f->imem0);
    fclose(f->imem1);
    fclose(f->imem2);
    fclose(f->imem3);
    fclose(f->memin);
    fclose(f->memout);
    fclose(f->regout0);
    fclose(f->regout1);
    fclose(f->regout2);
    fclose(f->regout3);
    fclose(f->core0trace);
    fclose(f->core1trace);
    fclose(f->core2trace);
    fclose(f->core3trace);
    fclose(f->dsram0);
    fclose(f->dsram1);
    fclose(f->dsram2);
    fclose(f->dsram3);
    fclose(f->tsram0);
    fclose(f->tsram1);
    fclose(f->tsram2);
    fclose(f->tsram3);
    fclose(f->stats0);
    fclose(f->stats1);
    fclose(f->stats2);
    fclose(f->stats3);
}







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












// Executes the processor run
void run(filesnames* filesnames, processor* cpu, main_memory* memory)
{
    processor* cpu = init_processor(filesnames);
    files* files;
    open_files(files, filesnames);
    
    if(cpu->core0_instructions->memory->opcode == 16)
    while(!finish(cpu)) {
        step(files, cpu, memory);
    }
    

    close_files(files);
    // Ensure to free allocated memory at the end of the function
    free_processor(cpu);
}





// Check if all the cores finished running
bool finish(processor* cpu) 
{
    if(!cpu){
        return false;
    }
    bool b0 = (cpu->core0->done == true);
    bool b1 = (cpu->core1->done == true);
    bool b2 = (cpu->core2->done == true);
    bool b3 = (cpu->core3->done == true);
    
    return (b0 && b1 && b2 && b3);
}


// Frees the core's memory including its cache
void free_processor(processor* cpu)
{
    if (!cpu) {
        printf("Error: Null pointer passed to free_processor.\n");
        return;
    }
    // Free instructions allocated memory
    free_instructions(cpu->core0_instructions);
    free_instructions(cpu->core1_instructions);
    free_instructions(cpu->core2_instructions);
    free_instructions(cpu->core3_instructions);
    // Free cores allocated memory
    free_core(cpu->core0);
    free_core(cpu->core1);
    free_core(cpu->core2);
    free_core(cpu->core3);
    // Free the processor itself
    free(cpu);
}






void convert_cache_block_to_mem_block(cache_block* c_block, memory_block* m_block) 
{
    if (!c_block || !m_block) {
        printf("Error: NULL pointer in convert_cache_block_to_mem_block.\n");
        return;
    }
    m_block->tag = c_block->tag;
    // copy the data
    for (int i = 0; i < BLOCK_SIZE && i < CACHE_BLOCK_SIZE; i++) {
        m_block->data[i] = c_block->data[i];
    }
}


void convert_mem_block_to_cache_block(cache_block* c_block, memory_block* m_block) 
{
    if (!c_block || !m_block) {
        return;
    }
    c_block->tag = m_block->tag;
    c_block->state = SHARED;  // (default)
    // copy the data
    for (int i = 0; i < CACHE_BLOCK_SIZE && i < BLOCK_SIZE; i++) {
        c_block->data[i] = m_block->data[i]; 
    }
}



/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/



