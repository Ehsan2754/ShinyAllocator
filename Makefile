# Specify the compiler and flags
# CC = arm-none-eabi-gcc
CC = g++
CFLAGS = -std=c++11 -O2 -Wall -Wextra -fPIC

# Specify the source and header files
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)
DOC = docs/*
# Specify the object files
OBJECTS = $(SOURCES:.c=.o)

# Specify the library file
LIBRARY = libshinyallocator.a

# Default target
all: $(LIBRARY)

# Rule to build the library
$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

# Rule to build the object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Clean target
clean:
	rm -rf $(OBJECTS) $(LIBRARY) $(DOC)

# Release target
release: CFLAGS += -O3
release: $(LIBRARY)

# Documentation target
docs: FORCE
	doxygen Doxyfile

# Force target
.PHONY: FORCE