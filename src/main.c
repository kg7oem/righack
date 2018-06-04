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

#include <errno.h>
#include <hamlib/rig.h>
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

static int
hamlib_debug_handler(enum rig_debug_level_e debug_level, UNUSED rig_ptr_t user_data, UNUSED const char *fmt, UNUSED va_list args) {
    enum log_level log_at = log_level_fatal;

    switch(debug_level) {
    case RIG_DEBUG_NONE:
        log_at = log_level_trace;
        break;
    case RIG_DEBUG_BUG:
    case RIG_DEBUG_ERR:
        log_at = log_level_error;
        break;
    case RIG_DEBUG_WARN:
        log_at = log_level_warn;
        break;
    case RIG_DEBUG_VERBOSE:
        log_at = log_level_verbose;
        break;
    case RIG_DEBUG_TRACE:
        log_at = log_level_trace;
        break;
    }

    if (log_at < log_get_current_level()) {
        return RIG_OK;
    }

    // remove the trailing new line from the hamlib messages
    char *new_fmt = ad_strdup(fmt);
    size_t len = strlen(new_fmt);
    if (new_fmt[len - 1] == '\n') {
        new_fmt[len - 1] = 0;
    }

    log__level_args(log_source_hamlib, log_at, NULL, NULL, -1, new_fmt, args);
    free(new_fmt);

    return RIG_OK;
}

static void
autodie_handler(const char *function, int error, const char *message) {
    // TODO some other error numbers might make sense to have dedicated
    // exit values
    // EMFILE - Too  many  open  files
    if (error == ENOMEM) {
        // when memory allocation has failed the error handling
        // is kept away from the logging because it might try to
        // acquire RAM and fail
        fprintf(stderr, "%s() died \"%s\": %s\n", function, message, strerror(error));
        guts_exit(exit_nomem);
    }

    util_fatal("%s() died \"%s\": %s", function, message, strerror(error));
}


void
bootstrap(void) {
    autodie_register_handler(autodie_handler);
    rig_set_debug(RIG_DEBUG_WARN);
    rig_set_debug_callback(hamlib_debug_handler, NULL);
}

int
main(int argc, char **argv) {
    bootstrap();

    log_set_current_level(log_level_debug);

    log_info("main just started");

    if (argc != 2) {
        log_fatal("Usage: specify exactly one config file as an argument");
        guts_exit(exit_args);
    }

    char *config_file = argv[1];
    log_info("Loading configuration file: %s", config_file);
    configfile_load(argv[1]);

    const char *section_name;
    for(int i = 0; (section_name = configfile_get_section_name(i)); i++) {
        log_debug("config section #%d: %s", i, section_name);
    }

    section_name = configfile_get_section_name(0);
    const char *p;

    p = configfile_rgets_section_key(section_name, "driver.type");
    if (strcmp(p, "ptt")) {
        util_fatal("only the ptt driver type is supported, not '%s'\n", p);
    }

    p = configfile_rgets_section_key(section_name, "io.type");
    if (strcmp(p, "vserial")) {
        util_fatal("only the vserial io type is supported, not '%s'\n", p);
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
