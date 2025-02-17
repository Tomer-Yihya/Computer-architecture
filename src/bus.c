#include "bus.h"

Bus bus;
bool bus_busy = false;
char bus_delay = 0;
bool bus_read = false;
bool bus_write = false;
char data_source = 4;
char first_flush = 4;
uint32_t flush_address = 0;
static FILE *bustrace_file;
bool extra_cycle = true;

void update_cycle()
{
    if (bus_delay > 0)
        bus_delay--;
}

void set_bus(char orig_id, enum BusCmd bus_cmd, uint32_t bus_addr, uint32_t bus_data)
{
    bus.orig_id = orig_id;
    bus.bus_cmd = bus_cmd;
    bus.bus_addr = bus_addr;
    bus.bus_data = bus_data;
    bus.bus_shared = false;
}

void set_shared()
{
    bus.bus_shared = true;
}

void create_bustrace_file(processor *cpu)
{
    bustrace_file = NULL;
    open_file(&bustrace_file, cpu->filenames->bustrace_str, "w");
}

void close_bustrace_file()
{
    fclose(bustrace_file);
}

void write_line_to_bustrace_file(processor *cpu, uint32_t cycle)
{
    if (!bustrace_file)
    {
        printf("Error: Invalid file pointer or uninitialized core/cache.\n");
        return;
    }
    // Write the clock cycle number
    fprintf(bustrace_file, "%d ", cycle);
    fprintf(bustrace_file, "%d ", bus.orig_id);
    fprintf(bustrace_file, "%d ", bus.bus_cmd);
    fprintf(bustrace_file, "%05X ", bus.bus_addr);
    fprintf(bustrace_file, "%08X ", bus.bus_data);
    fprintf(bustrace_file, "%d \n", bus.bus_shared);
}