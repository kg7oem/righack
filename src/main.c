/*
 * main.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver.test.h"
#include "runloop.h"
#include "vserial.h"

int
main(int argc, char **argv) {
    char *name = NULL;
    struct driver_info *test_driver = test_driver_info();

    if (argc > 1) {
        name = argv[1];
    }

    VSERIAL *test = vserial_create(name);

    printf("Fake serial device name: %s\n", vserial_get_name(test));
    runloop_add_vserial(test);

    vserial_set_handlers(test, &test_driver->vserial);

    if (test_driver->init != NULL) {
        test_driver->init(test);
    }

    printf("We are good: starting runloop\n");

    runloop_start();

    if (test_driver->cleanup) {
        test_driver->cleanup(test);
    }

    vserial_destroy(test);

    exit(0);
}
