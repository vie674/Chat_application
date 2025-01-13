# Makefile for the chat_app project

# Project name and executable target
TARGET = chat_app

# Compiler and flags
CC = gcc
CFLAGS = -Wall -I.

# Debug and Release build flags
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -O2

# Source and object files
SRCS = main.c ConnectionManager.c TCPSocket.c
OBJS = $(SRCS:.c=.o)

# Default target (build debug by default)
all: debug

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Release build
release: CFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lpthread

# Rule to compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to avoid conflicts with file names
.PHONY: all debug release clean
