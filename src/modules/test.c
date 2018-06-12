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

#include <hamlib/rig.h>
#include <stdlib.h>
#include <string.h>

#include "../configfile.h"
#include "../driver.h"
#include "../external/autodie.h"
#include "../log.h"
#include "test.h"
#include "../types.h"
#include "../util.h"

#define MODULE_NAME "test"

struct context {
    struct driver *vserial;
    RIG *rig;
};

static void
update_flow_control(UNUSED struct driver *driver, struct driver_rs232_fc *control_lines) {
    log_debug("got flow control status change");

    if (control_lines->cts) log_trace("  CTS");
    if (control_lines->rts) log_trace("  RTS");
    if (control_lines->dtr) log_trace("  DTR");
    if (control_lines->dsr) log_trace("  DSR");

    struct context *context = driver->user;

    ptt_t ptt_state = RIG_PTT_OFF;
    int ret;

    if (control_lines->rts) {
        ptt_state = RIG_PTT_ON;
    }

    ret = rig_set_ptt(context->rig, RIG_VFO_CURR, ptt_state);
    if (ret != RIG_OK) {
        util_fatal("Could not set transmit: %s", rigerror(ret));
    }

    if (ptt_state == RIG_PTT_ON) {
        log_info("Starting to transmit");
    } else {
        log_info("Done transmitting");
    }
}

static void
test_lifecycle_bootstrap(void) {
    log_lots("bootstrapping the test module");
}

static void
test_lifecycle_start(struct module *module) {
    struct context *context = util_zalloc(sizeof(struct context));

    context->vserial = driver_create("vserial", module->label);

    if (context->vserial == NULL) {
        util_fatal("could not create instance of vserial driver");
    }

    context->vserial->cb->rs232.fc_changed = update_flow_control;
    context->vserial->user = context;

    rig_model_t rigid = configfile_rgeti_section_key(module->label, "hamlib.rigid");
    context->rig = rig_init(rigid);

    log_info("using hamlib rigid %d", rigid);

    if (! context->rig) {
        util_fatal("could not rig_init(%d)", rigid);
    }

    const char *hamlib_serial_port = configfile_gets_section_key(module->label, "hamlib.serial.port");
    if (hamlib_serial_port != NULL) {
        strncpy(
                context->rig->state.rigport.pathname,
                hamlib_serial_port,
                FILPATHLEN - 1
            );
    }

    int ret = rig_open(context->rig);
    if (ret != RIG_OK) {
        util_fatal("Could not rig_open(): %s", rigerror(ret));
    }

    module->private = context;
    log_info("test module started: '%s'", module->label);
}

static void
test_lifecycle_stop(struct module *module) {
    log_debug("test module stopping");

    struct context *context = module->private;

    if (context->vserial != NULL) {
        driver_destroy(context->vserial, NULL);
    }

    if (context->rig != NULL) {
        rig_close(context->rig);
        rig_cleanup(context->rig);
    }

    free(context);
}

const struct module_info *
test_module_info(void) {
    static const struct module_info our_module_info = {
            .name = MODULE_NAME,
            .lifecycle = MODULE_LIFECYCLE(test),
    };

    return &our_module_info;
}
