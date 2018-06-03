/*
 * autodie.h
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
// 3. The function name must be identical to the libc name
//    with ad_ in front of it
// 4. No behavior changes except for error checking
// 5. No bug fixes - don't do anything the libc didn't do
// 6. Do not malloc - these functions might get called in a situation where
//    malloc() has already failed and there is no available RAM
// 7. no macros - users must be able to get a function pointer
// 8. If the rules must be broken then it needs to be added to the
//    per function documentation
//
// tl;dr - don't do anything but error checking

/*
 *         FUNCTION SPECIFIC BEHAVIOR CHANGES
 *
 *   ad_malloc will fail with EINVAL if a size of 0 is specified
 *
 *   ad_calloc will fail with EINVAL if nmemb or size is 0
 *
 */

#ifndef SRC_EXTERNAL_AUTODIE_H_
#define SRC_EXTERNAL_AUTODIE_H_

typedef void (*autodie_handler_t)(const char *, int, const char *);

// this call back will be invoked with the value of errno generated
// during the failure
void autodie_register_handler(autodie_handler_t);

void ad__test_(void);

void * ad_calloc(size_t, size_t);
void * ad_malloc(size_t);

char * ad_strdup(const char *);

#endif /* SRC_EXTERNAL_AUTODIE_H_ */
