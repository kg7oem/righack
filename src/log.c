/*
 * log.c
 *
 *  Created on: Jun 2, 2018
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "guts.h"
#include "log.h"
#include "types.h"
#include "util.h"

enum log_level current_level = log_level_debug;

const char *
log_levelname(enum log_level level) {
    switch (level) {
    case log_level_fatal:
        return "FATAL";
    case log_level_warn:
        return "WARN";
    case log_level_notice:
        return "NOTICE";
    case log_level_info:
        return "INFO";
    case log_level_verbose:
        return "VERBOSE";
    case log_level_debug:
        return "DEBUG";
    case log_level_lots:
        return "LOTS";
    case log_level_trace:
        return "TRACE";
    }

    // control should never get here and if it does a core
    // would be nice to have
    abort();
}

static const char *
log_basename(const char *path) {
    const char *filename = path;
    for(const char *p = path; *p; p++) {
        if (*p == '/') {
            filename = ++p;
        }
    }

    return filename;
}

// FIXME figure out how to get the common parts of the log__level* functions
// into their own functions
void
log__level(UNUSED enum log_level level, const char *function, const char *path, int line, const char *fmt, ...) {
    if (level < current_level) {
        return;
    }

    const char *file = log_basename(path);
    static TLOCAL char *message_buf = NULL;
    const char *level_name = log_levelname(level);
    va_list args;

    if (message_buf == NULL) {
        // don't use the project memory management
        // to avoid bootstrap issues
        message_buf = malloc(CONFIG_LOG_SIZE);
        if (message_buf == NULL) {
            perror("could not malloc: ");
            guts_exit(exit_nomem);
        }
    }

    va_start(args, fmt);
    // FIXME detect when the message does not fit
    vsnprintf(message_buf, CONFIG_LOG_SIZE, fmt, args);
    va_end(args);

    FILE *output = stdout;
    if (level >= log_level_warn) {
        output = stderr;
    }

    fprintf(output, "%s %s:%d %s() %s\n", level_name, file, line, function, message_buf);
}

void
log__level_exit(UNUSED enum log_level level, const char *function, const char *path, int line, const char *fmt, ...) {
    static TLOCAL char *message_buf = NULL;
    FILE *output = stdout;
    va_list args;

    if (message_buf == NULL) {
        message_buf = ad_malloc(CONFIG_LOG_SIZE);
    }

    if (level >= current_level) {
        const char *file = log_basename(path);
        const char *level_name = log_levelname(level);

        va_start(args, fmt);
        // FIXME detect when the message does not fit
        vsnprintf(message_buf, CONFIG_LOG_SIZE, fmt, args);
        va_end(args);

        if (level >= log_level_warn) {
            output = stderr;
        }

        fprintf(output, "%s %s:%d %s() %s\n", level_name, file, line, function, message_buf);
    }

    guts_exit(exit_fatal);
}
