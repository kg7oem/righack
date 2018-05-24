/*
 * main.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <stdio.h>
#include <stdlib.h>

#include "vserial.h"

int
main(int argc, char **argv) {
    char *name = NULL;

    if (argc > 1) {
        name = argv[1];
    }

    VSERIAL *test = vserial_create(name);
    vserial_set_send_ready_enabled(test, true);

    printf("Fake serial device name: %s\n", vserial_get_name(test));
    printf("send_ready_enabled: %d\n", vserial_get_send_ready_enabled(test));

    vserial_destroy(test);

    printf("We are good\n");
    exit(0);
}
