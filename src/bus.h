#ifndef BUS_H
#define BUS_H

#include "core.h"
#include "processor.h"

enum BusCmd
{
    NoCommand = 0,
    BusRd = 1,
    BusRdX = 2,
    Flush = 3
};

typedef struct
{
    char orig_id;
    enum BusCmd bus_cmd;
    uint32_t bus_addr;
    uint32_t bus_data;
    bool bus_shared;
} Bus;

extern Bus bus;
extern char data_source;
extern char first_flush;
extern uint32_t flush_address;
extern bool address_done;

void set_bus(char orig_id, enum BusCmd bus_cmd, uint32_t bus_addr, uint32_t bus_data);
void set_shared();
void create_bustrace_file(processor *cpu);
void close_bustrace_file();
void write_line_to_bustrace_file(processor *cpu, uint32_t cycle);

#endif // BUS_H
