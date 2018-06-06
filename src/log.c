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

enum log_level
log_get_current_level(void) {
    return current_level;
}

enum log_level
log_set_current_level(enum log_level new_level) {
    enum log_level old_level = current_level;
    current_level = new_level;
    return old_level;
}

const char *
log_levelname(enum log_level level) {
    switch (level) {
    case log_level_fatal:
        return "FATAL";
    case log_level_error:
        return "ERROR";
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

const char *
log_sourcename(enum log_source source) {
    switch(source) {
    case log_source_unknown:
        return "unknown";
    case log_source_righack:
        return "righack";
    case log_source_hamlib:
        return "hamlib";
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

void log__level_args(enum log_source source, UNUSED enum log_level level, const char *function, const char *path, int line, const char *fmt, va_list args) {
    static TLOCAL char *message_buf = NULL;

    if (level < current_level) {
        return;
    }

    FILE *output = stdout;

    if (message_buf == NULL) {
        message_buf = ad_malloc(CONFIG_LOG_SIZE);
    }

    if (level >= log_level_warn) {
        output = stderr;
    }

    if (path == NULL) {
        path = "(no file)";
    }

    if (function == NULL) {
        function = "(no func)";
    }

    const char *file = log_basename(path);
    const char *level_name = log_levelname(level);
    const char *source_name = log_sourcename(source);

    // FIXME detect when the message does not fit
    vsnprintf(message_buf, CONFIG_LOG_SIZE, fmt, args);
    // FIXME detect errors on fprintf too
    fprintf(output, "%s %s %s:%d %s() %s\n", source_name, level_name, file, line, function, message_buf);
}

void
log__level_va(enum log_source source, enum log_level level, const char *function, const char *path, int line, const char *fmt, ...) {
    if (level < current_level) {
        return;
    }

    va_list args, copy;

    va_start(args, fmt);
    va_copy(copy, args);
    va_end(args);

    log__level_args(source, level, function, path, line, fmt, args);
}
