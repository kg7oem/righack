/*
 * config.c
 *
 *  Created on: Jun 1, 2018
 *      Author: tyler
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

const char *
config_get_vserial_name(int num) {
    config_guard();

    toml_array_t *vserials = toml_array_in(toml_root, "vserial");
    if (vserials == NULL) {
        util_fatal("Could not find a vserial section in config file");
    }

    toml_table_t *table = toml_table_at(vserials, num);
    if (table == NULL) {
        util_fatal("Could not get vserial config #%d", num);
    }

    const char *name = toml_raw_in(table, "name");
    if (name == NULL) {
        util_fatal("Could not get name from vserial config #%d\n", num);
    }

    return name;
}
