/*
 * ptt.c
 *
 *  Created on: Jun 2, 2018
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
#include <stdio.h>
#include <string.h>

#include "../configfile.h"
#include "../driver.h"
#include "../log.h"
#include "../types.h"
#include "../util.h"

struct ptt_context {
    RIG *rig;
};

static void
ptt_init_handler(UNUSED VSERIAL *vserial, const char *name) {
    struct ptt_context *context = util_zalloc(sizeof(struct ptt_context));
    rig_model_t rigid = configfile_rgeti_section_key(name, "driver.hamlib.rigid");
    ptt_t ptt_state;
    int ret;

    log_debug("  Inside ptt_init_handler; my name = '%s'", name);

    RIG *rig = rig_init(rigid);
    if (! rig) {
        log_fatal("could not rig_init(%d)\n", rigid);
    }

    strncpy(
            rig->state.rigport.pathname,
            configfile_rgets_section_key(name, "driver.hamlib.serial"),
            FILPATHLEN - 1
        );

    ret = rig_open(rig);
    if (ret != RIG_OK) {
        log_fatal("Could not rig_open(): %s\n", rigerror(ret));
    }

    log_lots("Opened rig!");

    ret = rig_get_ptt(rig, RIG_VFO_CURR, &ptt_state);
    if (ret != RIG_OK) {
        log_fatal("could not get PTT state: %s\n", rigerror(ret));
    }

    context->rig = rig;

    vserial_set_context(vserial, context);
}

static void
ptt_cleanup_handler(UNUSED VSERIAL *vserial) {
    struct ptt_context *context = vserial_get_context(vserial);
    log_lots("  Inside ptt_cleanup_handler\n");

    rig_close(context->rig);
    rig_cleanup(context->rig);
    free(context);
}

static void
ptt_control_line_handler(UNUSED VSERIAL *vserial, UNUSED struct vserial_control_line *control_line) {
    struct ptt_context *context = vserial_get_context(vserial);
    log_lots("  Inside ptt_control_line_handler\n");
    ptt_t ptt_state = RIG_PTT_OFF;
    int ret;

    if (control_line->rts) {
        ptt_state = RIG_PTT_ON;
    }

    ret = rig_set_ptt(context->rig, RIG_VFO_CURR, ptt_state);
    if (ret != RIG_OK) {
        log_fatal("Could not set transmit: %s\n", rigerror(ret));
    }
}

struct driver_info *
ptt_driver_info(void) {
    struct driver_info info = {
            .init = ptt_init_handler,
            .cleanup = ptt_cleanup_handler,
    };

    info.vserial.control_line = ptt_control_line_handler;

    return util_memdup(&info, sizeof(info));
}
