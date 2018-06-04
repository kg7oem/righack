/*
 * runloop.c
 *
 *  Created on: May 24, 2018
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

#define _GNU_SOURCE

#include <sys/ioctl.h>
#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "config.h"
#include "log.h"
#include "runloop.h"
#include "types.h"
#include "util.h"
#include "vserial-private.h"

#define TIOCPKT_MSET (1 << 7)

// true if the runloop should be running
volatile bool should_run = false;

struct {
    // number of fds in watched
    int num_fd;
    // the fd set to poll
    struct pollfd *watched;
    // an array where the index is a file descriptor number and the
    // value is a VSERIAL *
    VSERIAL * *vserial_lookup;
    // largest value for a file descriptor that's stored in vserial_lookup
    int max_fd;
} watched_descriptors;

void
runloop_resize_vserial_lookup(int max_fd) {
    size_t size = sizeof(VSERIAL *) * (max_fd + 1);

    if (watched_descriptors.vserial_lookup == NULL) {
        watched_descriptors.vserial_lookup = util_zalloc(size);
        watched_descriptors.max_fd = max_fd;
    } else if (watched_descriptors.max_fd < max_fd) {
        size_t copy_bytes = sizeof(VSERIAL *) * (watched_descriptors.max_fd + 1);
        VSERIAL * *new_lookup = util_zalloc(size);
        VSERIAL * *old_lookup = watched_descriptors.vserial_lookup;

        memcpy(new_lookup, old_lookup, copy_bytes);

        watched_descriptors.vserial_lookup = new_lookup;
        watched_descriptors.max_fd = max_fd;

        free(old_lookup);
    } else {
        util_fatal("attempt to reduce size of the vserial_lookup array\n");
    }
}

int
runloop_count_vserial(void) {
    int quantity = 0;

    for(int i = 0; i <= watched_descriptors.max_fd; i++) {
        if (watched_descriptors.vserial_lookup[i] != NULL) {
            quantity++;
        }
    }

    return quantity;
}

struct pollfd *
runloop_create_watched(void) {
    int num_fd = runloop_count_vserial();
    struct pollfd *watched = util_zalloc(sizeof(struct pollfd) * num_fd);
    int fd_slot = 0;

    for(int i = 0; i <= watched_descriptors.max_fd; i++) {
        VSERIAL *vserial = watched_descriptors.vserial_lookup[i];

        if (vserial != NULL) {
            struct pollfd *poll_entry = &watched[fd_slot];

            poll_entry->fd = vserial->pty_master.fd;
            // POLLPRI events will be generated with the termios
            // information changes on the slave pty
            poll_entry->events |= POLLPRI;

            //printf("new poll_entry events: %d\n", poll_entry->events);
            fd_slot++;
        }
    }

    //printf("Just built %d poll entries from vserial_lookup max_fd=%d\n", fd_slot, watched_descriptors.max_fd);

    return watched;
}

void
runloop_add_vserial(VSERIAL *vserial) {
    int master_fd = vserial->pty_master.fd;
    struct pollfd *old_watched = watched_descriptors.watched;

    if (master_fd > watched_descriptors.max_fd) {
        runloop_resize_vserial_lookup(master_fd);
    }

    if (watched_descriptors.vserial_lookup[master_fd] != NULL) {
        util_fatal("attempt to add duplicate fd: %d\n", master_fd);
    }

    watched_descriptors.vserial_lookup[master_fd] = vserial;
    watched_descriptors.watched = runloop_create_watched();
    watched_descriptors.num_fd = runloop_count_vserial();

    free(old_watched);

    //printf("new Master fd: %d\n", master_fd);
    //printf("new max_fd: %d\n", watched_descriptors.max_fd);
    //printf("Number of things in vserial_lookup: %d\n", runloop_count_vserial());
}

VSERIAL *
runloop_get_vserial_by_fd(int fd) {
    VSERIAL *vserial = watched_descriptors.vserial_lookup[fd];

    if (vserial == NULL) {
        return NULL;
    }

    if (vserial->pty_master.fd != fd) {
        util_fatal("Got fd %d from vserial_lookup with key of %d\n", vserial->pty_master.fd, fd);
    }

    return vserial;
}

sigset_t *
runloop_create_empty_sigset(void) {
    sigset_t *set = util_zalloc(sizeof(sigset_t));

    if (sigemptyset(set)) {
        util_fatal("could not sigemptyset: %m");
    }

    return set;
}

sigset_t *
runloop_create_sigint_sigset(void) {
    sigset_t *set = runloop_create_empty_sigset();

    if (sigaddset(set, SIGINT)) {
        util_fatal("could not sigaddset: %m");
    }

    return set;
}

void
runloop_block_sigint(void) {
    sigset_t *will_block = runloop_create_sigint_sigset();
    int retval = pthread_sigmask(SIG_BLOCK, will_block, NULL);

    if (retval) {
        // FIXME need to improve this so it has the error text
        util_fatal("could not pthread_sigmask() - and can't perror, no: %d", retval);
    }

    free(will_block);
}

void
runloop_unblock_sigint(void) {
    sigset_t *will_unblock = runloop_create_sigint_sigset();
    int retval = pthread_sigmask(SIG_BLOCK, will_unblock, NULL);

    if (retval) {
        // FIXME need to improve this so it has the error text
        util_fatal("could not pthread_sigmask() - and can't perror, no: %d", retval);
    }

    free(will_unblock);
}

void
runloop_sigint_handler(UNUSED int signal) {
    should_run = 0;
    alarm(1);
}

void
runloop_sigalrm_handler(UNUSED int signal) {
    util_fatal("Timeout when trying to cleanup from runloop\n");
}

void
runloop_install_signal_handlers(void) {
    if (signal(SIGINT, runloop_sigint_handler) == SIG_ERR) {
        util_fatal("Could not register INT signal handler: %m");
    }

    if (signal(SIGALRM, runloop_sigalrm_handler) == SIG_ERR) {
        util_fatal("Could not register ALRM signal handler: %m");
    }
}

void
runloop_remove_signal_handlers(void) {
    if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
        util_fatal("Could not remove INT signal handler: %m");
    }

    if (signal(SIGALRM, SIG_DFL) == SIG_ERR) {
        util_fatal("could not remove ALRM signal handler: %m");
    }
}

bool
runloop_enable_read(int enable_fd) {
    struct pollfd *watched = watched_descriptors.watched;
    int nfds = watched_descriptors.num_fd;
    bool found_it = false;
    bool old_value;

    for (int i = 0; i < nfds; i++) {
        struct pollfd *poll_entry = &watched[i];
        int watched_fd = poll_entry->fd;

        if (watched_fd == enable_fd) {
            found_it = 1;
            old_value = poll_entry->events & POLLIN;
            poll_entry->events |= POLLIN;
            break;
        }
    }

    if (! found_it) {
        util_fatal("Could not find fd %d in watched_descriptors", enable_fd);
    }

    return old_value;
}

bool
runloop_disable_read(int disable_fd) {
    struct pollfd *watched = watched_descriptors.watched;
    int nfds = watched_descriptors.num_fd;
    bool found_it = false;
    bool old_value;

    for (int i = 0; i < nfds; i++) {
        struct pollfd *poll_entry = &watched[i];
        int watched_fd = poll_entry->fd;

        if (watched_fd == disable_fd) {
            found_it = 1;
            old_value = poll_entry->events & POLLIN;
            poll_entry->events &= ~POLLIN;
            break;
        }
    }

    if (! found_it) {
        util_fatal("Could not find fd %d in watched_descriptors", disable_fd);
    }

    return old_value;
}

bool
runloop_disable_write(int disable_fd) {
    struct pollfd *watched = watched_descriptors.watched;
    int nfds = watched_descriptors.num_fd;
    bool found_it = false;
    bool old_value;

    for (int i = 0; i < nfds; i++) {
        struct pollfd *poll_entry = &watched[i];
        int watched_fd = poll_entry->fd;

        if (watched_fd == disable_fd) {
            found_it = 1;
            old_value = poll_entry->events & POLLOUT;
            poll_entry->events &= ~POLLOUT;
            break;
        }
    }

    if (! found_it) {
        util_fatal("Could not find fd %d in watched_descriptors", disable_fd);
    }

    return old_value;
}

bool
runloop_enable_write(int enable_fd) {
    struct pollfd *watched = watched_descriptors.watched;
    int nfds = watched_descriptors.num_fd;
    bool found_it = false;
    bool old_value;

    for (int i = 0; i < nfds; i++) {
        struct pollfd *poll_entry = &watched[i];
        int watched_fd = poll_entry->fd;

        if (watched_fd == enable_fd) {
            found_it = 1;
            old_value = poll_entry->events & POLLOUT;
            poll_entry->events |= POLLOUT;
            break;
        }
    }

    if (! found_it) {
        util_fatal("Could not find fd %d in watched_descriptors", enable_fd);
    }

    return old_value;
}

// FIXME some of this logic should be moved to vserial.c
int
runloop_start(void) {
    struct pollfd *watched = watched_descriptors.watched;
    int nfds = watched_descriptors.num_fd;
    sigset_t *masked_signals = runloop_create_empty_sigset();
    uint8_t *read_buf = util_zalloc(CONFIG_READ_SIZE);

    should_run = 1;

    runloop_install_signal_handlers();

    while(1) {
        //printf("About to call poll(*, %d, -1); should_run: %i\n", nfds, should_run);

        if (! should_run) {
            //printf("Leaving runloop because of ctrl+c\n");
            break;
        }

        // FIXME when the IO loop is busy ctrl+c does not work
//        runloop_block_sigint();
        int retval = ppoll(watched, nfds, NULL, masked_signals);
//        runloop_unblock_sigint();

        if (retval == -1) {
            if (errno == EINTR) {
                continue;
            }

            util_fatal("ppoll() failed: %m");
        }

        //printf("poll() returned: %d\n", retval);
        //printf("poll's revents: %d\n", watched[0].revents);

        for(int i = 0; i < nfds; i++) {
            //printf("checking poll results; i=%d\n", i);
            short revents = watched[i].revents;

            if (revents & POLLERR) {
                util_fatal("Got POLLERR\n");
            }

            if (revents & (POLLOUT | POLLIN | POLLPRI)) {
                int fd = watched[i].fd;
                VSERIAL *vserial = runloop_get_vserial_by_fd(fd);

                if (vserial == NULL) {
                    util_fatal("Could not find VSERIAL for fd %d", fd);
                }

                if (revents & POLLOUT) {
                    //printf("Got POLLOUT\n");
                    if (vserial->send_buffer == NULL) {
                        // tell the driver that it can add
                        // some data to the send buffer
                        vserial_call_send_ready_handler(vserial);
                    }

                    if (vserial->send_buffer != NULL) {
                        size_t write_size;

                        if (vserial->send_buffer_size > CONFIG_WRITE_SIZE) {
                            write_size = CONFIG_WRITE_SIZE;
                        } else {
                            write_size = vserial->send_buffer_size;
                        }

                        ssize_t retval = write(fd, vserial->send_buffer, write_size);

                        if (retval == -1) {
                            util_fatal("Could not write(): %m");
                        }

                        //printf("Wrote %ld bytes; send_buffer_size = %ld\n", retval, vserial->send_buffer_size);
                        vserial->send_buffer_size -= retval;
                        //printf("New send_buffer_size: %ld\n", vserial->send_buffer_size);

                        if (vserial->send_buffer_size < 0) {
                            util_fatal("send_buffer_size < 0: %d", vserial->send_buffer_size);
                        }

                        if (vserial->send_buffer_size == 0) {
                            free(vserial->send_buffer);
                            vserial->send_buffer = NULL;
                        } else {
                            vserial->send_buffer += retval;
                        }
                    }
                }

                if (revents & (POLLIN | POLLPRI)) {
                    //printf("got POLLIN\n");
                    ssize_t retval = read(fd, read_buf, CONFIG_READ_SIZE);
                    if (retval == -1) {
                        util_fatal("Could not read from fd: %m");
                    }

                    //printf("Read %ld bytes\n", retval);

                    if (retval >= 1) {
                        uint8_t packet_type = read_buf[0];

                        if (packet_type == TIOCPKT_DATA) {
                            // only send the data, skip the status byte
                            vserial_call_recv_data_handler(vserial, read_buf + 1, retval - 1);
                        } else {
                            //printf("Packet type: %u\n", packet_type);
                        }

                        // FIXME move to a handler implemented in vserial.c that
                        // takes a VSERIAL *
                        if (packet_type & TIOCPKT_IOCTL) {
                            //printf("should call a speed handler but can't\n");
                        }

                        // FIXME move to a handler implemented in vserial.c that
                        // takes a VSERIAL *
                        if (packet_type & TIOCPKT_MSET) {
                            //printf("Calling control line handler\n");
                            vserial_call_control_line_handler(vserial);
                        }
                    }
                }
            }
        }

        //printf("\n");
    }

    alarm(0);
    runloop_unblock_sigint();

    free(masked_signals);
    free(read_buf);

    return 0;
}
