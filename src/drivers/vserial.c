/*
 * vserial.c
 *
 *  Created on: Jun 6, 2018
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
#include "../plugin.h"
#include "../runloop.h"
#include "../types.h"
#include "../util.h"
#include "vserial.h"

#define DRIVER_NAME "vserial"

struct vserial_context {
    struct runloop_timer *timer;
};

static void
vserial_timer_cb(bool should_run, void *context) {
    static int count = 0;
    struct runloop_timer *timer = context;
    if (should_run) {
        log_debug("the timer wanted to say something: %s", timer->context);

        if (++count >= 10) {
            runloop_timer_cancel(timer);
        }
    } else {
        log_debug("this timer is telling us we are done");
    }
}

static void
vserial_lifecycle_bootstrap(void) {
    log_debug("vserial driver is being bootstrapped");
}

static void
vserial_lifecycle_init(struct driver *driver) {
    log_debug("vserial driver instance is initializing");
    struct vserial_context *context = ad_malloc(sizeof(struct vserial_context));

    context->timer = runloop_timer_create(vserial_timer_cb, "i am a timer");

    driver->user = context;

    runloop_timer_schedule(context->timer, 0, 500);
}

static void
vserial_lifecycle_cleanup(UNUSED struct driver *driver) {
    log_debug("vserial driver instance is being destroyed");

    struct vserial_context *context = driver->user;

    if (context != NULL) {
        runloop_timer_destroy(context->timer);
        free(context);

        driver->user = NULL;
    }

    return;
}

const struct driver_info *
vserial_driver_info(void) {
    static struct driver_info our_info = {
            .name = DRIVER_NAME,
            .lifecycle = LIFECYCLE_OPS(vserial),
            .op = {
                    .stream = {
                            .terminate = NULL,
                    },
            },
    };

    return &our_info;
}
