CC      := clang
TARGET  := app

SRC     := main.c impl/obfs.c
OBJ     := $(SRC:.c=.o)

INC_DIR := include

# Common flags
CFLAGS_COMMON := -std=c11 -Wall -Wextra -Wpedantic -I$(INC_DIR)

# Profiles
CFLAGS_DEV     := -g -O0 -fsanitize=address,undefined
CFLAGS_RELEASE := -O3 -DNDEBUG

# Default profile
CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_DEV)

.PHONY: all dev release clean

all: dev

dev: CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_DEV)
dev: $(TARGET)

release: CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_RELEASE)
release: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

