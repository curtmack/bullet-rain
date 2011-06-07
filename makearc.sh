#!/bin/sh
# Make a bullet rain .arc file from a directory
# Usage: makearc <directory>

# replace with whatever you have that's compatible
TAR="tar -czf"

# set up names
BASENAME=`basename $1`
ARCNAME="${BASENAME}.arc"

# Perform compression
echo "Making archive ..."
echo "cd $1"
cd $1
echo "${TAR} ${ARCNAME} *"
${TAR} ${ARCNAME} *
echo "mv ${ARCNAME} .."
mv ${ARCNAME} ..
echo "Done!"
