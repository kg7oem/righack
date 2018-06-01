/*
 * main.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runloop.h"
#include "vserial.h"

void
control_line_handler(VSERIAL *vserial, struct vserial_control_line *control_lines) {
    char *message = vserial_get_context(vserial);

    printf("In the control line handler; context=%p\n", message);
    printf("RTS:%d CTS:%d\n", control_lines->rts, control_lines->cts);
    printf("DTR:%d DSR:%d\n", control_lines->dtr, control_lines->dsr);
    printf("Message: %s\n", message);
    printf("\n");

    if (control_lines->rts) {
        printf("TRANSMIT!!!!\n");
        system("rig T 1");
    } else {
        printf("Done with transmit\n");
        system("rig T 0");
    }

    printf("\n\n");
}

int
main(int argc, char **argv) {
    char *name = NULL;
    struct vserial_handlers handlers = {
            .control_line = control_line_handler,
    };

    if (argc > 1) {
        name = argv[1];
    }

    VSERIAL *test = vserial_create(name);
    vserial_set_handlers(test, &handlers);
    vserial_set_context(test, strdup("This is a message"));

    printf("Fake serial device name: %s\n", vserial_get_name(test));

    runloop_add_vserial(test);

    printf("We are good: starting runloop\n");

    runloop_start();

    free(vserial_get_context(test));
    vserial_destroy(test);

    exit(0);
}
