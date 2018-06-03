/*
 * configfile.c
 *
 *  Created on: Jun 1, 2018
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "configfile.h"
#include "external/toml.h"
#include "util.h"

toml_table_t* toml_root;

void
configfile_load(char *path) {
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
configfile_guard(void) {
    if (toml_root == NULL) {
        util_fatal("toml_root is NULL\n");
    }
}

int
configfile_get_section_count(void) {
    configfile_guard();


    const char *p;
    int i;
    while((p = configfile_get_section_name(i))) {
        i++;
    }

    return i;
}

const char *
configfile_get_section_name(int num) {
    configfile_guard();
    return toml_key_in(toml_root, num);
}

const char *
configfile_gets_section_key(const char *section, const char *key) {
    static TLOCAL char *buf = NULL;
    configfile_guard();

    if (buf != NULL) {
        free(buf);
    }

    toml_table_t *section_table = toml_table_in(toml_root, section);
    if (section_table == NULL) {
        util_fatal("could not find config entry for '%s'\n", section);
    }

    printf("configfile_get_section_key(): looking up '%s' '%s'\n", section, key);

    const char *raw = toml_raw_in(section_table, key);
    if (raw == NULL) {
        printf("configfile_get_section_key(): nothing found");
        return NULL;
    }

    printf("configfile_get_section_key(): raw = '%s'\n", raw);
    if (toml_rtos(raw, &buf)) {
        util_fatal("configfile_get_section_key(): toml_rtos() failed\n");
    }
    printf("configfile_get_section_key(): buf = '%s'\n", buf);

    return buf;
}

// A "reliable" version of configfile_get_section_key()
// Reliable means that it will never fail. It does that
// but causing a fatal error if anything goes wrong.
const char *
configfile_rgets_section_key(const char *section, const char *key) {
    const char *value;
    configfile_guard();

    value = configfile_gets_section_key(section, key);
    if (value == NULL) {
        util_fatal("Could not get required key: '%s' from '%s'\n", key, section);
    }

    return value;
}

bool
configfile_geti_section_key(const char *section, const char *key, int64_t *dest) {
    configfile_guard();

    toml_table_t *section_table = toml_table_in(toml_root, section);
    if (section_table == NULL) {
        util_fatal("could not find config entry for '%s'\n", section);
    }

    const char *raw = toml_raw_in(section_table, key);
    if (raw == NULL) {
        return false;
    }

    if (toml_rtoi(raw, dest)) {
        util_fatal("could not toml_rtoi()\n");
    }

    return true;
}

int64_t
configfile_rgeti_section_key(const char *section, const char *key) {
    configfile_guard();
    int64_t result;

    if (! configfile_geti_section_key(section, key, &result)) {
        util_fatal("could not find '%s' in '%s'\n", key, section);
    }

    return result;
}

// FIXME untested
//const char **
//configfile_get_section_names(void) {
//    configfile_guard();
//
//    static TLOCAL const char **list = NULL;
//    int num_sections = configfile_get_section_count();
//
//    if (list != NULL) {
//        free(list);
//    }
//
//    list = util_malloc(sizeof(const char *) * (num_sections + 1));
//
//    for(int i = 0; i < num_sections; i++) {
//        list[i] = configfile_get_section_name(i);
//    }
//
//    return list;
//}
