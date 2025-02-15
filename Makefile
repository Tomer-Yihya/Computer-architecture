CC = gcc
CFLAGS = -g -Wall
EXEC = sim.exe
SRCS = sim.c processor.c core.c memory.c sram.c
TRACE_FILES = core0trace.txt core1trace.txt core2trace.txt core3trace.txt
REGOUT_FILES = regout0.txt regout1.txt regout2.txt regout3.txt
STATS_FILES = stats0.txt stats1.txt stats2.txt stats3.txt
DSRAM_FILES = dsram0.txt dsram1.txt dsram2.txt dsram3.txt
TSRAM_FILES = tsram0.txt tsram1.txt tsram2.txt tsram3.txt
BUS_FILE = bustrace.txt
MEM_FILE = memout.txt

ARGS = imem0.txt imem1.txt imem2.txt imem3.txt memin.txt memout.txt regout0.txt regout1.txt regout2.txt regout3.txt \
       core0trace.txt core1trace.txt core2trace.txt core3trace.txt bustrace.txt dsram0.txt dsram1.txt dsram2.txt dsram3.txt \
       tsram0.txt tsram1.txt tsram2.txt tsram3.txt stats0.txt stats1.txt stats2.txt stats3.txt

all: clean  # Ensure old trace files are deleted before recompiling
	$(CC) $(CFLAGS) -o $(EXEC) $(SRCS)

clean:
	rm -f $(EXEC) $(TRACE_FILES) $(REGOUT_FILES) $(STATS_FILES) $(DSRAM_FILES) $(TSRAM_FILES) $(BUS_FILE) $(MEM_FILE)

run: $(EXEC)
	./$(EXEC) $(ARGS)