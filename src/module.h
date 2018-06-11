/*
 * module.h
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

#ifndef SRC_MODULE_H_
#define SRC_MODULE_H_

#define MODULE_LIFECYCLE(name) { .bootstrap = name##_lifecycle_bootstrap, .start = name##_lifecycle_start, .stop = name##_lifecycle_stop }

enum module_status {
    module_unknown = 0,
    module_ok,
    module_config_error,
    module_failed,
};

struct module_info;

struct module {
    char *label;
    void *private;
    const struct module_info *info;
};

typedef void (*module_bootstrap_handler)(void);
typedef void (*module_start_handler)(struct module *);
typedef void (*module_stop_handler)(struct module *);
typedef void (*module_cleanup_handler)(struct module *);

struct module_lifecycle_op {
    module_bootstrap_handler bootstrap;
    module_start_handler start;
    module_stop_handler stop;
};

struct module_info {
    const char *name;
    struct module_lifecycle_op lifecycle;
};

void module_bootstrap(void);
const struct module_info * module_get_info(const char *name);

struct module * module_create(const struct module_info *, const char *);
struct module * module_start(const char *, const char *);
void module_stop_all(void);

#endif /* SRC_MODULE_H_ */
