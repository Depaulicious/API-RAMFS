#!/bin/bash

# Get c2singlefile from https://github.com/Depaulicious/c2singlefile
which c2singlefile > /dev/null

file="singlefile.c"

if [ $? == 0 ]; then
	file="singlefile1.c"
	c2singlefile utils.h utils.c hashtable.h hashtable.c ramfs.h ramfs.c ramfs_wrapped.h ramfs_wrapped.c main.c > $file
fi

gcc -DEVAL -static -std=c99 -O2 -o api-ramfs $file -lm
