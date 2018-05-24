/*
 * vserial.h
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#ifndef SRC_VSERIAL_H_
#define SRC_VSERIAL_H_

#include <stdbool.h>

// VSERIAL * is supposed to be an opaque handle and the vserial interface
// isn't tied to POSIX so this could in theory work on Windows
struct _vserial_t;
typedef struct _vserial_t VSERIAL;

struct vserial_control_lines {
    bool rts;
    bool cts;
    bool dtr;
    bool dsr;
};

typedef int (*vserial_control_lines_handler)(struct vserial_control_lines *, void *);

void vserial_destroy(VSERIAL *);
VSERIAL * vserial_create(char *);
char * vserial_get_name(VSERIAL *);

void vserial_register_control_lines(VSERIAL *, vserial_control_lines_handler);

#endif /* SRC_VSERIAL_H_ */
