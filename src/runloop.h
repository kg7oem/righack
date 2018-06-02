/*
 * runloop.h
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 */

#ifndef SRC_RUNLOOP_H_
#define SRC_RUNLOOP_H_

#include <stdbool.h>

struct vserial_t;
typedef struct vserial_t VSERIAL;

int runloop_start(void);
void runloop_add_vserial(VSERIAL *);
bool runloop_enable_read(int);
bool runloop_disable_read(int);
bool runloop_enable_write(int);
bool runloop_disable_write(int);

#endif /* SRC_RUNLOOP_H_ */
