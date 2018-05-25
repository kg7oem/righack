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
#include <unistd.h>

#include "util.h"
#include "vserial.h"
#include "vserial-private.h"

void
vserial_destroy(VSERIAL *p) {
    if (p == NULL) {
        util_fatal("attempt to vserial_free() a null pointer\n");
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
    VSERIAL *p = util_malloc(sizeof(VSERIAL));
    struct termios *master_terminfo = &(p->pty_master.terminfo);
    char *name = NULL;
    int *master = &(p->pty_master.fd);
    int *slave = &(p->pty_slave.fd);
    char slave_path[PATH_MAX];
    int nonzero = 1;

    if (openpty(master, slave, slave_path, NULL, NULL)) {
        util_fatal_perror("could not openpty(): ");
    }

    // enable packet mode so the master will be notified about
    // ioctl() on the slave
    if(ioctl(*master, TIOCPKT, &nonzero) == -1) {
        util_fatal_perror("could not ioctl()");
    }

    if (tcgetattr(*master, master_terminfo)) {
        util_fatal_perror("could not tcgettr()");
    }

    // FIXME enable line mode? Is this correct?
    master_terminfo->c_lflag |= EXTPROC;

    if (tcsetattr(*master, TCSANOW, master_terminfo)) {
        util_fatal_perror("could not tcsetattr()");
    }

    if (name_arg == NULL) {
        name = slave_path;
    } else {
        name = name_arg;
    }

    if (strlen(name) >= PATH_MAX) {
        util_fatal("length of name(%d) >= PATH_MAX(%d)\n", strlen(name), PATH_MAX);
    }

    if (strlen(slave_path) >= PATH_MAX) {
        util_fatal("length of slave_path(%d) >= PATH_MAX(%d)\n", strlen(slave_path), PATH_MAX);
    }

    if ((p->name = strndup(name, PATH_MAX)) == NULL) {
        util_fatal("strndup() returned NULL\n");
    }

    if ((p->pty_slave.path = strndup(slave_path, PATH_MAX)) == NULL) {
        util_fatal("strndup() returned NULL\n");
    }

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
