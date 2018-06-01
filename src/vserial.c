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

char *
vserial_get_name(VSERIAL *p) {
    return p->name;
}

void
vserial_set_handlers(VSERIAL *vserial, struct vserial_handlers *handlers) {
    memcpy(&vserial->handlers, handlers, sizeof(struct vserial_handlers));
}

void *
vserial_get_context(VSERIAL *vserial) {
    return vserial->handler_context;
}

void
vserial_set_context(VSERIAL *vserial, void *p) {
    vserial->handler_context = p;
}

void
vserial_call_control_line_handler(VSERIAL *vserial) {
    int slave_fd = vserial->pty_slave.fd;
    struct vserial_control_line control_lines;
    int modem_bits;

    if (vserial->handlers.control_line == NULL) {
        return;
    }

    printf("the slave FD is %d\n", slave_fd);

    if (ioctl(slave_fd, TIOCMGET, &modem_bits) == -1) {
        util_fatal_perror("Could not ioctl(TIOCMGET): ");
    }

    control_lines.cts = modem_bits & TIOCM_CTS;
    control_lines.rts = modem_bits & TIOCM_RTS;
    control_lines.dtr = modem_bits & TIOCM_DTR;
    control_lines.dsr = modem_bits & TIOCM_DSR;

    vserial->handlers.control_line(vserial, &control_lines);

    return;
}
