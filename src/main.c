/*
 * main.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configfile.h"
#include "drivers.h"
#include "runloop.h"
#include "util.h"
#include "vserial.h"

int
main(int argc, char **argv) {
    char *name = NULL;
    struct driver_info *driver = ptt_driver_info();

    if (argc != 2) {
        util_fatal("specify only a path to a config file");
    }

    configfile_load(argv[1]);
    printf("vserial sections in config: %d\n", configfile_count_vserial());

    for(int i = 0; i < configfile_count_vserial(); i++) {
        printf(
                "  virtual serial port %s = %s\n",
                configfile_get_vserial_name(i),
                configfile_get_vserial_driver(i)
        );
    }

    VSERIAL *vserial = vserial_create(name);

    printf("Fake serial device name: %s\n", vserial_get_name(vserial));
    runloop_add_vserial(vserial);

    vserial_set_handlers(vserial, &driver->vserial);

    if (driver->init != NULL) {
        driver->init(vserial);
    }

    printf("We are good: starting runloop\n");

    runloop_start();

    if (driver->cleanup) {
        driver->cleanup(vserial);
    }

    vserial_destroy(vserial);
    free(driver);

    exit(0);
}
