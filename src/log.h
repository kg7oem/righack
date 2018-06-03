/*
 * log.h
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

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

enum log_level {
    log_level_fatal = 100,  // execution will stop
    log_level_warn = 7,     // output to stderr
    log_level_notice = 6,   // show to user even if quiet is on
    log_level_info = 5,     // show to user by default
    log_level_verbose = 4,  // opt-in here and down - must turn on the log level
    log_level_debug = 3,    // show values being used in logic
    log_level_lots = 2,     // lots of stuff
    log_level_trace = 1,    // log input and output of subsystems
};

enum log_source {
    log_source_unknown = 0,
    log_source_righack,
    log_source_hamlib,
};

#define log_fatal(...) log__level_va(log_source_righack, log_level_fatal, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log__level_va(log_source_righack, log_level_warn, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_notice(...) log__level_va(log_source_righack, log_level_notice, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log__level_va(log_source_righack, log_level_info, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_verbose(...) log__level_va(log_source_righack, log_level_verbose, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log__level_va(log_source_righack, log_level_debug, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_lots(...) log__level_va(log_source_righack, log_level_lots, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define log_trace(...) log__level_va(log_source_righack, log_level_trace, __func__, __FILE__, __LINE__, __VA_ARGS__)

void log__level_va(enum log_source, enum log_level, const char *, const char *, int, const char *, ...);
void log__level_args(enum log_source, enum log_level, const char *, const char *, int, const char *, va_list);
void log__level_exit(enum log_source, enum log_level, const char *, const char *, int, const char *, ...);

#endif /* SRC_LOG_H_ */
