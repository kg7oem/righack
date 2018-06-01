/*
 * vserial.h
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#ifndef SRC_VSERIAL_H_
#define SRC_VSERIAL_H_

#include <stdbool.h>
// FIXME for size_t
#include <stdlib.h>

// VSERIAL * is supposed to be an opaque handle and the vserial interface
// isn't tied to POSIX so this could in theory work on Windows
struct vserial_t;
typedef struct vserial_t VSERIAL;

struct vserial_control_line {
    bool rts;
    bool cts;
    bool dtr;
    bool dsr;
};

typedef void (*vserial_control_line_handler)(struct vserial_control_line *, void *);
typedef void (*vserial_recv_data_handler)(char *, size_t, void *);
typedef void (*vserial_send_ready_handler)(void *);

struct vserial_handlers {
    vserial_control_line_handler control_line;
    vserial_recv_data_handler recv_data;
    vserial_send_ready_handler send_ready;
};

void vserial_destroy(VSERIAL *);
VSERIAL * vserial_create(char *);

char * vserial_get_name(VSERIAL *);
void vserial_copy_handlers(VSERIAL *, struct vserial_handlers *);
void vserial_set_handlers(VSERIAL *, struct vserial_handlers *);
void * vserial_get_context(VSERIAL *);
void vserial_set_context(VSERIAL *, void *);

#endif /* SRC_VSERIAL_H_ */
