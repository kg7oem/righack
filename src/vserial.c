/*
 * vserial.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
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
        util_fatal("attempt to vserial_destroy() a null pointer\n");
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
    struct termios *master_terminfo = &p->pty_master.terminfo;
    char *name = NULL;
    int *master = &p->pty_master.fd;
    int *slave = &p->pty_slave.fd;
    char slave_path[PATH_MAX];
    int nonzero = 1;

    if (openpty(master, slave, slave_path, NULL, NULL)) {
        util_fatal_perror("could not openpty(): ");
    }

    // packet mode enables delivery of status information
    // about the slave to the master
    if(ioctl(*master, TIOCPKT, &nonzero) == -1) {
        util_fatal_perror("could not ioctl()");
    }

    if (tcgetattr(*master, master_terminfo)) {
        util_fatal_perror("could not tcgettr()");
    }

    // EXTPROC causes an event whenever tcsetattr() is called
    // on the slave
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

    p->name = util_strndup(name, PATH_MAX);
    p->pty_slave.path = util_strndup(slave_path, PATH_MAX);

    if (name_arg != NULL) {
        fprintf(stderr, "Would symlink %s to %s\n", p->name, p->pty_slave.path);
    }

    return p;
}

char *
vserial_get_name(VSERIAL *p) {
    return p->name;
}

// copies the veserial_handlers from vserial into handlers
void
vserial_copy_handlers(UNUSED VSERIAL *vserial, UNUSED struct vserial_handlers *handlers) {
    util_fatal("vserial_copy_handlers(): not implemented");
}

// updates the handlers stored in vserial with the contents of handlers
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

bool
vserial_enable_recv(VSERIAL *vserial) {
    return runloop_enable_read(vserial->pty_master.fd);
}

bool
vserial_disable_recv(VSERIAL *vserial) {
    return runloop_disable_read(vserial->pty_master.fd);
}

bool
vserial_enable_send(VSERIAL *vserial) {
    return runloop_enable_write(vserial->pty_master.fd);
}

bool
vserial_disable_send(VSERIAL *vserial) {
    return runloop_disable_write(vserial->pty_master.fd);
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

void
vserial_call_send_ready_handler(VSERIAL *vserial) {
    if (vserial->handlers.send_ready == NULL) {
        return;
    }

    vserial->handlers.send_ready(vserial);
}
void
vserial_call_recv_data_handler(VSERIAL *vserial, uint8_t *buf, size_t len) {
    if (vserial->handlers.recv_data == NULL) {
        return;
    }

    vserial->handlers.recv_data(vserial, buf, len);

    return;
}

//FIXME should this only work from inside a send_ready handler?
void
vserial_send(VSERIAL *vserial, void *buf, size_t len) {
    if (vserial->send_buffer != NULL) {
        util_fatal("attempt to call vserial_send() when send_buffer was not empty");
    }

    vserial->send_buffer = util_malloc(len);
    memcpy(vserial->send_buffer, buf, len);
    vserial->send_buffer_size = len;

    runloop_enable_write(vserial->pty_master.fd);
}
