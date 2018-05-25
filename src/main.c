/*
 * main.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <stdio.h>
#include <stdlib.h>

#include "runloop.h"
#include "vserial.h"

void
control_line_handler(struct vserial_control_line *control_lines, void *context) {
    printf("In the control line handler %p %p\n", control_lines, context);
    printf("RTS:%d CTS:%d\n", control_lines->rts, control_lines->cts);
    printf("DTR:%d DSR:%d\n", control_lines->dtr, control_lines->dsr);
    printf("\n");
}

int
main(int argc, char **argv) {
    char *name = NULL;

    if (argc > 1) {
        name = argv[1];
    }

    VSERIAL *test = vserial_create(name);
    vserial_set_send_ready_enabled(test, true);
    vserial_register_control_line(test, control_line_handler, NULL);

    printf("Fake serial device name: %s\n", vserial_get_name(test));
    printf("send_ready_enabled: %d\n", vserial_get_send_ready_enabled(test));

    runloop_add_vserial(test);

    printf("We are good: starting runloop\n");

    runloop_start();

    vserial_destroy(test);

    exit(0);
}
