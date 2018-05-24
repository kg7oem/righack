/*
 * util.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void *
util_malloc(size_t bytes) {
    void *p = calloc(1, bytes);

    if (p == NULL) {
        util_fatal_perror("Could not allocate %d bytes: ", bytes);
    }

    return p;
}

void
util_fatal(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    abort();
}

void
util_fatal_perror(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    perror("");
    abort();
}
