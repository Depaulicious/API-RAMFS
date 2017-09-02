//
// Created by depaulicious on 10/08/17.
//

#ifndef API_RAMFS_UTILS_H
#define API_RAMFS_UTILS_H

// start:includes
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "ramfs.h"
// end:includes

// start:macros
#define BASE_BUF_SIZE 64

// Add ssize_t if missing
#if !defined(ssize_t)
typedef intmax_t ssize_t;
#endif

// end:macros

// start:declarations
void *malloc_or_die(size_t size);
void *calloc_or_die(size_t nmemb, size_t size);
void *realloc_or_die(void *ptr, size_t size);
ssize_t getline_depau(char **lineptr, size_t *n, FILE *stream);
char *strtok_depau(char *s, const char *delim, char **save_ptr);
char *strtok_escape(char *s, const char *delim, char **save_ptr, char escape_char);
char *readcmd(char *s, char **save_ptr);
char *strcat_auto(int n_args, ...);

void print_status(int ret);
void call_with_1(int (*func)(fs_node_t *, char *string), fs_node_t *root, char *cmd);
void call_with_2(int (*func)(fs_node_t *, char *, char *string), fs_node_t *root, char *cmd);

uint32_t hash(const char * data, size_t len);

#ifdef DEBUG
unsigned long get_linecount();
void increment_linecount();
#endif
// end:declarations

#endif //API_RAMFS_UTILS_H
