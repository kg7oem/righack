/*
 * util.c
 *
 *  Created on: May 23, 2018
 *      Author: tyler
 */

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void *
util_malloc(size_t bytes) {
    void *p = calloc(1, bytes);

    if (p == NULL) {
        perror("could not malloc()");
        abort();
    }

    return p;
}

