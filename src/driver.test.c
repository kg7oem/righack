/*
 * test.c
 *
 *  Created on: Jun 1, 2018
 *      Author: tyler
 */

#include <stdio.h>
#include <string.h>

#include "driver.test.h"
#include "vserial.h"
#include "util.h"

static void
test_init_handler(UNUSED VSERIAL *vserial) {
    char *context = util_strdup("This is something in the context\n");
    vserial_set_context(vserial, context);
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

struct driver_info *
test_driver_info(void) {
    struct driver_info *info = util_malloc(sizeof(struct driver_info));

    info->init = test_init_handler;
    info->cleanup = test_cleanup_handler;
    info->vserial.control_line = test_control_line_handler;
    info->vserial.recv_data = test_recv_data_handler;

    return info;
}

