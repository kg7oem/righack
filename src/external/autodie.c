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
// void * ad_malloc(size_t size)
//
//
// RULES
//
// 1. The functions must exist in libc - no making new ones
// 2. The function signatures must match the original
// 3. No behavior changes except for error checking
// 4. No bug fixes - don't do anything the libc didn't do
// 5. Do not malloc - these functions might get called in a situation where
//    malloc() has already failed and there is no available RAM
// 6. no macros - users must be able to get a function pointer
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
// thread local tracking of reentrance
__thread bool inside_autodie = false;

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

#define autodie_invoke(...) autodie__invoke(__func__, __VA_ARGS__)

// no using autodie functions in here or in anything called by it
static void
autodie__invoke(const char *function, int error_value, const char *fmt, ...) {
    bool should_abort = false;
    char buf[MESSAGE_MAX_LEN];
    const char *message;
    va_list args;

    if (inside_autodie) {
        fprintf(stderr, "autodie: handler went reentrant\n");
        fprintf(stderr, "autodie: function = %s\n", function);
        abort();
    }

    inside_autodie = true;

    if (fmt == NULL) {
        message = "(no message)";
    } else {
        va_start(args, fmt);
        // FIXME detect when the message does not fit
        vsnprintf(buf, MESSAGE_MAX_LEN, fmt, args);
        va_end(args);

        message = buf;
    }

    if (die_handler != NULL) {
        die_handler(function, error_value, message);
        fprintf(stderr, "autodie: registered die handler did not stop the program\n");
        should_abort = 1;
    }

    fprintf(stderr, "autodie %s(): %s: %s\n", function, message, strerror(error_value));

    if (should_abort) {
        abort();
    }

    exit(autodie_exit_value);
}

void
ad__test_(void) {
    autodie_invoke(EADDRINUSE, "forced failure induced by test");
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

// FIXME There is a case here that should probably break the rules
// malloc() with a size of zero can return either NULL or a pointer
// which must go to free(). With a size of zero getting NULL back
// is not a failure.
//
// Since ad_malloc() should never return NULL (that's the point)
// it seems like it should be fatal to attempt to use a size of
// zero to enforce predictable behavior.
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

