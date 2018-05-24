/*
 * runloop.h
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

#ifndef SRC_RUNLOOP_H_
#define SRC_RUNLOOP_H_

#include "vserial.h"

int runloop_start(void);
void runloop_add_vserial(VSERIAL *);

#endif /* SRC_RUNLOOP_H_ */
