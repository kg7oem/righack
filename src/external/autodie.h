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
