/*
 * runloop.h
 *
 *  Created on: May 24, 2018
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

#ifndef SRC_RUNLOOP_H_
#define SRC_RUNLOOP_H_

#include <stdbool.h>

#include "vserial.h"

int runloop_start(void);
void runloop_add_vserial(VSERIAL *);
bool runloop_enable_read(int);
bool runloop_disable_read(int);
bool runloop_enable_write(int);
bool runloop_disable_write(int);

#endif /* SRC_RUNLOOP_H_ */
