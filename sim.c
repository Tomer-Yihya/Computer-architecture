#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sram.h"
#include "core.h"
#include "memory.h"




// check the core functions
int main() {

    // Step 1: Initialize the cores from the files "imem0.txt", "imem1.txt", "imem2.txt", "imem3.txt"
    core* cpu0 = core_initialization(0,"imem0.txt");
    core* cpu1 = core_initialization(1,"imem1.txt");
    core* cpu2 = core_initialization(2,"imem2.txt");
    core* cpu3 = core_initialization(3,"imem3.txt");
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



/*
// check the core functions
//int main() {
int main(int argc, char* argv[])
{
    filesnames* filesnames = NULL;
    if (argc == 28) {
        init_files_name(filesnames, argv);
    }
    else{
        defult_files_name(filesnames);
    }
    // Step 1: Initialize the cpu
    processor* cpu = init_processor(filesnames);
    if (!cpu) {
        perror("Failed to allocate memory for the cpu");
        exit(EXIT_FAILURE);
    }
    // Step 2: Initialize the main memory from the file "memin.txt"
    main_memory* memory = main_memory_initialization(filesnames);
    if (!memory) {
        perror("Failed to allocate memory for core");
        exit(EXIT_FAILURE);
    }

    // Step 3: Run the cpu
    void run(filesnames, cpu, memory);


    free_main_memory(memory);

    return 0;
}
*/



