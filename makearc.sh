#!/bin/sh
# Make a bullet rain .arc file from a directory
# Usage: makearc <directory>

# replace with whatever you have that's compatible
TAR = "tar -czf"

# Set up the name of the archive
BASENAME = `basename $1`
DIRNAME = `dirname $1`
ARCNAME = "${DIRNAME}/${BASENAME}.arc"

# Perform compression
${TAR} ${TARNAME} $1/*
