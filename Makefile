# Compiler and flags
# CC = arm-none-eabi-gcc
CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra

# Test compiler and flags
# CXX = arm-none-eabi-g++
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra

# Source and header files
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

# Test source and object files
TEST_SOURCES = $(wildcard tests/*.cc)
TEST_OBJECTS = $(TEST_SOURCES:.cc=.o)

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Library file
LIBRARY = libshinyallocator.a

# Default target
all: test

# Test target
test: $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) -o test -lgtest -lpthread
	./test
	rm -f test
# Clean target
clean:
	rm -rf $(OBJECTS) $(LIBRARY) $(TEST_OBJECTS)

# Release target
release: CFLAGS += -O3
release: $(LIBRARY)

# Documentation target
docs: FORCE
	doxygen Doxyfile

# Force target
.PHONY: FORCE