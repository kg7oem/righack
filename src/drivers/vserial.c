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
#include "../log.h"
#include "../types.h"
#include "vserial.h"

#define DRIVER_NAME "vserial"

void
vserial_driver_create(struct driver *driver) {
    log_debug("vserial driver is being created");

    driver->private = NULL;
}

void
vserial_driver_destroy(UNUSED struct driver *driver) {
    log_debug("vserial driver is being destroyed");
}

const struct driver_info *
vserial_driver_info(void) {
    static const struct driver_stream_int our_stream_interface = {
            .op = {
                    .terminate = NULL,
                    .get_mask = NULL,
                    .clear_mask = NULL,
                    .set_mask = NULL,
            },
    };

    static const struct driver_info our_driver_info = {
            .name = DRIVER_NAME,
            .create = vserial_driver_create,
            .destroy = vserial_driver_destroy,
            .interface = {
                    .stream = &our_stream_interface,
            },
    };

    return &our_driver_info;
}
