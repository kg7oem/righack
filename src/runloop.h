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

#include "config.h"

typedef void (*runloop_generic_cb)(void *);
typedef void (*runloop_stateful_cb)(bool, void *);

struct run_once_private;
struct runloop_timer_private;

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

struct run_once runloop_run_later(runloop_stateful_cb, void *);

struct runloop_timer {
    runloop_stateful_cb cb;
    void *context;
    struct runloop_timer_private *private;
};

struct runloop_timer * runloop_timer_create(runloop_stateful_cb, void *);
void runloop_timer_destroy(struct runloop_timer *);
void runloop_timer_schedule(struct runloop_timer *, uint64_t, uint64_t);
void runloop_timer_cancel(struct runloop_timer *);
void runloop_timer_reset(struct runloop_timer *);

#ifdef CONFIG_OS_UNIX

#define PEVENT_ERROR (1 << 0)
#define PEVENT_READ (1 << 1)
#define PEVENT_WRITE (1 << 2)
#define PEVENT_PRIO (1 << 3)

struct runloop_poll;
struct runloop_poll_private;
typedef void (*runloop_poll_cb)(struct runloop_poll *, uint64_t);

struct runloop_poll {
    runloop_poll_cb cb;
    void *context;
    struct runloop_poll_private *private;
};

struct runloop_poll * runloop_poll_create(int, runloop_poll_cb);
void runloop_poll_destroy(struct runloop_poll *);
void runloop_poll_start(struct runloop_poll *, uint64_t);

#endif /* CONFIG_OS_UNIX */

#endif /* SRC_RUNLOOP_H_ */
