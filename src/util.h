/*
 * util.h
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

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

#include <pthread.h>
#include <stdarg.h>

#include "external/autodie.h"
#include "log.h"
#include "types.h"

#define util_fatal(...) util__fatal_va(log_source_righack, log_level_fatal, __func__, __FILE__, __LINE__, __VA_ARGS__)
void util__fatal_va(enum log_source, enum log_level, const char *, const char *, int, const char *, ...)  NORETURN;

void * util_zalloc(size_t);
void * util_memdup(void *, size_t);
char * util_strdup(const char *);
char * util_strndup(const char *, size_t);

void util_set_nonblock(int);

const char * util_get_link_target(const char *);

#endif /* SRC_UTIL_H_ */
