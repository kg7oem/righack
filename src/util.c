/*
 * util.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 *
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "guts.h"
#include "util.h"

void
util__fatal_va(enum log_source source, enum log_level level, const char *function, const char *path, int line, const char *fmt, ...) {
    va_list args, copy;

    va_start(args, fmt);
    va_copy(copy, args);
    va_end(args);

    log__level_args(source, level, function, path, line, fmt, args);
    guts_exit(exit_fatal);
}

// FIXME rename this to util_zalloc() - zero alloc, always zeros things
void *
util_zalloc(size_t bytes) {
    return ad_calloc(1, bytes);
}

// FIXME make this be a realloc() that also has zerod memory
// void * util_zrealloc(void *, int)

// FIXME make ths be a zrealloc() that also frees the original memory
// void * util_zreallocf(void *, int)

char *
util_strndup(const char *s, size_t n) {
    char *string = strndup(s, n);

    if (string == NULL) {
        util_fatal("Could not strndup(): %m");
    }

    return string;
}

char *
util_strdup(const char *s) {
    char *string = strdup(s);

    if (string == NULL) {
        util_fatal("Could not strdup(): %m");
    }

    return string;
}

void *
util_memdup(void *p, size_t len) {
    void *ret = util_zalloc(len);
    memcpy(ret, p, len);
    return ret;
}
