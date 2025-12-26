# Compiler
CC = clang
CFLAGS = -Wall -Wextra -Iinclude -g

# Directories
SRC_DIR = impl
INCLUDE_DIR = include
OBJ_DIR = obj
BIN = main

# Sources and objects
SRCS = $(wildcard $(SRC_DIR)/*.c) main.c
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(notdir $(SRCS)))

# Default target
all: $(BIN)

# Link object files to create executable
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Special case for main.c in root
$(OBJ_DIR)/main.o: main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean

