/*
 * test.c
 *
 *  Created on: Jun 1, 2018
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

#include <stdio.h>
#include <string.h>

#include "test.h"
#include "../vserial.h"
#include "../util.h"

static void
test_init_handler(UNUSED VSERIAL *vserial) {
    char *context = util_strdup("This is something in the context\n");
    vserial_set_context(vserial, context);
    vserial_enable_recv(vserial);
    vserial_enable_send(vserial);
}

static void
test_cleanup_handler(UNUSED VSERIAL *vserial) {
    char *context = vserial_get_context(vserial);
    if (context != NULL) {
        free(context);
    }
}

static void
test_control_line_handler(UNUSED VSERIAL *vserial, UNUSED struct vserial_control_line *control_line) {
    char *message = vserial_get_context(vserial);
    printf("test_control_line_handler executed\n");
    printf("Message: %s\n", message);
}

static void
test_recv_data_handler(UNUSED VSERIAL *vserial, UNUSED uint8_t *buf, size_t len) {
    char *text = util_malloc(len + 1);

    memcpy(text, buf, len);
    text[len] = 0;

    printf("Got this: %s\n", text);
    free(text);
}

static void
test_send_data_handler(UNUSED VSERIAL *vserial) {
    static bool sent = 0;
    printf("  inside test_send_data_handler\n");

    if (sent) {
        vserial_disable_send(vserial);
    } else {
        char *message = "Well this is something\n";
        vserial_send(vserial, message, strlen(message));
        sent = 1;
    }
}

struct driver_info *
test_driver_info(void) {
    struct driver_info *info = util_malloc(sizeof(struct driver_info));

    info->init = test_init_handler;
    info->cleanup = test_cleanup_handler;
    info->vserial.control_line = test_control_line_handler;
    info->vserial.recv_data = test_recv_data_handler;
    info->vserial.send_ready = test_send_data_handler;

    return info;
}

