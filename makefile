# Makefile for building and testing the hash map

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Executable name
TARGET = test_hashmap

# Source files
SRCS = hash_test.c

# Header files
HEADERS = hashmap.h

# Object files (generated from the source files)
OBJS = $(SRCS:.c=.o)

# Default target: build the executable
all: $(TARGET)

# Build the executable from the object files
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Rule to compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean the build
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program with valgrind (automated memory check)
run: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Phony targets (not actual files)
.PHONY: all clean run
