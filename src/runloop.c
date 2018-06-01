/*
 * runloop.c
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

#include <asm-generic/ioctls.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "runloop.h"
#include "util.h"
#include "vserial-private.h"

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
        watched_descriptors.vserial_lookup = util_malloc(size);
        watched_descriptors.max_fd = max_fd;
    } else if (watched_descriptors.max_fd < max_fd) {
        size_t copy_bytes = sizeof(VSERIAL *) * (watched_descriptors.max_fd + 1);
        VSERIAL * *new_lookup = util_malloc(size);
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
    struct pollfd *watched = util_malloc(sizeof(struct pollfd) * num_fd);
    int fd_slot = 0;

    for(int i = 0; i <= watched_descriptors.max_fd; i++) {
        VSERIAL *vserial = watched_descriptors.vserial_lookup[i];

        if (vserial != NULL) {
            struct pollfd *poll_entry = &watched[fd_slot];

            poll_entry->fd = vserial->pty_master.fd;
            // POLLPRI events will be generated with the termios
            // information changes on the slave pty
            poll_entry->events |= POLLPRI;
            poll_entry->events |= POLLIN;

            printf("new poll_entry events: %d\n", poll_entry->events);
            fd_slot++;
        }
    }

    printf("Just built %d poll entries from vserial_lookup max_fd=%d\n", fd_slot, watched_descriptors.max_fd);

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

    printf("new Master fd: %d\n", master_fd);
    printf("new max_fd: %d\n", watched_descriptors.max_fd);
    printf("Number of things in vserial_lookup: %d\n", runloop_count_vserial());
}

void
runloop_call_control_line_handler(int fd) {
    VSERIAL *vserial = watched_descriptors.vserial_lookup[fd];

    if (vserial == NULL) {
        util_fatal("Could not find a vserial entry for fd %d\n", fd);
    }

    if (vserial->pty_master.fd != fd) {
        util_fatal("Got fd %d from vserial_lookup with key of %d\n", vserial->pty_master.fd, fd);
    }

    vserial_call_control_line_handler(vserial);
    return;
}

int
runloop_start(void) {
    struct pollfd *watched = watched_descriptors.watched;
    int nfds = watched_descriptors.num_fd;

    while(1) {
        printf("About to call poll(*, %d, -1)\n", nfds);
        int retval = poll(watched, nfds, -1);

        if (retval == -1) {
            util_fatal_perror("poll() failed:");
        }

        printf("poll() returned: %d\n", retval);
        printf("poll's revents: %d\n", watched[0].revents);

        for(int i = 0; i < nfds; i++) {
            printf("checking poll results; i=%d\n", i);
            short revents = watched[i].revents;

            if (revents & POLLERR) {
                printf("Got POLLERR\n");
            }

            if (revents & POLLIN) {
                uint8_t tmp[1024];
                printf("got POLLIN\n");
                ssize_t retval = read(watched[i].fd, tmp, 1024);
                if (retval == -1) {
                    util_fatal_perror("Could not read from fd:");
                }

                printf("Read %ld bytes\n", retval);

                if (retval >= 1) {
                    uint8_t packet_type = tmp[0];

                    if (packet_type == TIOCPKT_DATA) {
                        printf("Got a data packet\n");
                    } else {
                        printf("Packet type: %u\n", packet_type);
                    }

                    if (packet_type & 128) {
                        printf("Calling control line handler\n\n");
                        runloop_call_control_line_handler(watched[i].fd);
                    }
                }
            }
        }
    }

    return 1;
}
