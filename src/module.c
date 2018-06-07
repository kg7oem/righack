/*
 * module.c
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

#include <string.h>

#include "external/autodie.h"
#include "module.h"
#include "modules.h"
#include "runloop.h"
#include "util.h"

struct loaded_module_list {
    const struct module_info *info;
    struct loaded_module_list *next;
};

struct running_module_list {
    struct module *module;
    struct running_module_list *next;
};

static struct loaded_module_list *loaded_modules = NULL;
static struct running_module_list *running_modules = NULL;

const struct module_info *
module_get_info(const char *name) {
    struct loaded_module_list *p = loaded_modules;

    while(p != NULL) {
        if (strcmp(name, p->info->name) == 0) {
            break;
        }
    }

    if (p == NULL) {
        return NULL;
    }

    return p->info;
}

static void module_setup(const struct module_info *info) {
    struct loaded_module_list *orig = loaded_modules;
    struct loaded_module_list *will_add = ad_malloc(sizeof(struct loaded_module_list));
    const char *name = info->name;

    if (info->init == NULL) {
        util_fatal("module %s did not define an init handler", name);
    }

    if (info->start == NULL) {
        util_fatal("module %s did not define a start handler", name);
    }

    if (info->stop == NULL) {
        util_fatal("module %s did not define a stop handler", name);
    }

    if (module_get_info(name)) {
        util_fatal("attempt to add a module with a name that is a duplicate: %s", name);
    }

    will_add->next = orig;
    will_add->info = info;

    loaded_modules = will_add;

    will_add->info->init();
}

void
module_bootstrap(void) {
    module_setup(test_module_info());
}

struct module *
module_start(const struct module_info *info, const char *config_section) {
    log_verbose("starting module '%s' for configuration '%s'", info->name, config_section);

    struct module *new_module = info->start(config_section);
    struct running_module_list *new_member = ad_malloc(sizeof(struct running_module_list));


    new_member->next = NULL;

    if (new_module == NULL) {
        util_fatal("did not get a running module back from module start method");
    }

    new_member->module = new_module;
    if (running_modules == NULL) {
        running_modules = new_member;
    } else {
        new_member->next = running_modules;
        running_modules = new_member;
    }

    log_debug("config section '%s' started", config_section);

    return new_module;
}

enum module_status
module_stop(UNUSED struct module *module) {
    util_fatal("won't even try to stop because there is no free() anywhere");
//    return module->info->stop(module);
    return module_failed;
}

void
module_stop_all(void) {
    struct running_module_list *p = running_modules;

    while(p != NULL) {
        module_stop(p->module);
        p = p->next;
    }
}
