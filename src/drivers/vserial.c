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
#include "../types.h"
#include "../util.h"
#include "vserial.h"

#define DRIVER_NAME "vserial"

static void
vserial_lifecycle_bootstrap(void) {
    log_debug("vserial driver is being bootstrapped");
}

static struct driver *
vserial_lifecycle_create(UNUSED const struct driver_info *info) {
    struct driver new_driver = {
            .private = NULL,
            .info = info,
            .cb = ad_malloc(sizeof(struct driver_interface_cb)),
    };

    log_debug("vserial driver was created");

    return util_memdup(&new_driver, sizeof(new_driver));
}

static void
vserial_lifecycle_destroy(UNUSED struct driver *driver) {
    log_debug("vserial driver is being destroyed");

    if (driver->private != NULL) {
        free(driver->private);
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
