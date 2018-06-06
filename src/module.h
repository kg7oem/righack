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

enum module_status {
    module_unknown = 0,
    module_ok,
    module_config_error,
    module_failed,
};

struct module_info;

struct module {
    const char *label;
    void *private;
    struct module_info *info;
};

typedef enum module_status (*module_init_handler)(void);
typedef struct module * (*module_start_handler)(const char *);
typedef enum module_status (*module_stop_handler)(struct module *);

struct module_info {
    const char *name;
    // called once during startup so the module can initialize
    module_init_handler init;
    // called on an instance of a module once when it is started
    module_start_handler start;
    // called on an instance of a module once when it is stopped
    module_stop_handler stop;
};

void module_bootstrap(void);
struct module_info * module_get_info(const char *name);

struct module * module_start(struct module_info *, const char *);
void module_stop_all(void);

#endif /* SRC_MODULE_H_ */
