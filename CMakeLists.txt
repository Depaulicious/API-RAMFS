cmake_minimum_required(VERSION 3.8)
project(API_RAMFS)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "-std=c99")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(SOURCE_FILES main.c utils.c utils.h ramfs_wrapped.c ramfs_wrapped.h ramfs.c ramfs.h hashtable.c hashtable.h)
add_executable(API_RAMFS ${SOURCE_FILES})