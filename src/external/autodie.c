/*
 * autodie.c
 *
 *  Created on: Jun 3, 2018
 *      Author: Tyler Riddle
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

// versions of libc and system functions that will never return
// unless there is a success. When ever a failure happens execution
// of the program is stopped.
//
// For any given function in libc there exists (if implemented) an
// autodie enabled version with the same function prototype and with
// the same name with an ad_ prefix, such as
//
// void * ad_malloc(size_t size);
//
//
// RULES
//
// 1. The functions must exist in libc - no making new ones
// 2. No behavior changes except for error checking
// 3. No bug fixes - don't do anything the libc didn't do
//
// tl;dr - don't do anything but error checking


#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autodie.h"

#define MESSAGE_MAX_LEN 1024

autodie_handler_t die_handler;
int autodie_exit_value = 1;

void
autodie_register_handler(autodie_handler_t handler) {
    die_handler = handler;
}

int
autodie_get_exit_value(void) {
    return autodie_exit_value;
}

int
autodie_set_exit_value(int new_value) {
    int old_value = autodie_exit_value;
    autodie_exit_value = new_value;
    return old_value;
}

static void
autodie_invoke(int error_value, const char *fmt, ...) {
    bool should_abort = false;
    char buf[MESSAGE_MAX_LEN];
    const char *message;
    va_list args;

    if (fmt == NULL) {
        message = "(No error message supplied)";
    } else {
        va_start(args, fmt);
        // FIXME detect when the message does not fit
        vsnprintf(buf, MESSAGE_MAX_LEN, fmt, args);
        va_end(args);

        message = buf;
    }

    if (die_handler != NULL) {
        die_handler(error_value, message);
        fprintf(stderr, "autodie: registered die handler did not stop the program\n");
        should_abort = 1;
    }

    fprintf(stderr, "autodie: %s: %s\n", message, strerror(error_value));

    if (should_abort) {
        abort();
    }

    exit(autodie_exit_value);
}

// stdlib.h

void *
ad_calloc(size_t nmemb, size_t size) {
    void *p = calloc(nmemb, size);

    if (p == NULL) {
        autodie_invoke(errno, "Could not allocate %d * %d = %d bytes", nmemb, size, nmemb * size);
    }

    return p;
}

void *
ad_malloc(size_t bytes) {
    void *p = malloc(bytes);

    if (p == NULL) {
        autodie_invoke(errno, "Could not allocate %d bytes", bytes);
    }

    return p;
}

// string.h

char *
ad_strdup(const char *s) {
    char *p = strdup(s);

    if (p == NULL) {
        autodie_invoke(errno, NULL);
    }

    return p;
}

