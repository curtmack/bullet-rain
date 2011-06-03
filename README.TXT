REQUIRED LIBRARIES:

 - C standard library
 - libarchive
 x SDL
 x Lua

(x means "Not actually used or required yet, but will be some day")

Note that you will need the development versions of these libraries to
build the engine.


COMPILING:

Since everything is still rather crude, you'll need to edit everything
yourself like a caveman.

Open up src/compile.h and change the compilation settings to what you
need for your platform. The comments should help you, but you're mostly
on your own here.

Then, you may need to edit the Makefile. All of the definitions should
be fine as they are, but you might need to tweak the variables at the
beginning:

 - CC:   The name (including path if necessary) of a C compiler. The
         make process assumes it supports GCC-like commandline switches
         for specifying output file, compilation mode, etc. In fact,
         maybe you should just use GCC.
 - LINK: The name of a linker that accepts whatever object format CC
         spits out. NOTE: This varies for different C compilers. GCC,
		 for example, requires that you send .o files through gcc
		 rather than directly through ld, because gcc has some built-in
		 libraries it passes to ld. In this case, you should use gcc as
		 both CC and LINK.
 - LIBS: Commandline switches given to the compiler to import the
         necessary libraries. This should be fine as-is.
 - EXE:  Binary extension. For example, .exe on Windows, nothing on
         UNIX. Include the dot.
 - BIN:  Directory to put it in. Usually "bin/debug" for debug builds,
         "bin/release" for release builds.
 - RM:   A program that deletes files specified on its commandline.
         Used for "make clean." rm or del are the usual options.

Your version of make will need to understand and accept / as a
directory separator, even if that's not the native separator for your
system. Alternatively, you can go through and fix them all yourself.
This is left as an exercise to the reader.