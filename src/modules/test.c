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

#include "../external/autodie.h"
#include "../log.h"
#include "test.h"
#include "../types.h"

#define MODULE_NAME "test"

enum module_status test_init_handler(void) {
    log_debug("test module initialized");
    return module_ok;
}

struct module *
test_start_handler(UNUSED const char *config_name) {
    struct module *new_module = ad_malloc(sizeof(struct module));

    new_module->label = config_name;
    new_module->info = module_get_info(MODULE_NAME);

    log_info("test module started");

    return new_module;
}

enum module_status
test_stop_handler(UNUSED struct module *info) {
    log_debug("test module stopping");
    return module_ok;
}

struct module_info *
test_module_info(void) {
    struct module_info *info = ad_malloc(sizeof(struct module_info));

    info->name = MODULE_NAME;
    info->init = test_init_handler;
    info->start = test_start_handler;
    info->stop = test_stop_handler;

    return info;
}
