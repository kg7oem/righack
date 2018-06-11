/*
 * runloop.c
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

#define _GNU_SOURCE

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "external/autodie.h"
#include "module.h"
#include "runloop.h"
#include "types.h"
#include "util.h"

struct run_once_private {
    bool should_run;
    runloop_stateful_cb cb;
    void *context;
};

struct run_once_list {
    struct run_once_private *run;
    struct run_once_list *next;
};

// each thread has their own event loop...
// this should explode some heads
static TLOCAL uv_loop_t *thread_loop = NULL;

// if execution is happening with control given
// to the runloop then this is set true
static TLOCAL bool in_runloop_context = false;

static TLOCAL struct run_once_list *run_once_queue = NULL;
static TLOCAL struct run_once_list *run_once_last = NULL;
static TLOCAL uv_prepare_t *run_once_prepare = NULL;
static TLOCAL bool run_once_prepare_started = false;

void
runloop_bootstrap(void) {
    log_debug("bootstrapping the runloop");
    thread_loop = util_zalloc(sizeof(uv_loop_t));
    uv_loop_init(thread_loop);

    run_once_prepare = util_zalloc(sizeof(uv_prepare_t));
    uv_prepare_init(thread_loop, run_once_prepare);
}

static void
cleanup_handle_cb(uv_handle_t *handle) {
    free(handle);

    log_trace("freed a handle");

    if (handle == (struct uv_handle_s *)run_once_prepare) {
        log_trace("setting run_once_prepare to null");
        run_once_prepare = NULL;
    }
}

static void
walk_runloop_cb(uv_handle_t *handle, UNUSED void *context) {
    if (! uv_is_closing(handle)) {
        log_trace("closing handle in runloop walker");
        uv_close(handle, cleanup_handle_cb);
    }
}

void
runloop_cleanup(void) {
    if (thread_loop == NULL) {
        return;
    }

    log_debug("starting to cleanup the runloop");

    // first stop any modules and give them a chance to process
    // their cleanup callbacks
    module_stop_all();
    uv_run(thread_loop, UV_RUN_DEFAULT);

    // close any handles that are not already closing
    log_lots("walking the runloop to close needed handles");
    uv_walk(thread_loop, walk_runloop_cb, NULL);

    // start the runloop back up so it can process all the queued
    // cleanup callbacks
    log_debug("starting the runloop again to process the cleanup call backs");
    uv_run(thread_loop, UV_RUN_DEFAULT);
    log_debug("out of the runloop now");

    if (uv_loop_close(thread_loop) == UV_EBUSY) {
        util_fatal("could not cleanup runloop because it was not empty");
    }

    free(thread_loop);
    thread_loop = NULL;

    log_lots("runloop is completely cleaned up");
}

bool
runloop_run(void) {
    if (thread_loop == NULL) {
        util_fatal("runloop_run() called but there was no per thread runloop defined");
    }

    log_debug("about to give control to runloop");
    in_runloop_context = true;
    bool retval = uv_run(thread_loop, UV_RUN_DEFAULT);
    in_runloop_context = false;
    log_debug("control from runloop returned; retval = %i", retval);

    return retval;
}

bool
runloop_has_control(void) {
    return in_runloop_context;
}

void
run_once_prepare_cb(uv_prepare_t *prepare) {
    log_trace("Inside the run once prepare handler");
    struct run_once_list *p = run_once_queue;

    while(p != NULL) {
        p->run->cb(p->run->should_run, p->run->context);

        struct run_once_list *old = p;
        p = p->next;

        log_trace("running a run_once job");

        free(old->run);
        free(old);
    }

    run_once_queue = NULL;

    uv_prepare_stop(prepare);
    run_once_prepare_started = false;
}

struct run_once
runloop_run_later(runloop_stateful_cb cb, void *context) {
    struct run_once_private *new_job = util_zalloc(sizeof(struct run_once_private));
    struct run_once_list *queue_entry = util_zalloc(sizeof(struct run_once_list));
    struct run_once retval = {
            .cb = cb,
            .context = context,
            .private = new_job,
    };

    log_trace("scheduled job to run at the next runloop iteration");

    new_job->cb = retval.cb;
    new_job->context = retval.context;
    new_job->should_run = true;

    queue_entry->run = new_job;
    queue_entry->next = NULL;

    if (run_once_queue == NULL) {
        run_once_queue = queue_entry;
        run_once_last = queue_entry;
    } else {
        run_once_last->next = queue_entry;
        run_once_last = queue_entry;
    }

    if (! run_once_prepare_started) {
        uv_prepare_start(run_once_prepare, run_once_prepare_cb);
    }

    return retval;
}

struct runloop_timer_private {
    uv_timer_t uv_timer;
    uint64_t initial;
    uint64_t repeat;
};

struct runloop_timer *
runloop_timer_create(runloop_stateful_cb cb, void *context) {
    struct runloop_timer *timer = util_zalloc(sizeof(struct runloop_timer));
    timer->private = util_zalloc(sizeof(struct runloop_timer_private));

    timer->context = context;
    timer->cb = cb;

    uv_timer_init(thread_loop, &timer->private->uv_timer);
    timer->private->uv_timer.data = timer;

    return timer;
}

static void
runloop_timer_close_cb(uv_handle_t *uv_timer) {
    struct runloop_timer *timer = uv_timer->data;

    timer->cb(false, timer);

    free(timer->private);
    free(timer);
}

static void
runloop_timer_run_cb(uv_timer_t *uv_timer) {
    struct runloop_timer *timer = uv_timer->data;
    timer->cb(true, timer);
}

void
runloop_timer_destroy(struct runloop_timer *timer) {
    if (uv_is_active((uv_handle_t *)&timer->private->uv_timer)) {
        util_fatal("attempt to destroy a timer that is active");
    }

    uv_close((uv_handle_t *)&timer->private->uv_timer, runloop_timer_close_cb);
}

void
runloop_timer_schedule(struct runloop_timer *timer, uint64_t initial, uint64_t repeat) {
    timer->private->initial = initial;
    timer->private->repeat = repeat;

    uv_timer_start(&timer->private->uv_timer, runloop_timer_run_cb, timer->private->initial, timer->private->repeat);
    return;
}

void
runloop_timer_reset(struct runloop_timer *timer) {
    if(! uv_is_active((uv_handle_t *)&timer->private->uv_timer)) {
        util_fatal("attempt to reset a timer that is not active");
    }

    uv_timer_start(&timer->private->uv_timer, runloop_timer_run_cb, timer->private->initial, timer->private->repeat);
}

void runloop_timer_cancel(struct runloop_timer *timer) {
    if(! uv_is_active((uv_handle_t *)&timer->private->uv_timer)) {
        util_fatal("Attempt to cancel a timer that is not active");
    }
    uv_timer_stop(&timer->private->uv_timer);
}

#ifdef CONFIG_OS_UNIX

struct runloop_poll_private {
    uv_poll_t uv_poll;
    int fd;
};

struct runloop_poll *
runloop_poll_create(int fd, runloop_poll_cb cb) {
    log_info("creating a poll dohicky");

    struct runloop_poll *new_poll = util_zalloc(sizeof(struct runloop_poll));
    new_poll->cb = cb;

    new_poll->private = util_zalloc(sizeof(struct runloop_poll_private));
    new_poll->private->fd = fd;

    uv_poll_init(thread_loop, &new_poll->private->uv_poll, new_poll->private->fd);
    new_poll->private->uv_poll.data = new_poll;

    return new_poll;
}

static void
runloop_poll_close_cb(uv_handle_t *uv_poll) {
    struct runloop_poll *poll = uv_poll->data;

    poll->cb(poll, 0);

    free(poll->private);
    free(poll);
}

void
runloop_poll_destroy(struct runloop_poll *poll) {
    if (uv_is_active((uv_handle_t *)&poll->private->uv_poll)) {
        util_fatal("attempt to destroy a poll handle that is active");
    }

    uv_close((uv_handle_t *)&poll->private->uv_poll, runloop_poll_close_cb);
}

void
runloop_poll_run_cb(uv_poll_t *uv_poll, int uv_status, int uv_events) {
    struct runloop_poll *poll = uv_poll->data;

    if (uv_status < 0) {
        // FIXME there needs to be an error handling system made that
        // does not require exposing libuv
        util_fatal("Can not yet handle a uv poll error: %s", uv_strerror(uv_status));
    }

    uint64_t events = 0;
    if (uv_events & UV_READABLE) events |= PEVENT_READ;
    if (uv_events & UV_WRITABLE) events |= PEVENT_WRITE;
    if (uv_events & UV_PRIORITIZED) events |= PEVENT_PRIO;

    if (events == 0) {
        util_fatal("libuv did not signal any events");
    }

    poll->cb(poll, events);
}

void
runloop_poll_start(struct runloop_poll *poll, uint64_t events) {
    int uv_events = 0;

    if (events & PEVENT_READ) uv_events |= UV_READABLE;
    if (events & PEVENT_WRITE) uv_events |= UV_WRITABLE;
    if (events & PEVENT_PRIO) uv_events |= UV_PRIORITIZED;

    uv_poll_start(&poll->private->uv_poll, uv_events, runloop_poll_run_cb);
}

#endif /* CONFIG_OS_UNIX */
