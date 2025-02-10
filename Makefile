CC = gcc
CFLAGS = -g -Wall
EXEC = sim.exe
SRCS = sim.c core.c memory.c sram.c
TRACE_FILES = core0trace.txt core1trace.txt core2trace.txt core3trace.txt
REGOUT_FILES = regout0.txt regout1.txt regout2.txt regout3.txt

all: clean  # Ensure old trace files are deleted before recompiling
	$(CC) $(CFLAGS) -o $(EXEC) $(SRCS)

clean:
	rm -f $(EXEC) $(TRACE_FILES) $(REGOUT_FILES)
