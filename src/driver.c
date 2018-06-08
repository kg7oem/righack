/*
 * driver.c
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
#include <string.h>

#include "driver.h"
#include "drivers.h"
#include "external/autodie.h"
#include "log.h"
#include "types.h"
#include "util.h"

struct driver_list {
    const struct driver_info *driver;
    struct driver_list *next;
};

static TLOCAL struct driver_list *loaded_drivers = NULL;

static void
driver_register(const struct driver_info *info) {
    struct driver_list *new_member = ad_malloc(sizeof(struct driver_list));

    if (driver_get_info(info->name)) {
        util_fatal("attempt to double register driver with name '%s'", info->name);
    }

    log_debug("loading driver '%s'", info->name);

    new_member->driver = info;
    new_member->next = loaded_drivers;

    loaded_drivers = new_member;
}

void
driver_bootstrap(void) {
    log_debug("bootstrapping the driver system");
}

void
driver_load_plugins(void) {
    driver_register(vserial_driver_info());
}

const struct driver_info *
driver_get_info(const char *name) {
    struct driver_list *p = loaded_drivers;

    while(p != NULL) {
        if (strcmp(p->driver->name, name) == 0) {
            return p->driver;
        }
    }

    return NULL;
}

const char *
driver_get_status_string(enum driver_status status) {
    switch (status) {
    case driver_status_unstarted:
        return "unstarted";
    case driver_status_starting:
        return "starting";
    case driver_status_started:
        return "started";
    case driver_status_stopping:
        return "stopping";
    case driver_status_stopped:
        return "stopped";
    }

    util_fatal("should never get here");
    return NULL;
}

struct driver *
driver_create(const char *name) {
    const struct driver_info *info = driver_get_info(name);

    if (info == NULL) {
        return NULL;
    }

    return info->lifecycle.create(info);
}

void
driver_destroy(struct driver *driver, void *notify_cb) {
    enum driver_status status = driver->status;

    if (notify_cb != NULL) {
        util_fatal("cleanup notification does not work yet");
    }

    if (status != driver_status_unstarted && status != driver_status_stopped) {
        util_fatal("attempt to destroy a driver in status %s", driver_get_status_string(status));
    }

    driver->info->lifecycle.destroy(driver);

    if (driver->cb != NULL) {
        free(driver->cb);
    }

    free(driver);
}

enum driver_status
driver_get_status(struct driver *driver) {
    return driver->status;
}

enum driver_status
driver_set_status(struct driver *driver, enum driver_status status) {
    enum driver_status old = driver->status;
    driver->status = status;
    return old;
}
