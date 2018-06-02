/*
 * util.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

// FIXME rename this to util_zalloc() - zero alloc, always zeros things
void *
util_malloc(size_t bytes) {
    void *p = calloc(1, bytes);

    if (p == NULL) {
        util_fatal_perror("Could not allocate %d bytes: ", bytes);
    }

    return p;
}

// FIXME make this be a realloc() that also has zerod memory
// void * util_zrealloc(void *, int)

// FIXME make ths be a zrealloc() that also frees the original memory
// void * util_zreallocf(void *, int)

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

char *
util_strndup(const char *s, size_t n) {
    char *string = strndup(s, n);

    if (string == NULL) {
        util_fatal_perror("Could not strndup(): ");
    }

    return string;
}

char *
util_strdup(const char *s) {
    char *string = strdup(s);

    if (string == NULL) {
        util_fatal_perror("Could not strdup(): ");
    }

    return string;
}
