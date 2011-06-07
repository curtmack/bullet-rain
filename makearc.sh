#!/bin/sh
# Make a bullet rain .arc file from a directory
# Usage: makearc <directory>

# replace with whatever you have that's compatible
TAR = "tar -czf"

# Perform compression
${TAR} $1.arc $1/*
