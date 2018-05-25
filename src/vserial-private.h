/*
 * vserial-private.h
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

#ifndef SRC_VSERIAL_PRIVATE_H_
#define SRC_VSERIAL_PRIVATE_H_

#include <termios.h>

struct vserial_pty_t {
    char *path;
    int fd;
    struct termios terminfo;
};

struct vserial_handlers_t {
    vserial_control_line_handler control_line;
    void *control_line_context;
    vserial_recv_data_handler recv_data;
    void *recv_data_context;
    vserial_send_ready_handler send_ready;
    void *send_ready_context;
    bool send_ready_enabled;
};

struct vserial_t {
    char *name;
    struct vserial_pty_t pty_master;
    struct vserial_pty_t pty_slave;
    struct vserial_handlers_t handlers;
};


#endif /* SRC_VSERIAL_PRIVATE_H_ */
