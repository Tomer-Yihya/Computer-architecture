#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sram.h"
#include "core.h"
#include "memory.h"
#include "processor.h"



/*
// check the core functions
int main() {

    // Step 1: Initialize the cores from the files "imem0.txt", "imem1.txt", "imem2.txt", "imem3.txt"
    core* cpu0 = core_initialization(0,"imem0.txt","core0trace.txt", "regout0.txt", "stats0.txt", "dsram0.txt", "tsram0.txt");
    core* cpu1 = core_initialization(1,"imem1.txt","core1trace.txt", "regout1.txt", "stats1.txt", "dsram1.txt", "tsram1.txt");
    core* cpu2 = core_initialization(2,"imem2.txt","core2trace.txt", "regout2.txt", "stats2.txt", "dsram2.txt", "tsram2.txt");
    core* cpu3 = core_initialization(3,"imem3.txt","core3trace.txt", "regout3.txt", "stats3.txt", "dsram3.txt", "tsram3.txt");
    if (!cpu0 || !cpu1 || !cpu2 || !cpu3) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }

    // Step 2: Initialize the main memory from the file "memin.txt"
    main_memory* memory = init_main_memory("memin.txt");
    if (!memory) {
        perror("Failed to allocate memory for main memory");
        exit(EXIT_FAILURE);
    }

    // Step 3: Run the core
    run_core(cpu0, memory);
    run_core(cpu1, memory);
    //run_core(cpu2, memory);
    //run_core(cpu3, memory);

    // Step 4: free memory
    //free_main_memory(memory);

    return 0;
}
*/


// check the core functions
//int main()
int main(int argc, char* argv[])
{
    // Step 1: Initialize the cpu
    processor* cpu = init_processor();
    if (!cpu) {
        perror("Failed to allocate memory for the cpu");
        exit(EXIT_FAILURE);
    }
    if (argc == 28) {
        set_file_names(cpu->filenames, argv);
    }
    // Step 2: Initialize the main memory from the file "memin.txt"
    main_memory* memory = init_main_memory(cpu->filenames->memin_str);
    if (!memory) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }

    // Step 3: Run the cpu
    run(cpu, memory);

    // Step 4: free memory
    free_main_memory(memory);
    free_processor(cpu);

    return 0;
}



