# Makefile for NeuroChef

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Target executable
TARGET = neurochef

# Source files
SRC = main.c

# Object files
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)

# Run tests
test:
	poetry run pytest

# Install Python dependencies
install:
	poetry install

# Run the chatbot
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean test install run
