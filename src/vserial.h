/*
 * vserial.h
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

#ifndef SRC_VSERIAL_H_
#define SRC_VSERIAL_H_

#include <inttypes.h>
#include <stdbool.h>
// FIXME for size_t
#include <stdlib.h>

// VSERIAL * is supposed to be an opaque handle and the vserial interface
// isn't tied to POSIX so this could in theory work on Windows
struct vserial_t;
typedef struct vserial_t VSERIAL;

struct vserial_control_line {
    bool rts;
    bool cts;
    bool dtr;
    bool dsr;
};

typedef void (*vserial_control_line_handler)(VSERIAL *, struct vserial_control_line *);
typedef void (*vserial_recv_data_handler)(VSERIAL *, uint8_t *, size_t);
typedef void (*vserial_send_ready_handler)(VSERIAL *);

struct vserial_handlers {
    vserial_control_line_handler control_line;
    vserial_recv_data_handler recv_data;
    vserial_send_ready_handler send_ready;
};

void vserial_destroy(VSERIAL *);
VSERIAL * vserial_create(const char *);

char * vserial_get_name(VSERIAL *);
void vserial_copy_handlers(VSERIAL *, struct vserial_handlers *);
void vserial_set_handlers(VSERIAL *, struct vserial_handlers *);
void * vserial_get_context(VSERIAL *);
void vserial_set_context(VSERIAL *, void *);

bool vserial_enable_recv(VSERIAL *);
bool vserial_disable_recv(VSERIAL *);
bool vserial_enable_send(VSERIAL *);
bool vserial_disable_send(VSERIAL *);

void
vserial_send(VSERIAL *, void *, size_t);

#endif /* SRC_VSERIAL_H_ */
