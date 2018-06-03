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
#include "guts.h"
#include "log.h"
#include "runloop.h"
#include "util.h"
#include "vserial.h"

int
main(int argc, char **argv) {
    log_trace("main just started");

    if (argc != 2) {
        log_fatal_ret("Usage: specify exactly one config file as an argument");
        guts_exit(exit_args);
    }

    char *config_file = argv[1];
    log_info("Loading configuration file: %s", config_file);
    configfile_load(argv[1]);

    const char *section_name;
    for(int i = 0; (section_name = configfile_get_section_name(i)); i++) {
        log_debug("config section #%d: %s\n", i, section_name);
    }

    section_name = configfile_get_section_name(0);
    const char *p;

    p = configfile_rgets_section_key(section_name, "driver.type");
    if (strcmp(p, "ptt")) {
        log_fatal("only the ptt driver type is supported, not '%s'\n", p);
    }

    p = configfile_rgets_section_key(section_name, "io.type");
    if (strcmp(p, "vserial")) {
        log_fatal("only the vserial io type is supported, not '%s'\n", p);
    }

    const char *name = configfile_gets_section_key(section_name, "io.port");
    VSERIAL *vserial = vserial_create(name);

    log_info("Fake serial device name: %s", vserial_get_name(vserial));
    runloop_add_vserial(vserial);

    struct driver_info *driver = ptt_driver_info();
    vserial_set_handlers(vserial, &driver->vserial);

    if (driver->init != NULL) {
        driver->init(vserial, section_name);
    }

    log_info("We are good: starting runloop");

    runloop_start();

    if (driver->cleanup) {
        driver->cleanup(vserial);
    }

    vserial_destroy(vserial);
    free(driver);

    exit(0);
}
