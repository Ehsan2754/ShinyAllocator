# Compiler and flags
CC = arm-none-eabi-gcc
# CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra

# Test compiler and flags
CXX = arm-none-eabi-g++
CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra
LIBSXX= -lgtest -lpthread 
# GTEST_FILTER="logging*"


# GDB
# GBD = arm-none-eabi-gdb
GDB = gdb
# Source and header files
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

# Test source and object files
TEST_SOURCES = $(wildcard tests/*.cc)
TEST_OBJECTS = $(TEST_SOURCES:.cc=.oo)

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Library file
LIBRARY = libshinyallocator.so

# Default target
all: build test



# Rule to build the library
$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^


# Rule to build the object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

# Rule to build the test object files
%.oo: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

# Build target
build: $(LIBRARY)

#Build the Qemu kernel
cortexm0:
	cd ./CortexM0 && make clean && make && mv ./build/CortexM0.elf ../kernel.elf
	./scripts/memAnalyse.sh kernel.elf > release_note.md

# Test target
test: $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -Iinclude -o unitTests $(TEST_OBJECTS) $(SOURCES) $(LIBSXX)
	./unitTests #--gtest_filter=$(GTEST_FILTER)
	rm -f unitTests

# Leak check with Valgrind
valgrind: $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -Iinclude -o unitTests $(TEST_OBJECTS) $(SOURCES) $(LIBSXX)
	@valgrind --tool=callgrind --callgrind-out-file=shinyProfile.valgrind ./unitTests #--gtest_filter=$(GTEST_FILTER)
	valgrind --leak-check=full ./unitTests #--gtest_filter=$(GTEST_FILTER)
	kcachegrind shinyProfile.valgrind&
	rm -f unitTests 


# Debug target
debug: CXXFLAGS += -g
debug: $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -Iinclude -o unitTests $(TEST_OBJECTS) $(SOURCES) $(LIBSXX)
	$(GDB) ./unitTests
	rm -f unitTests


# Release target
release: CFLAGS += -O3
release: build

# Clean target
clean:
	rm -rf $(OBJECTS) $(LIBRARY) $(TEST_OBJECTS) unitTests shinyProfile.valgrind *.elf

# Documentation target
docs: FORCE
	doxygen Doxyfile

# Force target
.PHONY: FORCE