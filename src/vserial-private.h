/*
 * vserial-private.h
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

/*
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
