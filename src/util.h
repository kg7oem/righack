/*
 * util.h
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_

// FIXME is there a better include for this?
#include <linux/types.h>
#include <stdarg.h>

void * util_malloc(size_t);
void util_fatal(char *, ...);
void util_fatal_perror(char *fmt, ...);

#endif /* SRC_UTIL_H_ */
