#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"
#include "core.h"
#include "processor.h"
#include "bus.h"

//#define DEBUG true 
#define DEBUG false


/*******************************************************/
/*************** Processor Functions *******************/
/*******************************************************/

// Initializes file names to the argv[] arguments
void set_file_names(filenames* filenames, char* argv[])
{
    if (!filenames) {
        printf("Error: filenames is NULL!\n");
        return;
    }
    // inputs files:
    filenames->imem0_str = argv[1];
    filenames->imem1_str = argv[2];
    filenames->imem2_str = argv[3];
    filenames->imem3_str = argv[4];
    filenames->memin_str = argv[5];
    filenames->memout_str = argv[6];
    // outputs files:
    filenames->regout0_str = argv[7];
    filenames->regout1_str = argv[8];
    filenames->regout2_str = argv[9];
    filenames->regout3_str = argv[10];
    filenames->core0trace_str = argv[11];
    filenames->core1trace_str = argv[12];
    filenames->core2trace_str = argv[13];
    filenames->core3trace_str = argv[14];
    filenames->bustrace_str = argv[15];
    filenames->dsram0_str = argv[16];
    filenames->dsram1_str = argv[17];
    filenames->dsram2_str = argv[18];
    filenames->dsram3_str = argv[19];
    filenames->tsram0_str = argv[20];
    filenames->tsram1_str = argv[21];
    filenames->tsram2_str = argv[22];
    filenames->tsram3_str = argv[23];
    filenames->stats0_str = argv[24];
    filenames->stats1_str = argv[25];
    filenames->stats2_str = argv[26];
    filenames->stats3_str = argv[27];
}


// Initializes file names to the defined default values
void set_default_file_names(filenames* filenames)
{
    if (!filenames) {
        printf("Error: filenames is NULL!\n");
        return;
    }
    // inputs files:
    filenames->imem0_str = "imem0.txt";
    filenames->imem1_str = "imem1.txt";
    filenames->imem2_str = "imem2.txt";
    filenames->imem3_str = "imem3.txt";
    filenames->memin_str = "memin.txt";
    filenames->memout_str = "memout.txt";
    // outputs files:
    filenames->bustrace_str = "bustrace.txt";
    filenames->regout0_str = "regout0.txt";
    filenames->regout1_str = "regout1.txt";
    filenames->regout2_str = "regout2.txt";
    filenames->regout3_str = "regout3.txt";
    filenames->stats0_str = "stats0.txt";
    filenames->stats1_str = "stats1.txt";
    filenames->stats2_str = "stats2.txt";
    filenames->stats3_str = "stats3.txt";
    filenames->core0trace_str = "core0trace.txt";
    filenames->core1trace_str = "core1trace.txt";
    filenames->core2trace_str = "core2trace.txt";
    filenames->core3trace_str = "core3trace.txt";
    filenames->dsram0_str = "dsram0.txt";
    filenames->dsram1_str = "dsram1.txt";
    filenames->dsram2_str = "dsram2.txt";
    filenames->dsram3_str = "dsram3.txt";
    filenames->tsram0_str = "tsram0.txt";
    filenames->tsram1_str = "tsram1.txt";
    filenames->tsram2_str = "tsram2.txt";
    filenames->tsram3_str = "tsram3.txt";
}



/*
 * Initializes the entire processor structure.
 * - Sets pc to 0.
 * - Initializes each core registers and prev_registers to 0.
 * - Initializes each core imem and prev_imem according to to the file instructions.
 * - Initializes each core cache and prev_cache using their respective initialization function.
 */
processor* init_processor(filenames* filenames)
//processor* init_processor(filenames* filenames)
{
    processor* cpu = malloc(sizeof(processor));
    if (!cpu) {
        perror("Failed to allocate memory for the processor");
        exit(EXIT_FAILURE);
    }
    cpu->filenames = filenames;

    cpu->core0 = init_core(0, cpu->filenames->imem0_str, cpu->filenames->core0trace_str, cpu->filenames->regout0_str, cpu->filenames->stats0_str, cpu->filenames->dsram0_str, cpu->filenames->tsram0_str);
    cpu->core1 = init_core(1, cpu->filenames->imem1_str, cpu->filenames->core1trace_str, cpu->filenames->regout1_str, cpu->filenames->stats1_str, cpu->filenames->dsram1_str, cpu->filenames->tsram1_str);
    cpu->core2 = init_core(2, cpu->filenames->imem2_str, cpu->filenames->core2trace_str, cpu->filenames->regout2_str, cpu->filenames->stats2_str, cpu->filenames->dsram2_str, cpu->filenames->tsram2_str);
    cpu->core3 = init_core(3, cpu->filenames->imem3_str, cpu->filenames->core3trace_str, cpu->filenames->regout3_str, cpu->filenames->stats3_str, cpu->filenames->dsram3_str, cpu->filenames->tsram3_str);
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
    // Initializing the queue
    cpu->round_robin_queue[0] = cpu->core0;
    cpu->round_robin_queue[1] = cpu->core1;
    cpu->round_robin_queue[2] = cpu->core2;
    cpu->round_robin_queue[3] = cpu->core3;

    return cpu;
}


// Executes the processor run
void run(processor* cpu, main_memory* memory)
{
    create_bustrace_file(cpu);
    FILE *memout;
    open_file(&memout, cpu->filenames->memout_str, "w");

    // blocks to transfer data
    memory_block *mem_block = NULL;
    cache_block *data_from_memory = NULL;
    cache_block *data_to_memory = NULL;
    bool extra_delay = false;
    uint32_t address = 0;
    uint32_t tag = 0;
    core* temp_core = (core*)malloc(sizeof(core));
    if (!temp_core) {
        printf("Memory allocation failed!\n");
        return;
    }
    if(DEBUG) { print_bus_status(cpu); }
    while(!finish(cpu)) {
        temp_core = NULL;
        // No core is working with the bus at the moment
        if (!cpu->core0->hold_the_bus && !cpu->core1->hold_the_bus && !cpu->core2->hold_the_bus && !cpu->core3->hold_the_bus)
        {
            // Checks if one of the cores needs the bus
            if (cpu->core0_instructions->memory->opcode == 16 || cpu->core0_instructions->memory->opcode == 17)
            {
                cpu->core0->need_the_bus = !search_block(cpu->core0->cache, (uint32_t)cpu->core0_instructions->memory->ALU_result);
            }
            else
            {
                cpu->core0->need_the_bus = false;
            }
            if (cpu->core1_instructions->memory->opcode == 16 || cpu->core1_instructions->memory->opcode == 17)
            {
                cpu->core1->need_the_bus = !search_block(cpu->core1->cache, (uint32_t)cpu->core1_instructions->memory->ALU_result);
            }
            else
            {
                cpu->core1->need_the_bus = false;
            }
            if (cpu->core2_instructions->memory->opcode == 16 || cpu->core2_instructions->memory->opcode == 17)
            {
                cpu->core2->need_the_bus = !search_block(cpu->core2->cache, (uint32_t)cpu->core2_instructions->memory->ALU_result);
            }
            else
            {
                cpu->core2->need_the_bus = false;
            }
            if (cpu->core3_instructions->memory->opcode == 16 || cpu->core3_instructions->memory->opcode == 17)
            {
                cpu->core3->need_the_bus = !search_block(cpu->core3->cache, (uint32_t)cpu->core3_instructions->memory->ALU_result);
            }
            else
            {
                cpu->core3->need_the_bus = false;
            }

            // if at least one of the cores needs the bus
            if(cpu->core0->need_the_bus || cpu->core1->need_the_bus || cpu->core2->need_the_bus || cpu->core3->need_the_bus){
                // choose who will work with the bus and update the queue
                // core* temp = cpu->round_robin_queue[0];
                temp_core = cpu->round_robin_queue[0];
                if(temp_core->need_the_bus) {
                    temp_core->hold_the_bus = true;
                    // move the core to be the last in the queue
                    cpu->round_robin_queue[0] = cpu->round_robin_queue[1];
                    cpu->round_robin_queue[1] = cpu->round_robin_queue[2];
                    cpu->round_robin_queue[2] = cpu->round_robin_queue[3];
                    cpu->round_robin_queue[3] = temp_core;
                }
                else {
                    temp_core = cpu->round_robin_queue[1];
                    if(temp_core->need_the_bus) {
                        temp_core->hold_the_bus = true;
                        // move the core to be the last in the queue
                        cpu->round_robin_queue[1] = cpu->round_robin_queue[2];
                        cpu->round_robin_queue[2] = cpu->round_robin_queue[3];
                        cpu->round_robin_queue[3] = temp_core;
                    }
                    else {
                        temp_core = cpu->round_robin_queue[2];
                        if(temp_core->need_the_bus) {
                            temp_core->hold_the_bus = true;
                            // move the core to be the last in the queue
                            cpu->round_robin_queue[2] = cpu->round_robin_queue[3];
                            cpu->round_robin_queue[3] = temp_core;
                        }
                        else{
                            temp_core = cpu->round_robin_queue[3];
                            if(temp_core->need_the_bus) {
                                temp_core->hold_the_bus = true;
                            }
                        }
                    }
                }
            }
        }
        // check uniqe modified block in caches
        tag = address / BLOCK_SIZE;
        bool about_to_be_overwritten = false;
        uint32_t address_of_overwritten = 0;
        uint32_t core_of_overwritten = 0;
        int core_num = search_modified_block(cpu, address, &about_to_be_overwritten, &address_of_overwritten, &core_of_overwritten);
        if (about_to_be_overwritten)
        {
            first_flush = core_of_overwritten;
            flush_address = address_of_overwritten;
            switch (core_of_overwritten)
            {
            case 0:
                data_to_memory = get_cache_block(cpu->core0->cache, address_of_overwritten);
                break;
            case 1:
                data_to_memory = get_cache_block(cpu->core1->cache, address_of_overwritten);
                break;
            case 2:
                data_to_memory = get_cache_block(cpu->core2->cache, address_of_overwritten);
                break;
            case 3:
                data_to_memory = get_cache_block(cpu->core3->cache, address_of_overwritten);
                break;
            default:
                break;
            }
            data_to_memory->state = EXCLUSIVE;
            mem_block = convert_cache_block_to_mem_block(data_to_memory);
            uint32_t tag_of_overwritten = address_of_overwritten / (BLOCK_SIZE * NUM_OF_BLOCKS);
            insert_block_to_memory(memory, tag_of_overwritten, *mem_block);
        }
        if (!extra_delay && core_num > 0)
        {
            data_source = core_num - 1;
            extra_delay = about_to_be_overwritten;

            switch (core_num)
            {
            case 1:
                data_to_memory = get_cache_block(cpu->core0->cache, address);
                break;
            case 2:
                data_to_memory = get_cache_block(cpu->core1->cache, address);
                break;
            case 3:
                data_to_memory = get_cache_block(cpu->core2->cache, address);
                break;
            case 4:
                data_to_memory = get_cache_block(cpu->core3->cache, address);
                break;
            default:
                break;
            }
            data_to_memory->state = EXCLUSIVE;
            mem_block = convert_cache_block_to_mem_block(data_to_memory);
            insert_block_to_memory(memory, tag, *mem_block);
        }
        mem_block = get_block(memory, tag);
        data_from_memory = convert_mem_block_to_cache_block(mem_block);

        // bool extra_delay = function!!!!!!!!!!!!!
        // make one step in each core
        cpu->cycle++;
        cache_block *b1 = pipeline_step(cpu->core0, cpu->core0_instructions, data_from_memory, &address, &extra_delay);
        cache_block *b2 = pipeline_step(cpu->core1, cpu->core1_instructions, data_from_memory, &address, &extra_delay);
        cache_block *b3 = pipeline_step(cpu->core2, cpu->core2_instructions, data_from_memory, &address, &extra_delay);
        cache_block *b4 = pipeline_step(cpu->core3, cpu->core3_instructions, data_from_memory, &address, &extra_delay);
        update_cache_stats(b1, b2, b3, b4, NULL);

        if (cpu->core0->hold_the_bus && extra_delay && cpu->core0_instructions->memory->extra_delay != 0)
        {
            set_bus(first_flush, Flush, (flush_address & ~0x03) + 4 - cpu->core0_instructions->memory->extra_delay, get_block(memory, tag)->data[4 - cpu->core0_instructions->memory->extra_delay]);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }
        else if (cpu->core1->hold_the_bus && extra_delay && cpu->core1_instructions->memory->extra_delay != 0)
        {
            set_bus(first_flush, Flush, (flush_address & ~0x03) + 4 - cpu->core1_instructions->memory->extra_delay, get_block(memory, tag)->data[4 - cpu->core1_instructions->memory->extra_delay]);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }
        else if (cpu->core2->hold_the_bus && extra_delay && cpu->core2_instructions->memory->extra_delay != 0)
        {
            set_bus(first_flush, Flush, (flush_address & ~0x03) + 4 - cpu->core2_instructions->memory->extra_delay, get_block(memory, tag)->data[4 - cpu->core2_instructions->memory->extra_delay]);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }
        else if (cpu->core3->hold_the_bus && extra_delay && cpu->core3_instructions->memory->extra_delay != 0)
        {
            set_bus(first_flush, Flush, (flush_address & ~0x03) + 4 - cpu->core3_instructions->memory->extra_delay, get_block(memory, tag)->data[4 - cpu->core3_instructions->memory->extra_delay]);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }

        else if ((temp_core && temp_core == cpu->core0) || (extra_delay && cpu->core0_instructions->memory->bus_delay == BUS_DELAY))
        {
            set_bus(0, cpu->core0_instructions->memory->opcode == 16 ? BusRd : BusRdX, address, 0);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }
        else if ((temp_core && temp_core == cpu->core1) || (extra_delay && cpu->core1_instructions->memory->bus_delay == BUS_DELAY))
        {
            set_bus(1, cpu->core1_instructions->memory->opcode == 16 ? BusRd : BusRdX, address, 0);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }
        else if ((temp_core && temp_core == cpu->core2) || (extra_delay && cpu->core2_instructions->memory->bus_delay == BUS_DELAY))
        {
            set_bus(2, cpu->core2_instructions->memory->opcode == 16 ? BusRd : BusRdX, address, 0);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }
        else if ((temp_core && temp_core == cpu->core3) || (extra_delay && cpu->core3_instructions->memory->bus_delay == BUS_DELAY))
        {
            set_bus(3, cpu->core3_instructions->memory->opcode == 16 ? BusRd : BusRdX, address, 0);
            write_line_to_bustrace_file(cpu, cpu->cycle + 1);
        }

        else if (cpu->core0->hold_the_bus && cpu->core0_instructions->memory->bus_delay == 0 && cpu->core0_instructions->memory->block_delay == 0)
        {
            for (int i = 0; i < 4; i++)
            {
                set_bus(data_source, Flush, (address & ~0x03) + i, get_block(memory, tag)->data[i]);
                if (cpu->core0_instructions->memory->opcode==16 && data_source != 4)
                {
                    set_shared();
                }
                write_line_to_bustrace_file(cpu, cpu->cycle - 3 + i);
            }
            first_flush = 4;
            data_source = 4;
        }
        else if (cpu->core1->hold_the_bus && cpu->core1_instructions->memory->bus_delay == 0 && cpu->core1_instructions->memory->block_delay == 0)
        {
            for (int i = 0; i < 4; i++)
            {
                set_bus(data_source, Flush, (address & ~0x03) + i, get_block(memory, tag)->data[i]);
                if (cpu->core1_instructions->memory->opcode==16 && data_source != 4)
                {
                    set_shared();
                }
                write_line_to_bustrace_file(cpu, cpu->cycle - 3 + i);
            }
            first_flush = 4;
            data_source = 4;
        }
        else if (cpu->core2->hold_the_bus && cpu->core2_instructions->memory->bus_delay == 0 && cpu->core2_instructions->memory->block_delay == 0)
        {
            for (int i = 0; i < 4; i++)
            {
                set_bus(data_source, Flush, (address & ~0x03) + i, get_block(memory, tag)->data[i]);
                if (cpu->core2_instructions->memory->opcode==16 && data_source != 4)
                {
                    set_shared();
                }
                write_line_to_bustrace_file(cpu, cpu->cycle - 3 + i);
            }
            first_flush = 4;
            data_source = 4;
        }
        else if (cpu->core3->hold_the_bus && cpu->core3_instructions->memory->bus_delay == 0 && cpu->core3_instructions->memory->block_delay == 0)
        {
            for (int i = 0; i < 4; i++)
            {
                set_bus(data_source, Flush, (address & ~0x03) + i, get_block(memory, tag)->data[i]);
                if (cpu->core3_instructions->memory->opcode==16 && data_source != 4)
                {
                    set_shared();
                }
                write_line_to_bustrace_file(cpu, cpu->cycle - 3 + i);
            }
            first_flush = 4;
            data_source = 4;
        }

        if (DEBUG) {
            print_bus_status(cpu);
            // print_core_trace_hex(cpu->core2, cpu->core2_instructions);
        }
    }
    create_memout_file(memory, cpu->filenames->memout_str);
    close_bustrace_file();
    create_output_files(cpu->core0);
    create_output_files(cpu->core1);
    create_output_files(cpu->core2);
    create_output_files(cpu->core3);
    // Ensure to free allocated memory at the end of the function
    free_core(temp_core);
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
    // Free the filenames struct
    free(cpu->filenames);
    // Free the processor itself
    free(cpu);
}



memory_block* convert_cache_block_to_mem_block(cache_block* c_block) 
{
    // without cache_block we have nothing to convert
    if (!c_block) {
        return NULL;
    }
    // the memory_block is null => create one
    memory_block* m_block = (memory_block*)malloc(sizeof(memory_block));
    if (!m_block) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    m_block->tag = c_block->tag;
    // copy the data
    for (int i = 0; i < BLOCK_SIZE && i < CACHE_BLOCK_SIZE; i++) {
        m_block->data[i] = c_block->data[i];
    }
    return m_block;
}



cache_block* convert_mem_block_to_cache_block(memory_block* m_block) 
{
    // without cache_block we have nothing to convert
    if (!m_block) {
        return NULL;
    }
    // create cache clock
    cache_block* c_block = (cache_block*)malloc(sizeof(cache_block));
    if (!c_block) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    c_block->tag = m_block->tag;
    c_block->state = SHARED;
    // copy the data
    for (int i = 0; i < BLOCK_SIZE && i < CACHE_BLOCK_SIZE; i++) {
        c_block->data[i] = m_block->data[i];
    }
    return c_block;
}

int search_modified_block(processor *cpu, uint32_t address, bool *about_to_be_overwritten, uint32_t *address_of_overwritten, uint32_t *core_of_overwritten)
{
    if (address_done)
    {
        return 0;
    }

    *about_to_be_overwritten = false;
    uint32_t index = (address / CACHE_BLOCK_SIZE) % NUM_BLOCKS;
    if (cpu->core0->hold_the_bus && cpu->core0->cache->blocks[index].state == MODIFIED)
    {
        *about_to_be_overwritten = true;
        *core_of_overwritten = 0;
        *address_of_overwritten = cpu->core0->cache->blocks[index].tag * (CACHE_BLOCK_SIZE * NUM_BLOCKS);
    }
    if (cpu->core1->hold_the_bus && cpu->core1->cache->blocks[index].state == MODIFIED)
    {
        *about_to_be_overwritten = true;
        *core_of_overwritten = 1;
        *address_of_overwritten = cpu->core1->cache->blocks[index].tag * (CACHE_BLOCK_SIZE * NUM_BLOCKS);
    }
    if (cpu->core2->hold_the_bus && cpu->core2->cache->blocks[index].state == MODIFIED)
    {
        *about_to_be_overwritten = true;
        *core_of_overwritten = 2;
        *address_of_overwritten = cpu->core2->cache->blocks[index].tag * (CACHE_BLOCK_SIZE * NUM_BLOCKS);
    }
    if (cpu->core3->hold_the_bus && cpu->core3->cache->blocks[index].state == MODIFIED)
    {
        *about_to_be_overwritten = true;
        *core_of_overwritten = 3;
        *address_of_overwritten = cpu->core3->cache->blocks[index].tag * (CACHE_BLOCK_SIZE * NUM_BLOCKS);
    }

    if (search_block(cpu->core0->cache, address) && get_cache_block(cpu->core0->cache, address)->state == MODIFIED)
        return 1;
    if (search_block(cpu->core1->cache, address) && get_cache_block(cpu->core1->cache, address)->state == MODIFIED)
        return 2;
    if (search_block(cpu->core2->cache, address) && get_cache_block(cpu->core2->cache, address)->state == MODIFIED)
        return 3;
    if (search_block(cpu->core3->cache, address) && get_cache_block(cpu->core3->cache, address)->state == MODIFIED)
        return 4;

    return 0;
}

void update_cache_stats(cache_block *core0_block, cache_block *core1_block, cache_block *core2_block, cache_block *core3_block, cache_block *mem_block)
{

    cache_block *latest = core0_block;
    if (core1_block && (!latest || (core1_block->cycle > latest->cycle)))
        latest = core1_block;
    if (core2_block && (!latest || (core2_block->cycle > latest->cycle)))
        latest = core2_block;
    if (core3_block && (!latest || (core3_block->cycle > latest->cycle)))
        latest = core3_block;
    if (!latest)
        return;

    if (latest->state == MODIFIED)
    {
        if (core0_block && latest != core0_block && latest->tag == core0_block->tag)
            core0_block->state = INVALID;
        if (core1_block && latest != core1_block && latest->tag == core1_block->tag)
            core1_block->state = INVALID;
        if (core2_block && latest != core2_block && latest->tag == core2_block->tag)
            core2_block->state = INVALID;
        if (core3_block && latest != core3_block && latest->tag == core3_block->tag)
            core3_block->state = INVALID;
    }

    else if (latest->state != INVALID)
    {
        if (core0_block && latest != core0_block && latest->tag == core0_block->tag)
        {
            core0_block->state = SHARED;
            latest->state = SHARED;
        }
        if (core1_block && latest != core1_block && latest->tag == core1_block->tag)
        {
            core1_block->state = SHARED;
            latest->state = SHARED;
        }
        if (core2_block && latest != core2_block && latest->tag == core2_block->tag)
        {
            core2_block->state = SHARED;
            latest->state = SHARED;
        }
        if (core3_block && latest != core3_block && latest->tag == core3_block->tag)
        {
            core3_block->state = SHARED;
            latest->state = SHARED;
        }
    }
}

/*******************************************************/
/*************** Debugging functions *******************/
/*******************************************************/

void print_bus_status(processor* cpu)
{
    char* core0_mem_inst = get_instruction_as_a_string(cpu->core0_instructions->memory);
    char* core1_mem_inst = get_instruction_as_a_string(cpu->core1_instructions->memory);
    char* core2_mem_inst = get_instruction_as_a_string(cpu->core2_instructions->memory);
    char* core3_mem_inst = get_instruction_as_a_string(cpu->core3_instructions->memory);

    int delay = 0;
    core* core;
    if(cpu->core0->hold_the_bus)      {core = cpu->core0; delay = cpu->core0_instructions->memory->block_delay + cpu->core0_instructions->memory->bus_delay; }
    else if(cpu->core1->hold_the_bus) {core = cpu->core1; delay = cpu->core1_instructions->memory->block_delay + cpu->core1_instructions->memory->bus_delay; }
    else if(cpu->core2->hold_the_bus) {core = cpu->core2; delay = cpu->core2_instructions->memory->block_delay + cpu->core2_instructions->memory->bus_delay; }
    else if(cpu->core3->hold_the_bus) {core = cpu->core3; delay = cpu->core3_instructions->memory->block_delay + cpu->core3_instructions->memory->bus_delay; }
    else { core = NULL;}
    if(!core) {
        printf("cycle %d: the bus is ready and waiting for request\n", cpu->cycle);
        return;
    }else{
        printf("cycle %d: core%d hold the bus, left %d cycles on the bus\n", cpu->cycle, core->core_number, delay);
        if(cpu->core0->need_the_bus && !cpu->core0->hold_the_bus) { printf("core0 is waiting for the bus, core0 mem_instructions: %s\n", core0_mem_inst); }
        if(cpu->core1->need_the_bus && !cpu->core1->hold_the_bus) { printf("core1 is waiting for the bus, core1 mem_instructions: %s\n", core1_mem_inst); }
        if(cpu->core2->need_the_bus && !cpu->core2->hold_the_bus) { printf("core2 is waiting for the bus, core2 mem_instructions: %s\n", core2_mem_inst); }
        if(cpu->core3->need_the_bus && !cpu->core3->hold_the_bus) { printf("core3 is waiting for the bus, core3 mem_instructions: %s\n", core3_mem_inst); }
    }
}
