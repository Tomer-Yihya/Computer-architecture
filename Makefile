# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Target executable
TARGET = sim.exe

# Source files
SRC = sim.c core.c sram.c memory.c

# Header files
HEADERS = core.h sram.h memory.h

# Default target: build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRC)
	@$(CC) $(CFLAGS) -o $@ $(SRC)

# Clean target to remove build artifacts
clean:
	@cmd /c del $(TARGET) >nul 2>&1


