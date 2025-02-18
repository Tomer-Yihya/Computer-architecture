CC = gcc
CFLAGS = -g -Wall -I src
EXEC = sim.exe
SRC_DIR = src
SRCS = $(SRC_DIR)/sim.c $(SRC_DIR)/processor.c $(SRC_DIR)/core.c $(SRC_DIR)/memory.c $(SRC_DIR)/sram.c $(SRC_DIR)/bus.c
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

#ARGS = 1.txt 1.txt 1.txt 1.txt memin.txt memout.txt regout0.txt regout1.txt regout2.txt regout3.txt \
       core0trace.txt core1trace.txt core2trace.txt core3trace.txt bustrace.txt dsram0.txt dsram1.txt dsram2.txt dsram3.txt \
       tsram0.txt tsram1.txt tsram2.txt tsram3.txt stats0.txt stats1.txt stats2.txt stats3.txt

all: clean  # Ensure old trace files are deleted before recompiling
	$(CC) $(CFLAGS) -o $(EXEC) $(SRCS)

clean:
	rm -f $(EXEC) $(TRACE_FILES) $(REGOUT_FILES) $(STATS_FILES) $(DSRAM_FILES) $(TSRAM_FILES) $(BUS_FILE) $(MEM_FILE)

run: $(EXEC)
	./$(EXEC) $(ARGS)

FILES = memout.txt  \
		regout0.txt core0trace.txt stats0.txt dsram0.txt tsram0.txt \
        regout1.txt core1trace.txt stats1.txt dsram1.txt tsram1.txt \
        regout2.txt core2trace.txt stats2.txt dsram2.txt tsram2.txt \
        dsram3.txt tsram3.txt regout3.txt core3trace.txt stats3.txt \
        bustrace.txt

compare:
	@for file in $(FILES); do \
		echo -e "\033[36mComparing $$file with examples/$$file\033[0m"; \
		diff -y --suppress-common-lines $$file examples/$$file || echo "Differences found in $$file"; \
		echo -e "\033[33m---------------------------------------\033[0m"; \
	done