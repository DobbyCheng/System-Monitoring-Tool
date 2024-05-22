CC=gcc
CFLAGS=-Wall -Werror -std=c99

# Object files for compilation
OBJ=system.o stats_functions.o

# Target executable name
TARGET=system

# Default target
all: $(TARGET)

# Compile and link the program
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

# To compile individual .c files into .o files
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Help information
help:
	@echo "Available commands:"
	@echo "  all      : Compiles and links the program (default)"
	@echo "  clean    : Removes object files and the executable"
	@echo "  help     : Displays this help message"

# Rule to clean compiled files
clean:
	rm -f $(OBJ) $(TARGET)

