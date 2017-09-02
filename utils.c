//
// Created by depaulicious on 10/08/17.
//

// start:includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "utils.h"
// end:includes

// start:definitions
// Shortcuts for commonly used functions

/*
 * {m,c,re}alloc_or_die: wrappers for malloc, calloc, realloc
 * that `exit(1)` if memory allocation failed.
 */

inline void *malloc_or_die(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        exit(3);
    }
    return ptr;
}

inline void *calloc_or_die(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
    if (ptr == NULL) {
        exit(3);
    }
    return ptr;
}

inline void *realloc_or_die(void *ptr, size_t size) {
    void *newptr = realloc(ptr, size);
    if (newptr == NULL) {
        exit(3);
    }
    return newptr;
}

/*
 * Implementation of POSIX `getline`.
 * Reads `stream` until a new line character is found.
 * `**lineptr` is where the read line will be stored. It must be
 * allocated with {m,c,re}alloc. It will be realloc'd if there's
 * not enough space to store the line. `*n` is its initial size.
 * It will be updated if a realloc was needed.
 * Returns the number of characters read, or -1 on error.
 */

ssize_t getline_depau(char **lineptr, size_t *n, FILE *stream) {
    ssize_t charsread;
    char *ptr;

    if (*lineptr == (char *) NULL) {
        *lineptr = malloc_or_die(BASE_BUF_SIZE * sizeof(char));
    }
    if (*n == 0)
        *n = BASE_BUF_SIZE;

    ptr = *lineptr;

    // charsread starts at 1 so we are already counting the terminator
    for (charsread = 1;; charsread++) {
        char c = (char) getc(stream);

        // allocate more space for command line
        if ((size_t) charsread > *n - 2) {
            *n += BASE_BUF_SIZE * sizeof(char);
            *lineptr = realloc_or_die(*lineptr, *n);
            ptr = *lineptr + (charsread - 1);
        }

        if (c == EOF) {
            *ptr = '\0';
            return -1;
        }

        if (c == '\n') {
            *ptr = '\0';
            return charsread;
        }

        *ptr = c;
        ptr++;
    }
}

/*
 * Reimplementation of POSIX `strtok_r`.
 */

char *strtok_depau(char *s, const char *delim, char **save_ptr) {
    return strtok_escape(s, delim, save_ptr, (char)(intptr_t) NULL);
}

/*
 * Implementation of `strtok_r` that also allows an `escape_char`
 * to be specified. If a delimiter character is preceded by `escape_char`,
 * it will not be considered a delimiter.
 */

char *strtok_escape(char *s, const char *delim, char **save_ptr, char escape_char) {
    char *end;
    size_t span = 0;
    if (s == NULL)
        s = *save_ptr;
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }
    // Scan leading delimiters.
    s += strspn(s, delim);
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }
    // Find the end of the token.
    span = strcspn(s, delim);
    if (span > 0) {
        // Check for escape character
        end = s + span;
        while (escape_char != (char)(intptr_t) NULL && *(end-1) == escape_char) {
            end++;
            end = end + strcspn(end, delim);
        }
    } else {
        end = s;
    }
    if (*end == '\0') {
        *save_ptr = end;
        return s;
    }
    // Terminate the token and make *s point past it.
    *end = '\0';
    *save_ptr = end + 1;
    return s;
}

/*
 * Wraps `strtok_escape` in order to read the parts of a command from input.
 * Returns pointers to the beginning of each space-separated token of input.
 * If a token starts with a double-quote, the whole substring surrounded by
 * double-quotes is returned. Double-quotes can be escaped with \
 */

char *readcmd(char *s, char **save_ptr) {
    uint8_t quoted = 0;

    // Check current token starts with ". If it does, consider it as one token
    if (s != NULL) {
        if (*s == '"') {
            s++;
            quoted = 1;
        }
    } else {
        if (**save_ptr == '"') {
            (*save_ptr)++;
            quoted = 1;
        }
    }
    if (quoted)
        return strtok_escape(s, "\"", save_ptr, '\\');
    return strtok_depau(s, " \n", save_ptr);
}

/*
 * Concatenate `s2` on top of `s1` into a new string.
 * A pointer to the new string is returned.
 */

char *strcat_auto(int n_args, ...) {
    va_list ap;
    char *ptr;
    size_t pos = 0;
    size_t len = BASE_BUF_SIZE;
    char *newstr = malloc_or_die(len*sizeof(char));

    va_start(ap, n_args);

    // Iterate over the two strings
    for (int i = 0; i < n_args; i++) {
        ptr = va_arg(ap, char *);

        if (ptr == NULL)
            continue;

        // Copy characters from strings
        for (; *ptr != '\0'; ptr++) {
            // If no room for next char + terminator, realloc
            if (pos + 2 > len) {
                len += BASE_BUF_SIZE;
                newstr = realloc_or_die(newstr, len);
            }
            newstr[pos] = *ptr;
            pos++;
        }
    }
    newstr[pos] = '\0';
    va_end(ap);
    return newstr;
}


/*
 * Print "ok" if ret is 0, "no" if ret < 0, "ok `ret`" if ret > 0.
 */

inline void print_status(int ret) {
    if (ret < 0)
        printf("no\n");
    else if (ret == 0)
        printf("ok\n");
    else
        printf("ok %i\n", ret);
}

/*
 * Reads one string argument from `cmd`, then calls `func` passing
 * `root` and that argument to it.
 */

inline void call_with_1(int (*func)(fs_node_t *, char *), fs_node_t *root, char *cmd) {
    char *save_ptr;
    char *arg1 = readcmd(cmd, &save_ptr);
    int ret = (*func)(root, arg1);
    print_status(ret);
}

/*
 * Reads two string arguments from `cmd`, then calls `func` passing
 * `root` and those arguments to it.
 */

inline void call_with_2(int (*func)(fs_node_t *, char *, char *), fs_node_t *root, char *cmd) {
    char *save_ptr;
    char *arg1 = readcmd(cmd, &save_ptr);
    char *arg2 = readcmd(NULL, &save_ptr);

    int ret = (*func)(root, arg1, arg2);
    print_status(ret);
}

/*
 * SuperFastHash by Paul Hsieh
 * http://www.azillionmonkeys.com/qed/hash.html
 */
uint32_t hash(const char *data, size_t len) {
    int rem;
    uint32_t tmp;
    uint32_t hash = (uint32_t) len;

    if (len <= 0 || data == NULL) return 0;

    rem = (int) len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits(data);
        tmp    = (get16bits(data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits(data);
            hash ^= hash << 16;
            hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
            hash += hash >> 11;
            break;
        case 2: hash += get16bits(data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1: hash += (signed char)*data;
            hash ^= hash << 10;
            hash += hash >> 1;
        default:break;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

#ifdef DEBUG
unsigned long linecount = 0;

unsigned long get_linecount() {
    return linecount;
}

void increment_linecount() {
    linecount++;
}

#endif

// end:definitions