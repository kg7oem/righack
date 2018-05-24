/*
 * vserial.h
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#ifndef SRC_VSERIAL_H_
#define SRC_VSERIAL_H_

struct _vserial_t;

// VSERIAL * is supposed to be an opaque handle and the vserial interface
// isn't tied to POSIX so this could in theory work on Windows
typedef struct _vserial_t VSERIAL;

void vserial_destroy(VSERIAL *);
VSERIAL * vserial_create(char *);
char * vserial_get_name(VSERIAL *);

#endif /* SRC_VSERIAL_H_ */
