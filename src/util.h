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

// FIXME is there a better include for this?
#include <linux/types.h>
#include <stdarg.h>

#define UNUSED __attribute__((unused))

void * util_malloc(size_t);
void * util_memdup(void *, size_t);
char * util_strdup(const char *);
char * util_strndup(const char *, size_t);

void util_fatal(char *, ...);
void util_fatal_perror(char *fmt, ...);

#endif /* SRC_UTIL_H_ */
