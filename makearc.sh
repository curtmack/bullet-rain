#!/bin/sh
# Make a bullet rain .arc file from a directory
# Usage: makearc <directory>

# replace with whatever you have that's compatible
TAR="tar -czf"

if [ -z "$1" ]
then
    echo "usage: sh makearc.sh [dirname]"
else

# set up names
BASENAME=`basename $1`
ARCNAME="${BASENAME}.tgz"

# Perform compression
echo "Making archive..."
echo "cd $1"
cd $1
echo "mkdir ../temp"
mkdir ../temp
echo "mv .svn ../temp"
mv .svn ../temp
echo "${TAR} ${ARCNAME} *"
${TAR} ${ARCNAME} *
echo "mv ${ARCNAME} .."
mv ${ARCNAME} ..
echo "mv ../temp/.svn ."
mv ../temp/.svn .
echo "rmdir ../temp"
rmdir ../temp
echo "Done!"

fi
