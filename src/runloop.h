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
#include <uv.h>

typedef void (*runloop_generic_cb)(void *);
typedef void (*runloop_stateful_cb)(bool, void *);

struct run_once_private;

struct run_once {
    runloop_stateful_cb cb;
    void *context;
    struct run_once_private *private;
};

void runloop_bootstrap(void);
// FIXME this should be called runloop_enter_loop()
bool runloop_run(void);
void runloop_cleanup(void);
bool runloop_has_control(void);

#endif /* SRC_RUNLOOP_H_ */

// FIXME this should be called runloop_delay_execution()
struct run_once runloop_run_once(runloop_stateful_cb, void *);

struct runloop_timer * runloop_create_timer(runloop_stateful_cb);
void runloop_destroy_timer(struct runloop_timer *);
