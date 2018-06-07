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
#include "interface.h"
#include "log.h"
#include "module.h"
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

static void
start_module_handler(bool should_run, void *context) {
    const void **args = context;
    const struct module_info *module_info = args[0];
    const char *section_name = args[1];

    if (should_run) {
        log_debug("Starting module instance '%s' in runloop", section_name);

        if (module_start(module_info, section_name) == NULL) {
            util_fatal("Could not start module '%s'", section_name);
        }

    }

    free(args);
}

void
handle_config_section(const char *section) {
    const char *module_name = configfile_rgets_section_key(section, "module.name");

    log_debug("config section '%s' module.name = '%s'", section, module_name);
    const struct module_info *new_module_info = module_get_info(module_name);

    if (new_module_info == NULL) {
        util_fatal("could not find module implementation for '%s'", module_name);
    }

    const void **args = ad_calloc(2, sizeof(void *));
    args[0] = new_module_info;
    args[1] = section;

    log_trace("scheduling module '%s' to start later", section);
    runloop_run_once(start_module_handler, args);
}

void
bootstrap(void) {
    autodie_register_handler(autodie_handler);

    rig_set_debug(RIG_DEBUG_WARN);
    rig_set_debug_callback(hamlib_debug_handler, NULL);

    runloop_bootstrap();
    module_bootstrap();
}

int
main(UNUSED int argc, UNUSED char **argv) {
    bootstrap();

    log_set_current_level(log_level_trace);

    log_info("righack is starting");

    if (argc != 2) {
        util_fatal("usage: specify exactly one config file");
    }

    const char *config_file = argv[1];
    log_debug("loading configuration from %s", config_file);

    configfile_load(config_file);
    int num_sections = configfile_get_section_count();
    for(int i = 0; i < num_sections; i++) {
        const char *section_name = configfile_get_section_name(i);
        handle_config_section(section_name);
    }

    runloop_run();
    runloop_cleanup();

    guts_exit(exit_ok);
}
