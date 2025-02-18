#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sram.h"
#include "core.h"
#include "memory.h"
#include "processor.h"


// check the core functions
//int main()
int main(int argc, char* argv[])
{
    // Step 1: Initialize the cpu
    filenames *file_names = malloc(sizeof(filenames));
    if (!file_names) {
        perror("Failed to allocate memory for filenames");
        exit(EXIT_FAILURE);
    }
    if (argc == 28) {
        set_file_names(file_names, argv);
    }else{
        set_default_file_names(file_names);
    }
    processor *cpu = init_processor(file_names);
    if (!cpu)
    {
        perror("Failed to allocate memory for the cpu");
        exit(EXIT_FAILURE);
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

    return 0;
}
