/*
 * vserial-private.h
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

#ifndef SRC_VSERIAL_PRIVATE_H_
#define SRC_VSERIAL_PRIVATE_H_

#include <termios.h>

#include "vserial.h"

struct vserial_pty_t {
    char *path;
    int fd;
    struct termios terminfo;
};

struct vserial_t {
    char *name;
    struct vserial_pty_t pty_master;
    struct vserial_pty_t pty_slave;
    struct vserial_handlers handlers;
    void * handler_context;
    uint8_t *send_buffer;
    ssize_t send_buffer_size;
};

void vserial_call_control_line_handler(VSERIAL *);
void vserial_call_recv_data_handler(VSERIAL *, uint8_t *, size_t);
void vserial_call_send_ready_handler(VSERIAL *);

#endif /* SRC_VSERIAL_PRIVATE_H_ */
