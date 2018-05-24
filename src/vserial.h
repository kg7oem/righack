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

void vserial_destroy(VSERIAL *);
VSERIAL * vserial_create(char *);

void vserial_register_control_line(VSERIAL *, vserial_control_line_handler, void *);
void vserial_register_recv_data(VSERIAL *, vserial_recv_data_handler, void *);
void vserial_register_send_ready(VSERIAL *, vserial_send_ready_handler, void *);

char * vserial_get_name(VSERIAL *);
bool vserial_get_send_ready_enabled(VSERIAL *);
void vserial_set_send_ready_enabled(VSERIAL *, bool);

#endif /* SRC_VSERIAL_H_ */
