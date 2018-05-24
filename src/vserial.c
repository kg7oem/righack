/*
 * vserial.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <fcntl.h>
#include <linux/limits.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "vserial.h"

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

static VSERIAL *
vserial_alloc(void) {
    VSERIAL *p = malloc(sizeof(VSERIAL));

    if (p == NULL) {
//        abort("malloc() returned NULL");
        abort();
    }

    memset(p, 0, sizeof(VSERIAL));

    return(p);
}

void
vserial_destroy(VSERIAL *p) {
    if (p == NULL) {
//        abort("attempt to vserial_free() a null pointer");
        abort();
    }

    if (p->pty_master.path != NULL) {
        free(p->pty_master.path);
    }

    if (p->pty_slave.path != NULL) {
        free(p->pty_slave.path);
    }

    if (p->name != NULL) {
        free(p->name);
    }

    free(p);

    return;
}

VSERIAL *
vserial_create(char *name_arg) {
    VSERIAL *p = vserial_alloc();
    struct termios *master_terminfo = &(p->pty_master.terminfo);
    char *name = NULL;
    int master, slave;
    char slave_path[PATH_MAX];
    int nonzero = 1;

    if (openpty(&master, &slave, slave_path, NULL, NULL)) {
        perror("could not openpty()");
        abort();
    }

    // enable packet mode so the master will be notified about
    // ioctl() on the slave
    if(ioctl(master, TIOCPKT, &nonzero) == -1) {
        perror("could not ioctl()");
        abort();
    }

    if (tcgetattr(master, master_terminfo)) {
        perror("could not tcgettr()");
        abort();
    }

    master_terminfo->c_lflag |= EXTPROC;

    if (tcsetattr(master, TCSANOW, master_terminfo)) {
        perror("could not tcsetattr()");
        abort();
    }

    if (name_arg == NULL) {
        name = slave_path;
    } else {
        name = name_arg;
    }

    if (strlen(name) >= PATH_MAX) {
        abort();
    }

    if (strlen(slave_path) >= PATH_MAX) {
        abort();
    }

    p->name = strndup(name, PATH_MAX);
    p->pty_slave.path = strndup(slave_path, PATH_MAX);

    if (name_arg != NULL) {
        fprintf(stderr, "Would symlink %s to %s\n", p->name, p->pty_slave.path);
    }

    return p;
}

void
vserial_register_control_line(VSERIAL *vserial, vserial_control_line_handler handler, void *context) {
    vserial->handlers.control_line = handler;
    vserial->handlers.control_line_context = context;
}

void
vserial_register_recv_data(VSERIAL *vserial, vserial_recv_data_handler handler, void *context) {
    vserial->handlers.recv_data = handler;
    vserial->handlers.recv_data_context = context;
}

void
vserial_register_send_ready(VSERIAL *vserial, vserial_send_ready_handler handler, void *context) {
    vserial->handlers.send_ready = handler;
    vserial->handlers.send_ready_context = context;
}

char *
vserial_get_name(VSERIAL *p) {
    return p->name;
}

bool
vserial_get_send_ready_enabled(VSERIAL *p) {
    return p->handlers.send_ready_enabled;
}

void
vserial_set_send_ready_enabled(VSERIAL *p, bool value) {
    p->handlers.send_ready_enabled = value;
}
