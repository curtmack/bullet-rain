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
LIBS = -larchive -lpthread `sdl-config --libs` -lSDL_ttf -lSDL_image
# Executable extension
EXE = .exe
# File deleting program, preferably one that ignores missing files
#   (via commandline switch if necessary)
RM = rm -f

# These macros speed up typing, you shouldn't need to change them
OBJS = src/main.o src/debug.o src/resource.o src/geometry.o src/fixed.o \
       src/endian.o src/menu.o src/init.o
# Debugging objects, you'll see why we need these separately
DOBJS = src/main.do src/debug.do src/resource.do src/geometry.do src/fixed.do \
		src/endian.do src/menu.do src/init.do

# Make definitions follow
# Default target

debug: bullet-rain-debug$(EXE)

# Currently have nothing to do here
# release: bullet-rain$(EXE)

bullet-rain-debug$(EXE): $(DOBJS)
	$(LINK) $(LFLAGS) $(DOBJS) $(LIBS) -d -o bullet-rain-debug$(EXE)

# Object files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Here we do some naught tricks to get object files with debugging flags
%.do: %.c
	$(CC) $(CFLAGS) -g -c $< -o $@

# Clean target
clean:
	- $(RM) $(OBJS)
	- $(RM) $(DOBJS)
	- $(RM) bullet-rain-debug$(EXE)
#	- $(RM) bullet-rain$(EXE)
