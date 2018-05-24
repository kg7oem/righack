/*
 * runloop.c
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

#include "runloop.h"

VSERIAL *virtual_serial = NULL;

void
runloop_add_vserial(VSERIAL *vserial) {
    virtual_serial = vserial;
}
