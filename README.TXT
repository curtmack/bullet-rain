REQUIRED LIBRARIES:

 - C standard library
 - libarchive
 - SDL
   - SDL_image
   x SDL_mixer, which MUST be configured for OGG files
   - SDL_ttf
 x Lua

(x means "Not actually used or required yet, but will be some day")

Note that you will need the development versions of these libraries to
build the engine.

ADDITIONALLY, if you are going to make a SYSTEM_TEST build (currently,
the only kind of build), then you'll need to supply a font for the
program to use. I couldn't find any monospaced fonts with licenses
that permit me to distribute them with the source, so you're on your
own. I recommend ProFont, which is free:

http://www.tobias-jung.de/seekingprofont/

Whatever font you use, rename it monofont.ttf and put it in the res
directory.


COMPILING:

Since everything is still rather crude, you'll need to edit everything
yourself like a caveman.

If you're using Linux or MinGW, there are some defaults already set up.
Rename or copy Makefile.______ to Makefile, and src/compile.h.______
to src/compile.h, where ______ is the system in question. You may still
need to fiddle with the settings, however.

If you're not using either system, pick whichever one you think is the
closest, and start with that.

Open up src/compile.h and change the compilation settings to what you
need for your platform. The comments should help you, but you're mostly
on your own here.

Then, you may need to edit the Makefile. All of the definitions should
be fine as they are, but you might need to tweak the variables at the
beginning:

 - CC:     The name (including path if necessary) of a C compiler. The
           make process assumes it supports gcc-like commandline switches
           for specifying output file, compilation mode, etc. In fact,
           maybe you should just use gcc.
 - CFLAGS: Miscellaneous commandline flags given to CC.
 - LINK:   The name of a linker that accepts whatever object format CC
           spits out. NOTE: This varies for different C compilers. gcc,
           for example, requires that you send .o files through gcc
           rather than directly through ld, because gcc has some built-in
           libraries it passes to ld. In this case, you should use gcc as
           both CC and LINK.
 - LFLAGS: Miscellaneous commandline flags given to LINK.
 - LIBS:   Commandline switches given to the compiler to import the
           necessary libraries. This should be fine as-is.
 - BIN:    The name of the output file. Include a .exe if your system
           requires it. "bullet-hell" is the standard base, but of
           course you can use whatever you fancy.
 - RM:     A program that deletes files specified on its commandline.
           Used for "make clean." rm -f or del are the usual options.

Your version of make will need to understand and accept / as a
directory separator, even if that's not the native separator for your
system. Alternatively, you can go through and fix them all yourself.
This is left as an exercise to the reader.
