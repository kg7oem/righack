/*
 * test.c
 *
 *  Created on: Jun 5, 2018
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

#include <stdlib.h>

#include "../driver.h"
#include "../external/autodie.h"
#include "../log.h"
#include "test.h"
#include "../types.h"
#include "../util.h"

#define MODULE_NAME "test"

static void
update_flow_control(UNUSED struct driver *driver, struct driver_rs232_fc *control_lines) {
    log_debug("got flow control status change");

    if (control_lines->cts) log_trace("  CTS");
    if (control_lines->rts) log_trace("  RTS");
    if (control_lines->dtr) log_trace("  DTR");
    if (control_lines->dsr) log_trace("  DSR");
}

static void
test_lifecycle_bootstrap(void) {
    log_lots("bootstrapping the test module");
}

static void
test_lifecycle_start(struct module *module) {
    struct driver *vserial = driver_create("vserial");

    if (vserial == NULL) {
        util_fatal("could not create instance of vserial driver");
    }

    vserial->cb->rs232.fc_changed = update_flow_control;

    module->private = vserial;
    log_info("test module started: '%s'", module->label);
}

static void
test_lifecycle_stop(struct module *module) {
    log_debug("test module stopping");

    struct driver *vserial = module->private;

    if (vserial != NULL) {
        driver_destroy(vserial, NULL);
    }
}

const struct module_info *
test_module_info(void) {
    static const struct module_info our_module_info = {
            .name = MODULE_NAME,
            .lifecycle = MODULE_LIFECYCLE(test),
    };

    return &our_module_info;
}
