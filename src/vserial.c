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
    void *control_line; // function pointer that is invoked for control
                        // line changes
    void *recv_ready; // function pointer for when data is ready on the master PTY
    void *send_ready; // function pointer for when data can be sent on the master PTY
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
    char *name = NULL;
    int master, slave;
    char slave_path[PATH_MAX];

    if (openpty(&master, &slave, slave_path, NULL, NULL)) {
        perror("could not openpty()");
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

char *
vserial_get_name(VSERIAL *p) {
    return p->name;
}
