/*
 * config.c
 *
 *  Created on: Jun 1, 2018
 *      Author: tyler
 */

/*
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

#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "external/toml.h"
#include "util.h"

toml_table_t* toml_root;

void
config_load(char *path) {
    char error[1024];
    FILE *fh;

    if (toml_root != NULL) {
        util_fatal("attempt to load config file when a config is already loaded");
    }

    fh = fopen(path, "r");
    if (fh == 0) {
        util_fatal_perror("could not fopen(%s): ", path);
    }

    toml_root = toml_parse_file(fh, error, sizeof(error));

    if (fclose(fh)) {
        util_fatal_perror("could not fclose(%s): ", path);
    }

    if (toml_root == NULL) {
        fprintf(stderr, "Could not load %s:\n%s\n\n", path, error);
        util_fatal("Unable to continue after error loading config file");
    }
}

static void
config_guard(void) {
    if (toml_root == NULL) {
        util_fatal("toml_root is NULL\n");
    }
}

int
config_count_vserial(void) {
    toml_array_t *vserials;
    int i;

    config_guard();

    vserials = toml_array_in(toml_root, "vserial");

    if (vserials == NULL) {
        return 0;
    }

    for(i = 0; true; i++) {
        toml_table_t *table = toml_table_at(vserials, i);
        if (table == NULL) {
            break;
        }
    }

    return i;
}

static toml_array_t *
config_get_vserials(void) {
    toml_array_t *vserials = toml_array_in(toml_root, "vserial");
    if (vserials == NULL) {
        util_fatal("Could not find a vserial section in config file");
    }
    return vserials;
}

static toml_table_t *
config_get_vserial_num(int num) {
    toml_table_t *table = toml_table_at(config_get_vserials(), num);
    if (table == NULL) {
        util_fatal("Could not get vserial config #%d", num);
    }
    return table;
}

const char *
config_get_vserial_name(int num) {
    config_guard();

    toml_table_t *table = config_get_vserial_num(num);
    const char *raw = toml_raw_in(table, "name");

    if (raw == NULL) {
        util_fatal("Could not get name from vserial config #%d\n", num);
    }

    char *name;
    toml_rtos(raw, &name);

    return name;
}

const char *
config_get_vserial_driver(int num) {
    config_guard();

    toml_table_t *table = config_get_vserial_num(num);
    const char *raw = toml_raw_in(table, "driver");

    if (raw == NULL) {
        util_fatal("could not get driver from vserial config #%d\n", num);
    }

    char *driver;
    toml_rtos(raw, &driver);

    return driver;
}
