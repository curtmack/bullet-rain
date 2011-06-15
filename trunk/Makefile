#
# Makefile for bullet-rain
# Licensed under the MIT license
# See LICENSE.TXT for details
# See also README.TXT for compilation instructions
#

# These macros should be changed to fit your compilation needs
# GCC-ish C compiler
CC = gcc
# CC flags
CFLAGS = -Wall `sdl-config --cflags`
# GCC-ish linker
LINK = gcc
# LINK flags
LFLAGS = -Wall `sdl-config --cflags`
# Library switches
LIBS = -larchive -lpthread `sdl-config --libs`
# Executable extension
EXE = .exe
# Output directory
BIN = bin/debug
# File deleting program, preferably one that ignores missing files
#   (via commandline switch if necessary)
RM = rm -f

# These macros speed up typing, you shouldn't need to change them
OBJS = src/main.o src/debug.o src/resource.o src/geometry.o src/fixed.o

# Make definitions follow
# Default target

$(BIN)/bullet-rain$(EXE): $(OBJS)
	$(LINK) $(LFLAGS) $(OBJS) $(LIBS) -o $(BIN)/bullet-rain$(EXE)

# Object files

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
	
# Source files - used to detect which files need to be updated
#   when header files change

# c files
src/main.c: src/compile.h src/debug.h src/fixed.h src/resource.h src/geometry.h

src/debug.c: src/compile.h src/debug.h

src/fixed.c: src/fixed.h

src/resource.c: src/compile.h src/debug.h src/resource.h

src/geometry.c: src/geometry.h src/fixed.h

# header files
src/debug.h: src/compile.h

src/fixed.h: src/debug.h

src/resource.h: src/compile.h

src/geometry.h: src/fixed.h
# compile.h doesn't depend on anything

# Clean target
clean:
	- $(RM) $(OBJS)
	- $(RM) $(BIN)/bullet-rain$(EXE)
