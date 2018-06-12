/*
 * vserial.c
 *
 *  Created on: Jun 6, 2018
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

#include <errno.h>
#include <fcntl.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "../config.h"
#include "../configfile.h"
#include "../driver.h"
#include "../external/autodie.h"
#include "../log.h"
#include "../runloop.h"
#include "../types.h"
#include "../util.h"
#include "vserial.h"

#define DRIVER_NAME "vserial"

#define TIOCPKT_MSET (1 << 7)

struct vserial_pty {
    char *path;
    int fd;
    struct termios terminfo;
};

struct vserial {
    char *name;
    struct vserial_pty pty_master;
    struct vserial_pty pty_slave;
};

struct vserial_context {
    struct driver *driver;
    struct runloop_poll *poll;
    struct vserial *vserial;
    int modem_bits;
};

void
vserial_deliver_fc_changed(struct vserial_context *context) {
    struct driver_rs232_fc control_lines = {
            .cts = context->modem_bits & TIOCM_CTS,
            .rts = context->modem_bits & TIOCM_RTS,
            .dtr = context->modem_bits & TIOCM_DTR,
            .dsr = context->modem_bits & TIOCM_DSR,
    };

    DRIVER_CALL_CB(context->driver, rs232, fc_changed, &control_lines);
}

void
vserial_deliver_fc_changed_cb(bool should_run, void *context) {
    if (should_run) {
        vserial_deliver_fc_changed((struct vserial_context *)context);
    }
}

void
vserial_handle_mset(struct vserial_context *context) {
    int slave_fd = context->vserial->pty_slave.fd;
    int new_modem_bits;

    if (ioctl(slave_fd, TIOCMGET, &new_modem_bits) == -1) {
        util_fatal("Could not ioctl(TIOCMGET): %m");
    }

    if (context->modem_bits == new_modem_bits) {
        return;
    }

    context->modem_bits = new_modem_bits;
    vserial_deliver_fc_changed(context);
}

void
vserial_handle_packet(uint8_t status, struct vserial_context *context) {
    log_trace("handling status packet");
    if (status & TIOCPKT_FLUSHREAD) log_trace("  FLUSHREAD");
    if (status & TIOCPKT_FLUSHWRITE) log_trace("  FLUSHWRITE");
    if (status & TIOCPKT_STOP) log_trace("  STOP");
    if (status & TIOCPKT_START) log_trace("  START");
    if (status & TIOCPKT_DOSTOP) log_trace("  DOSTOP");
    if (status & TIOCPKT_NOSTOP) log_trace("  NOSTOP");
    if (status & TIOCPKT_IOCTL) log_trace("  IOCTL");

    if (status & TIOCPKT_MSET) {
        log_trace("  MSET");
        vserial_handle_mset(context);
    }
}

void
vserial_handle_read(struct vserial_context *context) {
    int master_fd = context->vserial->pty_master.fd;
    uint8_t buf[CONFIG_READ_SIZE];

    ssize_t did_read = read(master_fd, &buf, CONFIG_READ_SIZE);
    if (did_read < 0) {
        util_fatal("read() failed: %m");
    } else if (did_read == 0) {
        util_fatal("got EOF from master PTY");
    }

    if (buf[0] == 0) {
        util_fatal("read data from the slave PTY");
    } else {
        log_debug("Got a status packet: %u", buf[0]);
        vserial_handle_packet(buf[0], context);
    }
}

void
vserial_poll_cb(UNUSED struct runloop_poll *poll, uint64_t events) {
    log_debug("got poll event(s): %d", events);

    if (events & PEVENT_ERROR) log_trace("  ERROR");
    if (events & PEVENT_READ) log_trace("  READ");
    if (events & PEVENT_WRITE) log_trace("  WRITE");
    if (events & PEVENT_PRIO) log_trace("  PRIORITY");

    if ((events & PEVENT_PRIO) && ! (events & PEVENT_READ)) {
        util_fatal("Got a PRIO event with out READ");
    }

    if (events & PEVENT_READ) {
        vserial_handle_read((struct vserial_context *)poll->context);
    }
}

static void
vserial_manage_symlink(const char *target, const char *pty_slave) {
    struct stat target_info;
    bool create_link = false;

    if (lstat(target, &target_info)) {
        if (errno == ENOENT) {
            create_link = true;
        } else {
            util_fatal("could not lstat(%s): %m", target);
        }
    } else if (target_info.st_mode & S_IFLNK) {
        // the desired target is already a symlink
        const char *links_to = util_get_link_target(target);
        if (links_to == NULL) {
            util_fatal("could not get target of %s: %m", target);
        }
        log_debug("symlink %s -> %s", target, links_to);
        if (strcmp(pty_slave, links_to)) {
            log_debug("  symlink: need to update to point at %s", pty_slave);
            if(unlink(target)) {
                util_fatal("could not unlink(%s): %m", target);
            }
            create_link = true;
        }
    } else {
        util_fatal("will not overwrite %s with symlink\n", target);
    }

    if (create_link) {
        log_debug("Creating symlink: %s -> %s\n", pty_slave, target);
        if (symlink(pty_slave, target)) {
            util_fatal("could not symlink(%s, %s): %m", pty_slave, target);
        }
    }
}

struct vserial *
vserial_create(const char *name_arg) {
    struct vserial *vserial = util_zalloc(sizeof(struct vserial));
    struct termios *master_terminfo = &vserial->pty_master.terminfo;
    int *master = &vserial->pty_master.fd;
    int *slave = &vserial->pty_slave.fd;
    char slave_path[CONFIG_PATH_LEN];
    int nonzero = 1;
    const char *name = NULL;

    if (openpty(master, slave, slave_path, NULL, NULL)) {
        util_fatal("could not openpty(): %m");
    }

    util_set_nonblock(*master);

    // packet mode enables delivery of status information
    // about the slave to the master
    if(ioctl(*master, TIOCPKT, &nonzero) == -1) {
        util_fatal("could not ioctl(): %m");
    }

    if (tcgetattr(*master, master_terminfo)) {
        util_fatal("could not tcgettr(): %m");
    }

    // EXTPROC causes an event whenever tcsetattr() is called
    // on the slave
    master_terminfo->c_lflag |= EXTPROC;

    if (tcsetattr(*master, TCSANOW, master_terminfo)) {
        util_fatal("could not tcsetattr(): %m");
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

    vserial->name = util_strndup(name, PATH_MAX);
    vserial->pty_slave.path = util_strndup(slave_path, PATH_MAX);

    if (name_arg != NULL) {
        vserial_manage_symlink(name, vserial->pty_slave.path);
    }

    return vserial;
}

void
vserial_destroy(struct vserial *vserial) {
    free(vserial->name);
    free(vserial->pty_slave.path);
    free(vserial);
}

static void
vserial_lifecycle_bootstrap(void) {
    log_debug("vserial driver is being bootstrapped");
}

static void
vserial_lifecycle_init(struct driver *driver, const char *config_section) {
    log_debug("vserial driver instance is initializing");
    struct vserial_context *context = util_zalloc(sizeof(struct vserial_context));
    const char *config_path = configfile_rgets_section_key(config_section, "vserial.port");
    int modem_bits = 0;

    context->driver = driver;
    context->vserial = vserial_create(config_path);
    context->poll = runloop_poll_create(context->vserial->pty_master.fd, vserial_poll_cb);
    context->poll->context = context;

    driver->private = context;

    if (ioctl(context->vserial->pty_slave.fd, TIOCMGET, &modem_bits) == -1) {
        util_fatal("Could not ioctl(TIOCMGET): %m");
    }

    context->modem_bits = modem_bits;
    runloop_run_later(vserial_deliver_fc_changed_cb, context);

    runloop_poll_start(context->poll, PEVENT_READ | PEVENT_PRIO);
}

static void
vserial_lifecycle_cleanup(UNUSED struct driver *driver) {
    log_debug("vserial driver instance is being destroyed");

    if (driver->private != NULL) {
        struct vserial_context *context = driver->private;

        runloop_poll_destroy(context->poll);
        vserial_destroy(context->vserial);

        free(driver->user);
        driver->user = NULL;
    }

    return;
}

const struct driver_info *
vserial_driver_info(void) {
    static struct driver_info our_info = {
            .name = DRIVER_NAME,
            .lifecycle = DRIVER_LIFECYCLE(vserial),
            .op = {
                    .stream = {
                            .terminate = NULL,
                    },
            },
    };

    return &our_info;
}
